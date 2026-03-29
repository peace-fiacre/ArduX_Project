#ifndef MORPION_H
#define MORPION_H

#include "Game.h"

class MorpionGame : public Game {
  private:

    bool needsRedraw;

    // Grille 3x3 : 0=vide, 1=J1 (humain), 2=J2 (IA)
    int board[3][3];

    // Position du curseur
    int cursorX, cursorY;

    // Joueur actuel : 1 (humain) ou 2 (IA)
    int currentPlayer;

    // Résultat : 0=en cours, 1=J1 gagne, 2=J2 gagne, 3=nul
    int result;

    // Indique que l'IA doit jouer ce tour
    bool aiTurn;

    // Horodatage du dernier coup du joueur (pour le délai IA)
    unsigned long aiWaitStart;

    // Taille d'une case
    const int cellSize = 36;

    // Origine de la grille (centrée)
    const int gridX = (240 - 36 * 3) / 2;  // = 54px
    const int gridY = (135 - 36 * 3) / 2 + 10;  // = 22px

    // Vérifier si quelqu'un a gagné
    int checkWinner() {
      for (int row = 0; row < 3; row++) {
        if (board[row][0] != 0 &&
            board[row][0] == board[row][1] &&
            board[row][1] == board[row][2]) {
          return board[row][0];
        }
      }
      for (int col = 0; col < 3; col++) {
        if (board[0][col] != 0 &&
            board[0][col] == board[1][col] &&
            board[1][col] == board[2][col]) {
          return board[0][col];
        }
      }
      if (board[0][0] != 0 &&
          board[0][0] == board[1][1] &&
          board[1][1] == board[2][2]) {
        return board[0][0];
      }
      if (board[0][2] != 0 &&
          board[0][2] == board[1][1] &&
          board[1][1] == board[2][0]) {
        return board[0][2];
      }
      return 0;
    }

