#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H

#include "Game.h"

#define SI_BUZZER 17

class SpaceInvadersGame : public Game {
  private:

    int shipX;
    const int shipY=118, shipW=16, shipH=8, shipSpd=4;

    struct Bullet { int x, y; bool active; };
    Bullet bullets[3];
    Bullet enemyBullets[3];

    struct Invader { int x, y, type; bool alive; };
    static const int COLS=8, ROWS=3;
    Invader invaders[ROWS][COLS];

    int   invaderDirX, invaderOffsetX, invaderOffsetY;
    float invaderSpeed;
    int   invaderMoveInterval, aliveCount;
    unsigned long lastInvaderMove;

    struct Explosion { int x, y, timer; bool active; };
    Explosion explosions[4];

    unsigned long lastUpdate, lastShot, lastEnemyShot;
    const int updateInterval = 16;
    bool firstDraw;
    int  wave;

    void drawInvader(int x, int y, int type, uint16_t color) {
      switch(type) {
        case 0:
          screen->fillRect(x+3, y,   6, 3, color);
          screen->fillRect(x,   y+3, 12,4, color);
          screen->fillRect(x+2, y+7, 3, 2, color);
          screen->fillRect(x+7, y+7, 3, 2, color); break;
        case 1:
          screen->fillRect(x+2, y,   8, 2, color);
          screen->fillRect(x,   y+2, 12,5, color);
          screen->fillRect(x+1, y+7, 3, 2, color);
          screen->fillRect(x+8, y+7, 3, 2, color); break;
        case 2:
          screen->fillRect(x+1, y,   10,3, color);
          screen->fillRect(x,   y+3, 12,4, color);
          screen->fillRect(x,   y+7, 4, 2, color);
          screen->fillRect(x+8, y+7, 4, 2, color); break;
      }
    }

    void drawShip(int x, uint16_t color) {
      screen->fillRect(x+6, shipY-4, 4,  4, color);
      screen->fillRect(x+2, shipY,   12, 4, color);
      screen->fillRect(x,   shipY+4, 16, 4, color);
    }

