# ArduX — Embedded Game Console

ArduX is a DIY retro game console built from scratch on an **ESP32 TTGO T-Display** board, developed during **Arduino Days 2026** at SCOP (Benin). The project started as a challenge to build a fully functional handheld console using only an ESP32, a 135x240 IPS LCD screen, 6 push buttons, and a passive buzzer.

The console runs 6 fully playable games — Snake, Tic-Tac-Toe, Pong, Dino Run, Space Invaders, and Breakout — all accessible through a scrollable menu and sharing a unified game over screen with persistent high score saving.

---

## Hardware

| Component | Details |
|-----------|---------|
| Board | ESP32 TTGO T-Display (dual-core 240 MHz Xtensa LX6, 520 KB SRAM, 4 MB Flash) |
| Screen | IPS LCD ST7789 — 135 × 240 px, driven via SPI |
| Buttons | 6 push buttons (UP / DOWN / LEFT / RIGHT / A / B) |
| Audio | Passive buzzer (GPIO 17) |
| Memory | Flash-emulated EEPROM (high score saving) |

---

## Available Games

| # | Game | Description |
|---|------|-------------|
| 0 | Snake | Eat apples without biting yourself |
| 1 | Tic-Tac-Toe | 2 players — align 3 symbols to win |
| 2 | Pong | Beat the AI in 5 points |
| 3 | Dino Run | Jump over cacti and birds as long as possible |
| 4 | Space Invaders | Eliminate the invaders before they reach you |
| 5 | Breakout | Destroy all bricks across multiple waves |

---

## Architecture

The codebase is built around a clean object-oriented architecture with three separated layers:

```
HARDWARE LAYER   →   Buttons, TFT screen, buzzer, EEPROM
CONSOLE LAYER    →   State machine, scrollable menu, game over screen
GAME LAYER       →   Each game is a self-contained C++ class
```

The **hardware layer** handles direct interaction with buttons, the TFT screen, the buzzer, and EEPROM. The **console layer** manages the finite state machine, the scrollable menu, and the game over logic. The **game layer** contains each game as a self-contained C++ class inheriting from a common abstract base class.

### Console State Machine

```
STARTING → MENU → PLAYING → GAMEOVER → PLAYING (replay)
                                      → MENU (back)
```

### Game Contract

Every game implements the same four-function contract:

```cpp
void init()                    // reset state and initialize
void update(Buttons buttons)   // handle inputs and physics
void render()                  // draw on screen
bool isGameOver()              // signal end of game
```

---

## Game Details

**Snake** uses an array of segments shifted from tail to head each frame. Only the new head and the erased tail are redrawn each frame, keeping rendering minimal.

**Tic-Tac-Toe** runs in 2-player mode on the same device and checks all 8 winning combinations after every move. Nothing is redrawn unless the player performs an action.

**Pong** uses floating-point velocity for smooth ball movement and calculates the bounce angle based on where the ball hits the paddle relative to its center. The AI opponent tracks the ball with adjustable difficulty.

**Dino Run** features gravity accumulation for realistic jump physics, procedural cactus and bird spawning, speed progression as the game goes on, obstacle recycling for memory efficiency, and a parallax ground scrolling effect.

**Space Invaders** moves the entire formation as a single entity, reverses direction when any invader hits a wall, accelerates as invaders are destroyed, and includes explosion animations and random enemy fire across increasing waves.

**Breakout** uses AABB collision detection for ball-brick interaction, multi-HP bricks in the top rows that require multiple hits, bounce angle calculation based on paddle impact offset, and a wave system that regenerates the brick grid with increasing difficulty.

---

## Optimizations

Six optimizations were applied to keep the console smooth and responsive on the ESP32:

- **Non-blocking timing** - all `delay()` calls replaced with `millis()`-based timers. The main loop never freezes and buttons are always responsive. Each component manages its own independent timer.
- **Differential rendering** - only pixels that actually changed are erased and redrawn each frame, eliminating flickering on a TFT screen that has no hardware double buffer.
- **`firstDraw` flag** - each game performs one full-screen draw on startup to establish the visual baseline, then switches to the lighter differential approach for all subsequent frames.
- **`needsRedraw` flag** - static games like Tic-Tac-Toe skip rendering entirely when nothing has changed, saving CPU cycles every frame.
- **Rising edge inputs** - button presses register only once per press, even when held down continuously. The `pressed` flag is true only on the single frame the button transitions from released to pressed.
- **Conditional EEPROM writes** - the high score is saved only when the current score beats the stored record, protecting flash memory from excessive write cycles.

---

## Boot Sequence & High Score

On startup, the console plays an animated boot sequence featuring a star field background, a letter-by-letter title reveal synchronized with a buzzer melody, an animated border, and a loading bar before auto-transitioning to the main menu.

