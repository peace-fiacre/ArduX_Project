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
#include "Memory.h"

#define BTN_UP     25
#define BTN_DOWN   26
#define BTN_LEFT   27
#define BTN_RIGHT  32
#define BTN_A      13
#define BTN_B      15
#define BUZZER_PIN 17
#define TFT_BL      4

// FIX: adresses EEPROM pour stocker le highscore sur 2 octets (max 65535)
#define EEPROM_ADDR_HI 0
#define EEPROM_ADDR_LO 1
#define EEPROM_SIZE    16

Buttons buttons;
enum stateConsole { STARTING, MENU, PLAYING, GAMEOVER };
int currentGameId     = -1;
TFT_eSPI screen       = TFT_eSPI();
Menu menu(&screen);
stateConsole consoleState = STARTING;
Game* currentGame     = nullptr;
int highScore         = 0;

// FIX: flag global (pas static local) pour l'écran Game Over
bool gameOverDrawn = false;

// ─────────────────────────────────────────────────────────────
//  readButtons
// ─────────────────────────────────────────────────────────────
void readButtons() {
  bool pU = buttons.up,  pD = buttons.down,
       pL = buttons.left, pR = buttons.right,
       pA = buttons.a,    pB = buttons.b;

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

// ─────────────────────────────────────────────────────────────
//  EEPROM helpers — highscore sur 2 octets (0-65535)
// ─────────────────────────────────────────────────────────────
int readHighScore() {
  return (EEPROM.read(EEPROM_ADDR_HI) << 8) | EEPROM.read(EEPROM_ADDR_LO);
}

void writeHighScore(int s) {
  EEPROM.write(EEPROM_ADDR_HI, (s >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_LO,  s       & 0xFF);
  EEPROM.commit();
}

// ─────────────────────────────────────────────────────────────
//  showBoot — séquence de démarrage
//  FIX: delay() remplacés par des boucles avec yield() pour
//       nourrir le watchdog ESP32 (évite reboot pendant le boot)
// ─────────────────────────────────────────────────────────────
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    yield();          // nourrit le watchdog ESP32
  }
}

void showBoot() {
  screen.fillScreen(TFT_BLACK);

  // Étoiles
  for (int i = 0; i < 40; i++)
    screen.drawPixel(random(0, 240), random(0, 135), TFT_WHITE);

  // Titre lettre par lettre
  screen.setTextSize(3);
  const char* title = "ARDUBOY";
  int startX = (240 - strlen(title) * 18) / 2;
  for (int i = 0; i < (int)strlen(title); i++) {
    tone(BUZZER_PIN, 400 + i * 80, 60);
    screen.setTextColor(screen.color565(0, 180 + i * 10, 255), TFT_BLACK);
    screen.setCursor(startX + i * 18, 20);
    screen.print(title[i]);
    safeDelay(80);    // FIX: yield() intégré
  }

  // Sous-titre
  screen.setTextSize(1);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setCursor(42, 58);
  screen.print("UNE CONSOLE DE JEUX");

  // Bordure animée
  for (int i = 0; i <= 8; i++) {
    screen.drawRect(i, i, 240 - i * 2, 78 - i * 2,
                    screen.color565(0, 255 - i * 25, 255));
    safeDelay(30);    // FIX
  }

  // Mélodie
  int notes[] = {523, 659, 784, 1047, 784, 1047};
  int durs[]  = {100, 100, 100,  200, 100,  300};
  for (int i = 0; i < 6; i++) {
    tone(BUZZER_PIN, notes[i], durs[i]);
    safeDelay(durs[i] + 30);   // FIX
  }

  // Barre de chargement
  screen.drawRect(10, 90, 220, 14, TFT_DARKGREY);
  for (int i = 0; i <= 100; i += 2) {
    screen.fillRect(12, 92, (i * 216) / 100, 10, screen.color565(0, 200, i * 2));
    screen.setTextColor(TFT_WHITE, TFT_BLACK);
    screen.setTextSize(1);
    screen.setCursor(100, 108); screen.print(i); screen.print("%  ");
    safeDelay(20);   // FIX
  }

  tone(BUZZER_PIN, 1047, 100); safeDelay(120);
  tone(BUZZER_PIN, 1319, 300); safeDelay(350);
  screen.fillScreen(TFT_WHITE); safeDelay(80);
  screen.fillScreen(TFT_BLACK); safeDelay(80);

  consoleState = MENU;
}

