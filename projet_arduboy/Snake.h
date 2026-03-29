#ifndef SNAKE_H
#define SNAKE_H

#include <Arduino.h>
#include "Game.h"

// Directions possibles
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
  Segment snake[200];  // 200 suffit pour une grille max de 30×16 = 480 cases
  int snakeLength;
  snakeDirection direction;
  snakeDirection nextDirection;  // Évite les demi-tours intempestifs
  int appleX, appleY;

  Segment lastTail;
  bool moved;
  bool appleEaten;
  bool firstDraw;

  const int gridSize = 8;
  const int gridW = 240 / 8;  // = 30
  const int gridH = 135 / 8;  // = 16 (hauteur totale en cellules)
  const int scoreHeight = 2;  // hauteur réservée au score (en cellules)

  unsigned long lastMove;
  const int moveInterval = 150;

  // Génère une pomme à un endroit libre (pas sur le serpent)
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
      if (attempts > 200) break;  // sécurité
    } while (onSnake);
  }

  // Déplacement du serpent
  void moveSnake() {
    if (millis() - lastMove < moveInterval) return;
    lastMove = millis();

    direction = nextDirection;  // applique la direction validée
    lastTail = snake[snakeLength - 1];

    // Décale le corps
    for (int i = snakeLength - 1; i > 0; i--) {
      snake[i] = snake[i - 1];
    }

    // Avance la tête
    switch (direction) {
      case SNAKE_UP: snake[0].y -= 1; break;
      case SNAKE_DOWN: snake[0].y += 1; break;
      case SNAKE_LEFT: snake[0].x -= 1; break;
      case SNAKE_RIGHT: snake[0].x += 1; break;
    }

    moved = true;
  }

  // Vérifie les collisions (murs et corps)
  void checkCollision() {
    if (snake[0].x < 0 || snake[0].x >= gridW || snake[0].y < scoreHeight || snake[0].y >= gridH) {
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

  // Vérifie si la pomme est mangée
  void checkApple() {
    if (snake[0].x == appleX && snake[0].y == appleY) {
      snakeLength++;
      score++;
      appleEaten = true;
      spawnApple();
    }
  }

public:
  SnakeGame(TFT_eSPI* display)
    : Game(display) {}
  void forceRedraw() override {
    firstDraw = true;
  }

  void init() override {
    snakeLength = 3;
    direction = SNAKE_RIGHT;
    nextDirection = SNAKE_RIGHT;
    lastMove = 0;
    score = 0;
    state = IN_PROGRESS;
    moved = false;
    appleEaten = false;
    firstDraw = true;

    // Position initiale centrée
    int startY = scoreHeight + (gridH - scoreHeight) / 2;
    for (int i = 0; i < snakeLength; i++) {
      snake[i].x = (gridW / 2) - i;
      snake[i].y = startY;
    }

    spawnApple();
    drawScoreLabel();  // affiche "SCORE : " en haut
  }

  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;

    // Gestion des directions (uniquement sur front montant)
    if (buttons.upPressed && direction != SNAKE_DOWN) nextDirection = SNAKE_UP;
    if (buttons.downPressed && direction != SNAKE_UP) nextDirection = SNAKE_DOWN;
    if (buttons.rightPressed && direction != SNAKE_LEFT) nextDirection = SNAKE_RIGHT;
    if (buttons.leftPressed && direction != SNAKE_RIGHT) nextDirection = SNAKE_LEFT;

    moved = false;
    appleEaten = false;

    moveSnake();
    if (state != GAME_OVER) checkCollision();
    if (state != GAME_OVER) checkApple();
  }

  void render() override {
    // Mise à jour du score affiché (méthode parente)
    displayScore();

    // Premier dessin complet
    if (firstDraw) {
      firstDraw = false;
      // On efface seulement la zone de jeu (pas la bande du score)
      screen->fillRect(0, scoreHeight * gridSize, 240, 135 - scoreHeight * gridSize, TFT_BLACK);
      screen->drawRect(0, scoreHeight * gridSize - 1, 240, 135 - scoreHeight * gridSize + 1, TFT_DARKGREY);

      // Pomme
      screen->fillRect(appleX * gridSize, appleY * gridSize, gridSize - 1, gridSize - 1, TFT_RED);
      // Serpent
      for (int i = 0; i < snakeLength; i++) {
        screen->fillRect(snake[i].x * gridSize, snake[i].y * gridSize, gridSize - 1, gridSize - 1,
                         i == 0 ? TFT_WHITE : TFT_GREEN);
      }
      return;
    }

    // Si pas de mouvement, rien à redessiner
    if (!moved) return;

    // Efface l'ancienne queue (sauf si la pomme a été mangée, on redessine tout)
    if (!appleEaten) {
      screen->fillRect(lastTail.x * gridSize, lastTail.y * gridSize, gridSize - 1, gridSize - 1, TFT_BLACK);
    }

    // Dessine la nouvelle tête
    screen->fillRect(snake[0].x * gridSize, snake[0].y * gridSize, gridSize - 1, gridSize - 1, TFT_WHITE);

    // Redessine le deuxième segment (devient vert)
    if (snakeLength > 1) {
      screen->fillRect(snake[1].x * gridSize, snake[1].y * gridSize, gridSize - 1, gridSize - 1, TFT_GREEN);
    }

    // Si pomme mangée, on nettoie et redessine toute la zone de jeu
    if (appleEaten) {
      screen->fillRect(0, scoreHeight * gridSize, 240, 135 - scoreHeight * gridSize, TFT_BLACK);
      screen->drawRect(0, scoreHeight * gridSize - 1, 240, 135 - scoreHeight * gridSize + 1, TFT_DARKGREY);
      for (int i = 0; i < snakeLength; i++) {
        screen->fillRect(snake[i].x * gridSize, snake[i].y * gridSize, gridSize - 1, gridSize - 1,
                         i == 0 ? TFT_WHITE : TFT_GREEN);
      }
      screen->fillRect(appleX * gridSize, appleY * gridSize, gridSize - 1, gridSize - 1, TFT_RED);
    }
  }

  virtual ~SnakeGame() {}
};

#endif