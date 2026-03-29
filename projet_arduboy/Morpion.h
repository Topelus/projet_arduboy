#ifndef MORPION_H
#define MORPION_H

#include "Game.h"

class MorpionGame : public Game {
  private:

    bool needsRedraw;

    // Grille 3x3 : 0=vide, 1=J1, 2=J2
    int board[3][3];

    // Position du curseur
    int cursorX, cursorY;

    // Joueur actuel : 1 ou 2
    int currentPlayer;

    // Résultat : 0=en cours, 1=J1 gagne, 2=J2 gagne, 3=nul
    int result;

    // Taille d'une case
    const int cellSize = 36;

    // Origine de la grille (centrée)
    const int gridX = (240 - 36 * 3) / 2;  // = 54px
    const int gridY = (135 - 36 * 3) / 2 + 10;  // = 22px

    // Vérifier si quelqu'un a gagné
    int checkWinner() {

      // Lignes horizontales
      for (int row = 0; row < 3; row++) {
        if (board[row][0] != 0 &&
            board[row][0] == board[row][1] &&
            board[row][1] == board[row][2]) {
          return board[row][0];
        }
      }

      // Lignes verticales
      for (int col = 0; col < 3; col++) {
        if (board[0][col] != 0 &&
            board[0][col] == board[1][col] &&
            board[1][col] == board[2][col]) {
          return board[0][col];
        }
      }

      // Diagonale 
      if (board[0][0] != 0 &&
          board[0][0] == board[1][1] &&
          board[1][1] == board[2][2]) {
        return board[0][0];
      }

      // Diagonale
      if (board[0][2] != 0 &&
          board[0][2] == board[1][1] &&
          board[1][1] == board[2][0]) {
        return board[0][2];
      }

      return 0;  // pas de gagnant
    }

    // Vérifier si match nul
    bool checkDraw() {
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
          if (board[row][col] == 0) return false;  // case vide
        }
      }
      return true;  // toutes les cases remplies
    }

  public:
    MorpionGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      // Vider la grille
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
          board[row][col] = 0;
        }
      }

      cursorX       = 1;   // centre
      cursorY       = 1;   // centre
      currentPlayer = 1;   // J1 commence
      result        = 0;
      score         = 0;
      state         = IN_PROGRESS;
      needsRedraw = true;
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      bool changed = false;   // détecte si quelque chose a changé

      if (buttons.upPressed && cursorY > 0) {
        cursorY--;
        changed = true;
      }
      else if (buttons.downPressed && cursorY < 2) {
        cursorY++;
        changed = true;
      }
      else if (buttons.leftPressed && cursorX > 0) {
        cursorX--;
        changed = true;
      }
      else if (buttons.rightPressed && cursorX < 2) {
        cursorX++;
        changed = true;
      }
      else if (buttons.aPressed) {
        if (board[cursorY][cursorX] == 0) {
          board[cursorY][cursorX] = currentPlayer;
          changed = true;

          result = checkWinner();
          if (result != 0) {
            score = result;
            state = GAME_OVER;
            needsRedraw = true;
            return;
          }
          if (checkDraw()) {
            result = 3;
            state  = GAME_OVER;
            needsRedraw = true;
            return;
          }
          currentPlayer = (currentPlayer == 1) ? 2 : 1;
        }
      }

      if (changed) needsRedraw = true;
    }


    void render() override {

      if (!needsRedraw) return;   // rien changé, on ne redessine pas !
      needsRedraw = false;

      screen->fillScreen(TFT_BLACK);

      // Info joueur actuel
      screen->setTextSize(1);
      if (state == IN_PROGRESS) {
        screen->setTextColor(
          currentPlayer == 1 ? TFT_RED : TFT_CYAN,
          TFT_BLACK
        );
        screen->setCursor(5, 4);
        screen->print("Tour : Joueur ");
        screen->print(currentPlayer);
        screen->print(currentPlayer == 1 ? "  [X]" : "  [O]");
      }

      // Grille 
      screen->drawLine(gridX + cellSize,     gridY,               gridX + cellSize,     gridY + cellSize * 3, TFT_WHITE);
      screen->drawLine(gridX + cellSize * 2, gridY,               gridX + cellSize * 2, gridY + cellSize * 3, TFT_WHITE);
      screen->drawLine(gridX,               gridY + cellSize,     gridX + cellSize * 3, gridY + cellSize,     TFT_WHITE);
      screen->drawLine(gridX,               gridY + cellSize * 2, gridX + cellSize * 3, gridY + cellSize * 2, TFT_WHITE);

      // Pièces et curseur
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {

          int cx = gridX + col * cellSize + cellSize / 2;
          int cy = gridY + row * cellSize + cellSize / 2;

          // Curseur
          if (col == cursorX && row == cursorY && state == IN_PROGRESS) {
            screen->drawRect(
              gridX + col * cellSize + 2,
              gridY + row * cellSize + 2,
              cellSize - 4,
              cellSize - 4,
              TFT_YELLOW  // curseur jaune
            );
          }

          // Joueur 1
          if (board[row][col] == 1) {
            int m = 8;  // marge
            screen->drawLine(
              gridX + col * cellSize + m,
              gridY + row * cellSize + m,
              gridX + col * cellSize + cellSize - m,
              gridY + row * cellSize + cellSize - m,
              TFT_RED
            );
            screen->drawLine(
              gridX + col * cellSize + cellSize - m,
              gridY + row * cellSize + m,
              gridX + col * cellSize + m,
              gridY + row * cellSize + cellSize - m,
              TFT_RED
            );
          }

          // Joueur 2
          if (board[row][col] == 2) {
            screen->drawCircle(cx, cy, cellSize / 2 - 6, TFT_CYAN);
          }
        }
      }
    }

    virtual ~MorpionGame() {}
};

#endif