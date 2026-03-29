#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H

#include "Game.h"

#define SI_BUZZER 17

class SpaceInvadersGame : public Game {
  private:

    int shipX;
    const int shipY=118, shipW=16, shipH=8, shipSpd=4;

    int  lives;
    bool shieldActive;
    unsigned long shieldStart;
    const unsigned long SHIELD_DURATION = 2000UL;

    struct Bullet { int x, y; bool active; };
    Bullet bullets[3];
    Bullet enemyBullets[3];

    struct Invader { int x, y, type; bool alive; };
    static const int COLS=6, ROWS=3;
    Invader invaders[ROWS][COLS];

    int   invaderDirX, invaderOffsetX, invaderOffsetY;
    int   invaderMoveInterval, aliveCount;
    unsigned long lastInvaderMove;

    struct Explosion { int x, y, timer, frame; bool active; };
    Explosion explosions[4];

    struct UFO { int x; bool active; int scoreValue; };
    UFO ufo;
    unsigned long lastUfoCheck;

    unsigned long lastUpdate, lastShot, lastEnemyShot;
    const int updateInterval = 16;

    bool firstDraw, showingIntro, introDrewStatic;
    int  wave;

    int  starX[25], starY[25];
    bool starsInit;

    void drawInvader(int x, int y, int type, uint16_t color) {
      screen->fillRect(x,y,13,10,TFT_BLACK);
      switch(type){
        case 0:
          screen->fillRect(x+3,y,6,3,color); screen->fillRect(x,y+3,13,4,color);
          screen->fillRect(x+2,y+7,3,2,color); screen->fillRect(x+8,y+7,3,2,color); break;
        case 1:
          screen->fillRect(x+2,y,9,2,color); screen->fillRect(x,y+2,13,5,color);
          screen->fillRect(x+1,y+7,3,2,color); screen->fillRect(x+9,y+7,3,2,color); break;
        case 2:
          screen->fillRect(x+1,y,11,3,color); screen->fillRect(x,y+3,13,4,color);
          screen->fillRect(x,y+7,4,2,color); screen->fillRect(x+9,y+7,4,2,color); break;
      }
    }

    void drawShip(int x, uint16_t color) {
      screen->fillRect(x+6,shipY-4,4,4,color);
      screen->fillRect(x+2,shipY,12,4,color);
      screen->fillRect(x,shipY+4,16,4,color);
    }

    void drawMiniShip(int x, int y, uint16_t color) {
      screen->fillRect(x+3,y,2,2,color);
      screen->fillRect(x+1,y+2,6,2,color);
      screen->fillRect(x,y+4,8,2,color);
    }

    void initWave() {
      for (int r=0;r<ROWS;r++) for (int c=0;c<COLS;c++)
        invaders[r][c]={18+c*33,20+r*18,ROWS-1-r,true};
      invaderDirX=1; invaderOffsetX=0; invaderOffsetY=0;
      invaderMoveInterval=max(180,650-wave*35);
      lastInvaderMove=0; aliveCount=ROWS*COLS;
    }

    void initStars() {
      for (int i=0;i<25;i++) { starX[i]=random(0,240); starY[i]=random(14,108); }
      starsInit=true;
    }

    void takeDamage() {
      lives--; tone(SI_BUZZER,150,400);
      if (lives<=0) { state=GAME_OVER; return; }
      shieldActive=true; shieldStart=millis();
      screen->fillRect(shipX-2,shipY-6,shipW+4,shipH+8,TFT_BLACK);
    }