    int countAlive() {
      int n=0;
      for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) if(invaders[r][c].alive) n++;
      return n;
    }

    void initWave() {
      for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
        invaders[r][c] = { 10+c*26, 18+r*16, r, true };
      }
      invaderDirX=1; invaderOffsetX=0; invaderOffsetY=0;
      invaderMoveInterval = max(100, 400-wave*40);
      lastInvaderMove=0; aliveCount=ROWS*COLS;
    }

  public:
    SpaceInvadersGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      shipX=112; score=0; state=IN_PROGRESS;
      firstDraw=true; lastUpdate=0; lastShot=0; lastEnemyShot=0; wave=1;
      for(int i=0;i<3;i++){ bullets[i].active=false; enemyBullets[i].active=false; }
      for(int i=0;i<4;i++) explosions[i].active=false;
      initWave();
    }

    void update(Buttons buttons) override {
      if(state==GAME_OVER) return;
      if(millis()-lastUpdate < updateInterval) return;
      lastUpdate=millis();

      if(buttons.left  && shipX>0)          shipX-=shipSpd;
      if(buttons.right && shipX<240-shipW)  shipX+=shipSpd;

      if(buttons.aPressed && millis()-lastShot>300) {
        lastShot=millis();
        for(int i=0;i<3;i++) if(!bullets[i].active) {
          bullets[i]={shipX+shipW/2, shipY-4, true};
          tone(SI_BUZZER, 1200, 60); break;
        }
      }

      for(int i=0;i<3;i++) {
        if(!bullets[i].active) continue;
        bullets[i].y -= 5;
        if(bullets[i].y<0){ bullets[i].active=false; continue; }
        for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
          if(!invaders[r][c].alive) continue;
          int ix=invaders[r][c].x+invaderOffsetX;
          int iy=invaders[r][c].y+invaderOffsetY;
          if(bullets[i].x>=ix && bullets[i].x<=ix+12 &&
             bullets[i].y>=iy && bullets[i].y<=iy+10) {
            invaders[r][c].alive=false; bullets[i].active=false;
            score+=(ROWS-r)*10*wave; aliveCount--;
            tone(SI_BUZZER, 400, 100);
            for(int e=0;e<4;e++) if(!explosions[e].active) {
              explosions[e]={ix,iy,5,true}; break;
            }
          }
        }
      }

      if(millis()-lastInvaderMove > invaderMoveInterval) {
        lastInvaderMove=millis();
        int maxX=0, minX=240;
        for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
          if(!invaders[r][c].alive) continue;
          int ix=invaders[r][c].x+invaderOffsetX;
          if(ix+12>maxX) maxX=ix+12;
          if(ix<minX)    minX=ix;
        }
        if((invaderDirX==1 && maxX>=238)||(invaderDirX==-1 && minX<=2)) {
          invaderDirX=-invaderDirX; invaderOffsetY+=8; tone(SI_BUZZER,150,80);
        } else { invaderOffsetX+=invaderDirX*3; }
        invaderMoveInterval=max(40, aliveCount*12);
      }

      if(millis()-lastEnemyShot>1200 && aliveCount>0) {
        lastEnemyShot=millis();
        for(int tries=0;tries<20;tries++) {
          int r=random(0,ROWS), c=random(0,COLS);
          if(invaders[r][c].alive) {
            for(int i=0;i<3;i++) if(!enemyBullets[i].active) {
              enemyBullets[i]={invaders[r][c].x+invaderOffsetX+6,
                               invaders[r][c].y+invaderOffsetY+10, true}; break;
            }
            break;
          }
        }
      }

      for(int i=0;i<3;i++) {
        if(!enemyBullets[i].active) continue;
        enemyBullets[i].y+=3;
        if(enemyBullets[i].y>135){ enemyBullets[i].active=false; continue; }
        if(enemyBullets[i].x>=shipX && enemyBullets[i].x<=shipX+shipW &&
           enemyBullets[i].y>=shipY && enemyBullets[i].y<=shipY+shipH) {
          state=GAME_OVER; tone(SI_BUZZER,100,800); return;
        }
      }

      for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
        if(!invaders[r][c].alive) continue;
        if(invaders[r][c].y+invaderOffsetY>=shipY-10) {
          state=GAME_OVER; tone(SI_BUZZER,100,800); return;
        }
      }

      if(aliveCount<=0) {
        wave++;
        tone(SI_BUZZER,1047,200); delay(300);
        tone(SI_BUZZER,1319,400);
        initWave(); firstDraw=true;
      }

      for(int e=0;e<4;e++) {
        if(explosions[e].active) if(--explosions[e].timer<=0) explosions[e].active=false;
      }
    }

    void render() override {
      if(firstDraw) {
        firstDraw=false;
        screen->fillScreen(TFT_BLACK);
        for(int i=0;i<30;i++)
          screen->drawPixel(random(0,240), random(10,110), TFT_WHITE);
      }

      screen->fillRect(0,0,240,12,TFT_BLACK);
      screen->setTextColor(TFT_GREEN,TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2,2);  screen->print("SCORE:"); screen->print(score);
      screen->setCursor(140,2); screen->print("VAGUE:"); screen->print(wave);
      screen->drawFastHLine(0,113,240,TFT_GREEN);

      for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
        int ix=invaders[r][c].x+invaderOffsetX;
        int iy=invaders[r][c].y+invaderOffsetY;
        screen->fillRect(ix-1,iy-1,15,12,TFT_BLACK);
        if(invaders[r][c].alive) {
          uint16_t col=(r==0)?TFT_CYAN:(r==1)?TFT_GREEN:TFT_MAGENTA;
          drawInvader(ix,iy,invaders[r][c].type,col);
        }
      }

      for(int e=0;e<4;e++) {
        if(!explosions[e].active) continue;
        screen->setTextColor(TFT_ORANGE,TFT_BLACK);
        screen->setCursor(explosions[e].x, explosions[e].y);
        screen->print("*+*");
      }

      for(int i=0;i<3;i++) {
        if(!bullets[i].active) continue;
        screen->fillRect(bullets[i].x, bullets[i].y+3, 2,5, TFT_BLACK);
        screen->fillRect(bullets[i].x, bullets[i].y,   2,5, TFT_YELLOW);
      }
      for(int i=0;i<3;i++) {
        if(!enemyBullets[i].active) continue;
        screen->fillRect(enemyBullets[i].x, enemyBullets[i].y-3, 2,5, TFT_BLACK);
        screen->fillRect(enemyBullets[i].x, enemyBullets[i].y,   2,5, TFT_RED);
      }

      screen->fillRect(0, shipY-5, 240, 18, TFT_BLACK);
      screen->drawFastHLine(0, 113, 240, TFT_GREEN);
      drawShip(shipX, TFT_WHITE);
    }

    virtual ~SpaceInvadersGame() {}
};
#endif