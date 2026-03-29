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
    Segment snake[200];         // BUG 1 FIX : 100 cases insuffisant (gridW×gridH = 30×11 = 330 cases max)
    int snakeLength;
    snakeDirection direction;
    snakeDirection nextDirection; // BUG 2 FIX : file de direction pour éviter le demi-tour en 2 appuis rapides
    int appleX, appleY;

    Segment lastTail;
    bool moved;
    bool appleEaten;
    bool firstDraw;

    const int gridSize    =  8;
    const int gridW       = 240 / 8;   // = 30
    const int gridH       = 110 / 8;   // = 13
    const int scoreHeight =  2;        // rangées réservées au score (en cellules)

    unsigned long lastMove;
    const int moveInterval = 150;

    // BUG 3 FIX : spawnApple vérifie que la pomme ne tombe pas sur le serpent
    void spawnApple() {
      bool onSnake;
      int attempts = 0;
      do {
        onSnake = false;
        appleX = random(0, gridW);
        appleY = random(scoreHeight, gridH);
        for (int i = 0; i < snakeLength; i++) {
          if (snake[i].x == appleX && snake[i].y == appleY) {
            onSnake = true;
            break;
          }
        }
        attempts++;
      } while (onSnake && attempts < 100);
    }

    void moveSnake() {
      if (millis() - lastMove < moveInterval) return;
      lastMove = millis();

      // BUG 2 FIX : appliquer la direction validée
      direction = nextDirection;

      lastTail = snake[snakeLength - 1];

      for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
      }

      switch (direction) {
        case SNAKE_UP:    snake[0].y -= 1; break;
        case SNAKE_DOWN:  snake[0].y += 1; break;
        case SNAKE_LEFT:  snake[0].x -= 1; break;
        case SNAKE_RIGHT: snake[0].x += 1; break;
      }

      moved = true;
    }

    void checkCollision() {
      // BUG 4 FIX : la limite basse était gridH (13) mais l'écran va jusqu'à 135px
      // soit 135/8 = 16 cellules — on corrige en utilisant 135/gridSize
      const int gridHFull = 135 / gridSize;  // = 16

      if (snake[0].x < 0 || snake[0].x >= gridW ||
          snake[0].y < scoreHeight || snake[0].y >= gridHFull) {
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
        appleEaten = true;
        spawnApple();
      }
    }

  public:
    SnakeGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      snakeLength   = 3;
      direction     = SNAKE_RIGHT;
      nextDirection = SNAKE_RIGHT;  // BUG 2 FIX : initialiser nextDirection
      lastMove      = 0;
      score         = 0;
      state         = IN_PROGRESS;
      moved         = false;
      appleEaten    = false;
      firstDraw     = true;

      // BUG 5 FIX : centrer le serpent verticalement dans la vraie zone de jeu
      const int gridHFull = 135 / gridSize;
      int startY = scoreHeight + (gridHFull - scoreHeight) / 2;

      for (int i = 0; i < snakeLength; i++) {
        snake[i].x = (gridW / 2) - i;
        snake[i].y = startY;
      }

      spawnApple();
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      // BUG 2 FIX : on mémorise la PROCHAINE direction sans l'appliquer immédiatement
      // Cela empêche le demi-tour si on appuie 2 fois vite (ex: RIGHT puis DOWN → pas de LEFT)
      if (buttons.upPressed    && direction != SNAKE_DOWN)  nextDirection = SNAKE_UP;
      if (buttons.downPressed  && direction != SNAKE_UP)    nextDirection = SNAKE_DOWN;
      if (buttons.rightPressed && direction != SNAKE_LEFT)  nextDirection = SNAKE_RIGHT;
      if (buttons.leftPressed  && direction != SNAKE_RIGHT) nextDirection = SNAKE_LEFT;

      // BUG 6 FIX : utiliser Pressed (front montant) et non l'état continu
      // pour éviter que maintenir une touche change la direction plusieurs fois

      moved      = false;
      appleEaten = false;

      moveSnake();
      if (state != GAME_OVER) checkCollision();
      if (state != GAME_OVER) checkApple();
    }

    void render() override {
      const int gridHFull = 135 / gridSize;

      // Premier dessin complet
      if (firstDraw) {
        firstDraw = false;
        screen->fillScreen(TFT_BLACK);
        displayScore();

        // BUG 7 FIX : dessiner la bordure de jeu pour mieux visualiser les limites
        screen->drawRect(0, scoreHeight * gridSize - 1, 240, 135 - scoreHeight * gridSize + 1, TFT_DARKGREY);

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

      // Pas de mouvement → rien à redessiner
      if (!moved) return;

      // Effacer l'ancienne queue (sauf si la pomme a été mangée)
      if (!appleEaten) {
        screen->fillRect(
          lastTail.x * gridSize, lastTail.y * gridSize,
          gridSize - 1, gridSize - 1,
          TFT_BLACK
        );
      }

      // Nouvelle tête
      screen->fillRect(
        snake[0].x * gridSize, snake[0].y * gridSize,
        gridSize - 1, gridSize - 1,
        TFT_WHITE
      );

      // 2e segment : était blanc, redevient vert
      if (snakeLength > 1) {
        screen->fillRect(
          snake[1].x * gridSize, snake[1].y * gridSize,
          gridSize - 1, gridSize - 1,
          TFT_GREEN
        );
      }

      // Pomme mangée : redessiner toute la zone proprement
      if (appleEaten) {
        screen->fillRect(
          0,
          gridSize * scoreHeight,
          240,
          135 - gridSize * scoreHeight,
          TFT_BLACK
        );

        // Redessiner la bordure
        screen->drawRect(0, scoreHeight * gridSize - 1, 240, 135 - scoreHeight * gridSize + 1, TFT_DARKGREY);

        // Redessiner le serpent entier
        for (int i = 0; i < snakeLength; i++) {
          screen->fillRect(
            snake[i].x * gridSize,
            snake[i].y * gridSize,
            gridSize - 1,
            gridSize - 1,
            i == 0 ? TFT_WHITE : TFT_GREEN
          );
        }

        // Nouvelle pomme
        screen->fillRect(
          appleX * gridSize,
          appleY * gridSize,
          gridSize - 1,
          gridSize - 1,
          TFT_RED
        );

        displayScore();
      }
    }

    virtual ~SnakeGame() {}
};

#endif