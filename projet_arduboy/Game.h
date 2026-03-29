#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>

struct Buttons {
  bool up, down, left, right, a, b;
  bool upPressed, downPressed, leftPressed, rightPressed, aPressed, bPressed;

  void clearPressed() {
    upPressed = downPressed = leftPressed = rightPressed = aPressed = bPressed = false;
  }
};

enum stateGame {
  START,
  IN_PROGRESS,
  GAME_OVER,
  VICTORY  // ← Ajouté pour les victoires
};

class Game {
  protected:
    TFT_eSPI* screen;
    int score;
    stateGame state;
    int lastDisplayedScore;

  public:
    Game(TFT_eSPI* display) {
      screen = display;
      score = 0;
      state = START;
      lastDisplayedScore = -1;
    }
    virtual void forceRedraw() = 0;

    virtual ~Game() {}

    virtual void init() = 0;
    virtual void update(Buttons buttons) = 0;
    virtual void render() = 0;

    bool isGameOver() const { return state == GAME_OVER || state == VICTORY; }
    bool isVictory() const { return state == VICTORY; }  // ← Nouvelle méthode
    int getScore() const { return score; }

    virtual void displayScore() {
      if (score == lastDisplayedScore) return;
      screen->fillRect(70, 2, 60, 20, TFT_CYAN);
      screen->setTextColor(TFT_WHITE, TFT_CYAN);
      screen->setTextSize(2);
      screen->setCursor(72, 2);
      screen->print(score);
      lastDisplayedScore = score;
    }

    void drawScoreLabel() {
      screen->fillRect(0, 0, 240, 20, TFT_CYAN);
      screen->setTextColor(TFT_WHITE, TFT_CYAN);
      screen->setTextSize(2);
      screen->setCursor(2, 2);
      screen->print("SCORE : ");
      lastDisplayedScore = -1;
      displayScore();
    }
};

#endif