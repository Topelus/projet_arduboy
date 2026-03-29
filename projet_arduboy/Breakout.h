#ifndef BREAKOUT_H
#define BREAKOUT_H

#include "Game.h"

//  BREAKOUT — Casse toutes les briques avec la balle
//  Boutons : GAUCHE / DROITE pour bouger la raquette
//  Game Over : si la balle tombe sous la raquette
//  Victoire : si toutes les briques sont détruites

class BreakoutGame : public Game {
  private:

    // ── Raquette ──
    int   paddleX;               // position X du centre de la raquette
    const int paddleY   = 122;   // position Y fixe (bas de l'écran)
    const int paddleH   =  5;    // hauteur de la raquette
    const int paddleW   = 36;    // largeur de la raquette
    const int paddleSpd =  4;    // vitesse de déplacement

    // ── Balle ──
    float ballX, ballY;          // position (float pour précision)
    float velX,  velY;           // vitesse
    const int ballSize = 5;      // taille de la balle
    const float ballSpeedBase = 2.2f;  // vitesse initiale

    // ── Briques ──
    static const int COLS  = 10;   // colonnes de briques
    static const int ROWS  =  5;   // rangées de briques
    static const int BRICK_W = 22; // largeur d'une brique
    static const int BRICK_H =  8; // hauteur d'une brique
    static const int BRICK_GAP = 2; // espace entre briques
    static const int BRICK_OFF_X = 4;  // marge gauche
    static const int BRICK_OFF_Y = 18; // marge haute (sous le score)

    bool bricks[ROWS][COLS];     // true = brique présente
    int  brickCount;             // briques restantes

    // Couleurs par rangée
    const uint16_t rowColors[ROWS] = {
      TFT_RED,
      TFT_ORANGE,
      TFT_YELLOW,
      TFT_GREEN,
      TFT_CYAN
    };

    // ── Vies ──
    int lives;
    const int MAX_LIVES = 3;

    // ── Rendu optimisé ──
    float prevBallX,  prevBallY;
    int   prevPaddleX;
    bool  firstDraw;

    // ── Timing ──
    unsigned long lastUpdate;
    const int updateInterval = 16;  // ~60fps

    // ── Écran de départ ──
    bool waitingToStart;

