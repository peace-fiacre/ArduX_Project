#ifndef SNAKE_H
#define SNAKE_H

#include "Game.h"

// ─────────────────────────────────────────────────────────────────────────────
//  SnakeGame — LILYGO TTGO T-Display V1.1  (240 × 135 px)
//  Basé sur le Snake original — toutes corrections appliquées :
//
//  • snake[200] → snake[SNAKE_MAX=400] + garde-fou (évite crash mémoire)
//  • score = 0 explicite dans init()
//  • nextDir bufferisé (évite demi-tour instantané)
//  • tailGrew : ne pas effacer la queue quand le serpent grandit
//  • spawnApple() sécurisé contre boucle infinie
//  • Vitesse progressive (5 paliers selon score)
//  • Super pomme or : 30% de chance, vaut 3 pts, expire en 7s, clignote
//  • Passage à travers les murs (wrapMode) débloqué à score ≥ 10
//  • Bordure de jeu visible
//  • HUD compact : score, longueur, vitesse, WRAP, [B]Quit
//  • Bouton B → quitter
// ─────────────────────────────────────────────────────────────────────────────

#define SNK_BUZZER  17
#define SNAKE_MAX   400   // FIX: tableau agrandi (était 100, crash mémoire rapide)

enum snakeDirection { SNAKE_UP, SNAKE_DOWN, SNAKE_LEFT, SNAKE_RIGHT };

struct Segment { int x, y; };

class SnakeGame : public Game {
private:

  /* ── Serpent ── */
  Segment snake[SNAKE_MAX];
  int snakeLength;
  snakeDirection direction, nextDir;   // FIX: nextDir bufferisé

  /* ── Pomme normale ── */
  int appleX, appleY;

  /* ── Super pomme (or) ── */
  bool  superAlive;
  int   superX, superY;
  unsigned long superSpawnTime;
  static const unsigned long SUPER_DURATION = 7000UL;  // 7 s

  /* ── État interne ── */
  Segment lastTail;
  bool moved, appleEaten, superEaten;
  bool tailGrew;    // FIX: queue a grandi → ne pas effacer lastTail
  bool firstDraw;

  /* ── Grille ── */
  static const int GS  = 8;    // taille case en pixels
  static const int GW  = 29;   // 240 / 8 = 30, mais on garde 29 pour la bordure
  static const int GH  = 15;   // (135 - 12) / 8 = 15 lignes
  static const int HUD = 12;   // hauteur HUD en pixels

  /* ── Timing ── */
  unsigned long lastMove;

  /* ── Vitesse progressive ── */
  int moveInterval() const {
    if (score >= 20) return 60;
    if (score >= 15) return 80;
    if (score >= 10) return 100;
    if (score >= 5)  return 120;
    return 150;
  }

  const char* speedLabel() const {
    if (score >= 20) return "MAX";
    if (score >= 15) return "4";
    if (score >= 10) return "3";
    if (score >= 5)  return "2";
    return "1";
  }

  uint16_t speedColor() const {
    if (score >= 20) return TFT_RED;
    if (score >= 15) return TFT_ORANGE;
    if (score >= 10) return TFT_YELLOW;
    if (score >= 5)  return TFT_CYAN;
    return TFT_GREEN;
  }

  bool wrapMode() const { return score >= 10; }

  /* ── Coordonnées pixel ── */
  int cellX(int gx) const { return 1 + gx * GS; }
  int cellY(int gy) const { return HUD + 1 + gy * GS; }

  /* ── Spawn pomme sécurisé ── */
  void spawnApple() {
    int tries = 0;
    do {
      appleX = random(0, GW);
      appleY = random(0, GH);
      tries++;
      if (tries > GW * GH) {   // FIX: grille pleine → victoire
        state = GAME_OVER;
        return;
      }
    } while (onSnake(appleX, appleY));
  }

