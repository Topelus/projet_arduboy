#ifndef PONG_H
#define PONG_H

#include <Arduino.h>      // pour random(), millis(), constrain()
#include "Game.h"

//  PONG — Jeu de raquette solo contre l'IA
//  Boutons : HAUT / BAS pour bouger la raquette
//  Game Over : le premier à 5 points gagne

class PongGame : public Game {
  private:
    // Raquette du joueur (gauche)
    int paddleY;               // position Y du centre
    const int paddleX   = 8;
    const int paddleH   = 24;
    const int paddleW   = 4;
    const int paddleSpd = 3;

    // Raquette de l'IA (droite)
    int aiY;
    const int aiX     = 228;
    const int aiSpd   = 2;

    // Balle
    int   ballX, ballY;
    float velX, velY;
    const int ballSize = 4;

    // Score
    int playerScore;
    int aiScore;

    // Optimisation d'affichage
    int   prevBallX,  prevBallY;
    int   prevPaddleY;
    int   prevAiY;
    bool  firstDraw;

    // Timing (~60 FPS)
    unsigned long lastUpdate;
    const int updateInterval = 16;

    // Remet la balle au centre avec une direction aléatoire
    void resetBall() {
      ballX = 120;
      ballY = 67;
      velX = (random(0, 2) == 0) ? 2.0 : -2.0;
      velY = (random(0, 2) == 0) ? 1.5 : -1.5;
    }

    // IA : suit la balle avec une vitesse limitée
    void updateAI() {
      if (ballY > aiY + paddleH / 2) aiY += aiSpd;
      else if (ballY < aiY - paddleH / 2) aiY -= aiSpd;
      aiY = constrain(aiY, paddleH / 2, 135 - paddleH / 2);
    }

  public:
    PongGame(TFT_eSPI* display) : Game(display) {}

    void forceRedraw() override {
    firstDraw = true;
}

    void init() override {
      // Initialisation du générateur aléatoire (une seule fois)
      static bool seedDone = false;
      if (!seedDone) {
        randomSeed(analogRead(0));  // utilise une broche analogique flottante
        seedDone = true;
      }

      paddleY      = 67;
      aiY          = 67;
      playerScore  = 0;
      aiScore      = 0;
      score        = 0;
      state        = IN_PROGRESS;
      firstDraw    = true;
      lastUpdate   = 0;
      resetBall();
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;
      if (millis() - lastUpdate < updateInterval) return;
      lastUpdate = millis();

      // Sauvegarde pour effacement
      prevBallX   = ballX;
      prevBallY   = ballY;
      prevPaddleY = paddleY;
      prevAiY     = aiY;

      // Contrôle joueur
      if (buttons.up)   paddleY -= paddleSpd;
      if (buttons.down) paddleY += paddleSpd;
      paddleY = constrain(paddleY, paddleH / 2, 135 - paddleH / 2);

      updateAI();

      // Déplacement balle
      ballX += (int)velX;
      ballY += (int)velY;

      // Rebonds murs haut/bas
      if (ballY <= 0 || ballY >= 135) velY = -velY;

      // Collision raquette joueur (gauche)
      if (ballX <= paddleX + paddleW &&
          ballY >= paddleY - paddleH / 2 &&
          ballY <= paddleY + paddleH / 2) {
        velX = abs(velX) + 0.1;        // accélération
        velY = (ballY - paddleY) * 0.15;
      }

      // Collision raquette IA (droite)
      if (ballX >= aiX - paddleW &&
          ballY >= aiY - paddleH / 2 &&
          ballY <= aiY + paddleH / 2) {
        velX = -(abs(velX) + 0.1);
        velY = (ballY - aiY) * 0.15;
      }

      // Point marqué ?
      if (ballX < 0) {
        aiScore++;
        if (aiScore >= 5) { state = GAME_OVER; score = playerScore; return; }
        resetBall();
      }
      if (ballX > 240) {
        playerScore++;
        score = playerScore;
        if (playerScore >= 5) { state = GAME_OVER; return; }
        resetBall();
      }
    }

    void render() override {
      // Premier dessin : tout afficher
      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);

        // Ligne centrale pointillée
        for (int y = 0; y < 135; y += 8) {
          screen->fillRect(119, y, 2, 4, TFT_DARKGREY);
        }

        // Scores
        screen->setTextColor(TFT_WHITE, TFT_BLACK);
        screen->setTextSize(2);
        screen->setCursor(80, 4);
        screen->print(playerScore);
        screen->setCursor(148, 4);
        screen->print(aiScore);

        // Raquettes et balle
        screen->fillRect(paddleX, paddleY - paddleH/2, paddleW, paddleH, TFT_WHITE);
        screen->fillRect(aiX,     aiY    - paddleH/2, paddleW, paddleH, TFT_WHITE);
        screen->fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);
        return;
      }

      // Effacement des anciens éléments
      screen->fillRect(prevBallX, prevBallY, ballSize, ballSize, TFT_BLACK);
      screen->fillRect(paddleX, prevPaddleY - paddleH/2, paddleW, paddleH, TFT_BLACK);
      screen->fillRect(aiX, prevAiY - paddleH/2, paddleW, paddleH, TFT_BLACK);

      // Dessin des nouveaux éléments
      screen->fillRect(ballX, ballY, ballSize, ballSize, TFT_WHITE);
      screen->fillRect(paddleX, paddleY - paddleH/2, paddleW, paddleH, TFT_WHITE);
      screen->fillRect(aiX, aiY - paddleH/2, paddleW, paddleH, TFT_WHITE);

      // Mise à jour des scores (si changés)
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