    void renderIntroStatic() {
      screen->fillScreen(TFT_BLACK);

      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(2);
      screen->setCursor(20, 3);
      screen->print("SPACE INVADERS");
      screen->drawFastHLine(10, 22, 220, TFT_CYAN);

      int ax=8, ay=27;
      screen->fillRect(ax+3,ay,6,3,TFT_RED); screen->fillRect(ax,ay+3,13,4,TFT_RED);
      screen->fillRect(ax+2,ay+7,3,2,TFT_RED); screen->fillRect(ax+8,ay+7,3,2,TFT_RED);
      screen->fillRect(ax+20,ay+2,12,4,TFT_MAGENTA); screen->fillRect(ax+23,ay-1,6,4,TFT_MAGENTA);
      screen->fillRect(ax+38,ay+5,3,3,TFT_WHITE); screen->fillRect(ax+36,ay+7,7,3,TFT_WHITE);
      screen->fillRect(ax+34,ay+9,11,2,TFT_WHITE);

      screen->setTextSize(1);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(55,27); screen->print("Defend la Terre contre");
      screen->setCursor(55,37); screen->print("l'invasion extraterrestre !");
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setCursor(55,49); screen->print("3 rangees d'envahisseurs.");
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setCursor(55,59); screen->print("UFO = 100 a 500 pts bonus !");
      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setCursor(55,69); screen->print("3 vies | Bouclier apres chaque hit.");

      screen->drawFastHLine(10,82,220,TFT_DARKGREY);
      screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
      screen->setCursor(98,85); screen->print("CONTROLES");
      screen->drawFastHLine(10,94,220,TFT_DARKGREY);

      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setCursor(10,98); screen->print("[<] [>]");
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->print(" Deplacer le vaisseau");

      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setCursor(10,108); screen->print("[A]");
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->print(" Tirer  |  ");
      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->print("[B]");
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->print(" Quitter");
    }

    void renderIntroBlinking() {
      screen->fillRect(0,119,240,16,TFT_BLACK);
      if ((millis()/500)%2==0) {
        screen->setTextColor(TFT_CYAN, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(30,122);
        screen->print(">> Appuie sur [A] pour commencer <<");
      } else {
        screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(72,122);
        screen->print("[B] Retour au menu");
      }
    }

  public:
    SpaceInvadersGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      shipX=112; score=0; state=IN_PROGRESS;
      firstDraw=true; lastUpdate=0; lastShot=0; lastEnemyShot=0; lastUfoCheck=0;
      wave=1; lives=3; shieldActive=false; shieldStart=0;
      ufo.active=false; ufo.x=0; ufo.scoreValue=0;
      starsInit=false; showingIntro=true; introDrewStatic=false;
      for (int i=0;i<3;i++) { bullets[i].active=false; enemyBullets[i].active=false; }
      for (int i=0;i<4;i++) explosions[i].active=false;
      initWave();
    }