The game over screen displays both the current score and the best score saved in EEPROM. When the player beats their personal best, a dedicated celebration melody plays and a `** NEW RECORD! **` message appears on screen. High scores persist across reboots and power cycles.

---

## File Structure

```
ArduX_Project/
├── arduX.ino          # Entry point — setup(), loop(), state machine
├── Game.h             # Abstract base class + Buttons struct
├── Menu.h             # Scrollable menu with navigation and scroll indicators
├── Snake.h            # Snake — differential rendering on tail/head only
├── Morpion.h          # Tic-Tac-Toe — needsRedraw flag, 8-combination winner check
├── Pong.h             # Pong — float velocity, angle on paddle impact, AI opponent
├── DinoRun.h          # Dino Run — gravity physics, birds, procedural cactus spawn
├── SpaceInvaders.h    # Space Invaders — formation movement, waves, explosions
└── Breakout.h         # Breakout — AABB collision, multi-HP bricks, wave system
```

---

## Installation

### Requirements

- Arduino IDE 2.3.8
- Board: **ESP32** (via Arduino board manager)
- Library: **TFT_eSPI** (via Arduino library manager)

### TFT_eSPI Configuration

In the `User_Setup.h` file of the TFT_eSPI library:

```cpp
#define ST7789_DRIVER
#define TFT_WIDTH  135
#define TFT_HEIGHT 240
```

### Flashing

```bash
git clone https://github.com/peace-fiacre/ArduX_Project.git
```

1. Open `arduX.ino` in Arduino IDE
2. Select **ESP32 Dev Module** as the target board
3. Upload

---

## Pin Mapping

| Function | GPIO |
|----------|------|
| UP | 25 |
| DOWN | 26 |
| LEFT | 27 |
| RIGHT | 32 |
| A (confirm) | 13 |
| B (back) | 15 |
| Buzzer | 17 |
| TFT Backlight | 4 |

---

## Changelog

### Snake.h — v1 → v2

#### Bug Fixes

| Issue | v1 | v2 |
|-------|----|----|
| Snake array size | `snake[100]` - fast memory crash | `snake[SNAKE_MAX=400]` + guard `snakeLength < SNAKE_MAX - 1` |
| Uninitialized score | absent from `init()` | explicit `score = 0` |
| Instant U-turn | direction applied immediately | `nextDir` buffered, applied only on next tick |
| Tail erase on grow | `appleEaten` handled it awkwardly | dedicated `tailGrew` flag, cleaner and correct |
| `spawnApple()` infinite loop | no protection | loop capped + `state = GAME_OVER` if grid full |
| ESP32 watchdog | implicit `delay()` possible | `delay(80)` explicitly removed |

#### New Features

- **Super apple (gold)** - 30% chance after each apple, worth 3 pts, expires in 7s, blinks
- **Progressive speed** - 5 levels based on score (150ms → 60ms), with color and label
- **Wrap mode** - pass through walls unlocked at score ≥ 10
- **Visible game border** (`drawBorder()`)
- **Redesigned compact HUD** - score, length, speed, WRAP, `[B]Quit`
- **Button B to quit**
- **Buzzer** (`SNK_BUZZER` pin 17) - distinct sound for apple, super apple, death
- **Utility functions** - `cellX()`, `cellY()`, `drawCell()`, `onSnake()`

#### Architectural Refactoring

- Grid switches from inline constants (`240/8`, `110/8`) to named `static const` (`GS`, `GW`, `GH`, `HUD`)
- Movement logic moves out of `moveSnake()` and integrates directly into `update()` - more readable
- `moveInterval()` becomes a method (no longer `const int`) to enable variable speed

> **Summary:** v2 fixes 6 real bugs (including a memory crash and an ESP32 watchdog bug) and adds 7 significant gameplay features.

---

### Morpion.h — v1 → v2

#### Logic Fixes and Improvements

| Issue | v1 | v2 |
|-------|----|----|
| Win line detection | `checkWinner()` returns just an int | Two functions: `checkWinnerSimple()` (for AI) + `checkWinner()` storing the 3 winning cells in `winCells[]` |
| Move handling | Inline logic in `update()` | Factored into `playMove()` - reusable for player and AI |
| Score | `score = result` (arbitrary value) | `score = 1` if player wins, `0` otherwise - clear logic |
| Grid Y origin | `gridY = +10` | `gridY = +8` - visual adjustment for centering with HUD |

#### New Features

**Strategic AI**
- Attack - plays the winning move immediately if available
- Defense - blocks the player's winning move if needed
- Priority: center > corners > sides
- Simulated delay - AI waits 450ms before playing ("thinking" effect)

