#ifndef SNAKE_H
#define SNAKE_H

#include "Game.h"

// ─────────────────────────────────────────────────────────────────────────────
//  SnakeGame — adapté pour LILYGO TTGO T-Display V1.1  (240 × 135 px)
//
//  Nouvelles features :
//    • Vitesse progressive selon le score (5 paliers)
//    • Super pomme (or) : apparaît aléatoirement, vaut 3 pts, expire en 7 s
//    • Passage à travers les murs débloqué dès score ≥ 10
//    • Bordure de jeu visible
//    • Bouton B → quitter (comme Breakout)
//    • HUD compact avec indicateur de vitesse coloré
// ─────────────────────────────────────────────────────────────────────────────

#define SNK_BUZZER 17

enum snakeDirection { SNAKE_UP, SNAKE_DOWN, SNAKE_LEFT, SNAKE_RIGHT };

struct Segment { int x, y; };

class SnakeGame : public Game {
private:

  /* ── Serpent ── */
  Segment snake[200];
  int snakeLength;
  snakeDirection direction, nextDir;

  /* ── Pomme normale ── */
  int appleX, appleY;

  /* ── Super pomme (or) ── */
  bool  superAlive;
  int   superX, superY;
  unsigned long superSpawnTime;
  static const unsigned long SUPER_DURATION = 7000UL;   // 7 s

  /* ── État interne ── */
  Segment lastTail;
  bool moved, appleEaten, superEaten;
  bool firstDraw;

  /* ── Grille ── */
  static const int GS  = 8;         // taille d'une case (pixels)
  static const int GW  = 29;        // 240 / 8  → 29 colonnes (232 px utilisés)
  static const int GH  = 15;        // (135 - 12) / 8 → 15 lignes (zone de jeu)
  static const int HUD = 12;        // hauteur du HUD en pixels
  // Zone de jeu : y de HUD à HUD + GH*GS = 12 + 120 = 132 (laisse 3 px en bas)

  /* ── Timing / vitesse ── */
  unsigned long lastMove;

  // Intervalle de déplacement selon le score (ms)
  int moveInterval() const {
    if (score >= 20) return 60;
    if (score >= 15) return 80;
    if (score >= 10) return 100;
    if (score >= 5)  return 120;
    return 150;
  }

  // Label HUD pour la vitesse
  const char* speedLabel() const {
    if (score >= 20) return "MAX";
    if (score >= 15) return "4";
    if (score >= 10) return "3";
    if (score >= 5)  return "2";
    return "1";
  }

  // Couleur HUD vitesse
  uint16_t speedColor() const {
    if (score >= 20) return TFT_RED;
    if (score >= 15) return TFT_ORANGE;
    if (score >= 10) return TFT_YELLOW;
    if (score >= 5)  return TFT_CYAN;
    return TFT_GREEN;
  }

  // Passage à travers les murs débloqué ?
  bool wrapMode() const { return score >= 10; }

  /* ── Coordonnées pixel d'une case ── */
  int cellX(int gx) const { return 1 + gx * GS; }           // +1 pour la bordure
  int cellY(int gy) const { return HUD + 1 + gy * GS; }     // +1 pour la bordure

  /* ── Spawn pomme (évite le corps du serpent) ── */
  void spawnApple() {
    do {
      appleX = random(0, GW);
      appleY = random(0, GH);
    } while (onSnake(appleX, appleY));
  }

  /* ── Spawn super pomme (10 % de chance après chaque pomme normale) ── */
  void trySpawnSuper() {
    if (superAlive) return;
    if (random(0, 10) < 3) {           // 30 % de chance
      do {
        superX = random(0, GW);
        superY = random(0, GH);
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

  /* ── Dessiner le HUD ── */
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

  /* ── Dessiner la bordure de jeu ── */
  void drawBorder() {
    screen->drawRect(0, HUD, GW * GS + 2, GH * GS + 2, TFT_DARKGREY);
  }

  /* ── Dessiner une case ── */
  void drawCell(int gx, int gy, uint16_t color) {
    screen->fillRect(cellX(gx), cellY(gy), GS - 1, GS - 1, color);
  }

public:
  SnakeGame(TFT_eSPI* display) : Game(display) {}

  /* ════════════════════════════════ INIT ════════════════════════════════ */
  void init() override {
    snakeLength = 3;
    direction   = SNAKE_RIGHT;
    nextDir     = SNAKE_RIGHT;
    lastMove    = 0;
    score       = 0;
    state       = IN_PROGRESS;
    moved       = false;
    appleEaten  = false;
    superEaten  = false;
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

    // Changement de direction (buffered pour éviter les demi-tours)
    if (buttons.up    && direction != SNAKE_DOWN)  nextDir = SNAKE_UP;
    if (buttons.down  && direction != SNAKE_UP)    nextDir = SNAKE_DOWN;
    if (buttons.right && direction != SNAKE_LEFT)  nextDir = SNAKE_RIGHT;
    if (buttons.left  && direction != SNAKE_RIGHT) nextDir = SNAKE_LEFT;

    moved      = false;
    appleEaten = false;
    superEaten = false;

    // Super pomme : expiration
    if (superAlive && millis() - superSpawnTime > SUPER_DURATION) {
      superAlive = false;
      // Effacer visuellement (sera repris dans render si firstDraw pas set)
      drawCell(superX, superY, TFT_BLACK);
    }

    // Déplacement cadencé
    if (millis() - lastMove < (unsigned long)moveInterval()) return;
    lastMove  = millis();
    direction = nextDir;

    // Sauvegarder la queue
    lastTail = snake[snakeLength - 1];

    // Décaler les segments
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

    // Pomme normale
    if (snake[0].x == appleX && snake[0].y == appleY) {
      snakeLength++;
      score++;
      appleEaten = true;
      tone(SNK_BUZZER, 880, 60);
      spawnApple();
      trySpawnSuper();
    }

    // Super pomme
    if (superAlive && snake[0].x == superX && snake[0].y == superY) {
      snakeLength += 3;
      score += 3;
      superAlive = false;
      superEaten = true;
      tone(SNK_BUZZER, 1047, 80); delay(80);
      tone(SNK_BUZZER, 1319, 80);
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

    /* ── Dessin différentiel ── */
    if (!appleEaten && !superEaten) {
      // Effacer uniquement l'ancienne queue
      drawCell(lastTail.x, lastTail.y, TFT_BLACK);
    } else {
      // Pomme mangée → redessiner toute la zone de jeu (rare, acceptable)
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

    // Nouvelle tête
    drawCell(snake[0].x, snake[0].y, TFT_WHITE);

    // 2e segment (était blanc → devient vert)
    if (snakeLength > 1)
      drawCell(snake[1].x, snake[1].y, TFT_GREEN);

    // Super pomme : clignotement (toutes les 500 ms)
    if (superAlive) {
      unsigned long elapsed = millis() - superSpawnTime;
      bool blink = (elapsed / 300) % 2 == 0;
      drawCell(superX, superY, blink ? TFT_YELLOW : screen->color565(180, 130, 0));
    }

    // HUD (score peut changer)
    drawHUD();
  }

  virtual ~SnakeGame() {}
};

#endif // SNAKE_H
