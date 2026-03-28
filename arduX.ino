#include <TFT_eSPI.h>
#include <EEPROM.h>

#include "Game.h"
#include "Menu.h"
#include "Snake.h"
#include "Morpion.h"
#include "Pong.h"
#include "DinoRun.h"
#include "SpaceInvaders.h"
#include "Breakout.h"

#define BTN_UP     25
#define BTN_DOWN   26
#define BTN_LEFT   27
#define BTN_RIGHT  32
#define BTN_A      13
#define BTN_B      15
#define BUZZER_PIN 17
#define TFT_BL      4

Buttons buttons;
enum stateConsole { STARTING, MENU, PLAYING, GAMEOVER };
int currentGameId = -1;
TFT_eSPI screen   = TFT_eSPI();
Menu menu(&screen);
stateConsole consoleState = STARTING;
Game* currentGame = nullptr;
int highScore     = 0;

void readButtons() {
  bool pU=buttons.up, pD=buttons.down, pL=buttons.left,
       pR=buttons.right, pA=buttons.a, pB=buttons.b;

  buttons.up    = !digitalRead(BTN_UP);
  buttons.down  = !digitalRead(BTN_DOWN);
  buttons.left  = !digitalRead(BTN_LEFT);
  buttons.right = !digitalRead(BTN_RIGHT);
  buttons.a     = !digitalRead(BTN_A);
  buttons.b     = !digitalRead(BTN_B);

  buttons.upPressed    = buttons.up    && !pU;
  buttons.downPressed  = buttons.down  && !pD;
  buttons.leftPressed  = buttons.left  && !pL;
  buttons.rightPressed = buttons.right && !pR;
  buttons.aPressed     = buttons.a     && !pA;
  buttons.bPressed     = buttons.b     && !pB;
}

void showBoot() {
  screen.fillScreen(TFT_BLACK);

  // Étoiles
  for(int i=0;i<40;i++)
    screen.drawPixel(random(0,240), random(0,135), TFT_WHITE);

  // Titre lettre par lettre
  screen.setTextSize(3);
  const char* title = "ARDUBOY";
  int startX = (240 - strlen(title)*18) / 2;
  for(int i=0;i<(int)strlen(title);i++) {
    tone(BUZZER_PIN, 400+i*80, 60);
    screen.setTextColor(screen.color565(0, 180+i*10, 255), TFT_BLACK);
    screen.setCursor(startX+i*18, 20);
    screen.print(title[i]);
    delay(80);
  }

  // Sous-titre
  screen.setTextSize(1);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setCursor(42, 58);
  screen.print("UNE CONSOLE DE JEUX");

  // Bordure animée
  for(int i=0;i<=8;i++) {
    screen.drawRect(i,i, 240-i*2, 78-i*2,
                    screen.color565(0, 255-i*25, 255));
    delay(30);
  }

  // Mélodie
  int notes[]={523,659,784,1047,784,1047};
  int durs[] ={100,100,100,200, 100,300};
  for(int i=0;i<6;i++){ tone(BUZZER_PIN,notes[i],durs[i]); delay(durs[i]+30); }

  // Barre de chargement
  screen.drawRect(10,90,220,14,TFT_DARKGREY);
  for(int i=0;i<=100;i+=2) {
    screen.fillRect(12,92,(i*216)/100,10, screen.color565(0,200,i*2));
    screen.setTextColor(TFT_WHITE,TFT_BLACK);
    screen.setTextSize(1);
    screen.setCursor(100,108); screen.print(i); screen.print("%  ");
    delay(20);
  }

  tone(BUZZER_PIN,1047,100); delay(120);
  tone(BUZZER_PIN,1319,300); delay(350);
  screen.fillScreen(TFT_WHITE); delay(80);
  screen.fillScreen(TFT_BLACK); delay(80);

  consoleState = MENU;
}