**Intro screen**
- Title, illustrated mini-grid, rules and controls explanation
- Blinking text `>> Press [A] to play <<` / `[B] Back to menu`
- Static drawn only once (`introDrewStatic`), only the blink refreshes

**Result screen** (`showingResult`)
- Distinct message for win / loss / draw
- Winning line highlighted (green if player wins, yellow if AI wins), 5px thickness
- Winning cells colored differently (X green, O yellow)
- `[A] Replay [B] Menu` displayed at the bottom

**Buzzer** (`MRP_BUZZER` pin 17)

| Event | Sound |
|-------|-------|
| Cursor move | 700 Hz, 12ms |
| Player places X | 900 Hz, 40ms |
| AI places O | 450 Hz, 40ms |
| Player wins | 1047 Hz, 80ms |
| AI wins | 220 Hz, 300ms |
| Draw | 600 Hz, 100ms |
| Game start | 800 Hz, 60ms |

**Improved rendering**
- Cursor - dark yellow background + double yellow border (was: simple rectangle)
- Cross (X) - drawn in triple thickness (`t = -1, 0, 1`) for bold rendering
- Circle (O) - drawn in triple radius (`rr-1, rr, rr+1`) for bold rendering
- Grid - double lines (`t = 0, 1`) for better readability
- HUD - clearly indicates "Your turn [X]" or "AI thinking... [O]"

#### Architectural Refactoring

- `playMove(row, col, player)` centralizes: board write, sound, win/draw check, player switch, AI trigger
- `findWinningMove(player, &row, &col)` isolates move search logic for AI
- `showingIntro` and `showingResult` replace implicit states - clearer game flow
- `introDrewStatic` avoids redrawing the full intro every frame (display optimization)

> **Summary:** v2 transforms a basic 2-player tic-tac-toe into a complete solo game against an AI, with animated intro, sound feedback, enriched visual rendering, and highlighted winning line.

---

### SpaceInvaders.h — v1 → v2

#### Structural Changes

| Element | v1 | v2 |
|---------|----|----|
| Invader rows | `ROWS=2` | `ROWS=3` - denser grid |
| Invader spacing | `c*30`, `r*20` | `c*33`, `r*18` - redistributed for 3 rows |
| Initial position | `x=20`, `y=25` | `x=18`, `y=20` - slightly higher |
| Bullet struct | `{x, y, px, py}` - px/py for differential drawing | `{x, y}` - simplified, global cleanup used |
| Explosion struct | `{x, y, timer}` - simple timer | `{x, y, timer, frame}` - 3 animation frames |
| Base invader speed | `max(200, 600 - wave*30)` | `max(180, 650 - wave*35)` - slightly faster |
| In-game acceleration | `max(40, aliveCount*12)` | `max(35, aliveCount*10)` - accelerates faster |
| Edge descent | `invaderOffsetY += 8` | `invaderOffsetY += 6` - smoother descent |
| Enemy fire rate | every 1200ms | every 1000ms - more aggressive |
| Player missile speed | 5px/frame | 6px/frame - faster |
| Player fire delay | 300ms | 280ms - slightly increased rate |

#### New Features

**Lives system**
- 3 lives instead of instant death on first hit
- Mini-ships drawn in HUD to represent lives (`drawMiniShip()`)
- `takeDamage()` centralizes: `lives--`, sound, game over check, shield activation

**Temporary shield**
- Activated automatically after each hit
- Duration: 2 seconds (`SHIELD_DURATION = 2000ms`)
- Ship blinks in cyan during protection
- Enemy shots ignored while shield is active

**UFO (flying saucer)**
- Appears from the right after 100 pts, random probability
- Worth 100 to 500 pts (in steps of 100, random)
- Blinks in magenta/red every 150ms
- Value displayed in HUD in real time
- Can be shot down by the player

**Persistent star background**
- 25 pre-calculated stars in `initStars()` - fixed positions
- Redrawn in dark grey (color565 80,80,80) every frame

**Intro screen**
- Title, illustrative mini-sprites, rules description
- Controls section
- Blinking text `[A] start` / `[B] Back`
- `introDrewStatic` to redraw static content only once

**Improved missile rendering**
- Player missile: yellow body (2x7px) + orange flame (2x2px)
- Enemy missile: red body (2x5px) + orange flame (2x2px)

**Improved explosion animations**
- 3 frames: `***` then `* *` then `.` (was: fixed `***`)
- Controlled by `frame` and `timer=3` between each frame

**Enriched HUD**
- Score, lives (icons), wave, UFO scoreValue, `[B]Quit`
- `drawMiniShip()` for life icons

**Extended buzzer**