  /* ── Spawn super pomme (30% après chaque pomme normale) ── */
  void trySpawnSuper() {
    if (superAlive) return;
    if (random(0, 10) < 3) {
      int tries = 0;
      do {
        superX = random(0, GW);
        superY = random(0, GH);
        tries++;
        if (tries > GW * GH) return;   // FIX: sécurité boucle infinie
      } while (onSnake(superX, superY) || (superX == appleX && superY == appleY));
      superAlive     = true;
      superSpawnTime = millis();
    }
  }

  bool onSnake(int x, int y) const {
    for (int i = 0; i < snakeLength; i++)
      if (snake[i].x == x && snake[i].y == y) return true;
    return false;
  }

  /* ── HUD compact ── */
  void drawHUD() {
    screen->fillRect(0, 0, 240, HUD, TFT_BLACK);
    screen->setTextSize(1);

    // Score
    screen->setTextColor(TFT_WHITE, TFT_BLACK);
    screen->setCursor(2, 2);
    screen->print("S:"); screen->print(score);

    // Longueur
    screen->setCursor(52, 2);
    screen->print("L:"); screen->print(snakeLength);

    // Wrap mode
    if (wrapMode()) {
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setCursor(96, 2);
      screen->print("WRAP");
    }

    // Vitesse
    screen->setTextColor(speedColor(), TFT_BLACK);
    screen->setCursor(136, 2);
    screen->print("SPD:"); screen->print(speedLabel());

    // Quitter
    screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
    screen->setCursor(185, 2);
    screen->print("[B]Quit");
  }

  void drawBorder() {
    screen->drawRect(0, HUD, GW * GS + 2, GH * GS + 2, TFT_DARKGREY);
  }

  void drawCell(int gx, int gy, uint16_t color) {
    screen->fillRect(cellX(gx), cellY(gy), GS - 1, GS - 1, color);
  }

public:
  SnakeGame(TFT_eSPI* display) : Game(display) {}

  /* ════════════════════════════════ INIT ════════════════════════════════ */
  void init() override {
    snakeLength = 3;
    direction   = SNAKE_RIGHT;
    nextDir     = SNAKE_RIGHT;   // FIX: nextDir sync avec direction au départ
    lastMove    = 0;
    score       = 0;             // FIX: initialisation explicite
    state       = IN_PROGRESS;
    moved       = false;
    appleEaten  = false;
    superEaten  = false;
    tailGrew    = false;
    superAlive  = false;
    firstDraw   = true;

    for (int i = 0; i < snakeLength; i++) {
      snake[i].x = (GW / 2) - i;
      snake[i].y = GH / 2;
    }

    spawnApple();
  }

  /* ════════════════════════════════ UPDATE ══════════════════════════════ */
  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;

    // Bouton B → quitter
    if (buttons.bPressed) { state = GAME_OVER; return; }

    // FIX: nextDir bufferisé → évite le demi-tour si on appuie trop vite
    if (buttons.up    && direction != SNAKE_DOWN)  nextDir = SNAKE_UP;
    if (buttons.down  && direction != SNAKE_UP)    nextDir = SNAKE_DOWN;
    if (buttons.right && direction != SNAKE_LEFT)  nextDir = SNAKE_RIGHT;
    if (buttons.left  && direction != SNAKE_RIGHT) nextDir = SNAKE_LEFT;

    moved      = false;
    appleEaten = false;
    superEaten = false;
    tailGrew   = false;

    // Super pomme : expiration
    if (superAlive && millis() - superSpawnTime > SUPER_DURATION) {
      superAlive = false;
      drawCell(superX, superY, TFT_BLACK);
    }

    // Cadence de déplacement
    if (millis() - lastMove < (unsigned long)moveInterval()) return;
    lastMove  = millis();
    direction = nextDir;   // FIX: appliquer la direction bufferisée

    // Sauvegarder la queue AVANT de décaler
    lastTail = snake[snakeLength - 1];

    // Décaler tous les segments
    for (int i = snakeLength - 1; i > 0; i--)
      snake[i] = snake[i - 1];

