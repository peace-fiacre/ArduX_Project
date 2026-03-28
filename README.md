# ArduX - Embedded Game Console

A DIY retro game console built on the **ESP32 TTGO T-Display**, developed during **Arduino Days 2026** (SCOP, Benin). The project implements 6 classic games with an object-oriented architecture, finite state machine, and optimized rendering.

---

## Hardware

| Component | Details |
|-----------|---------|
| Board | ESP32 TTGO T-Display |
| Screen | IPS LCD ST7789 — 135 × 240 px |
| Buttons | 6 push buttons (UP / DOWN / LEFT / RIGHT / A / B) |
| Audio | Passive buzzer |
| Memory | Flash-emulated EEPROM (high score saving) |

---

## Available Games

| # | Game | Description |
|---|------|-------------|
| 0 | 🐍 Snake | Eat apples without biting yourself |
| 1 | ❌ Tic-Tac-Toe | 2 players — align 3 symbols |
| 2 | 🏓 Pong | Beat the AI in 5 points |
| 3 | 🧱 Breakout | Destroy all the bricks |
| 4 | 👾 Space Invaders | Eliminate the invaders before they land |
| 5 | 🦕 Dino Run | Jump over cacti as long as possible |

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

- **Non-blocking timing** — `millis()` instead of `delay()`, each component has its own timer
- **Differential rendering** — only changed pixels are erased, not the entire screen
- **`needsRedraw` flag** — static games (Tic-Tac-Toe) only redraw on input events
- **Rising edge inputs** — `pressed` flags are true only on the frame the button is first pressed, preventing repeated triggers

---

## File Structure
```
ArduX_Project/
├── arduX.ino          # Entry point — setup(), loop(), state machine
├── Game.h             # Abstract base class + Buttons struct
├── Menu.h             # Scrollable menu with navigation
├── Snake.h            # Snake game
├── Morpion.h          # Tic-Tac-Toe game
├── Pong.h             # Pong game with AI
├── Breakout.h         # Breakout game
├── SpaceInvaders.h    # Space Invaders game
└── DinoRun.h          # Dino Run game
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

| Button | GPIO |
|--------|------|
| UP | 25 |
| DOWN | 26 |
| LEFT | 27 |
| RIGHT | 32 |
| A | 33 |
| B | 15 |
| Buzzer | 17 |

---

## Authors

Project developed during **Arduino Days 2026** — SCOP, Benin.

---

## License

Open source — free to use and modify.# ArduX — Embedded Game Console

---


