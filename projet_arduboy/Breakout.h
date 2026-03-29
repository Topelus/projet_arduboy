#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <Arduino.h>
#include "Game.h"

//  BREAKOUT — Casse toutes les briques avec la balle
//  Boutons : GAUCHE / DROITE pour bouger la raquette
//  Game Over : si la balle tombe sous la raquette ou plus de vies

class BreakoutGame : public Game {
private:
  // ── Raquette ──
  int paddleX;
  const int paddleY = 122;
  const int paddleH = 5;
  const int paddleW = 36;
  const int paddleSpd = 4;

  // ── Balle ──
  float ballX, ballY;
  float velX, velY;
  const int ballSize = 5;
  const float ballSpeedBase = 2.2f;

  // ── Briques ──
  static const int COLS = 10;
  static const int ROWS = 5;
  static const int BRICK_W = 22;
  static const int BRICK_H = 8;
  static const int BRICK_GAP = 2;
  static const int BRICK_OFF_X = 4;
  static const int BRICK_OFF_Y = 18;

  bool bricks[ROWS][COLS];
  int brickCount;

  const uint16_t rowColors[ROWS] = {
    TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN
  };

  // ── Vies ──
  int lives;
  const int MAX_LIVES = 3;

  // ── Rendu optimisé ──
  float prevBallX, prevBallY;
  int prevPaddleX;
  bool firstDraw;
  bool waitingToStart;

  // ── Timing ──
  unsigned long lastUpdate;
  const int updateInterval = 16;

  void resetBricks() {
    brickCount = 0;
    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        bricks[r][c] = true;
        brickCount++;
      }
    }
  }

  void resetBall() {
    ballX = paddleX - ballSize / 2.0f;
    ballY = paddleY - ballSize - 2;
    velX = (random(0, 2) == 0) ? ballSpeedBase : -ballSpeedBase;
    velY = -ballSpeedBase;
    waitingToStart = true;
    // IMPORTANT: synchroniser prevBall avec ball pour éviter la traînée
    prevBallX = ballX;
    prevBallY = ballY;
  }

  void drawBrick(int row, int col, uint16_t color) {
    int x = BRICK_OFF_X + col * (BRICK_W + BRICK_GAP);
    int y = BRICK_OFF_Y + row * (BRICK_H + BRICK_GAP);
    screen->fillRect(x, y, BRICK_W, BRICK_H, color);
    screen->drawFastHLine(x + 1, y + 1, BRICK_W - 2, TFT_WHITE);
  }

  void eraseBrick(int row, int col) {
    int x = BRICK_OFF_X + col * (BRICK_W + BRICK_GAP);
    int y = BRICK_OFF_Y + row * (BRICK_H + BRICK_GAP);
    screen->fillRect(x, y, BRICK_W, BRICK_H, TFT_BLACK);
  }

  void drawAllBricks() {
    for (int r = 0; r < ROWS; r++)
      for (int c = 0; c < COLS; c++)
        if (bricks[r][c]) drawBrick(r, c, rowColors[r]);
  }

  void drawPaddle(int x, uint16_t color) {
    screen->fillRect(x - paddleW / 2, paddleY, paddleW, paddleH, color);
    if (color != TFT_BLACK)
      screen->drawFastHLine(x - paddleW / 2 + 1, paddleY + 1, paddleW - 2, TFT_WHITE);
  }

  void drawBall(int x, int y, uint16_t color) {
    screen->fillRect(x, y, ballSize, ballSize, color);
  }

  void drawLives() {
    screen->fillRect(180, 2, 60, 16, TFT_BLACK);
    screen->setTextColor(TFT_WHITE, TFT_BLACK);
    screen->setTextSize(1);
    screen->setCursor(180, 5);
    screen->print("VIES:");
    for (int i = 0; i < lives; i++) {
      screen->fillRect(215 + i * 12, 3, 8, 8, TFT_RED);
    }
  }

  bool checkBrickCollision() {
    int bx = (int)ballX;
    int by = (int)ballY;

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        if (!bricks[r][c]) continue;

        int brickX = BRICK_OFF_X + c * (BRICK_W + BRICK_GAP);
        int brickY = BRICK_OFF_Y + r * (BRICK_H + BRICK_GAP);

        if (bx + ballSize > brickX && bx < brickX + BRICK_W && 
            by + ballSize > brickY && by < brickY + BRICK_H) {

          bricks[r][c] = false;
          brickCount--;
          eraseBrick(r, c);
          score += 10 + r * 5;

          bool fromLeft = (bx + ballSize - brickX) < 6;
          bool fromRight = (brickX + BRICK_W - bx) < 6;
          if (fromLeft || fromRight) velX = -velX;
          else velY = -velY;

          return true;
        }
      }
    }
    return false;
  }

