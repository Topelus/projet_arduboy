#ifndef FLAPPYBIRD_H
#define FLAPPYBIRD_H

#include "Game.h"

//  FLAPPY BIRD — Évite les tuyaux en appuyant sur A
//  Bouton : A pour faire battre les ailes (sauter)
//  Game Over : si l'oiseau touche un tuyau ou le sol/plafond

class FlappyBirdGame : public Game {
  private:

    // ── Oiseau ──
    float birdY;
    float birdVelY;
    const int   birdX      = 40;
    const int   birdSize   =  8;
    const float gravity    =  0.35f;
    const float flapForce  = -3.2f;

    // ── Tuyaux ──
    static const int PIPE_COUNT = 3;
    int pipeX[PIPE_COUNT];
    int pipeGapY[PIPE_COUNT];
    const int pipeW         = 14;
    const int pipeGap       = 36;
    const int pipeSpeedBase =  2;
    int       pipeSpeed;
    const int pipeSpacing   = 90;

    // ── Sol ──
    const int groundY = 128;

    // ── Rendu optimisé ──
    float prevBirdY;
    float prevFloatOffset;     // BUG FIX : mémoriser l'ancien offset flottant
    int   prevPipeX[PIPE_COUNT];
    bool  firstDraw;

    // ── Timing ──
    unsigned long lastUpdate;
    const int updateInterval = 16;

    // ── Écran de départ ──
    bool  waitingToStart;
    float floatOffset;         // BUG FIX : offset visuel séparé de birdY
    unsigned long startTime;

    // ──────────────────────────────────────────
    void resetPipes() {
      for (int i = 0; i < PIPE_COUNT; i++) {
        pipeX[i]    = 240 + i * pipeSpacing;
        pipeGapY[i] = random(28, groundY - pipeGap - 8);
      }
    }

    void drawPipe(int x, int gapY, uint16_t color) {
      screen->fillRect(x,     0,                      pipeW,      gapY - pipeGap / 2,                   color);
      screen->fillRect(x - 2, gapY - pipeGap / 2 - 4, pipeW + 4, 4,                                    color);
      screen->fillRect(x,     gapY + pipeGap / 2,     pipeW,      groundY - (gapY + pipeGap / 2),       color);
      screen->fillRect(x - 2, gapY + pipeGap / 2,     pipeW + 4, 4,                                    color);
    }

    void drawBird(int y, uint16_t bodyColor, uint16_t wingColor) {
      screen->fillRect(birdX,            y,     birdSize, birdSize, bodyColor);
      screen->fillRect(birdX + 5,        y + 2, 2,        2,        wingColor);
      screen->fillRect(birdX + birdSize, y + 4, 3,        2,        TFT_ORANGE);
    }

    bool checkCollision() {
      int by = (int)birdY;
      if (by <= 0 || by + birdSize >= groundY) return true;
      for (int i = 0; i < PIPE_COUNT; i++) {
        if (birdX + birdSize > pipeX[i] - 2 && birdX < pipeX[i] + pipeW + 2) {
          if (by < pipeGapY[i] - pipeGap / 2 || by + birdSize > pipeGapY[i] + pipeGap / 2)
            return true;
        }
      }
      return false;
    }

    void drawGround(uint16_t color) {
      screen->fillRect(0, groundY, 240, 135 - groundY, color);
    }

    void drawSky(int x, int w) {
      screen->fillRect(x, 0,  w, 45, 0x04DF);
      screen->fillRect(x, 45, w, 45, 0x051F);
      screen->fillRect(x, 90, w, 38, 0x07FF);
    }

