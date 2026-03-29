// ═══════════════════════════════════════════════════════════════
//  Pong.h  —  Version 3.5
//  Optimisé pour LILYGO TTGO T-Display V1.1 (135x240)
//
//  CORRECTIONS v3.5 :
//    ✓ BUG 1 — Boutons : up/down continu (mouvement fluide)
//    ✓ BUG 2 — Balle : accumulateurs float ballXf/ballYf
//    ✓ BUG 3 — BUZZER_PIN 32 → 17 (même GPIO que le .ino)
//    ✓ Bip court raquette joueur (520Hz) et IA (380Hz)
//    ✓ Son game over géré dans le .ino (showGameOver)
// ═══════════════════════════════════════════════════════════════

#ifndef PONG_H
#define PONG_H

#include "Game.h"

// ── Broche buzzer — DOIT correspondre au .ino ─────────────────
#ifndef BUZZER_PIN
  #define BUZZER_PIN 17
#endif

// ── Couleurs thème NEON (RGB565) ──────────────────────────────
#define NEON_CYAN    0x07FF
#define NEON_PINK    0xF81F
#define NEON_YELLOW  0xFFE0
#define DIM_GREY     0x4208
#define DARK_BLUE    0x0010

// ── Dimensions écran T-Display ────────────────────────────────
#define SCREEN_W  240
#define SCREEN_H  135
#define HUD_H      20

// ── Notes (Hz) ───────────────────────────────────────────────
#define NOTE_DO      262
#define NOTE_RE      294
#define NOTE_MI      330
#define NOTE_FA      349
#define NOTE_SOL     392
#define NOTE_LA      440
#define NOTE_SI      494
#define NOTE_DO_HIGH 523

// ── Vitesse Y minimale ────────────────────────────────────────
#define MIN_VELY 0.6f

// ═════════════════════════════════════════════════════════════
//  SpeedRamp
// ═════════════════════════════════════════════════════════════
class SpeedRamp {
private:
  float startMultiplier;
  float currentMultiplier;
  float targetMultiplier;
  unsigned long rampStartTime;
  unsigned long rampDuration;
  bool isRamping;

public:
  SpeedRamp() : startMultiplier(0.5f), currentMultiplier(0.5f),
                targetMultiplier(1.0f), rampStartTime(0),
                rampDuration(5000), isRamping(false) {}

  void startRamp(float from, float to, unsigned long duration) {
    startMultiplier   = from;
    currentMultiplier = from;
    targetMultiplier  = to;
    rampDuration      = duration;
    rampStartTime     = millis();
    isRamping         = true;
  }

  float getMultiplier() {
    if (!isRamping) return currentMultiplier;
    unsigned long elapsed = millis() - rampStartTime;
    if (elapsed >= rampDuration) {
      currentMultiplier = targetMultiplier;
      isRamping = false;
      return currentMultiplier;
    }
    float t = (float)elapsed / (float)rampDuration;
    return startMultiplier + (targetMultiplier - startMultiplier) * t;
  }

  void setMultiplier(float mult) {
    startMultiplier   = mult;
    currentMultiplier = mult;
    targetMultiplier  = mult;
    isRamping         = false;
  }

  float getFinalMultiplier() { return targetMultiplier; }
  bool isRampingUp() { return isRamping && targetMultiplier > startMultiplier; }
};

// ═════════════════════════════════════════════════════════════
//  Sons non-bloquants simples
// ═════════════════════════════════════════════════════════════
inline void soundWallBounce()    { tone(BUZZER_PIN, 320,          40);  }
inline void soundPlayerPoint()   { tone(BUZZER_PIN, NOTE_SOL,    150);  }
inline void soundAIPoint()       { tone(BUZZER_PIN, NOTE_RE,     150);  }
inline void soundVictory()       { tone(BUZZER_PIN, NOTE_DO_HIGH, 300); }
inline void soundSpeedIncrease() { tone(BUZZER_PIN, NOTE_FA,      50);  }

// ── Particules ───────────────────────────────────────────────
struct Particle {
  int      x, y;
  float    vx, vy;
  uint8_t  life;
  uint16_t color;
  bool     active;
};
#define MAX_PARTICLES 8

// ═════════════════════════════════════════════════════════════
//  PongGame
// ═════════════════════════════════════════════════════════════
class PongGame : public Game {
  private:

    // ── Raquette joueur (gauche) ─────────────────────────────
    int paddleY;
    const int paddleX   = 8;
    const int paddleH   = 26;
    const int paddleW   = 4;
    const int paddleSpd = 4;

    // ── Raquette IA (droite) ─────────────────────────────────
    int aiY;
    const int aiX = SCREEN_W - 12;
    int aiSpd;

