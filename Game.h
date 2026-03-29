#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>

struct Buttons {
    bool up, down, left, right, a, b;
    bool upPressed, downPressed;
    bool leftPressed, rightPressed;
    bool aPressed, bPressed;
};

enum stateGame { START, IN_PROGRESS, GAME_OVER };

class Game {
protected:
    TFT_eSPI* screen;
    uint16_t score;   // uint16_t : 2 octets RAM au lieu de 4 (int)
    stateGame state;

public:
    Game(TFT_eSPI* display) : screen(display), score(0), state(START) {}

    virtual void init()   = 0;
    virtual void update(Buttons buttons) = 0;
    virtual void render() = 0;

    bool     isGameOver() { return state == GAME_OVER; }
    uint16_t getScore()   { return score; }

    void displayScore() {
        screen->fillRect(0, 0, 240, 20, TFT_CYAN);
        screen->setTextColor(TFT_WHITE, TFT_CYAN);
        screen->setTextSize(2);
        screen->setCursor(2, 2);
        screen->print("SCORE = ");
        screen->print(score);
    }

    virtual ~Game() {}
};

#endif