#ifndef DINORUN_H
#define DINORUN_H

#include "Game.h"

#define DINO_BUZZER 17

class DinoRunGame : public Game {
  private:

    int dinoY, dinoVel;
    bool isJumping, isDucking;
    const int dinoX   = 30;
    const int groundY = 110;
    const int dinoW   = 14;
    const int dinoH   = 18;
    const int duckH   = 10;

    struct Cactus { int x, w, h; bool active; };
    Cactus cacti[3];

    struct Bird { int x, y, flapTimer; bool active, flapUp; };
    Bird bird;

    int   groundOffset;
    float speed;
    unsigned long lastUpdate;
    const int updateInterval = 20;
    bool firstDraw;

    void spawnCactus(int idx) {
      cacti[idx].x      = 250 + random(0, 100);
      cacti[idx].w      = 8  + random(0, 6);
      cacti[idx].h      = 18 + random(0, 14);
      cacti[idx].active = true;
    }

    void spawnBird() {
      bird = { 260, groundY - 40 - random(0,20), 0, true, true };
    }

    void drawDino(uint16_t color) {
      int h = isDucking ? duckH : dinoH;
      int y = isDucking ? groundY - duckH : dinoY;
      screen->fillRect(dinoX, y, dinoW, h, color);
      if (color != TFT_BLACK)
        screen->fillRect(dinoX + dinoW - 4, y + 2, 3, 3, TFT_RED);
    }

    void drawCactus(int idx, uint16_t color) {
      int cx = cacti[idx].x, ch = cacti[idx].h, cw = cacti[idx].w;
      screen->fillRect(cx,      groundY - ch,     cw, ch, color);
      screen->fillRect(cx - 4,  groundY - ch + 6,  4,  8, color);
      screen->fillRect(cx + cw, groundY - ch + 8,  4,  6, color);
    }

    void drawBird(uint16_t color) {
      int wingY = bird.flapUp ? bird.y - 4 : bird.y + 2;
      screen->fillRect(bird.x,     bird.y,  12, 6, color);
      screen->fillRect(bird.x + 2, wingY,    8, 4, color);
    }

    bool collides(int ax,int ay,int aw,int ah,int bx,int by,int bw,int bh) {
      return ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by;
    }

  public:
    DinoRunGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      dinoY = groundY - dinoH; dinoVel = 0;
      isJumping = false; isDucking = false;
      speed = 3.0; score = 0; state = IN_PROGRESS;
      firstDraw = true; groundOffset = 0; lastUpdate = 0;
      bird.active = false;
      for (int i = 0; i < 3; i++) spawnCactus(i);
      cacti[1].x = 350; cacti[2].x = 500;
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;
      if (millis() - lastUpdate < updateInterval) return;
      lastUpdate = millis();

      if ((buttons.upPressed || buttons.aPressed) && !isJumping) {
        dinoVel = -9; isJumping = true;
        tone(DINO_BUZZER, 880, 80);
      }
      isDucking = buttons.down && !isJumping;

      dinoVel += 1; dinoY += dinoVel;
      if (dinoY >= groundY - dinoH) {
        dinoY = groundY - dinoH; dinoVel = 0; isJumping = false;
      }

      groundOffset = (groundOffset + (int)speed) % 20;

      for (int i = 0; i < 3; i++) {
        if (!cacti[i].active) continue;
        cacti[i].x -= (int)speed;
        if (cacti[i].x < -20) spawnCactus(i);
      }

      if (!bird.active && random(0, 300) == 0 && score > 5) spawnBird();
      if (bird.active) {
        bird.x -= (int)(speed + 1);
        bird.flapTimer++;
        if (bird.flapTimer % 10 == 0) bird.flapUp = !bird.flapUp;
        if (bird.x < -20) bird.active = false;
      }

      speed  += 0.002;
      score   = (int)(millis() / 100) % 10000;

      int dh = isDucking ? duckH : dinoH;
      int dy = isDucking ? groundY - duckH : dinoY;

      for (int i = 0; i < 3; i++) {
        if (!cacti[i].active) continue;
        if (collides(dinoX+2, dy+2, dinoW-4, dh-4,
                     cacti[i].x, groundY-cacti[i].h, cacti[i].w, cacti[i].h)) {
          state = GAME_OVER; tone(DINO_BUZZER, 200, 500); return;
        }
      }
      if (bird.active && collides(dinoX+2, dy+2, dinoW-4, dh-4,
                                   bird.x, bird.y, 12, 6)) {
        state = GAME_OVER; tone(DINO_BUZZER, 200, 500); return;
      }
    }

    void render() override {
      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);
        for (int y = 0; y < groundY - 10; y += 2)
          screen->drawFastHLine(0, y, 240, screen->color565(0, 0, 30 + y/4));
        screen->fillRect(0, groundY, 240, 5, TFT_GREEN);
        screen->fillRect(0, groundY+5, 240, 20, screen->color565(80,40,0));
      }

      screen->fillRect(dinoX-2, groundY-dinoH-10, dinoW+8, dinoH+14, TFT_BLACK);
      for (int i = 0; i < 3; i++)
        screen->fillRect(cacti[i].x-6, groundY-50, cacti[i].w+14, 52, TFT_BLACK);
      if (bird.active)
        screen->fillRect(bird.x-2, bird.y-6, 18, 16, TFT_BLACK);

      screen->fillRect(0, groundY, 240, 3, TFT_GREEN);
      for (int x = -groundOffset; x < 240; x += 20)
        screen->fillRect(x, groundY+3, 10, 2, screen->color565(60,30,0));

      for (int i = 0; i < 3; i++)
        if (cacti[i].active) drawCactus(i, TFT_GREEN);
      if (bird.active) drawBird(TFT_CYAN);
      drawDino(TFT_WHITE);

      screen->fillRect(0, 0, 240, 16, TFT_BLACK);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 4);
      screen->print("DINO  SCORE:"); screen->print(score);
      screen->setCursor(180, 4);
      screen->print("SPD:"); screen->print((int)speed);
    }

    virtual ~DinoRunGame() {}
};
#endif