// ─────────────────────────────────────────────────────────────
//  showGameOver
// ─────────────────────────────────────────────────────────────
void showGameOver() {
  highScore = readHighScore();   // FIX: lecture 2 octets
  bool newRecord = false;

  if (currentGame->getScore() > highScore) {
    highScore = currentGame->getScore();
    writeHighScore(highScore);   // FIX: écriture 2 octets
    newRecord = true;
  }
  int lastScore = currentGame->getScore();

  // Flash rouge
  for (int i = 0; i < 3; i++) {
    screen.fillScreen(TFT_RED);   safeDelay(80);
    screen.fillScreen(TFT_BLACK); safeDelay(80);
  }

  // Fond dégradé
  for (int y = 0; y < 135; y++)
    screen.drawFastHLine(0, y, 240, screen.color565(y / 2, 0, 0));

  // Titre ombre
  screen.setTextSize(3);
  screen.setTextColor(screen.color565(180, 0, 0));
  screen.setCursor(22, 17); screen.print("GAME OVER");
  screen.setTextColor(TFT_WHITE);
  screen.setCursor(20, 15); screen.print("GAME OVER");

  screen.drawFastHLine(10, 48, 220, TFT_ORANGE);

  screen.setTextSize(2);
  screen.setTextColor(TFT_WHITE);
  screen.setCursor(15, 55); screen.print("SCORE  : ");
  screen.setTextColor(TFT_YELLOW); screen.print(lastScore);

  screen.setTextColor(TFT_WHITE);
  screen.setCursor(15, 78); screen.print("RECORD : ");
  screen.setTextColor(newRecord ? TFT_GREEN : TFT_YELLOW);
  screen.print(highScore);

  if (newRecord) {
    screen.setTextSize(1);
    screen.setTextColor(TFT_GREEN);
    screen.setCursor(45, 100);
    screen.print("** NOUVEAU RECORD ! **");
    tone(BUZZER_PIN, 784,  100); safeDelay(120);
    tone(BUZZER_PIN, 1047, 100); safeDelay(120);
    tone(BUZZER_PIN, 1319, 100); safeDelay(120);
    tone(BUZZER_PIN, 1568, 300); safeDelay(350);
  } else {
    tone(BUZZER_PIN, 392, 200); safeDelay(250);
    tone(BUZZER_PIN, 349, 200); safeDelay(250);
    tone(BUZZER_PIN, 330, 200); safeDelay(250);
    tone(BUZZER_PIN, 262, 500); safeDelay(550);
  }

  screen.drawFastHLine(10, 118, 220, TFT_DARKGREY);
  screen.setTextSize(1);
  screen.setTextColor(TFT_CYAN);
  screen.setCursor(8, 122);   screen.print("[A] Rejouer");
  screen.setTextColor(TFT_ORANGE);
  screen.setCursor(130, 122); screen.print("[B] Menu");
}

// ─────────────────────────────────────────────────────────────
//  launchGame
// ─────────────────────────────────────────────────────────────
void launchGame(int gameId) {
  if (currentGame != nullptr) { delete currentGame; currentGame = nullptr; }
  switch (gameId) {
    case 0: currentGame = new SnakeGame(&screen);         break;
    case 1: currentGame = new MorpionGame(&screen);       break;
    case 2: currentGame = new PongGame(&screen);          break;
    case 3: currentGame = new DinoRunGame(&screen);       break;
    case 4: currentGame = new SpaceInvadersGame(&screen); break;
    case 5: currentGame = new BreakoutGame(&screen);      break;
    case 6: currentGame = new MemoryGame(&screen);        break;
    default: currentGame = nullptr; break;
  }
  if (currentGame) {
    currentGame->init();
    gameOverDrawn = false;    // FIX: reset du flag à chaque nouveau jeu
    consoleState  = PLAYING;
  }
}

// ─────────────────────────────────────────────────────────────
//  setup
// ─────────────────────────────────────────────────────────────
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

  EEPROM.begin(EEPROM_SIZE);

  menu.addGame("Snake",          0, icon_snake,    TFT_GREEN);
  menu.addGame("Morpion",        1, icon_morpion,  TFT_CYAN);
  menu.addGame("Pong",           2, icon_pong,     TFT_WHITE);
  menu.addGame("Dino Run",       3, icon_dino,     TFT_YELLOW);
  menu.addGame("Space Invaders", 4, icon_space,    TFT_RED);
  menu.addGame("Breakout",       5, icon_breakout, TFT_ORANGE);
  // FIX: icon_memory au lieu de icon_breakout
  menu.addGame("Memory",         6, icon_memory,   TFT_BLUE);
  
  showBoot();
}

// ─────────────────────────────────────────────────────────────
//  loop
// ─────────────────────────────────────────────────────────────
void loop() {
  readButtons();

  switch (consoleState) {

    case STARTING: break;

    case MENU:
      menu.update(buttons);
      menu.render();
      if (buttons.aPressed) {
        currentGameId = menu.getSelectedId();
        launchGame(currentGameId);
      }
      break;

    case PLAYING:
      if (currentGame) {
        currentGame->update(buttons);
        currentGame->render();
        if (currentGame->isGameOver()) consoleState = GAMEOVER;
      }
      break;

    case GAMEOVER:
      // FIX: flag global au lieu de static local
      if (!gameOverDrawn) { showGameOver(); gameOverDrawn = true; }
      if (buttons.aPressed) { launchGame(currentGameId); }          // reset dans launchGame
      if (buttons.bPressed) { gameOverDrawn = false; consoleState = MENU; menu.forceRedraw(); }
      break;
  }

  yield();       // FIX: yield() au lieu de delay(1) — nourrit le watchdog sans latence fixe
}