| Event | v1 | v2 |
|-------|----|----|
| Edge reached | 150 Hz, 80ms | 160 Hz, 60ms |
| UFO destroyed | absent | 1800 Hz, 120ms |
| Hit received | game over direct | 150 Hz, 400ms + shield |

#### Architectural Refactoring

- `takeDamage()` consolidates: `lives--`, sound, game over check, shield activation
- Rendering switches from differential (px/py to erase old position) to global cleanup (`fillRect` on whole game zone) - simpler, slightly slower but imperceptible
- `showingIntro` / `introDrewStatic` added for intro flow (same pattern as Morpion)
- `starsInit` to initialize stars only once

> **Summary:** v2 massively enriches gameplay: 3 lives, shield, UFO, 3 enemy rows, animated intro. The differential rendering from v1 is dropped in favor of a more readable global cleanup, offset by targeted optimizations (fixed stars, selective HUD redraw).

---

### Pong.h — v1 → v2 (v3.5)

#### Bug Fixes

| Issue | v1 | v2 / v3.5 |
|-------|----|----|
| Jerky paddle movement | `buttons.up` / `buttons.down` used - but `paddleSpd=3` too slow | `paddleSpd=4`, same logic combined with other fixes |
| Incorrect diagonal ball | `ballX += (int)velX` - integer truncation every frame | `ballXf += velX; ballX = (int)ballXf` - float accumulators, precise movement |
| Buzzer pin undefined | not defined in this file | `BUZZER_PIN 17` explicitly defined with `#ifndef` guard |
| Possible null velY | no protection against `velY=0` | `clampVelY()` guarantees minimum vertical speed |
| Too predictable AI | follows ball Y directly, fixed speed | Adaptive AI with prediction (`velY * 2.5f`), variable speed based on score |

#### New Features

**Neon visual theme**
- Dedicated colors: `NEON_CYAN`, `NEON_PINK`, `NEON_YELLOW`, `DIM_GREY`, `DARK_BLUE`
- HUD redesigned in `DARK_BLUE` with cyan border
- Player paddle cyan, AI paddle pink, ball yellow

**Enriched HUD**
- Dedicated 20px HUD zone at top (`HUD_H = 20`)
- Centered "PONG" label + colored scores (cyan player, pink AI)
- `SPD+` indicator during progressive acceleration

**Particle system**
- Burst of 4 particles on each bounce (paddle or wall)
- Simple physics: random velocity + gravity (`vy += 0.08f`)
- Short lifespan (6-9 frames), drawn pixel by pixel

**Progressive acceleration (SpeedRamp)**
- Dedicated `SpeedRamp` class with linear interpolation
- At start: multiplier 0.5 → 1.0 over 5 seconds
- Ball starts slow and accelerates naturally

**Collision flash**
- `hitFlash`: white ball for 3 frames after player paddle bounce
- Immediate visual feedback on contact

**Point announcement**
- Colored box "POINT!" (cyan) or "OUCH!" (pink) for ~1s
- Game paused during announcement (`pointAnnounceTimer`)

**Adaptive AI**
- Position prediction: `target = ballY + velY * 2.5f`
- AI speed increases if player pulls ahead (`aiSpd` up to 5)
- ±2px dead zone to avoid oscillation

**Complete buzzer**

| Event | v1 | v2 |
|-------|----|----|
| Wall bounce | absent | 320 Hz, 40ms |
| Player paddle | absent | 520 Hz, 30ms |
| AI paddle | absent | 380 Hz, 30ms |
| Player point | absent | G (392 Hz), 150ms |
| AI point | absent | D (294 Hz), 150ms |
| Victory | absent | C high (523 Hz), 300ms |
| Speed+ | absent | F (349 Hz), 50ms |

**Improved paddle rendering**
- `drawPaddle()` adds a central white line (3D effect)
- `erasePaddle()` erases with +1px margin to avoid artifacts

#### Architectural Refactoring

- `clampVelY()` isolates minimum vertical speed constraint
- `spawnOne()` / `spawnBurst()` / `updateParticles()` encapsulate the particle system
- `soundWallBounce()`, `soundPlayerPoint()`, etc. - dedicated inline sound functions
- `SpeedRamp` standalone reusable class
- `resetBall()` uses `speedRamp.getFinalMultiplier()` for consistent relaunch speed
- Game zone constrained to `[HUD_H, SCREEN_H]` instead of raw `[0, 135]`

> **Summary:** v1 was a functional but minimal Pong - all white, no sound, no visual feedback. v2 (v3.5) transforms it into a full neon experience: distinct sounds for each event, particles, collision flash, point announcements, adaptive AI, and progressive speed ramp.

---

## License

Open source — free to use and modify.
