#ifndef BREAKOUT_H
#define BREAKOUT_H

#include "Game.h"

#define BRK_BUZZER 17

// ─────────────────────────────────────────────────────────────────────────────
//  BreakoutGame — adapté pour LILYGO TTGO T-Display V1.1
//  Résolution : 240 × 135 px (mode paysage, driver ST7789)
//
//  Système de vitesse progressive :
//    0 – 5 s   → 0.5×  (phase d'apprentissage)
//    5 – 15 s  → 1.0×  (vitesse normale)
//   15 – 30 s  → 1.4×
//   30 – 50 s  → 1.8×
//   50 s+      → 2.2×  (mode enfer)
// ─────────────────────────────────────────────────────────────────────────────

class BreakoutGame : public Game {
private:

  /* ── Raquette ── */
  int paddleX, prevPaddleX;
  static const int PADDLE_Y = 126;   // paddleY fixe, laisse 9 px en bas
  static const int PADDLE_W = 36;
  static const int PADDLE_H = 4;
  static const int PADDLE_SPD = 5;

  /* ── Balle ── */
  float ballX, ballY;
  float prevBX, prevBY;
  float velX, velY;               // direction + magnitude à 1× speed
  static const int BALL_R = 3;
  bool  ballLaunched;

  /* ── Briques ── */
  static const int COLS = 10;
  static const int ROWS = 4;     // 4 rangées (était 5) → libère de la place
  static const int BW   = 22;    // largeur brique
  static const int BH   = 7;     // hauteur brique
  static const int BPX  = 2;     // padding X
  static const int BPY  = 2;     // padding Y
  static const int BSY  = 13;    // Y de départ des briques (sous le HUD)

  struct Brick { bool alive; int hp; };
  Brick bricks[ROWS][COLS];

  int  bricksLeft, lives, wave;
  bool firstDraw;

  /* ── Timing ── */
  unsigned long lastUpdate;
  static const int UPDATE_INTERVAL = 14;  // ms entre frames logiques

  /* ── Vitesse progressive ── */
  unsigned long waveStartTime;   // reset à chaque nouveau wave

  // Renvoie le multiplicateur de vitesse selon le temps écoulé dans le wave
  float speedMult() const {
    unsigned long t = millis() - waveStartTime;
    if (t < 5000UL)  return 0.5f;
    if (t < 15000UL) return 1.0f;
    if (t < 30000UL) return 1.4f;
    if (t < 50000UL) return 1.8f;
    return 2.2f;
  }

  // Libellé affiché dans le HUD selon le palier
  const char* speedLabel() const {
    unsigned long t = millis() - waveStartTime;
    if (t < 5000UL)  return "0.5x";
    if (t < 15000UL) return "1.0x";
    if (t < 30000UL) return "1.4x";
    if (t < 50000UL) return "1.8x";
    return "2.2x";
  }

  // Couleur de l'indicateur de vitesse
  uint16_t speedColor() const {
    unsigned long t = millis() - waveStartTime;
    if (t < 5000UL)  return TFT_GREEN;
    if (t < 15000UL) return TFT_CYAN;
    if (t < 30000UL) return TFT_YELLOW;
    if (t < 50000UL) return TFT_ORANGE;
    return TFT_RED;
  }

  /* ── Couleur des briques ── */
  uint16_t brickColor(int row, int hp) const {
    if (hp == 3) return screen->color565(255, 50, 50);
    if (hp == 2) return TFT_ORANGE;
    const uint16_t cols[] = { TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN };
    return cols[row % 4];
  }

