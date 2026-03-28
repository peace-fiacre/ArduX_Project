#ifndef BREAKOUT_H
#define BREAKOUT_H

#include "Game.h"

#define BRK_BUZZER 17

class BreakoutGame : public Game {
  private:

    int paddleX;
    const int paddleY=122, paddleW=36, paddleH=5, paddleSpd=5;

    float ballX, ballY, velX, velY;
    const int ballR=3;
    bool ballLaunched;

    static const int COLS=10, ROWS=5;
    static const int BW=22, BH=7, BPX=2, BPY=2, BSY=14;

    struct Brick { bool alive; int hp; };
    Brick bricks[ROWS][COLS];

    int bricksLeft, lives, wave;
    unsigned long lastUpdate;
    const int updateInterval=14;
    bool firstDraw;

    uint16_t brickColor(int row, int hp) {
      if(hp==3) return TFT_RED;
      if(hp==2) return TFT_ORANGE;
      uint16_t cols[]={TFT_RED,TFT_ORANGE,TFT_YELLOW,TFT_GREEN,TFT_CYAN};
      return cols[row%5];
    }

    void initBricks() {
      bricksLeft=0;
      for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
        bricks[r][c].alive=true;
        bricks[r][c].hp=(r<2)?2:1;
        if(wave>=3 && r==0) bricks[r][c].hp=3;
        bricksLeft++;
      }
    }

    void drawBrick(int r, int c, bool erase=false) {
      int bx=c*(BW+BPX)+2, by=BSY+r*(BH+BPY);
      if(erase) screen->fillRect(bx,by,BW,BH,TFT_BLACK);
      else {
        screen->fillRect(bx,by,BW,BH,brickColor(r,bricks[r][c].hp));
        screen->drawRect(bx,by,BW,BH,TFT_BLACK);
      }
    }

    void resetBall() {
      ballX=paddleX+paddleW/2; ballY=paddleY-ballR-2;
      velX=(random(0,2)==0)?2.5:-2.5; velY=-3.0;
      ballLaunched=false;
    }

  public:
    BreakoutGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      paddleX=102; score=0; state=IN_PROGRESS;
      firstDraw=true; lastUpdate=0; lives=3; wave=1;
      initBricks(); resetBall();
    }

    void update(Buttons buttons) override {
      if(state==GAME_OVER) return;
      if(millis()-lastUpdate<updateInterval) return;
      lastUpdate=millis();

      if(buttons.left  && paddleX>0)           paddleX-=paddleSpd;
      if(buttons.right && paddleX<240-paddleW) paddleX+=paddleSpd;

      if(!ballLaunched && (buttons.aPressed||buttons.upPressed)) {
        ballLaunched=true; tone(BRK_BUZZER,660,60);
      }
      if(!ballLaunched){ ballX=paddleX+paddleW/2; return; }

      ballX+=velX; ballY+=velY;

      if(ballX-ballR<=0)   { ballX=ballR;       velX=abs(velX);  tone(BRK_BUZZER,500,40); }
      if(ballX+ballR>=240) { ballX=240-ballR;   velX=-abs(velX); tone(BRK_BUZZER,500,40); }
      if(ballY-ballR<=10)  { ballY=10+ballR;    velY=abs(velY);  tone(BRK_BUZZER,500,40); }

      if(ballY+ballR>=paddleY && ballY+ballR<=paddleY+paddleH &&
         ballX>=paddleX-ballR && ballX<=paddleX+paddleW+ballR) {
        float offset=(ballX-(paddleX+paddleW/2))/(paddleW/2);
        velX=offset*4.0; velY=-abs(velY);
        tone(BRK_BUZZER,880,50);
      }

      if(ballY>135) {
        lives--; tone(BRK_BUZZER,200,400);
        if(lives<=0){ state=GAME_OVER; return; }
        resetBall(); firstDraw=true; return;
      }

      for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) {
        if(!bricks[r][c].alive) continue;
        int bx=c*(BW+BPX)+2, by=BSY+r*(BH+BPY);
        if(ballX+ballR>=bx && ballX-ballR<=bx+BW &&
           ballY+ballR>=by && ballY-ballR<=by+BH) {
          bricks[r][c].hp--;
          score+=(ROWS-r)*5*wave;
          if(bricks[r][c].hp<=0){ bricks[r][c].alive=false; bricksLeft--; drawBrick(r,c,true); tone(BRK_BUZZER,700,60); }
          else { drawBrick(r,c); tone(BRK_BUZZER,550,40); }
          float oT=(ballY+ballR)-by, oB=(by+BH)-(ballY-ballR);
          float oL=(ballX+ballR)-bx, oR=(bx+BW)-(ballX-ballR);
          if(min(oT,oB)<min(oL,oR)) velY=-velY; else velX=-velX;
          goto done;
        }
      }
      done:

      if(bricksLeft<=0) {
        wave++;
        tone(BRK_BUZZER,1047,150); delay(200);
        tone(BRK_BUZZER,1319,150); delay(200);
        tone(BRK_BUZZER,1568,300);
        initBricks(); firstDraw=true;
      }
    }

    void render() override {
      if(firstDraw) {
        firstDraw=false;
        screen->fillScreen(TFT_BLACK);
        screen->drawFastHLine(0,10,240,TFT_DARKGREY);
        for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++)
          if(bricks[r][c].alive) drawBrick(r,c);
      }

      screen->fillRect(0,0,240,10,TFT_BLACK);
      screen->setTextColor(TFT_WHITE,TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2,1);   screen->print("SCR:"); screen->print(score);
      screen->setCursor(100,1); screen->print("VIE:");
      for(int i=0;i<lives;i++) screen->fillRect(130+i*10,1,7,7,TFT_RED);
      screen->setCursor(168,1); screen->print("W:"); screen->print(wave);

      screen->fillRect(0,paddleY-2,240,paddleH+4,TFT_BLACK);
      screen->fillRoundRect(paddleX,paddleY,paddleW,paddleH,2,TFT_CYAN);

      screen->fillCircle((int)ballX,(int)ballY+4,ballR+1,TFT_BLACK);
      screen->fillCircle((int)ballX,(int)ballY,  ballR,  TFT_WHITE);

      if(!ballLaunched) {
        screen->setTextColor(TFT_YELLOW,TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(60,105);
        screen->print("[A] Lance la balle!");
      } else {
        screen->fillRect(60,105,120,8,TFT_BLACK);
      }
    }

    virtual ~BreakoutGame() {}
};
#endif