    // ── Balle ────────────────────────────────────────────────
    int   ballX, ballY;
    float ballXf, ballYf;  // FIX BUG 2 : accumulateurs flottants
    float velX, velY;
    const int ballSize = 3;

    // ── Scores ───────────────────────────────────────────────
    int playerScore, aiScore;
    const int WIN_SCORE = 5;

    // ── Accélération ─────────────────────────────────────────
    SpeedRamp speedRamp;
    unsigned long lastSpeedRampCheck;
    bool speedRampActive;

    // ── Particules ───────────────────────────────────────────
    Particle particles[MAX_PARTICLES];

    // ── hitFlash ─────────────────────────────────────────────
    bool hitFlash;
    int  hitFlashTimer;

    // ── Annonce de point ─────────────────────────────────────
    bool  pointAnnounce;
    int   pointAnnounceTimer;
    bool  playerScored;

    // ── Rendu différentiel ───────────────────────────────────
    int  prevBallX, prevBallY;
    int  prevPaddleY, prevAiY;
    bool firstDraw;
    bool scoreChanged;

    // ── Timing ───────────────────────────────────────────────
    unsigned long lastUpdate;
    const int updateInterval = 16;  // ~60 fps

    // ─────────────────────────────────────────────────────────
    //  Bips raquette
    //  Joueur : 520 Hz — IA : 380 Hz (distincts à l'oreille)
    // ─────────────────────────────────────────────────────────
    void soundPaddlePlayer() { tone(BUZZER_PIN, 520, 30); }
    void soundPaddleAI()     { tone(BUZZER_PIN, 380, 30); }

    // ─────────────────────────────────────────────────────────
    //  Helpers velY
    // ─────────────────────────────────────────────────────────
    float clampVelY(float vy) {
      if (vy >= 0 && vy < MIN_VELY) return MIN_VELY;
      if (vy <  0 && vy > -MIN_VELY) return -MIN_VELY;
      return vy;
    }