    //  Initialiser toutes les briques
    void resetBricks() {
      brickCount = 0;
      for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
          bricks[r][c] = true;
          brickCount++;
        }
      }
    }

    //  Remettre la balle au-dessus de la raquette
    void resetBall() {
      ballX  = paddleX;
      ballY  = paddleY - ballSize - 2;
      velX   = (random(0, 2) == 0) ? ballSpeedBase : -ballSpeedBase;
      velY   = -ballSpeedBase;
      waitingToStart = true;
    }

    //  Dessiner une seule brique
    void drawBrick(int row, int col, uint16_t color) {
      int x = BRICK_OFF_X + col * (BRICK_W + BRICK_GAP);
      int y = BRICK_OFF_Y + row * (BRICK_H + BRICK_GAP);
      screen->fillRect(x, y, BRICK_W, BRICK_H, color);
      // Reflet (ligne claire en haut)
      screen->drawFastHLine(x + 1, y + 1, BRICK_W - 2, TFT_WHITE);
    }

    //  Effacer une seule brique
    void eraseBrick(int row, int col) {
      int x = BRICK_OFF_X + col * (BRICK_W + BRICK_GAP);
      int y = BRICK_OFF_Y + row * (BRICK_H + BRICK_GAP);
      screen->fillRect(x, y, BRICK_W, BRICK_H, TFT_BLACK);
    }

    //  Dessiner toutes les briques présentes
    void drawAllBricks() {
      for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
          if (bricks[r][c]) drawBrick(r, c, rowColors[r]);
        }
      }
    }

    //  Dessiner la raquette
    void drawPaddle(int x, uint16_t color) {
      screen->fillRect(x - paddleW / 2, paddleY, paddleW, paddleH, color);
      // Reflet
      if (color != TFT_BLACK)
        screen->drawFastHLine(x - paddleW / 2 + 1, paddleY + 1, paddleW - 2, TFT_WHITE);
    }

    //  Dessiner la balle
    void drawBall(int x, int y, uint16_t color) {
      screen->fillRect(x, y, ballSize, ballSize, color);
    }

    //  Afficher le HUD (score + vies)
    void drawHUD() {
      screen->fillRect(0, 0, 240, 16, TFT_BLACK);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 4);
      screen->print("SCORE:");
      screen->print(score);

      // Vies affichées comme petits carrés colorés
      screen->setCursor(160, 4);
      screen->print("VIES:");
      for (int i = 0; i < lives; i++) {
        screen->fillRect(198 + i * 12, 3, 8, 8, TFT_RED);
      }
    }

    //  Vérifier collision balle / briques
    //  Retourne true si une brique a été touchée
    bool checkBrickCollision() {
      int bx = (int)ballX;
      int by = (int)ballY;

      for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
          if (!bricks[r][c]) continue;

          int brickX = BRICK_OFF_X + c * (BRICK_W + BRICK_GAP);
          int brickY = BRICK_OFF_Y + r * (BRICK_H + BRICK_GAP);

          // Chevauchement ?
          if (bx + ballSize > brickX     &&
              bx            < brickX + BRICK_W &&
              by + ballSize > brickY     &&
              by            < brickY + BRICK_H) {

            // Détruire la brique
            bricks[r][c] = false;
            brickCount--;
            eraseBrick(r, c);
            score += 10 + r * 5;  // plus de points pour les rangées hautes

            // Rebond : déterminer si impact horizontal ou vertical
            bool fromLeft  = (bx + ballSize - brickX) < 6;
            bool fromRight = (brickX + BRICK_W - bx)  < 6;

            if (fromLeft || fromRight) velX = -velX;
            else                       velY = -velY;

            return true;  // une seule brique par frame
          }
        }
      }
      return false;
    }

  public:
    BreakoutGame(TFT_eSPI* display) : Game(display) {}

    //  Initialisation — appelée au démarrage et à chaque nouvelle partie
    void init() override {
      paddleX      = 120;
      prevPaddleX  = paddleX;
      lives        = MAX_LIVES;
      score        = 0;
      state        = IN_PROGRESS;
      firstDraw    = true;
      lastUpdate   = 0;

      resetBricks();
      resetBall();  // positionne balle + waitingToStart = true
    }

    //  Update — logique du jeu à chaque frame
    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;
      if (millis() - lastUpdate < (unsigned long)updateInterval) return;
      lastUpdate = millis();

      // ── Déplacement raquette (toujours actif) ──
      prevPaddleX = paddleX;

      if (buttons.left)  paddleX -= paddleSpd;
      if (buttons.right) paddleX += paddleSpd;
      paddleX = constrain(paddleX, paddleW / 2, 240 - paddleW / 2);

      // ── Écran de départ : attente appui A ──
      if (waitingToStart) {
        // La balle suit la raquette
        ballX = paddleX - ballSize / 2;
        if (buttons.aPressed) {
          waitingToStart = false;
        }
        return;
      }

      // ── Sauvegarder ancienne position balle ──
      prevBallX = ballX;
      prevBallY = ballY;

      // ── Déplacement balle ──
      ballX += velX;
      ballY += velY;

      // ── Rebonds murs gauche / droite ──
      if (ballX <= 0) {
        ballX = 0;
        velX  = abs(velX);
      }
      if (ballX + ballSize >= 240) {
        ballX = 240 - ballSize;
        velX  = -abs(velX);
      }

      // ── Rebond plafond ──
      if (ballY <= 16) {
        ballY = 16;
        velY  = abs(velY);
      }

      // ── Collision raquette ──
      if (ballY + ballSize >= paddleY &&
          ballY + ballSize <= paddleY + paddleH + abs(velY) &&
          ballX + ballSize >= paddleX - paddleW / 2 &&
          ballX            <= paddleX + paddleW / 2) {

        ballY = paddleY - ballSize;
        velY  = -abs(velY);

        // Angle selon la zone touchée sur la raquette
        float hit  = (ballX + ballSize / 2.0f) - paddleX;  // -paddleW/2 à +paddleW/2
        velX = hit * 0.12f;
        // Limiter la vitesse X
        velX = constrain(velX, -4.0f, 4.0f);
      }

      // ── Balle perdue (sous la raquette) ──
      if (ballY > 135) {
        lives--;
        drawHUD();

        if (lives <= 0) {
          state = GAME_OVER;
          return;
        }
        // Remettre la balle
        resetBall();
        return;
      }

      // ── Collision briques ──
      checkBrickCollision();

      // ── Victoire : toutes les briques détruites ──
      if (brickCount <= 0) {
        score += lives * 50;  // bonus de vies restantes
        state  = GAME_OVER;
      }
    }

    //  Render — affichage optimisé
    void render() override {

      // ── Premier dessin : tout afficher ──
      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);
        drawAllBricks();
        drawPaddle(paddleX, TFT_WHITE);
        drawBall((int)ballX, (int)ballY, TFT_WHITE);
        drawHUD();

        // Message de départ
        screen->setTextColor(TFT_YELLOW, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(52, 108);
        screen->print("[A] pour lancer la balle");
        return;
      }

      // ── Écran d'attente : balle colle à la raquette ──
      if (waitingToStart) {
        // Effacer / redessiner raquette si elle a bougé
        if (prevPaddleX != paddleX) {
          drawPaddle(prevPaddleX, TFT_BLACK);
          drawPaddle(paddleX,     TFT_WHITE);
        }
        // Balle suit la raquette
        drawBall((int)prevBallX, (int)prevBallY, TFT_BLACK);
        drawBall((int)ballX,     (int)ballY,     TFT_WHITE);
        drawHUD();
        return;
      }

      // ── Mise à jour partielle ──

      // Effacer / redessiner raquette
      if (prevPaddleX != paddleX) {
        drawPaddle(prevPaddleX, TFT_BLACK);
        drawPaddle(paddleX,     TFT_WHITE);
      }

      // Effacer / redessiner balle
      drawBall((int)prevBallX, (int)prevBallY, TFT_BLACK);
      drawBall((int)ballX,     (int)ballY,     TFT_WHITE);

      // HUD
      drawHUD();
    }

    virtual ~BreakoutGame() {}
};

#endif