  /* ── Init briques ── */
  void initBricks() {
    bricksLeft = 0;
    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        bricks[r][c].alive = true;
        bricks[r][c].hp    = (r < 2) ? 2 : 1;
        if (wave >= 3 && r == 0) bricks[r][c].hp = 3;
        bricksLeft++;
      }
    }
  }

  /* ── Dessiner / effacer une brique ── */
  void drawBrick(int r, int c, bool erase = false) {
    int bx = c * (BW + BPX) + 2;
    int by = BSY + r * (BH + BPY);
    if (erase) {
      screen->fillRect(bx, by, BW, BH, TFT_BLACK);
    } else {
      screen->fillRect(bx, by, BW, BH, brickColor(r, bricks[r][c].hp));
      screen->drawRect(bx, by, BW, BH, TFT_BLACK);
    }
  }

  /* ── Reset balle ── */
  void resetBall() {
    ballX = paddleX + PADDLE_W / 2.0f;
    ballY = PADDLE_Y - BALL_R - 2;
    prevBX = ballX;  prevBY = ballY;
    velX = (random(0, 2) == 0) ? 2.5f : -2.5f;
    velY = -3.0f;
    ballLaunched = false;
  }

public:
  BreakoutGame(TFT_eSPI* display) : Game(display) {}

  /* ════════════════════════════════ INIT ════════════════════════════════ */
  void init() override {
    paddleX = 102;  prevPaddleX = 102;
    score   = 0;    state = IN_PROGRESS;
    firstDraw = true;
    lastUpdate = 0;
    lives = 3;  wave = 1;
    waveStartTime = millis();
    initBricks();
    resetBall();
  }

  /* ════════════════════════════════ UPDATE ══════════════════════════════ */
  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;

    // Bouton B → quitter
    if (buttons.bPressed) { state = GAME_OVER; return; }

    if (millis() - lastUpdate < UPDATE_INTERVAL) return;
    lastUpdate = millis();

    /* ── Raquette ── */
    prevPaddleX = paddleX;
    if (buttons.left  && paddleX > 0)              paddleX -= PADDLE_SPD;
    if (buttons.right && paddleX < 240 - PADDLE_W) paddleX += PADDLE_SPD;

    /* ── Lancement ── */
    if (!ballLaunched && (buttons.aPressed || buttons.upPressed)) {
      ballLaunched  = true;
      waveStartTime = millis();   // chrono de vitesse repart à 0
      tone(BRK_BUZZER, 660, 60);
    }
    if (!ballLaunched) {
      ballX = paddleX + PADDLE_W / 2.0f;
      return;
    }

    /* ── Mouvement balle avec multiplicateur ── */
    float spd = speedMult();
    prevBX = ballX;  prevBY = ballY;
    ballX += velX * spd;
    ballY += velY * spd;

    /* ── Murs (écran 240×135) ── */
    if (ballX - BALL_R <= 0) {
      ballX = BALL_R;
      velX  = abs(velX);
      tone(BRK_BUZZER, 500, 40);
    }
    if (ballX + BALL_R >= 240) {
      ballX = 240 - BALL_R;
      velX  = -abs(velX);
      tone(BRK_BUZZER, 500, 40);
    }
    if (ballY - BALL_R <= 10) {
      ballY = 10 + BALL_R;
      velY  = abs(velY);
      tone(BRK_BUZZER, 500, 40);
    }

    /* ── Raquette ── */
    if (ballY + BALL_R >= PADDLE_Y &&
        ballY + BALL_R <= PADDLE_Y + PADDLE_H + 4 &&
        ballX >= paddleX - BALL_R &&
        ballX <= paddleX + PADDLE_W + BALL_R)
    {
      float offset = (ballX - (paddleX + PADDLE_W / 2.0f)) / (PADDLE_W / 2.0f);
      velX = offset * 4.0f;
      if (velX > 0 && velX < 0.5f)  velX =  0.5f;
      if (velX < 0 && velX > -0.5f) velX = -0.5f;
      velY = -abs(velY);
      tone(BRK_BUZZER, 880, 50);
    }

    /* ── Balle perdue (bas de l'écran 135px) ── */
    if (ballY > 135) {
      lives--;
      tone(BRK_BUZZER, 200, 400);
      if (lives <= 0) { state = GAME_OVER; return; }
      resetBall();
      firstDraw = true;
      return;
    }

    /* ── Collisions briques ── */
    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        if (!bricks[r][c].alive) continue;
        int bx = c * (BW + BPX) + 2;
        int by = BSY + r * (BH + BPY);
        if (ballX + BALL_R >= bx && ballX - BALL_R <= bx + BW &&
            ballY + BALL_R >= by && ballY - BALL_R <= by + BH)
        {
          bricks[r][c].hp--;
          score += (ROWS - r) * 5 * wave;

          if (bricks[r][c].hp <= 0) {
            bricks[r][c].alive = false;
            bricksLeft--;
            drawBrick(r, c, true);
            tone(BRK_BUZZER, 700, 60);
          } else {
            drawBrick(r, c);          // rafraîchit la couleur (hp baissé)
            tone(BRK_BUZZER, 550, 40);
          }

          float oT = (ballY + BALL_R) - by;
          float oB = (by + BH) - (ballY - BALL_R);
          float oL = (ballX + BALL_R) - bx;
          float oR = (bx + BW) - (ballX - BALL_R);
          if (min(oT, oB) < min(oL, oR)) velY = -velY;
          else                            velX = -velX;
          goto done;
        }
      }
    }
    done:

    /* ── Wave suivant ── */
    if (bricksLeft <= 0) {
      wave++;
      tone(BRK_BUZZER, 1047, 150); delay(200);
      tone(BRK_BUZZER, 1319, 150); delay(200);
      tone(BRK_BUZZER, 1568, 300);
      initBricks();
      firstDraw     = true;
      waveStartTime = millis();   // vitesse repart à 0.5× pour le nouveau wave
    }
  }

  /* ════════════════════════════════ RENDER ══════════════════════════════ */
  void render() override {
    /* ── Premier dessin complet ── */
    if (firstDraw) {
      firstDraw = false;
      screen->fillScreen(TFT_BLACK);
      screen->drawFastHLine(0, 10, 240, TFT_DARKGREY);
      for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
          if (bricks[r][c].alive) drawBrick(r, c);
    }

    /* ── HUD (10 px de hauteur, adapté à 135px) ── */
    screen->fillRect(0, 0, 240, 10, TFT_BLACK);
    screen->setTextColor(TFT_WHITE, TFT_BLACK);
    screen->setTextSize(1);

    // Score
    screen->setCursor(2, 1);
    screen->print("S:"); screen->print(score);

    // Vies (petits carrés rouges)
    screen->setCursor(72, 1);
    screen->print("V:");
    for (int i = 0; i < 3; i++)
      screen->fillRect(84 + i * 9, 1, 7, 7, i < lives ? TFT_RED : TFT_DARKGREY);

    // Wave
    screen->setCursor(113, 1);
    screen->print("W:"); screen->print(wave);

    // Indicateur de vitesse (coloré selon le palier)
    screen->setTextColor(speedColor(), TFT_BLACK);
    screen->setCursor(142, 1);
    screen->print(speedLabel());

    // Bouton quitter
    screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
    screen->setCursor(182, 1);
    screen->print("[B]Quit");

    /* ── Raquette ── */
    if (prevPaddleX != paddleX)
      screen->fillRect(prevPaddleX - 1, PADDLE_Y - 1,
                       PADDLE_W + 3, PADDLE_H + 2, TFT_BLACK);
    screen->fillRoundRect(paddleX, PADDLE_Y, PADDLE_W, PADDLE_H, 2, TFT_CYAN);

    /* ── Balle ── */
    screen->fillRect((int)prevBX - BALL_R - 1, (int)prevBY - BALL_R - 1,
                     BALL_R * 2 + 3, BALL_R * 2 + 3, TFT_BLACK);
    screen->fillCircle((int)ballX, (int)ballY, BALL_R, TFT_WHITE);

    /* ── Message de lancement ── */
    if (!ballLaunched) {
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(55, 108);
      screen->print("[A] Lance la balle!");
    } else {
      screen->fillRect(55, 108, 135, 8, TFT_BLACK);
    }
  }

  virtual ~BreakoutGame() {}
};

#endif // BREAKOUT_H