    void drawScore() {
      screen->fillRect(0, 0, 70, 14, TFT_BLACK);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 2);
      screen->print("SCORE:");
      screen->print(score);
    }

  public:
    FlappyBirdGame(TFT_eSPI* display) : Game(display) {}

    // ──────────────────────────────────────────
    void init() override {
      birdY           = 60.0f;
      birdVelY        = 0.0f;      // BUG FIX : gravité NON accumulée pendant l'attente
      floatOffset     = 0.0f;
      prevFloatOffset = 0.0f;
      score           = 0;
      pipeSpeed       = pipeSpeedBase;
      state           = IN_PROGRESS;
      firstDraw       = true;
      waitingToStart  = true;
      lastUpdate      = 0;
      startTime       = millis();
      prevBirdY       = birdY;

      resetPipes();
      for (int i = 0; i < PIPE_COUNT; i++) prevPipeX[i] = pipeX[i];
    }

    // ──────────────────────────────────────────
    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;
      if (millis() - lastUpdate < (unsigned long)updateInterval) return;
      lastUpdate = millis();

      // ── Attente du démarrage ──
      if (waitingToStart) {
        // BUG FIX 1 : birdY ne change PAS → pas d'accumulation de gravité
        // BUG FIX 2 : floatOffset est un simple décalage visuel calculé ici
        prevFloatOffset = floatOffset;
        floatOffset     = 4.0f * sinf((millis() - startTime) / 400.0f);

        // BUG FIX 3 : lecture de aPressed directement, sans condition supplémentaire
        if (buttons.aPressed) {
          waitingToStart = false;
          birdVelY       = flapForce;  // impulsion initiale propre
          // birdY reste à 60, aucune vélocité accumulée
        }
        return;
      }

      // ── Jeu en cours ──
      prevBirdY = birdY;
      for (int i = 0; i < PIPE_COUNT; i++) prevPipeX[i] = pipeX[i];

      if (buttons.aPressed) birdVelY = flapForce;
      birdVelY += gravity;
      birdVelY  = constrain(birdVelY, -6.0f, 8.0f);
      birdY    += birdVelY;

      for (int i = 0; i < PIPE_COUNT; i++) {
        pipeX[i] -= pipeSpeed;
        if (pipeX[i] + pipeW < 0) {
          int maxX = 0;
          for (int j = 0; j < PIPE_COUNT; j++) maxX = max(maxX, pipeX[j]);
          pipeX[i]    = maxX + pipeSpacing;
          pipeGapY[i] = random(28, groundY - pipeGap - 8);
          score++;
          if (score % 5 == 0 && pipeSpeed < 5) pipeSpeed++;
        }
      }

      if (checkCollision()) state = GAME_OVER;
    }

    // ──────────────────────────────────────────
    void render() override {

      // ── Premier dessin complet ──
      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);
        drawSky(0, 240);
        drawGround(TFT_GREEN);
        for (int i = 0; i < PIPE_COUNT; i++) drawPipe(pipeX[i], pipeGapY[i], TFT_GREEN);
        drawBird((int)(birdY + floatOffset), TFT_YELLOW, TFT_WHITE);
        drawScore();
        screen->setTextColor(TFT_WHITE, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(52, 108);
        screen->print("[A] pour commencer !");
        return;
      }

      // ── Animation pendant l'attente ──
      if (waitingToStart) {
        // Effacer l'oiseau à son ancienne position flottante
        drawBird((int)(birdY + prevFloatOffset), TFT_BLACK, TFT_BLACK);
        // Redessiner à la nouvelle position flottante
        drawBird((int)(birdY + floatOffset), TFT_YELLOW, TFT_WHITE);
        // birdY est fixe → le ciel/sol/tuyaux ne bougent pas, pas besoin de les redessiner
        return;
      }

      // ── Rendu partiel en jeu ──

      // Effacer ancien oiseau
      drawBird((int)prevBirdY, TFT_BLACK, TFT_BLACK);

      // Tuyaux : effacer ancien, redessiner nouveau
      for (int i = 0; i < PIPE_COUNT; i++) {
        if (prevPipeX[i] != pipeX[i]) {
          drawPipe(prevPipeX[i], pipeGapY[i], TFT_BLACK);
          drawSky(prevPipeX[i] - 2, pipeW + 6);  // restaurer le ciel derrière
          drawPipe(pipeX[i], pipeGapY[i], TFT_GREEN);
        }
      }

      drawGround(TFT_GREEN);
      drawBird((int)birdY, TFT_YELLOW, TFT_WHITE);
      drawScore();
    }

    virtual ~FlappyBirdGame() {}
};

#endif