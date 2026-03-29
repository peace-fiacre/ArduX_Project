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

## License

Open source — free to use and modify.
