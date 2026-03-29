#ifndef DINORUN_H
#define DINORUN_H

#include "Game.h"

#define DINO_BUZZER 17

class DinoRunGame : public Game {
  private:

    int dinoY, dinoVel;
    bool isJumping, isDucking;
    const int dinoX   = 30;
    const int groundY = 108;
    const int dinoW   = 14;
    const int dinoH   = 16;
    const int duckH   = 9;

    struct Cactus { int x, w, h; bool active; };
    Cactus cacti[3];

    struct Bird { int x, y, flapTimer; bool active, flapUp; };
    Bird bird;

    int   groundOffset;
    float speed;
    unsigned long lastUpdate;
    const int updateInterval = 20;

    void spawnCactus(int idx) {
      cacti[idx].x      = 260 + random(0, 120);
      cacti[idx].w      = 8  + random(0, 6);
      cacti[idx].h      = 16 + random(0, 12);
      cacti[idx].active = true;
    }

    void spawnBird() {
      bird.x = 260;
      bird.y = groundY - 38 - random(0, 15);
      bird.flapTimer = 0;
      bird.active = true;
      bird.flapUp = true;
    }

    bool collides(int ax,int ay,int aw,int ah,
                  int bx,int by,int bw,int bh) {
      return ax < bx+bw && ax+aw > bx &&
             ay < by+bh && ay+ah > by;
    }

  public:
    DinoRunGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      dinoY = groundY - dinoH;
      dinoVel = 0;
      isJumping = false;
      isDucking = false;
      speed = 3.0;
      score = 0;
      state = IN_PROGRESS;
      groundOffset = 0;
      lastUpdate = 0;
      bird.active = false;
      for(int i=0;i<3;i++) spawnCactus(i);
      cacti[1].x = 380;
      cacti[2].x = 520;
      screen->fillScreen(TFT_BLACK);
    }

    void update(Buttons buttons) override {
      if(state == GAME_OVER) return;

      // Bouton B → quitter
      if(buttons.bPressed) { state = GAME_OVER; return; }

      if(millis() - lastUpdate < updateInterval) return;
      lastUpdate = millis();

      // Saut
      if((buttons.upPressed || buttons.aPressed) && !isJumping) {
        dinoVel = -9;
        isJumping = true;
        tone(DINO_BUZZER, 880, 80);
      }

      isDucking = buttons.down && !isJumping;

      // Gravité
      dinoVel += 1;
      dinoY   += dinoVel;
      if(dinoY >= groundY - dinoH) {
        dinoY = groundY - dinoH;
        dinoVel = 0;
        isJumping = false;
      }

      groundOffset = (groundOffset + (int)speed) % 20;

      // Cactus
      for(int i=0;i<3;i++) {
        cacti[i].x -= (int)speed;
        if(cacti[i].x < -30) spawnCactus(i);
      }

      // Oiseau
      if(!bird.active && random(0,200)==0 && score > 5) spawnBird();
      if(bird.active) {
        bird.x -= (int)(speed + 1);
        bird.flapTimer++;
        if(bird.flapTimer % 10 == 0) bird.flapUp = !bird.flapUp;
        if(bird.x < -20) bird.active = false;
      }

      speed += 0.002;
      score  = (int)(millis() / 100) % 9999;

      // Collisions
      int dh = isDucking ? duckH : dinoH;
      int dy = isDucking ? groundY - duckH : dinoY;

      for(int i=0;i<3;i++) {
        if(collides(dinoX+2, dy+2, dinoW-4, dh-4,
                    cacti[i].x, groundY-cacti[i].h,
                    cacti[i].w, cacti[i].h)) {
          state = GAME_OVER;
          tone(DINO_BUZZER, 200, 500);
          return;
        }
      }
      if(bird.active) {
        if(collides(dinoX+2, dy+2, dinoW-4, dh-4,
                    bird.x, bird.y, 14, 8)) {
          state = GAME_OVER;
          tone(DINO_BUZZER, 200, 500);
          return;
        }
      }
    }

    void render() override {
      // Fond complet à chaque frame → élimine tous les pixels parasites
      screen->fillScreen(TFT_BLACK);

      // Ciel
      for(int y=0; y<groundY-8; y+=3)
        screen->drawFastHLine(0, y, 240, screen->color565(0, 0, 20+y/5));

      // Sol
      screen->fillRect(0, groundY, 240, 4, TFT_GREEN);
      screen->fillRect(0, groundY+4, 240, 15, screen->color565(80,40,0));
      for(int x = -groundOffset; x < 240; x += 20)
        screen->fillRect(x, groundY+4, 10, 2, screen->color565(60,30,0));

      // Cactus
      for(int i=0;i<3;i++) {
        int cx=cacti[i].x, ch=cacti[i].h, cw=cacti[i].w;
        screen->fillRect(cx,      groundY-ch,     cw, ch, TFT_GREEN);
        screen->fillRect(cx-4,    groundY-ch+5,   4,  7,  TFT_GREEN);
        screen->fillRect(cx+cw,   groundY-ch+7,   4,  6,  TFT_GREEN);
      }

      // Oiseau
      if(bird.active) {
        int wingY = bird.flapUp ? bird.y-3 : bird.y+2;
        screen->fillRect(bird.x,   bird.y, 14, 6, TFT_CYAN);
        screen->fillRect(bird.x+2, wingY,   9, 4, TFT_CYAN);
      }

      // Dino
      int dh = isDucking ? duckH : dinoH;
      int dy = isDucking ? groundY-duckH : dinoY;
      screen->fillRect(dinoX, dy, dinoW, dh, TFT_WHITE);
      screen->fillRect(dinoX+dinoW-4, dy+2, 3, 3, TFT_RED); // oeil

      // HUD
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 1);
      screen->print("SCORE:"); screen->print(score);
      screen->setCursor(140, 1);
      screen->print("SPD:"); screen->print((int)(speed*10)/10.0);
      screen->setCursor(190, 1);
      screen->print("[B]Quit");
    }

    virtual ~DinoRunGame() {}
};
#endif