    // Vérifier si match nul
    bool checkDraw() {
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
          if (board[row][col] == 0) return false;
        }
      }
      return true;
    }

    // Poser un coup et vérifier fin de partie
    void playMove(int row, int col, int player) {
      board[row][col] = player;
      result = checkWinner();
      if (result != 0) {
        score = (result == 1) ? 1 : 0;  // 1 point seulement si le joueur humain gagne
        state = GAME_OVER;
        needsRedraw = true;
        return;
      }
      if (checkDraw()) {
        result = 3;
        state  = GAME_OVER;
        needsRedraw = true;
        return;
      }
      currentPlayer = (player == 1) ? 2 : 1;
      if (currentPlayer == 2) {
        aiTurn = true;
        aiWaitStart = millis();  // démarre le chrono du délai
      } else {
        aiTurn = false;
      }
    }

    // -------------------------------------------------------
    // IA STRATÉGIQUE SIMPLE (3 niveaux de priorité)
    //   1. Gagner si un coup gagnant existe
    //   2. Bloquer le joueur humain
    //   3. Centre > coins > côtés
    // -------------------------------------------------------

    // Cherche un coup gagnant pour 'player'. Retourne true et
    // écrit (row, col) si trouvé.
    bool findWinningMove(int player, int &row, int &col) {
      for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
          if (board[r][c] == 0) {
            board[r][c] = player;       // simule le coup
            bool wins = (checkWinner() == player);
            board[r][c] = 0;            // annule
            if (wins) { row = r; col = c; return true; }
          }
        }
      }
      return false;
    }

    void aiPlay() {
      int r = -1, c = -1;

      // 1. L'IA gagne si possible
      if (findWinningMove(2, r, c)) { playMove(r, c, 2); return; }

      // 2. Bloquer le joueur humain
      if (findWinningMove(1, r, c)) { playMove(r, c, 2); return; }

      // 3. Prendre le centre
      if (board[1][1] == 0) { playMove(1, 1, 2); return; }

      // 4. Prendre un coin libre
      int corners[4][2] = {{0,0},{0,2},{2,0},{2,2}};
      for (int i = 0; i < 4; i++) {
        if (board[corners[i][0]][corners[i][1]] == 0) {
          playMove(corners[i][0], corners[i][1], 2);
          return;
        }
      }

      // 5. Prendre un côté libre
      int sides[4][2] = {{0,1},{1,0},{1,2},{2,1}};
      for (int i = 0; i < 4; i++) {
        if (board[sides[i][0]][sides[i][1]] == 0) {
          playMove(sides[i][0], sides[i][1], 2);
          return;
        }
      }
    }

  public:
    MorpionGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      for (int row = 0; row < 3; row++)
        for (int col = 0; col < 3; col++)
          board[row][col] = 0;

      cursorX       = 1;
      cursorY       = 1;
      currentPlayer = 1;   // humain commence
      result        = 0;
      score         = 0;
      state         = IN_PROGRESS;
      aiTurn        = false;
      aiWaitStart   = 0;
      needsRedraw   = true;
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      // Tour de l'IA : attendre 300ms avant de jouer
      if (aiTurn) {
        if (millis() - aiWaitStart >= 300) {
          aiPlay();
          needsRedraw = true;
        }
        return;
      }

      // Tour du joueur humain
      bool changed = false;

      if (buttons.upPressed    && cursorY > 0) { cursorY--; changed = true; }
      else if (buttons.downPressed  && cursorY < 2) { cursorY++; changed = true; }
      else if (buttons.leftPressed  && cursorX > 0) { cursorX--; changed = true; }
      else if (buttons.rightPressed && cursorX < 2) { cursorX++; changed = true; }
      else if (buttons.aPressed) {
        if (board[cursorY][cursorX] == 0) {
          playMove(cursorY, cursorX, 1);
          changed = true;
        }
      }

      if (changed) needsRedraw = true;
    }

    void render() override {
      if (!needsRedraw) return;
      needsRedraw = false;

      screen->fillScreen(TFT_BLACK);

      // En-tête
      screen->setTextSize(1);
      if (state == IN_PROGRESS) {
        if (currentPlayer == 1) {
          screen->setTextColor(TFT_RED, TFT_BLACK);
          screen->setCursor(5, 4);
          screen->print("Ton tour  [X]");
        } else {
          screen->setTextColor(TFT_CYAN, TFT_BLACK);
          screen->setCursor(5, 4);
          screen->print("IA joue   [O]");
        }
      } else {
        // Affichage résultat
        screen->setCursor(5, 4);
        if (result == 1) {
          screen->setTextColor(TFT_GREEN, TFT_BLACK);
          screen->print("Tu as gagne !");
        } else if (result == 2) {
          screen->setTextColor(TFT_RED, TFT_BLACK);
          screen->print("L'IA a gagne !");
        } else {
          screen->setTextColor(TFT_YELLOW, TFT_BLACK);
          screen->print("Match nul !");
        }
      }

      // Grille
      screen->drawLine(gridX + cellSize,     gridY,               gridX + cellSize,     gridY + cellSize * 3, TFT_WHITE);
      screen->drawLine(gridX + cellSize * 2, gridY,               gridX + cellSize * 2, gridY + cellSize * 3, TFT_WHITE);
      screen->drawLine(gridX,               gridY + cellSize,     gridX + cellSize * 3, gridY + cellSize,     TFT_WHITE);
      screen->drawLine(gridX,               gridY + cellSize * 2, gridX + cellSize * 3, gridY + cellSize * 2, TFT_WHITE);

      // Pièces et curseur
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {

          int cx = gridX + col * cellSize + cellSize / 2;
          int cy = gridY + row * cellSize + cellSize / 2;

          // Curseur (uniquement quand c'est le tour du joueur humain)
          if (col == cursorX && row == cursorY &&
              state == IN_PROGRESS && currentPlayer == 1) {
            screen->drawRect(
              gridX + col * cellSize + 2,
              gridY + row * cellSize + 2,
              cellSize - 4,
              cellSize - 4,
              TFT_YELLOW
            );
          }

          // Joueur 1 : X rouge
          if (board[row][col] == 1) {
            int m = 8;
            screen->drawLine(
              gridX + col * cellSize + m,
              gridY + row * cellSize + m,
              gridX + col * cellSize + cellSize - m,
              gridY + row * cellSize + cellSize - m,
              TFT_RED
            );
            screen->drawLine(
              gridX + col * cellSize + cellSize - m,
              gridY + row * cellSize + m,
              gridX + col * cellSize + m,
              gridY + row * cellSize + cellSize - m,
              TFT_RED
            );
          }

          // Joueur 2 / IA : O cyan
          if (board[row][col] == 2) {
            screen->drawCircle(cx, cy, cellSize / 2 - 6, TFT_CYAN);
          }
        }
      }
    }

    virtual ~MorpionGame() {}
};

#endif