    // Bouger la tête
    switch (direction) {
      case SNAKE_UP:    snake[0].y -= 1; break;
      case SNAKE_DOWN:  snake[0].y += 1; break;
      case SNAKE_LEFT:  snake[0].x -= 1; break;
      case SNAKE_RIGHT: snake[0].x += 1; break;
    }

    // Wrap ou collision mur
    if (wrapMode()) {
      if (snake[0].x < 0)   snake[0].x = GW - 1;
      if (snake[0].x >= GW) snake[0].x = 0;
      if (snake[0].y < 0)   snake[0].y = GH - 1;
      if (snake[0].y >= GH) snake[0].y = 0;
    } else {
      if (snake[0].x < 0 || snake[0].x >= GW ||
          snake[0].y < 0 || snake[0].y >= GH) {
        state = GAME_OVER;
        tone(SNK_BUZZER, 200, 500);
        return;
      }
    }

    // Collision avec soi-même
    for (int i = 1; i < snakeLength; i++) {
      if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
        state = GAME_OVER;
        tone(SNK_BUZZER, 200, 500);
        return;
      }
    }

    moved = true;

    // Pomme normale mangée
    if (snake[0].x == appleX && snake[0].y == appleY) {
      if (snakeLength < SNAKE_MAX - 1) {   // FIX: garde-fou tableau
        snakeLength++;
        tailGrew = true;
      }
      score++;
      appleEaten = true;
      tone(SNK_BUZZER, 880, 60);
      spawnApple();
      trySpawnSuper();
    }

    // Super pomme mangée
    if (superAlive && snake[0].x == superX && snake[0].y == superY) {
      int toAdd = min(3, SNAKE_MAX - 1 - snakeLength);   // FIX: garde-fou
      snakeLength += toAdd;
      if (toAdd > 0) tailGrew = true;
      score += 3;
      superAlive = false;
      superEaten = true;
      tone(SNK_BUZZER, 1047, 80);
      // FIX: delay(80) supprimé → provoquait watchdog reset sur ESP32
    }
  }

  /* ════════════════════════════════ RENDER ══════════════════════════════ */
  void render() override {

    /* ── Premier dessin complet ── */
    if (firstDraw) {
      firstDraw = false;
      screen->fillScreen(TFT_BLACK);
      drawBorder();
      drawHUD();
      drawCell(appleX, appleY, TFT_RED);
      for (int i = 0; i < snakeLength; i++)
        drawCell(snake[i].x, snake[i].y, i == 0 ? TFT_WHITE : TFT_GREEN);
      return;
    }

    if (!moved) return;

    /* ── Pomme ou super pomme mangée → redessiner toute la zone ── */
    if (appleEaten || superEaten) {
      screen->fillRect(1, HUD + 1, GW * GS, GH * GS, TFT_BLACK);
      drawBorder();
      for (int i = 0; i < snakeLength; i++)
        drawCell(snake[i].x, snake[i].y, i == 0 ? TFT_WHITE : TFT_GREEN);
      drawCell(appleX, appleY, TFT_RED);
      if (superAlive)
        drawCell(superX, superY, TFT_YELLOW);
      drawHUD();
      return;
    }

    /* ── Dessin différentiel (déplacement normal) ── */

    // FIX: effacer l'ancienne queue SEULEMENT si la queue n'a pas grandi
    if (!tailGrew)
      drawCell(lastTail.x, lastTail.y, TFT_BLACK);

    // Nouvelle tête
    drawCell(snake[0].x, snake[0].y, TFT_WHITE);

    // 2e segment (était blanc → redevient vert)
    if (snakeLength > 1)
      drawCell(snake[1].x, snake[1].y, TFT_GREEN);

    // Super pomme : clignotement toutes les 300 ms
    if (superAlive) {
      unsigned long elapsed = millis() - superSpawnTime;
      bool blink = (elapsed / 300) % 2 == 0;
      drawCell(superX, superY, blink ? TFT_YELLOW : screen->color565(180, 130, 0));
    }

    drawHUD();
  }

  virtual ~SnakeGame() {}
};

#endif // SNAKE_H
