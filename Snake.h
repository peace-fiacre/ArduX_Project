// Le fichier Snake.h
#ifndef SNAKE_H
#define SNAKE_H

#include "Game.h"

// Direction du serpent
enum snakeDirection {
  SNAKE_UP,
  SNAKE_DOWN,
  SNAKE_LEFT,
  SNAKE_RIGHT
};

struct Segment {
  int x, y;
};

class SnakeGame : public Game {
  private:
    Segment snake[100];
    int snakeLength;
    snakeDirection direction;
    int appleX, appleY;

    Segment lastTail;        // ancienne queue à effacer
    bool moved;              // le serpent a bougé ce frame ?
    bool appleEaten;         // pomme mangée ce frame ?
    bool firstDraw;          // premier dessin complet ?

    const int gridSize = 8;
    const int gridW = 240 / 8;
    const int gridH = 110 / 8;
    const int scoreHeight = 2;

    unsigned long lastMove;
    const int moveInterval = 150;

    void spawnApple() {
      appleX = random(0, gridW);
      appleY = random(scoreHeight, gridH);
    }

    void moveSnake() {
      if (millis() - lastMove < moveInterval) return;
      lastMove = millis();

      lastTail = snake[snakeLength - 1];   // sauvegarder AVANT de bouger

      for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
      }

      switch (direction) {
        case SNAKE_UP:    snake[0].y -= 1; break;
        case SNAKE_DOWN:  snake[0].y += 1; break;
        case SNAKE_LEFT:  snake[0].x -= 1; break;
        case SNAKE_RIGHT: snake[0].x += 1; break;
      }

      moved = true;   // signaler que le serpent a bougé
    }

    void checkCollision() {
      if (snake[0].x < 0 || snake[0].x >= gridW ||
          snake[0].y < scoreHeight || snake[0].y >= gridH) {
        state = GAME_OVER;
        return;
      }
      for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
          state = GAME_OVER;
          return;
        }
      }
    }

    void checkApple() {
      if (snake[0].x == appleX && snake[0].y == appleY) {
        snakeLength++;
        score++;
        appleEaten = true;   // signaler que la pomme a été mangée
        spawnApple();
      }
    }

  public:
    SnakeGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      snakeLength = 3;
      direction   = SNAKE_RIGHT;
      lastMove    = 0;
      score       = 0;
      state       = IN_PROGRESS;
      moved       = false;
      appleEaten  = false;
      firstDraw   = true;   // forcer un dessin complet au démarrage

      for (int i = 0; i < snakeLength; i++) {
        snake[i].x = (gridW / 2) - i;
        snake[i].y = scoreHeight + (gridH / 2);
      }

      spawnApple();
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      if (buttons.up    && direction != SNAKE_DOWN)  direction = SNAKE_UP;
      if (buttons.down  && direction != SNAKE_UP)    direction = SNAKE_DOWN;
      if (buttons.right && direction != SNAKE_LEFT)  direction = SNAKE_RIGHT;
      if (buttons.left  && direction != SNAKE_RIGHT) direction = SNAKE_LEFT;

      moved      = false;   // reset avant moveSnake
      appleEaten = false;   // reset avant checkApple

      moveSnake();
      checkCollision();
      checkApple();
    }

    void render() override {

      //Premier dessin : tout dessiner proprement
      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);
        displayScore();
        screen->fillRect(
          appleX * gridSize, appleY * gridSize,
          gridSize - 1, gridSize - 1,
          TFT_RED
        );
        for (int i = 0; i < snakeLength; i++) {
          screen->fillRect(
            snake[i].x * gridSize, snake[i].y * gridSize,
            gridSize - 1, gridSize - 1,
            i == 0 ? TFT_WHITE : TFT_GREEN
          );
        }
        return;
      }

      // Pas de mouvement rien à redessiner
      if (!moved) return;

      // Effacer UNIQUEMENT l'ancienne queue
      // (sauf si la pomme a été mangée, la queue grandit, rien à effacer)
      if (!appleEaten) {
        screen->fillRect(
          lastTail.x * gridSize, lastTail.y * gridSize,
          gridSize - 1, gridSize - 1,
          TFT_BLACK   // 1 seule case noire, imperceptible !
        );
      }

      // Redessiner la nouvelle tête
      screen->fillRect(
        snake[0].x * gridSize, snake[0].y * gridSize,
        gridSize - 1, gridSize - 1,
        TFT_WHITE
      );

      // Redessiner le 2e segment (était blanc, devient vert)
      if (snakeLength > 1) {
        screen->fillRect(
          snake[1].x * gridSize, snake[1].y * gridSize,
          gridSize - 1, gridSize - 1,
          TFT_GREEN
        );
      }

      // Redessiner la pomme si mangée (nouvelle position)

      // Redessiner la pomme si mangée (nouvelle position)
      if (appleEaten) {

        // Effacer toute la zone de jeu UNE SEULE FOIS
        // (acceptable car ça arrive rarement — seulement quand on mange)
        screen->fillRect(
          0,
          gridSize * scoreHeight,
          240,
          135 - gridSize * scoreHeight,
          TFT_BLACK
        );

        // Redessiner le serpent entier proprement
        for (int i = 0; i < snakeLength; i++) {
          screen->fillRect(
            snake[i].x * gridSize,
            snake[i].y * gridSize,
            gridSize - 1,
            gridSize - 1,
            i == 0 ? TFT_WHITE : TFT_GREEN
          );
        }

        // Redessiner la pomme à sa nouvelle position
        screen->fillRect(
          appleX * gridSize,
          appleY * gridSize,
          gridSize - 1,
          gridSize - 1,
          TFT_RED
        );

        // Mettre à jour le score
        displayScore();
      }

    }

    virtual ~SnakeGame() {}
};

#endif