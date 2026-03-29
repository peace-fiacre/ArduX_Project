# ArduX — Embedded Game Console

A DIY retro game console built on the **ESP32 TTGO T-Display**, developed during
**Arduino Days 2026** (SCOP, Benin). The project implements 6 classic games with
an object-oriented architecture, finite state machine, and optimized rendering.

---

## Hardware

| Component | Details |
|-----------|---------|
| Board | ESP32 TTGO T-Display |
| Screen | IPS LCD ST7789 — 135 × 240 px |
| Buttons | 6 push buttons (UP / DOWN / LEFT / RIGHT / A / B) |
| Audio | Passive buzzer (GPIO 17) |
| Memory | Flash-emulated EEPROM (high score saving) |

---

## Available Games

| # | Game | Description |
|---|------|-------------|
| 0 | 🐍 Snake | Eat apples without biting yourself |
| 1 | ❌ Tic-Tac-Toe | 2 players — align 3 symbols to win |
| 2 | 🏓 Pong | Beat the AI in 5 points |
| 3 | 🦕 Dino Run | Jump over cacti and birds as long as possible |
| 4 | 👾 Space Invaders | Eliminate the invaders before they reach you |
| 5 | 🧱 Breakout | Destroy all bricks across multiple waves |

---

## Architecture

The project is built on three separated layers:
```
HARDWARE LAYER   →   Buttons, TFT screen, buzzer, EEPROM
CONSOLE LAYER    →   State machine, scrollable menu, game over screen
GAME LAYER       →   Each game is a self-contained C++ class
```

### Console State Machine
```
STARTING → MENU → PLAYING → GAMEOVER → PLAYING (replay)
                                      → MENU (back)
```

### Game Contract

Every game inherits from the abstract `Game` class and implements:
```cpp
void init()                    // initialization / reset
void update(Buttons buttons)   // logic (inputs + physics)
void render()                  // optimized display
bool isGameOver()              // signals end of game
```

### Applied Optimizations

- **Non-blocking timing** — `millis()` instead of `delay()` everywhere in the
  game loop; each component manages its own timer independently
- **Differential rendering** — only changed pixels are erased and redrawn,
  not the entire screen; eliminates flickering on the TFT without double buffer
- **`needsRedraw` flag** — static games (Tic-Tac-Toe) skip rendering entirely
  when nothing has changed, saving CPU cycles every frame
- **Rising edge inputs** — `pressed` flags are true only on the single frame
  the button transitions from released to pressed, preventing repeated triggers
- **`firstDraw` flag** — each game performs one full screen draw on startup,
  then switches to differential rendering for all subsequent frames
- **Conditional EEPROM writes** — high score is written only when beaten,
  protecting flash memory from excessive write cycles

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

In the `User_Setup.h` file of the TFT_eSPI library, enable the correct driver:
```cpp
#define ST7789_DRIVER
#define TFT_WIDTH  135
#define TFT_HEIGHT 240
```

### Flashing

1. Clone the repository
```bash
git clone https://github.com/peace-fiacre/ArduX_Project.git
```
2. Open `arduX.ino` in Arduino IDE
3. Select **ESP32 Dev Module** as the target board
4. Upload

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

## Boot Sequence

On startup the console displays an animated boot screen — star field background,
letter-by-letter title reveal with buzzer melody, animated border, and a loading
bar — then transitions automatically to the main menu.

---

## High Score

The best score is saved in EEPROM and persists across reboots. A new record
triggers a dedicated melody and an on-screen `** NEW RECORD! **` message.

---

## Authors

Project developed by ArduX team during **Arduino Days 2026** — SCOP, Benin.

---

## License

Open source — free to use and modify.
