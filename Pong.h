#ifndef PONG_H
#define PONG_H

#include "Game.h"

#define PONG_BUZZER 17

class PongGame : public Game {
  private:

    // Raquette joueur
    int paddleY;
    const int paddleX = 8;
    const int paddleH = 24;
    const int paddleW = 4;
    const int paddleSpd = 3;

    // IA
    int aiY;
    const int aiX = 228;
    const int aiSpd = 2;

    // Balle
    int ballX, ballY;
    float velX, velY;
    const int ballSize = 4;

    // Score
    int playerScore;
    int aiScore;

    // Rendu
    int prevBallX, prevBallY;
    int prevPaddleY;
    int prevAiY;
    bool firstDraw;

    // Timing
    unsigned long lastUpdate;
    const int updateInterval = 16;

    // Reset balle
    void resetBall() {
      ballX = 120;
      ballY = 67;
      velX = (random(0, 2) == 0) ? 2.0 : -2.0;
      velY = (random(0, 2) == 0) ? 1.5 : -1.5;
    }

    // IA
    void updateAI() {
      if (ballY > aiY + paddleH / 2) aiY += aiSpd;
      else if (ballY < aiY - paddleH / 2) aiY -= aiSpd;

      aiY = constrain(aiY, paddleH / 2, 135 - paddleH / 2);
    }

  public:
    PongGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      pinMode(PONG_BUZZER, OUTPUT);

      paddleY = 67;
      aiY = 67;
      playerScore = 0;
      aiScore = 0;
      score = 0;
      state = IN_PROGRESS;
      firstDraw = true;
      lastUpdate = 0;

      resetBall();
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;
      if (millis() - lastUpdate < updateInterval) return;
      lastUpdate = millis();

      prevBallX = ballX;
      prevBallY = ballY;
      prevPaddleY = paddleY;
      prevAiY = aiY;

      // joueur
      if (buttons.up) paddleY -= paddleSpd;
      if (buttons.down) paddleY += paddleSpd;
      paddleY = constrain(paddleY, paddleH / 2, 135 - paddleH / 2);

      // IA
      updateAI();

      // balle
      ballX += (int)velX;
      ballY += (int)velY;

      // murs
      if (ballY <= 0 || ballY >= 135) {
        velY = -velY;
        tone(PONG_BUZZER, 600, 20);
      }

      // collision joueur
      if (ballX <= paddleX + paddleW &&
          ballY >= paddleY - paddleH / 2 &&
          ballY <= paddleY + paddleH / 2) {

        velX = abs(velX) + 0.1;
        velY = (ballY - paddleY) * 0.15;

        tone(PONG_BUZZER, 1200, 30);
      }

      // collision IA
      if (ballX >= aiX - paddleW &&
          ballY >= aiY - paddleH / 2 &&
          ballY <= aiY + paddleH / 2) {

        velX = -(abs(velX) + 0.1);
        velY = (ballY - aiY) * 0.15;

        tone(PONG_BUZZER, 900, 30);
      }

      // point IA
      if (ballX < 0) {
        aiScore++;
        tone(PONG_BUZZER, 200, 200);

        if (aiScore >= 5) {
          tone(PONG_BUZZER, 100, 600);
          state = GAME_OVER;
          score = playerScore;
          return;
        }

        resetBall();
      }

      // point joueur
      if (ballX > 240) {
        playerScore++;
        score = playerScore;

        tone(PONG_BUZZER, 1000, 200);

        if (playerScore >= 5) {
          tone(PONG_BUZZER, 1500, 500);
          state = GAME_OVER;
          return;
        }

        resetBall();
      }
    }

    void render() override {

      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);

        for (int y = 0; y < 135; y += 8) {
          screen->fillRect(119, y, 2, 4, TFT_DARKGREY);
        }

        screen->setTextColor(TFT_WHITE, TFT_BLACK);
        screen->setTextSize(2);

        screen->setCursor(80, 4);
        screen->print(playerScore);

        screen->setCursor(148, 4);
        screen->print(aiScore);

        screen->fillRect(paddleX, paddleY - paddleH / 2, paddleW, paddleH, TFT_WHITE);
        screen->fillRect(aiX, aiY - paddleH / 2, paddleW, paddleH, TFT_WHITE);
        screen->fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);

        return;
      }

      screen->fillRect(prevBallX, prevBallY, ballSize, ballSize, TFT_BLACK);
      screen->fillRect(paddleX, prevPaddleY - paddleH / 2, paddleW, paddleH, TFT_BLACK);
      screen->fillRect(aiX, prevAiY - paddleH / 2, paddleW, paddleH, TFT_BLACK);

      screen->fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);
      screen->fillRect(paddleX, paddleY - paddleH / 2, paddleW, paddleH, TFT_WHITE);
      screen->fillRect(aiX, aiY - paddleH / 2, paddleW, paddleH, TFT_WHITE);

      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(2);

      screen->setCursor(80, 4);
      screen->print(playerScore);

      screen->setCursor(148, 4);
      screen->print(aiScore);
    }

    virtual ~PongGame() {}
};

#endif