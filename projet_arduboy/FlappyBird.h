#ifndef FLAPPYBIRD_H
#define FLAPPYBIRD_H

#include <Arduino.h>
#include "Game.h"

//  FLAPPY BIRD — Évite les tuyaux en appuyant sur A
//  Bouton : A pour faire battre les ailes (sauter)
//  Game Over : si l'oiseau touche un tuyau ou le sol/plafond

class FlappyBirdGame : public Game {
private:

  // ── Oiseau ──
  float birdY;
  float birdVelY;
  const int birdX = 40;
  const int birdSize = 8;
  const float gravity = 0.25f;    // GRAVITÉ RÉDUITE (était 0.35f)
  const float flapForce = -2.8f;  // Force de saut ajustée (était -3.2f)

  // ── Tuyaux ──
  static const int PIPE_COUNT = 3;
  int pipeX[PIPE_COUNT];
  int pipeGapY[PIPE_COUNT];
  const int pipeW = 14;
  const int pipeGap = 36;
  const int pipeSpeedBase = 2;
  int pipeSpeed;
  const int pipeSpacing = 90;

  // ── Sol ──
  const int groundY = 128;

  // ── Rendu optimisé ──
  float prevBirdY;
  float prevFloatOffset;
  int prevPipeX[PIPE_COUNT];
  bool firstDraw;

  // ── Timing ──
  unsigned long lastUpdate;
  const int updateInterval = 16;

  // ── Écran de départ ──
  bool waitingToStart;
  float floatOffset;
  unsigned long startTime;

  // ── Score local ──
  int lastDisplayedScore;

  // ──────────────────────────────────────────
  void resetPipes() {
    for (int i = 0; i < PIPE_COUNT; i++) {
      pipeX[i] = 240 + i * pipeSpacing;
      pipeGapY[i] = random(28, groundY - pipeGap - 8);
    }
  }

  void drawPipe(int x, int gapY, uint16_t color) {
    screen->fillRect(x, 0, pipeW, gapY - pipeGap / 2, color);
    screen->fillRect(x - 2, gapY - pipeGap / 2 - 4, pipeW + 4, 4, color);
    screen->fillRect(x, gapY + pipeGap / 2, pipeW, groundY - (gapY + pipeGap / 2), color);
    screen->fillRect(x - 2, gapY + pipeGap / 2, pipeW + 4, 4, color);
  }

  void drawBird(int y, uint16_t bodyColor, uint16_t wingColor) {
    screen->fillRect(birdX - 2, y - 2, birdSize + 6, birdSize + 6, bodyColor);
    screen->fillRect(birdX, y, birdSize, birdSize, bodyColor);
    screen->fillRect(birdX + 5, y + 2, 2, 2, wingColor);
    screen->fillRect(birdX + birdSize, y + 4, 3, 2, TFT_ORANGE);
  }

  void eraseBird(int y) {
    int eraseY = y - 4;
    int eraseH = birdSize + 8;
    if (eraseY < 0) { eraseH += eraseY; eraseY = 0; }
    if (eraseY + eraseH > groundY) eraseH = groundY - eraseY;
    
    for (int yy = eraseY; yy < eraseY + eraseH && yy < groundY; yy++) {
      uint16_t skyColor;
      if (yy < 45) skyColor = 0x04DF;
      else if (yy < 90) skyColor = 0x051F;
      else skyColor = 0x07FF;
      screen->fillRect(birdX - 2, yy, birdSize + 6, 1, skyColor);
    }
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
    screen->fillRect(x, 0, w, 45, 0x04DF);
    screen->fillRect(x, 45, w, 45, 0x051F);
    screen->fillRect(x, 90, w, 38, 0x07FF);
  }

  void drawScoreLabel() {
    screen->fillRect(0, 0, 240, 20, 0x04DF);
    screen->setTextColor(TFT_WHITE, 0x04DF);
    screen->setTextSize(2);
    screen->setCursor(2, 2);
    screen->print("SCORE : ");
    lastDisplayedScore = -1;
    drawScoreValue();
  }