public:
  BreakoutGame(TFT_eSPI* display)
    : Game(display) {}

  void forceRedraw() override {
    firstDraw = true;
  }

  void init() override {
    static bool seeded = false;
    if (!seeded) {
      randomSeed(analogRead(0));
      seeded = true;
    }

    paddleX = 120;
    prevPaddleX = paddleX;
    lives = MAX_LIVES;
    score = 0;
    state = IN_PROGRESS;
    firstDraw = true;
    lastUpdate = 0;
    prevBallX = 0;
    prevBallY = 0;

    resetBricks();
    resetBall();

    drawScoreLabel();
  }

  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;
    if (millis() - lastUpdate < updateInterval) return;
    lastUpdate = millis();

    // Déplacement raquette
    prevPaddleX = paddleX;
    if (buttons.left) paddleX -= paddleSpd;
    if (buttons.right) paddleX += paddleSpd;
    paddleX = constrain(paddleX, paddleW / 2, 240 - paddleW / 2);

    // Écran d'attente : la balle suit la raquette
    if (waitingToStart) {
      // Mettre à jour prevBall AVANT de changer ballX/ballY
      prevBallX = ballX;
      prevBallY = ballY;
      
      ballX = paddleX - ballSize / 2.0f;
      ballY = paddleY - ballSize - 2;
      
      if (buttons.aPressed) waitingToStart = false;
      return;
    }

    // Sauvegarde ancienne position balle
    prevBallX = ballX;
    prevBallY = ballY;

    // Déplacement balle
    ballX += velX;
    ballY += velY;

    // Rebords gauche/droit
    if (ballX <= 0) {
      ballX = 0;
      velX = abs(velX);
    }
    if (ballX + ballSize >= 240) {
      ballX = 240 - ballSize;
      velX = -abs(velX);
    }

    // Plafond
    if (ballY <= 16) {
      ballY = 16;
      velY = abs(velY);
    }

    // Collision raquette
    if (ballY + ballSize >= paddleY && 
        ballY + ballSize <= paddleY + paddleH + abs(velY) && 
        ballX + ballSize >= paddleX - paddleW / 2 && 
        ballX <= paddleX + paddleW / 2) {

      ballY = paddleY - ballSize;
      velY = -abs(velY);
      float hit = (ballX + ballSize / 2.0f) - paddleX;
      velX = hit * 0.12f;
      velX = constrain(velX, -4.0f, 4.0f);
    }

    // Balle perdue
    if (ballY > 135) {
      lives--;
      if (lives <= 0) {
        state = GAME_OVER;
        return;
      }
      resetBall();
      drawLives();
      return;
    }

    // Collision briques
    checkBrickCollision();

    // Victoire
    if (brickCount <= 0) {
      score += lives * 50;
      state = GAME_OVER;
    }
  }

  void render() override {
    // Met à jour l'affichage du score
    displayScore();

    // Premier dessin complet
    if (firstDraw) {
      firstDraw = false;
      screen->fillScreen(TFT_BLACK);
      drawAllBricks();
      drawPaddle(paddleX, TFT_WHITE);
      drawBall((int)ballX, (int)ballY, TFT_WHITE);
      drawLives();
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(52, 108);
      screen->print("[A] pour lancer la balle");
      return;
    }

    // Écran d'attente : balle collée à la raquette
    if (waitingToStart) {
      // Effacer l'ancienne balle (maintenant correctement synchronisé)
      drawBall((int)prevBallX, (int)prevBallY, TFT_BLACK);
      
      if (prevPaddleX != paddleX) {
        drawPaddle(prevPaddleX, TFT_BLACK);
        drawPaddle(paddleX, TFT_WHITE);
      }
      
      drawBall((int)ballX, (int)ballY, TFT_WHITE);
      return;
    }

    // Mise à jour partielle en cours de jeu
    if (prevPaddleX != paddleX) {
      drawPaddle(prevPaddleX, TFT_BLACK);
      drawPaddle(paddleX, TFT_WHITE);
    }

    drawBall((int)prevBallX, (int)prevBallY, TFT_BLACK);
    drawBall((int)ballX, (int)ballY, TFT_WHITE);

    drawLives();
  }

  virtual ~BreakoutGame() {}
};

#endif