    void update(Buttons buttons) override {
      if (showingIntro) {
        if (buttons.bPressed) { state=GAME_OVER; return; }
        if (buttons.aPressed) {
          showingIntro=false; firstDraw=true;
          screen->fillScreen(TFT_BLACK);
          tone(SI_BUZZER,1200,60);
        }
        return;
      }
      if (state==GAME_OVER) return;
      if (buttons.bPressed) { state=GAME_OVER; return; }
      if (millis()-lastUpdate<updateInterval) return;
      lastUpdate=millis();

      if (shieldActive && millis()-shieldStart>SHIELD_DURATION) shieldActive=false;
      if (buttons.left  && shipX>0)         shipX-=shipSpd;
      if (buttons.right && shipX<240-shipW) shipX+=shipSpd;

      if (buttons.aPressed && millis()-lastShot>280) {
        lastShot=millis();
        for (int i=0;i<3;i++) if (!bullets[i].active) {
          bullets[i]={shipX+shipW/2,shipY-4,true}; tone(SI_BUZZER,1200,60); break;
        }
      }

      for (int i=0;i<3;i++) {
        if (!bullets[i].active) continue;
        bullets[i].y-=6;
        if (bullets[i].y<12) { bullets[i].active=false; continue; }
        if (ufo.active && bullets[i].y<22 && bullets[i].x>ufo.x && bullets[i].x<ufo.x+18) {
          score+=ufo.scoreValue; ufo.active=false; bullets[i].active=false;
          tone(SI_BUZZER,1800,120);
          for (int e=0;e<4;e++) if (!explosions[e].active) { explosions[e]={ufo.x+8,14,8,0,true}; break; }
          continue;
        }
        for (int r=0;r<ROWS;r++) for (int c=0;c<COLS;c++) {
          if (!invaders[r][c].alive) continue;
          int ix=invaders[r][c].x+invaderOffsetX, iy=invaders[r][c].y+invaderOffsetY;
          if (bullets[i].x>=ix && bullets[i].x<=ix+12 && bullets[i].y>=iy && bullets[i].y<=iy+10) {
            invaders[r][c].alive=false; bullets[i].active=false;
            score+=(ROWS-r)*10*wave; aliveCount--;
            tone(SI_BUZZER,400,80); screen->fillRect(ix-1,iy-1,16,13,TFT_BLACK);
            for (int e=0;e<4;e++) if (!explosions[e].active) { explosions[e]={ix,iy,6,0,true}; break; }
          }
        }
      }

      if (millis()-lastInvaderMove>invaderMoveInterval) {
        lastInvaderMove=millis(); int maxX=0,minX=240;
        for (int r=0;r<ROWS;r++) for (int c=0;c<COLS;c++) {
          if (!invaders[r][c].alive) continue;
          int ix=invaders[r][c].x+invaderOffsetX;
          if (ix+13>maxX) maxX=ix+13; if (ix<minX) minX=ix;
        }
        if ((invaderDirX==1&&maxX>=237)||(invaderDirX==-1&&minX<=2))
          { invaderDirX=-invaderDirX; invaderOffsetY+=6; tone(SI_BUZZER,160,60); }
        else invaderOffsetX+=invaderDirX*2;
        invaderMoveInterval=max(35,aliveCount*10);
      }

      if (millis()-lastEnemyShot>1000 && aliveCount>0) {
        lastEnemyShot=millis();
        for (int tries=0;tries<20;tries++) {
          int r=random(0,ROWS),c=random(0,COLS);
          if (invaders[r][c].alive) {
            for (int i=0;i<3;i++) if (!enemyBullets[i].active) {
              enemyBullets[i]={invaders[r][c].x+invaderOffsetX+6,invaders[r][c].y+invaderOffsetY+10,true}; break;
            }
            break;
          }
        }
      }

      for (int i=0;i<3;i++) {
        if (!enemyBullets[i].active) continue;
        enemyBullets[i].y+=3;
        if (enemyBullets[i].y>135) { enemyBullets[i].active=false; continue; }
        if (!shieldActive && enemyBullets[i].x>=shipX && enemyBullets[i].x<=shipX+shipW &&
            enemyBullets[i].y>=shipY && enemyBullets[i].y<=shipY+shipH)
          { enemyBullets[i].active=false; takeDamage(); if (state==GAME_OVER) return; }
      }

      for (int r=0;r<ROWS;r++) for (int c=0;c<COLS;c++)
        if (invaders[r][c].alive && invaders[r][c].y+invaderOffsetY>=shipY-10)
          { state=GAME_OVER; tone(SI_BUZZER,100,800); return; }

      if (aliveCount<=0) {
        wave++; tone(SI_BUZZER,1047,200); delay(300); tone(SI_BUZZER,1319,400);
        initWave(); firstDraw=true;
      }

      if (!ufo.active && millis()-lastUfoCheck>500) {
        lastUfoCheck=millis();
        if (score>100 && random(0,300)==0) { ufo.x=240; ufo.scoreValue=100+random(0,5)*100; ufo.active=true; }
      }
      if (ufo.active) { ufo.x-=2; if (ufo.x<-22) ufo.active=false; }

      for (int e=0;e<4;e++) if (explosions[e].active) {
        explosions[e].timer--;
        if (explosions[e].timer<=0) {
          explosions[e].frame++; explosions[e].timer=3;
          if (explosions[e].frame>=3) { screen->fillRect(explosions[e].x-2,explosions[e].y-2,20,14,TFT_BLACK); explosions[e].active=false; }
        }
      }
    }

