#ifndef MORPION_H
#define MORPION_H

#include "Game.h"

class MorpionGame : public Game {
private:
  bool needsRedraw;
  int board[3][3];  // 0=vide, 1=Joueur (X), 2=IA (O)
  int cursorX, cursorY;
  int currentPlayer;           // 1 = joueur, 2 = IA
  int result;                  // 0=en cours, 1=joueur gagne, 2=IA gagne, 3=nul
  bool waitingForAI;           // vrai si on attend que l'IA joue après délai
  unsigned long lastMoveTime;  // moment du dernier coup (joueur)

  const int cellSize = 36;
  const int gridX = (240 - 36 * 3) / 2;       // = 54
  const int gridY = (135 - 36 * 3) / 2 + 10;  // = 22

  // Vérifie s'il y a un gagnant
  int checkWinner() {
    // Lignes
    for (int row = 0; row < 3; row++) {
      if (board[row][0] != 0 && board[row][0] == board[row][1] && board[row][1] == board[row][2]) {
        return board[row][0];
      }
    }
    // Colonnes
    for (int col = 0; col < 3; col++) {
      if (board[0][col] != 0 && board[0][col] == board[1][col] && board[1][col] == board[2][col]) {
        return board[0][col];
      }
    }
    // Diagonales
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
      return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
      return board[0][2];
    }
    return 0;
  }

  bool checkDraw() {
    for (int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
        if (board[row][col] == 0) return false;
    return true;
  }

  // L'IA choisit la meilleure case disponible
  void aiMove() {
    // 1. Vérifier si l'IA peut gagner
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == 0) {
          board[row][col] = 2;  // essai
          if (checkWinner() == 2) {
            return;  // coup gagnant trouvé
          }
          board[row][col] = 0;  // annuler
        }
      }
    }

    // 2. Bloquer le joueur (si peut gagner au prochain tour)
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == 0) {
          board[row][col] = 1;  // simule le joueur
          if (checkWinner() == 1) {
            board[row][col] = 2;  // l'IA bloque ici
            return;
          }
          board[row][col] = 0;
        }
      }
    }

    // 3. Sinon prendre le centre si libre
    if (board[1][1] == 0) {
      board[1][1] = 2;
      return;
    }

    // 4. Sinon prendre un coin
    int corners[4][2] = { { 0, 0 }, { 0, 2 }, { 2, 0 }, { 2, 2 } };
    for (int i = 0; i < 4; i++) {
      int r = corners[i][0], c = corners[i][1];
      if (board[r][c] == 0) {
        board[r][c] = 2;
        return;
      }
    }

    // 5. Sinon premier vide trouvé
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == 0) {
          board[row][col] = 2;
          return;
        }
      }
    }
  }

public:
  void forceRedraw() override {
    needsRedraw = true;
  }
  MorpionGame(TFT_eSPI* display)
    : Game(display) {
    needsRedraw = true;
    waitingForAI = false;
  }

  void init() override {
    for (int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
        board[row][col] = 0;

    cursorX = 1;
    cursorY = 1;
    currentPlayer = 1;  // le joueur commence
    result = 0;
    score = 0;
    state = IN_PROGRESS;
    needsRedraw = true;
    waitingForAI = false;

    drawScoreLabel();  // affiche "SCORE : " en haut
  }

  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;

    // Gestion du délai pour l'IA
    if (waitingForAI && (millis() - lastMoveTime >= 1000)) {
      waitingForAI = false;
      // L'IA joue
      aiMove();
      needsRedraw = true;

      // Vérifier victoire ou nul après coup IA
      result = checkWinner();
      if (result != 0) {
        score = (result == 1) ? 1 : 2;
        state = GAME_OVER;
        return;
      }
      if (checkDraw()) {
        result = 3;
        score = 0;
        state = GAME_OVER;
        return;
      }
      // Passer le tour au joueur
      currentPlayer = 1;
    }

    // Seul le joueur (currentPlayer == 1) peut interagir
    if (currentPlayer == 1 && state == IN_PROGRESS) {
      bool changed = false;

      if (buttons.upPressed && cursorY > 0) {
        cursorY--;
        changed = true;
      } else if (buttons.downPressed && cursorY < 2) {
        cursorY++;
        changed = true;
      } else if (buttons.leftPressed && cursorX > 0) {
        cursorX--;
        changed = true;
      } else if (buttons.rightPressed && cursorX < 2) {
        cursorX++;
        changed = true;
      } else if (buttons.aPressed) {
        if (board[cursorY][cursorX] == 0) {
          board[cursorY][cursorX] = 1;  // joueur place X
          changed = true;

          result = checkWinner();
          if (result != 0) {
            score = 1;
            state = GAME_OVER;
            needsRedraw = true;
            return;
          }
          if (checkDraw()) {
            result = 3;
            score = 0;
            state = GAME_OVER;
            needsRedraw = true;
            return;
          }
          // Passer le tour à l'IA avec délai
          currentPlayer = 2;
          waitingForAI = true;
          lastMoveTime = millis();
        }
      }

      if (changed) needsRedraw = true;
    }
  }

  void render() override {
    if (!needsRedraw) return;
    needsRedraw = false;

    screen->fillScreen(TFT_BLACK);
    displayScore();

    // Affichage du statut
    screen->setTextSize(1);
    if (state == IN_PROGRESS) {
      if (currentPlayer == 1 && !waitingForAI) {
        screen->setTextColor(TFT_RED, TFT_BLACK);
        screen->setCursor(5, 4);
        screen->print("A vous (X)");
      } else if (currentPlayer == 2 || waitingForAI) {
        screen->setTextColor(TFT_CYAN, TFT_BLACK);
        screen->setCursor(5, 4);
        screen->print("IA reflechit...");
      }
    } else {
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setCursor(5, 4);
      if (result == 1) screen->print("Vous avez gagne !");
      else if (result == 2) screen->print("IA a gagne...");
      else if (result == 3) screen->print("Match nul !");
    }

    // Lignes de la grille
    screen->drawLine(gridX + cellSize, gridY, gridX + cellSize, gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX + cellSize * 2, gridY, gridX + cellSize * 2, gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize, gridX + cellSize * 3, gridY + cellSize, TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize * 2, gridX + cellSize * 3, gridY + cellSize * 2, TFT_WHITE);

    // Dessin des X / O et du curseur
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        int cx = gridX + col * cellSize + cellSize / 2;
        int cy = gridY + row * cellSize + cellSize / 2;

        // Curseur seulement si c'est au joueur de jouer
        if (col == cursorX && row == cursorY && state == IN_PROGRESS && currentPlayer == 1 && !waitingForAI) {
          screen->drawRect(
            gridX + col * cellSize + 2,
            gridY + row * cellSize + 2,
            cellSize - 4,
            cellSize - 4,
            TFT_YELLOW);
        }

        // X (Joueur)
        if (board[row][col] == 1) {
          int m = 8;
          screen->drawLine(
            gridX + col * cellSize + m,
            gridY + row * cellSize + m,
            gridX + col * cellSize + cellSize - m,
            gridY + row * cellSize + cellSize - m,
            TFT_RED);
          screen->drawLine(
            gridX + col * cellSize + cellSize - m,
            gridY + row * cellSize + m,
            gridX + col * cellSize + m,
            gridY + row * cellSize + cellSize - m,
            TFT_RED);
        }
        // O (IA)
        else if (board[row][col] == 2) {
          screen->drawCircle(cx, cy, cellSize / 2 - 6, TFT_CYAN);
        }
      }
    }
  }

  virtual ~MorpionGame() {}
};

#endif