  void drawScoreValue() {
    if (score == lastDisplayedScore) return;
    screen->fillRect(110, 2, 60, 16, 0x04DF);
    screen->setTextColor(TFT_WHITE, 0x04DF);
    screen->setTextSize(2);
    screen->setCursor(110, 2);
    screen->print(score);
    lastDisplayedScore = score;
  }

public:
  FlappyBirdGame(TFT_eSPI* display)
    : Game(display), lastDisplayedScore(-1) {}

  void forceRedraw() override {
    firstDraw = true;
  }

  void init() override {
    static bool seeded = false;
    if (!seeded) {
      randomSeed(analogRead(0));
      seeded = true;
    }

    birdY = 60.0f;
    birdVelY = 0.0f;
    floatOffset = 0.0f;
    prevFloatOffset = 0.0f;
    score = 0;
    pipeSpeed = pipeSpeedBase;
    state = IN_PROGRESS;
    firstDraw = true;
    waitingToStart = true;
    lastUpdate = 0;
    startTime = millis();
    prevBirdY = birdY;
    lastDisplayedScore = -1;

    resetPipes();
    for (int i = 0; i < PIPE_COUNT; i++) prevPipeX[i] = pipeX[i];

    drawScoreLabel();
  }

  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;
    if (millis() - lastUpdate < (unsigned long)updateInterval) return;
    lastUpdate = millis();

    if (waitingToStart) {
      prevFloatOffset = floatOffset;
      floatOffset = 4.0f * sinf((millis() - startTime) / 400.0f);
      
      if (buttons.aPressed) {
        waitingToStart = false;
        birdVelY = flapForce;
      }
      return;
    }

    prevBirdY = birdY;
    for (int i = 0; i < PIPE_COUNT; i++) prevPipeX[i] = pipeX[i];

    if (buttons.aPressed) birdVelY = flapForce;
    birdVelY += gravity;
    birdVelY = constrain(birdVelY, -5.0f, 6.0f);  // Vitesses max réduites aussi
    birdY += birdVelY;

    if (birdY < 0) { birdY = 0; birdVelY = 0; }
    if (birdY > groundY - birdSize) { birdY = groundY - birdSize; birdVelY = 0; }

    for (int i = 0; i < PIPE_COUNT; i++) {
      pipeX[i] -= pipeSpeed;
      if (pipeX[i] + pipeW < 0) {
        int maxX = 0;
        for (int j = 0; j < PIPE_COUNT; j++) maxX = max(maxX, pipeX[j]);
        pipeX[i] = maxX + pipeSpacing;
        pipeGapY[i] = random(28, groundY - pipeGap - 8);
        score++;
        if (score % 5 == 0 && pipeSpeed < 5) pipeSpeed++;
      }
    }

    if (checkCollision()) state = GAME_OVER;
  }

  void render() override {
    drawScoreValue();

    if (firstDraw) {
      firstDraw = false;
      screen->fillScreen(TFT_BLACK);
      drawSky(0, 240);
      drawGround(TFT_GREEN);
      for (int i = 0; i < PIPE_COUNT; i++) drawPipe(pipeX[i], pipeGapY[i], TFT_GREEN);
      drawBird((int)(birdY + floatOffset), TFT_YELLOW, TFT_WHITE);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(52, 108);
      screen->print("[A] pour commencer !");
      return;
    }

    if (waitingToStart) {
      eraseBird((int)(birdY + prevFloatOffset));
      drawBird((int)(birdY + floatOffset), TFT_YELLOW, TFT_WHITE);
      return;
    }

    eraseBird((int)prevBirdY);

    for (int i = 0; i < PIPE_COUNT; i++) {
      if (prevPipeX[i] != pipeX[i]) {
        drawPipe(prevPipeX[i], pipeGapY[i], TFT_BLACK);
        drawSky(prevPipeX[i] - 2, pipeW + 6);
        drawPipe(pipeX[i], pipeGapY[i], TFT_GREEN);
      }
    }

    drawGround(TFT_GREEN);
    drawBird((int)birdY, TFT_YELLOW, TFT_WHITE);
  }

  virtual ~FlappyBirdGame() {}
};

#endif