void showGameOver() {
  highScore = EEPROM.read(0);
  bool newRecord = false;

  if(currentGame->getScore() > highScore) {
    highScore = currentGame->getScore();
    EEPROM.write(0, highScore);
    EEPROM.commit();
    newRecord = true;
  }
  int lastScore = currentGame->getScore();

  // Flash rouge
  for(int i=0;i<3;i++) {
    screen.fillScreen(TFT_RED); delay(80);
    screen.fillScreen(TFT_BLACK); delay(80);
  }

  // Fond dégradé
  for(int y=0;y<135;y++)
    screen.drawFastHLine(0,y,240,screen.color565(y/2,0,0));

  // Titre ombre
  screen.setTextSize(3);
  screen.setTextColor(screen.color565(180,0,0));
  screen.setCursor(22,17); screen.print("GAME OVER");
  screen.setTextColor(TFT_WHITE);
  screen.setCursor(20,15); screen.print("GAME OVER");

  screen.drawFastHLine(10,48,220,TFT_ORANGE);

  screen.setTextSize(2);
  screen.setTextColor(TFT_WHITE);
  screen.setCursor(15,55); screen.print("SCORE  : ");
  screen.setTextColor(TFT_YELLOW); screen.print(lastScore);

  screen.setTextColor(TFT_WHITE);
  screen.setCursor(15,78); screen.print("RECORD : ");
  screen.setTextColor(newRecord ? TFT_GREEN : TFT_YELLOW);
  screen.print(highScore);

  if(newRecord) {
    screen.setTextSize(1);
    screen.setTextColor(TFT_GREEN);
    screen.setCursor(45,100);
    screen.print("** NOUVEAU RECORD ! **");
    tone(BUZZER_PIN,784,100);  delay(120);
    tone(BUZZER_PIN,1047,100); delay(120);
    tone(BUZZER_PIN,1319,100); delay(120);
    tone(BUZZER_PIN,1568,300); delay(350);
  } else {
    tone(BUZZER_PIN,300,150); delay(180);
    tone(BUZZER_PIN,200,300); delay(350);
  }

  screen.drawFastHLine(10,118,220,TFT_DARKGREY);
  screen.setTextSize(1);
  screen.setTextColor(TFT_CYAN);
  screen.setCursor(8,122);   screen.print("[A] Rejouer");
  screen.setTextColor(TFT_ORANGE);
  screen.setCursor(130,122); screen.print("[B] Menu");
}

void launchGame(int gameId) {
  if(currentGame != nullptr){ delete currentGame; currentGame=nullptr; }
  switch(gameId) {
    case 0: currentGame = new SnakeGame(&screen);         break;
    case 1: currentGame = new MorpionGame(&screen);       break;
    case 2: currentGame = new PongGame(&screen);          break;
    case 3: currentGame = new DinoRunGame(&screen);       break;
    case 4: currentGame = new SpaceInvadersGame(&screen); break;
    case 5: currentGame = new BreakoutGame(&screen);      break;
    default: currentGame = nullptr; break;
  }
  if(currentGame){ currentGame->init(); consoleState=PLAYING; }
}

void setup() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  screen.init();
  screen.setRotation(1);
  screen.fillScreen(TFT_BLACK);

  pinMode(BTN_UP,     INPUT_PULLUP);
  pinMode(BTN_DOWN,   INPUT_PULLUP);
  pinMode(BTN_LEFT,   INPUT_PULLUP);
  pinMode(BTN_RIGHT,  INPUT_PULLUP);
  pinMode(BTN_A,      INPUT_PULLUP);
  pinMode(BTN_B,      INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  EEPROM.begin(16);

  menu.addGame("Snake",          "Mange les pommes!",    "", 0);
  menu.addGame("Morpion",        "2 joueurs - A joue!",  "", 1);
  menu.addGame("Pong",           "Bat l'IA en 5 pts!",   "", 2);
  menu.addGame("Dino Run",       "Saute les obstacles!", "", 3);
  menu.addGame("Space Invaders", "Detruit les aliens!",  "", 4);
  menu.addGame("Breakout",       "Casse les briques!",   "", 5);

  showBoot();
}

void loop() {
  readButtons();
  switch(consoleState) {

    case STARTING: break;

    case MENU:
      menu.update(buttons);
      menu.render();
      if(buttons.aPressed){ currentGameId=menu.getSelectedId(); launchGame(currentGameId); }
      break;

    case PLAYING:
      if(currentGame){
        currentGame->update(buttons);
        currentGame->render();
        if(currentGame->isGameOver()) consoleState=GAMEOVER;
      }
      break;

    case GAMEOVER: {
      static bool drawn=false;
      if(!drawn){ showGameOver(); drawn=true; }
      if(buttons.aPressed){ drawn=false; launchGame(currentGameId); }
      if(buttons.bPressed){ drawn=false; consoleState=MENU; menu.forceRedraw(); }
      break;
    }
  }
  delay(1);
}