    void render() override {
      if (showingIntro) {
        if (!introDrewStatic) { renderIntroStatic(); introDrewStatic=true; }
        renderIntroBlinking(); return;
      }

      if (firstDraw) {
        firstDraw=false; screen->fillScreen(TFT_BLACK);
        if (!starsInit) initStars();
        for (int i=0;i<25;i++) screen->drawPixel(starX[i],starY[i],TFT_WHITE);
      }

      screen->fillRect(0,12,240,101,TFT_BLACK);
      for (int i=0;i<25;i++) screen->drawPixel(starX[i],starY[i],screen->color565(80,80,80));

      screen->fillRect(0,0,240,12,TFT_BLACK);
      screen->setTextColor(TFT_GREEN,TFT_BLACK); screen->setTextSize(1);
      screen->setCursor(2,2); screen->print("SC:"); screen->print(score);
      for (int i=0;i<3;i++) { uint16_t c=i<lives?TFT_CYAN:screen->color565(30,30,30); drawMiniShip(80+i*12,2,c); }
      screen->setCursor(120,2); screen->print("W:"); screen->print(wave);
      if (ufo.active) { screen->setTextColor(TFT_MAGENTA,TFT_BLACK); screen->setCursor(158,2); screen->print(ufo.scoreValue); screen->print("!"); }
      screen->setTextColor(TFT_DARKGREY,TFT_BLACK); screen->setCursor(190,2); screen->print("[B]Quit");
      screen->drawFastHLine(0,113,240,TFT_GREEN);

      if (ufo.active) {
        uint16_t uc=((millis()/150)%2)?TFT_MAGENTA:TFT_RED;
        screen->fillRect(ufo.x,14,18,5,uc); screen->fillRect(ufo.x+4,11,10,4,uc);
        screen->fillRect(ufo.x+7,18,4,2,screen->color565(255,180,0));
      }

      for (int r=0;r<ROWS;r++) for (int c=0;c<COLS;c++) {
        if (!invaders[r][c].alive) continue;
        int ix=invaders[r][c].x+invaderOffsetX, iy=invaders[r][c].y+invaderOffsetY, t=invaders[r][c].type;
        uint16_t col=(t==0)?TFT_RED:(t==1)?TFT_GREEN:TFT_CYAN;
        drawInvader(ix,iy,t,col);
      }

      for (int e=0;e<4;e++) if (explosions[e].active) {
        screen->setTextColor(TFT_ORANGE,TFT_BLACK); screen->setTextSize(1);
        screen->setCursor(explosions[e].x,explosions[e].y);
        if (explosions[e].frame==0) screen->print("***");
        else if (explosions[e].frame==1) screen->print("* *");
        else screen->print(" . ");
      }

      for (int i=0;i<3;i++) if (bullets[i].active) {
        screen->fillRect(bullets[i].x-1,bullets[i].y,2,7,TFT_YELLOW);
        screen->fillRect(bullets[i].x-1,bullets[i].y+7,2,2,screen->color565(255,180,0));
      }
      for (int i=0;i<3;i++) if (enemyBullets[i].active) {
        screen->fillRect(enemyBullets[i].x,enemyBullets[i].y,2,5,TFT_RED);
        screen->fillRect(enemyBullets[i].x,enemyBullets[i].y+5,2,2,TFT_ORANGE);
      }

      bool drawNow=!shieldActive||((millis()/120)%2==0);
      if (drawNow) {
        uint16_t sc=shieldActive?TFT_CYAN:TFT_WHITE;
        drawShip(shipX,sc);
        if (!shieldActive && ((millis()/80)%2==0)) screen->fillRect(shipX+5,shipY+shipH,5,3,screen->color565(255,100,0));
      } else screen->fillRect(shipX-1,shipY-5,shipW+2,shipH+6,TFT_BLACK);
    }

    virtual ~SpaceInvadersGame() {}
};
#endif