    // ─────────────────────────────────────────────────────────
    //  Particules
    // ─────────────────────────────────────────────────────────
    void spawnOne(int x, int y, uint16_t color) {
      for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
          float angle = random(0, 628) / 100.0f;
          float speed = 0.8f + random(0, 15) / 10.0f;
          particles[i] = { x, y,
                           cos(angle)*speed, sin(angle)*speed,
                           (uint8_t)(6 + random(0,4)),
                           color, true };
          return;
        }
      }
    }

    void spawnBurst(int x, int y, uint16_t color) {
      for (int k = 0; k < 4; k++) spawnOne(x, y, color);
    }

    void updateParticles() {
      for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        screen->fillRect(particles[i].x, particles[i].y, 1, 1, TFT_BLACK);
        particles[i].x  += particles[i].vx;
        particles[i].y  += particles[i].vy;
        particles[i].vy += 0.08f;
        particles[i].life--;
        if (particles[i].life == 0) { particles[i].active = false; continue; }
        if (particles[i].x > 0 && particles[i].x < SCREEN_W &&
            particles[i].y > HUD_H && particles[i].y < SCREEN_H) {
          screen->drawPixel(particles[i].x, particles[i].y, particles[i].color);
        }
      }
    }

    // ─────────────────────────────────────────────────────────
    //  Reset balle
    // ─────────────────────────────────────────────────────────
    void resetBall() {
      ballX  = SCREEN_W / 2;
      ballY  = HUD_H + (SCREEN_H - HUD_H) / 2;
      ballXf = (float)ballX;
      ballYf = (float)ballY;

      float multiplier = speedRamp.getFinalMultiplier();
      float baseSpeed  = 1.8f * multiplier;
      velX = (random(0, 2) == 0) ? baseSpeed : -baseSpeed;
      float vy = 0.9f + random(0, 10) / 20.0f;
      velY = clampVelY((random(0, 2) == 0) ? vy : -vy);
    }

    // ─────────────────────────────────────────────────────────
    //  IA adaptative
    // ─────────────────────────────────────────────────────────
    void updateAI() {
      int target = ballY + (int)(velY * 2.5f);
      target = constrain(target, HUD_H + paddleH/2, SCREEN_H - paddleH/2);
      if (abs(target - aiY) > 3) {
        if (target > aiY + 2) aiY += aiSpd;
        else if (target < aiY - 2) aiY -= aiSpd;
      }
      aiY = constrain(aiY, HUD_H + paddleH/2, SCREEN_H - paddleH/2);
    }

    // ─────────────────────────────────────────────────────────
    //  Dessins
    // ─────────────────────────────────────────────────────────
    void drawCenterLine() {
      for (int y = HUD_H; y < SCREEN_H; y += 6)
        screen->fillRect(SCREEN_W/2 - 1, y, 2, 3, DIM_GREY);
    }

    void drawHUD() {
      screen->fillRect(0, 0, SCREEN_W, HUD_H, DARK_BLUE);
      screen->drawFastHLine(0, HUD_H - 1, SCREEN_W, NEON_CYAN);
      screen->setTextColor(DIM_GREY, DARK_BLUE);
      screen->setTextSize(1);
      screen->setCursor(SCREEN_W/2 - 14, 4);
      screen->print("PONG");
      screen->setTextSize(2);
      screen->setTextColor(NEON_CYAN, DARK_BLUE);
      screen->setCursor(70, 3);
      screen->print(playerScore);
      screen->setTextColor(DIM_GREY, DARK_BLUE);
      screen->setCursor(SCREEN_W/2 - 6, 3);
      screen->print(":");
      screen->setTextColor(NEON_PINK, DARK_BLUE);
      screen->setCursor(148, 3);
      screen->print(aiScore);
      if (speedRamp.isRampingUp()) {
        screen->setTextSize(1);
        screen->setTextColor(NEON_YELLOW, DARK_BLUE);
        screen->setCursor(SCREEN_W - 35, 6);
        screen->print("SPD+");
      }
    }

    void drawPointAnnounce() {
      uint16_t col = playerScored ? NEON_CYAN : NEON_PINK;
      int boxX = 40, boxY = 40, boxW = 160, boxH = 45;
      screen->fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
      screen->drawRect(boxX, boxY, boxW, boxH, col);
      screen->setTextSize(2);
      screen->setTextColor(col, TFT_BLACK);
      screen->setCursor(playerScored ? 62 : 72, 50);
      screen->print(playerScored ? "POINT!" : "OUCH!");
    }

    void drawPaddle(int x, int y, uint16_t color) {
      int startY = y - paddleH/2;
      screen->fillRect(x, startY, paddleW, paddleH, color);
      screen->drawFastVLine(x+1, startY+2, paddleH-4, TFT_WHITE);
    }

    void erasePaddle(int x, int y) {
      screen->fillRect(x, y - paddleH/2, paddleW + 1, paddleH + 1, TFT_BLACK);
    }

    void drawBall(int x, int y, uint16_t color = NEON_YELLOW) {
      screen->fillRect(x, y, ballSize, ballSize, color);
    }

    void eraseBall(int x, int y) {
      screen->fillRect(x-1, y-1, ballSize+2, ballSize+2, TFT_BLACK);
    }

  // ══════════════════════════════════════════════════════════
  public:

    PongGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      paddleY            = HUD_H + (SCREEN_H - HUD_H) / 2;
      aiY                = HUD_H + (SCREEN_H - HUD_H) / 2;
      aiSpd              = 2;
      playerScore        = 0;
      aiScore            = 0;
      score              = 0;
      state              = IN_PROGRESS;
      firstDraw          = true;
      scoreChanged       = false;
      hitFlash           = false;
      hitFlashTimer      = 0;
      pointAnnounce      = false;
      pointAnnounceTimer = 0;
      lastUpdate         = 0;
      speedRampActive    = true;
      lastSpeedRampCheck = 0;
      speedRamp.startRamp(0.5f, 1.0f, 5000);
      for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
      resetBall();
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;
      if (millis() - lastUpdate < updateInterval) return;
      lastUpdate = millis();

      if (pointAnnounce) {
        pointAnnounceTimer--;
        if (pointAnnounceTimer <= 0) {
          pointAnnounce = false;
          firstDraw     = true;
        }
        return;
      }

      prevBallX    = ballX;
      prevBallY    = ballY;
      prevPaddleY  = paddleY;
      prevAiY      = aiY;
      scoreChanged = false;

      // FIX BUG 1 : état continu → mouvement fluide
      if (buttons.up)   paddleY -= paddleSpd;
      if (buttons.down) paddleY += paddleSpd;
      paddleY = constrain(paddleY, HUD_H + paddleH/2, SCREEN_H - paddleH/2);

      updateAI();

      // FIX BUG 2 : accumulateurs flottants → balle diagonale
      ballXf += velX;
      ballYf += velY;
      ballX = (int)ballXf;
      ballY = (int)ballYf;

      // ── Rebond mur haut ───────────────────────────────────
      if (ballY <= HUD_H) {
        ballY  = HUD_H + 1;
        ballYf = (float)ballY;
        velY   = clampVelY(abs(velY));
        spawnBurst(ballX, ballY, NEON_YELLOW);
        soundWallBounce();
      }

      // ── Rebond mur bas ────────────────────────────────────
      if (ballY >= SCREEN_H - ballSize) {
        ballY  = SCREEN_H - ballSize - 1;
        ballYf = (float)ballY;
        velY   = clampVelY(-abs(velY));
        spawnBurst(ballX, ballY, NEON_YELLOW);
        soundWallBounce();
      }

      // ── Collision raquette joueur ─────────────────────────
      if (ballX <= paddleX + paddleW &&
          ballX >= paddleX - 1 &&
          ballY + ballSize >= paddleY - paddleH/2 &&
          ballY <= paddleY + paddleH/2) {

        float currentSpeed = sqrt(velX*velX + velY*velY);
        float maxSpeed = 4.5f;
        velX = (currentSpeed < maxSpeed) ? min(abs(velX) * 1.08f, maxSpeed) : maxSpeed;

        velY = clampVelY((ballY + ballSize/2 - paddleY) * 0.22f);
        if (velY >  3.5f) velY =  3.5f;
        if (velY < -3.5f) velY = -3.5f;
        velY = clampVelY(velY);

        ballX  = paddleX + paddleW + 1;
        ballXf = (float)ballX;
        spawnBurst(ballX, ballY + ballSize/2, NEON_CYAN);
        soundPaddlePlayer();
        hitFlash      = true;
        hitFlashTimer = 3;
        aiSpd = constrain(aiSpd, 2, 4);
      }

      // ── Collision raquette IA ─────────────────────────────
      if (ballX + ballSize >= aiX &&
          ballX + ballSize <= aiX + paddleW + 1 &&
          ballY + ballSize >= aiY - paddleH/2 &&
          ballY <= aiY + paddleH/2) {

        float currentSpeed = sqrt(velX*velX + velY*velY);
        float maxSpeed = 4.5f;
        velX = (currentSpeed < maxSpeed) ? -min(abs(velX) * 1.08f, maxSpeed) : -maxSpeed;

        velY = clampVelY((ballY + ballSize/2 - aiY) * 0.22f);
        if (velY >  3.5f) velY =  3.5f;
        if (velY < -3.5f) velY = -3.5f;
        velY = clampVelY(velY);

        ballX  = aiX - ballSize - 1;
        ballXf = (float)ballX;
        spawnBurst(ballX + ballSize, ballY + ballSize/2, NEON_PINK);
        soundPaddleAI();
      }

      // ── L'IA marque ───────────────────────────────────────
      if (ballX < -10) {
        aiScore++;
        scoreChanged  = true;
        playerScored  = false;
        pointAnnounce = true;
        pointAnnounceTimer = 65;
        soundAIPoint();
        if (aiScore >= WIN_SCORE) {
          state = GAME_OVER;
          score = playerScore;
          return;
        }
        resetBall();
      }

      // ── Le joueur marque ──────────────────────────────────
      if (ballX > SCREEN_W + 10) {
        playerScore++;
        score         = playerScore;
        scoreChanged  = true;
        playerScored  = true;
        pointAnnounce = true;
        pointAnnounceTimer = 65;
        soundPlayerPoint();
        if (playerScore > aiScore) aiSpd = min(aiSpd + 1, 5);
        if (playerScore >= WIN_SCORE) {
          state = GAME_OVER;
          return;
        }
        resetBall();
      }

      if (hitFlash && --hitFlashTimer <= 0) hitFlash = false;
    }

    void render() override {
      if (pointAnnounce) { drawPointAnnounce(); return; }

      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);
        drawCenterLine();
        drawHUD();
        drawPaddle(paddleX, paddleY, NEON_CYAN);
        drawPaddle(aiX, aiY, NEON_PINK);
        drawBall(ballX, ballY);
        return;
      }

      eraseBall(prevBallX, prevBallY);
      erasePaddle(paddleX, prevPaddleY);
      erasePaddle(aiX, prevAiY);
      drawPaddle(paddleX, paddleY, NEON_CYAN);
      drawPaddle(aiX, aiY, NEON_PINK);
      drawBall(ballX, ballY, hitFlash ? TFT_WHITE : NEON_YELLOW);
      updateParticles();
      if (scoreChanged) drawHUD();
    }

    virtual ~PongGame() {}
};

#endif
// ═══════════════════════════════════════════════════════════════
//  Pong.h v3.5
//    ✓ BUZZER_PIN 17 (corrigé)
//    ✓ buttons.up / buttons.down (mouvement fluide)
//    ✓ accumulateurs ballXf/ballYf (balle diagonale)
//    ✓ Bip raquette joueur 520Hz / IA 380Hz
//    ✓ Son game over dans showGameOver() du .ino
// ═══════════════════════════════════════════════════════════════
