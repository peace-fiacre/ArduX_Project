#ifndef MEMORY_H
#define MEMORY_H

#include "Game.h"

#define MEM_BUZZER 17

enum MemoryState { MEM_PREVIEW, MEM_PLAYING, MEM_WAITING };

struct Card {
  int  logoID;
  bool revealed;
  bool matched;
};

class MemoryGame : public Game {
private:

  Card grid[12];
  int cursorCol, cursorRow;
  int firstCard, secondCard;
  int errors, matchesFound;
  unsigned long startTime, lastUpdate;
  int waitTimer, previewTimer;
  MemoryState memState;

  // ── Logos ──────────────────────────────────────────────
  void drawLogo(int x, int y, int id) {
    int cx = x + 28, cy = y + 18;
    switch (id) {
      case 0: screen->fillCircle(cx, cy, 12, TFT_RED);     break;
      case 1: screen->fillRect(cx-11, cy-11, 22, 22, TFT_BLUE); break;
      case 2: screen->fillTriangle(cx, cy-13, cx-12, cy+11, cx+12, cy+11, TFT_GREEN); break;
      case 3:
        screen->fillRect(cx-11, cy-4, 22, 8, TFT_ORANGE);
        screen->fillRect(cx-4, cy-11, 8, 22, TFT_ORANGE);
        break;
      case 4:
        screen->fillTriangle(cx, cy-13, cx-12, cy, cx+12, cy, TFT_MAGENTA);
        screen->fillTriangle(cx-12, cy, cx+12, cy, cx, cy+13, TFT_MAGENTA);
        break;
      case 5:
        screen->fillCircle(cx, cy, 13, TFT_CYAN);
        screen->fillCircle(cx, cy,  7, TFT_BLACK);
        break;
    }
  }

  void drawCard(int x, int y, int id, uint16_t bg) {
    screen->fillRect(x, y, 56, 36, bg);
    screen->drawRect(x, y, 56, 36, TFT_WHITE);
    drawLogo(x, y, id);
  }

  void drawCardBack(int x, int y, bool cursor) {
    screen->fillRect(x, y, 56, 36, screen->color565(0, 0, 80));
    screen->drawRect(x, y, 56, 36, cursor ? TFT_YELLOW : TFT_DARKGREY);
    if (cursor) screen->drawRect(x+1, y+1, 54, 34, TFT_YELLOW);
  }

  void drawHUD() {
    screen->fillRect(0, 0, 240, 20, TFT_DARKGREY);
    screen->setTextSize(1);
    screen->setTextColor(TFT_WHITE, TFT_DARKGREY);
    screen->setCursor(2,  6); screen->print("ERR:"); screen->print(errors);
    screen->setCursor(90, 6); screen->print("MEMORY");
    screen->setCursor(180,6); screen->print("SCR:"); screen->print(score);
  }

  void drawGrid() {
    for (int i = 0; i < 12; i++) {
      int col = i % 4, row = i / 4;
      int x = 4  + col * 60;
      int y = 22 + row * 38;
      bool cursor = (col == cursorCol && row == cursorRow);

      if      (grid[i].matched)   drawCard(x, y, grid[i].logoID, screen->color565(0,60,0));
      else if (grid[i].revealed)  drawCard(x, y, grid[i].logoID, TFT_BLACK);
      else                        drawCardBack(x, y, cursor);
    }
  }

public:
  MemoryGame(TFT_eSPI* display) : Game(display) {}

  // ── INIT ───────────────────────────────────────────────
  void init() override {
    score = 0; errors = 0; matchesFound = 0;
    firstCard = -1; secondCard = -1;
    cursorCol = 0;  cursorRow  = 0;
    waitTimer = 0;  previewTimer = 120;
    startTime = millis(); lastUpdate = 0;
    memState = MEM_PREVIEW;
    state    = IN_PROGRESS;

    // 6 paires
    for (int i = 0; i < 6; i++) {
      grid[i*2].logoID   = grid[i*2+1].logoID = i;
      grid[i*2].revealed = grid[i*2+1].revealed = true; // preview
      grid[i*2].matched  = grid[i*2+1].matched  = false;
    }
    // Fisher-Yates
    for (int i = 11; i > 0; i--) {
      int j = random(0, i + 1);
      Card tmp = grid[i]; grid[i] = grid[j]; grid[j] = tmp;
    }
  }

  // ── UPDATE ─────────────────────────────────────────────
  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;
    if (millis() - lastUpdate < 16) return;
    lastUpdate = millis();

    switch (memState) {

      case MEM_PREVIEW:
        if (--previewTimer <= 0) {
          for (int i = 0; i < 12; i++) grid[i].revealed = false;
          memState = MEM_PLAYING;
        }
        break;

      case MEM_PLAYING:
        if (buttons.leftPressed  && cursorCol > 0) cursorCol--;
        if (buttons.rightPressed && cursorCol < 3) cursorCol++;
        if (buttons.upPressed    && cursorRow > 0) cursorRow--;
        if (buttons.downPressed  && cursorRow < 2) cursorRow++;

        if (buttons.aPressed) {
          int idx = cursorRow * 4 + cursorCol;
          if (!grid[idx].revealed && !grid[idx].matched) {
            grid[idx].revealed = true;
            tone(MEM_BUZZER, 800, 30);
            if (firstCard == -1) {
              firstCard = idx;
            } else if (idx != firstCard) {
              secondCard = idx;
              memState   = MEM_WAITING;
              waitTimer  = 90;
            }
          }
        }
        break;

      case MEM_WAITING:
        if (--waitTimer <= 0) {
          if (grid[firstCard].logoID == grid[secondCard].logoID) {
            grid[firstCard].matched = grid[secondCard].matched = true;
            matchesFound++;
            score += 100;
            tone(MEM_BUZZER, 1047, 150);
            if (matchesFound == 6) {
              int elapsed = (int)((millis() - startTime) / 1000);
              score = max(0, 600 - errors * 10 - elapsed);
              state = GAME_OVER;
            }
          } else {
            grid[firstCard].revealed = grid[secondCard].revealed = false;
            errors++;
            score = max(0, score - 10);
            tone(MEM_BUZZER, 200, 200);
          }
          firstCard = secondCard = -1;
          memState  = MEM_PLAYING;
        }
        break;
    }
  }

  // ── RENDER ─────────────────────────────────────────────
  void render() override {
    screen->fillRect(0, 20, 240, 115, TFT_BLACK);
    drawHUD();
    drawGrid();
  }

  virtual ~MemoryGame() {}
};

#endif // MEMORY_H
