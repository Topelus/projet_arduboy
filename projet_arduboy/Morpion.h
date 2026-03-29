#ifndef MORPION_H
#define MORPION_H

#include "Game.h"

class MorpionGame : public Game {
private:
  bool needsRedraw;
  int board[3][3];       // 0=vide, 1=Joueur (X), 2=IA (O)
  int cursorX, cursorY;
  int currentPlayer;           // 1 = joueur, 2 = IA
  int result;                  // 0=en cours, 1=joueur gagne, 2=IA gagne, 3=nul
  bool waitingForAI;
  unsigned long lastMoveTime;

  // ── Ligne gagnante (pour la mise en évidence) ─────────────
  // Stocke les 3 cases [row][col] de la combinaison gagnante
  int winCells[3][2];          // { {r0,c0}, {r1,c1}, {r2,c2} }
  bool hasWinLine;             // true si une ligne gagnante a été trouvée

  const int cellSize = 36;
  const int gridX = (240 - 36 * 3) / 2;       // = 54
  const int gridY = (135 - 36 * 3) / 2 + 10;  // = 22

  // ── Vérifie s'il y a un gagnant et remplit winCells ───────
  // Retourne 1 (joueur), 2 (IA) ou 0 (aucun)
  int checkWinner() {
    // Lignes
    for (int row = 0; row < 3; row++) {
      if (board[row][0] != 0 &&
          board[row][0] == board[row][1] &&
          board[row][1] == board[row][2]) {
        winCells[0][0] = row; winCells[0][1] = 0;
        winCells[1][0] = row; winCells[1][1] = 1;
        winCells[2][0] = row; winCells[2][1] = 2;
        hasWinLine = true;
        return board[row][0];
      }
    }
    // Colonnes
    for (int col = 0; col < 3; col++) {
      if (board[0][col] != 0 &&
          board[0][col] == board[1][col] &&
          board[1][col] == board[2][col]) {
        winCells[0][0] = 0; winCells[0][1] = col;
        winCells[1][0] = 1; winCells[1][1] = col;
        winCells[2][0] = 2; winCells[2][1] = col;
        hasWinLine = true;
        return board[0][col];
      }
    }
    // Diagonale haut-gauche vers bas-droite
    if (board[0][0] != 0 &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2]) {
      winCells[0][0] = 0; winCells[0][1] = 0;
      winCells[1][0] = 1; winCells[1][1] = 1;
      winCells[2][0] = 2; winCells[2][1] = 2;
      hasWinLine = true;
      return board[0][0];
    }
    // Diagonale haut-droite vers bas-gauche
    if (board[0][2] != 0 &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0]) {
      winCells[0][0] = 0; winCells[0][1] = 2;
      winCells[1][0] = 1; winCells[1][1] = 1;
      winCells[2][0] = 2; winCells[2][1] = 0;
      hasWinLine = true;
      return board[0][2];
    }
    hasWinLine = false;
    return 0;
  }

  bool checkDraw() {
    for (int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
        if (board[row][col] == 0) return false;
    return true;
  }

  // ── Vérifie si une case fait partie de la ligne gagnante ──
  bool isWinCell(int row, int col) {
    if (!hasWinLine) return false;
    for (int i = 0; i < 3; i++)
      if (winCells[i][0] == row && winCells[i][1] == col) return true;
    return false;
  }

  // ── IA ────────────────────────────────────────────────────
  void aiMove() {
    // 1. Coup gagnant
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == 0) {
          board[row][col] = 2;
          if (checkWinner() == 2) return;
          board[row][col] = 0;
        }
      }
    }
    hasWinLine = false;

    // 2. Bloquer le joueur
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (board[row][col] == 0) {
          board[row][col] = 1;
          if (checkWinner() == 1) { board[row][col] = 2; hasWinLine = false; return; }
          board[row][col] = 0;
        }
      }
    }
    hasWinLine = false;

    // 3. Centre
    if (board[1][1] == 0) { board[1][1] = 2; return; }

    // 4. Coin
    int corners[4][2] = { {0,0},{0,2},{2,0},{2,2} };
    for (int i = 0; i < 4; i++) {
      int r = corners[i][0], c = corners[i][1];
      if (board[r][c] == 0) { board[r][c] = 2; return; }
    }

    // 5. Premier vide
    for (int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
        if (board[row][col] == 0) { board[row][col] = 2; return; }
  }

  // ── Écran de victoire du joueur ───────────────────────────
  void drawVictoryScreen() {
    screen->fillScreen(TFT_BLACK);

    // Bannière verte en haut
    screen->fillRect(0, 0, 240, 28, TFT_DARKGREEN);
    screen->setTextColor(TFT_YELLOW, TFT_DARKGREEN);
    screen->setTextSize(2);
    screen->setCursor(18, 6);
    screen->print("BRAVO ! VICTOIRE !");

    // Grille avec la ligne gagnante surlignée
    // Lignes de grille
    screen->drawLine(gridX + cellSize,     gridY, gridX + cellSize,     gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX + cellSize * 2, gridY, gridX + cellSize * 2, gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize,     gridX + cellSize * 3, gridY + cellSize,     TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize * 2, gridX + cellSize * 3, gridY + cellSize * 2, TFT_WHITE);

    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        int cx = gridX + col * cellSize + cellSize / 2;
        int cy = gridY + row * cellSize + cellSize / 2;

        // Fond vert sur les cases gagnantes
        if (isWinCell(row, col)) {
          screen->fillRect(
            gridX + col * cellSize + 1,
            gridY + row * cellSize + 1,
            cellSize - 2, cellSize - 2,
            TFT_DARKGREEN);
        }

        if (board[row][col] == 1) {
          // X joueur : rouge si normal, jaune si gagnant
          uint16_t col16 = isWinCell(row, col) ? TFT_YELLOW : TFT_RED;
          int m = 8;
          screen->drawLine(
            gridX + col * cellSize + m, gridY + row * cellSize + m,
            gridX + col * cellSize + cellSize - m, gridY + row * cellSize + cellSize - m,
            col16);
          screen->drawLine(
            gridX + col * cellSize + cellSize - m, gridY + row * cellSize + m,
            gridX + col * cellSize + m, gridY + row * cellSize + cellSize - m,
            col16);
        } else if (board[row][col] == 2) {
          screen->drawCircle(cx, cy, cellSize / 2 - 6, TFT_CYAN);
        }
      }
    }

    // Score et message bas
    screen->setTextColor(TFT_WHITE, TFT_BLACK);
    screen->setTextSize(1);
    screen->setCursor(40, 122);
    screen->print("Score : ");
    screen->setTextColor(TFT_YELLOW, TFT_BLACK);
    screen->print(score);
    screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
    screen->setCursor(130, 122);
    screen->print("[A] Rejouer");
  }

  // ── Écran de défaite / match nul ─────────────────────────
  void drawEndScreen() {
    screen->fillScreen(TFT_BLACK);

    if (result == 2) {
      // Défaite : bannière rouge
      screen->fillRect(0, 0, 240, 28, TFT_MAROON);
      screen->setTextColor(TFT_WHITE, TFT_MAROON);
      screen->setTextSize(2);
      screen->setCursor(35, 6);
      screen->print("IA a gagne !");
    } else {
      // Match nul : bannière grise
      screen->fillRect(0, 0, 240, 28, TFT_DARKGREY);
      screen->setTextColor(TFT_WHITE, TFT_DARKGREY);
      screen->setTextSize(2);
      screen->setCursor(45, 6);
      screen->print("Match nul !");
    }

    // Grille finale (cases gagnantes de l'IA surlignées en rouge)
    screen->drawLine(gridX + cellSize,     gridY, gridX + cellSize,     gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX + cellSize * 2, gridY, gridX + cellSize * 2, gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize,     gridX + cellSize * 3, gridY + cellSize,     TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize * 2, gridX + cellSize * 3, gridY + cellSize * 2, TFT_WHITE);

    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        int cx = gridX + col * cellSize + cellSize / 2;
        int cy = gridY + row * cellSize + cellSize / 2;

        // Cases gagnantes de l'IA : fond rouge foncé
        if (result == 2 && isWinCell(row, col)) {
          screen->fillRect(
            gridX + col * cellSize + 1,
            gridY + row * cellSize + 1,
            cellSize - 2, cellSize - 2,
            TFT_MAROON);
        }

        if (board[row][col] == 1) {
          int m = 8;
          screen->drawLine(
            gridX + col * cellSize + m, gridY + row * cellSize + m,
            gridX + col * cellSize + cellSize - m, gridY + row * cellSize + cellSize - m,
            TFT_RED);
          screen->drawLine(
            gridX + col * cellSize + cellSize - m, gridY + row * cellSize + m,
            gridX + col * cellSize + m, gridY + row * cellSize + cellSize - m,
            TFT_RED);
        } else if (board[row][col] == 2) {
          uint16_t col16 = (result == 2 && isWinCell(row, col)) ? TFT_WHITE : TFT_CYAN;
          screen->drawCircle(cx, cy, cellSize / 2 - 6, col16);
        }
      }
    }

    screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
    screen->setTextSize(1);
    screen->setCursor(130, 122);
    screen->print("[A] Rejouer");
  }

public:
  void forceRedraw() override { needsRedraw = true; }

  MorpionGame(TFT_eSPI* display)
    : Game(display) {
    needsRedraw  = true;
    waitingForAI = false;
    hasWinLine   = false;
  }

  void init() override {
    for (int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
        board[row][col] = 0;

    cursorX       = 1;
    cursorY       = 1;
    currentPlayer = 1;
    result        = 0;
    score         = 0;
    state         = IN_PROGRESS;
    needsRedraw   = true;
    waitingForAI  = false;
    hasWinLine    = false;

    drawScoreLabel();
  }

  void update(Buttons buttons) override {
    // Bloquer sur VICTORY et GAME_OVER
    if (state == GAME_OVER || state == VICTORY) return;

    // ── Délai IA ──────────────────────────────────────────────
    if (waitingForAI && (millis() - lastMoveTime >= 1000)) {
      waitingForAI = false;
      aiMove();
      needsRedraw = true;

      result = checkWinner();
      if (result != 0) {
        if (result == 1) {
          score = 1;
          state = VICTORY;   // ← Joueur gagne
        } else {
          score = 0;
          state = GAME_OVER; // ← IA gagne
        }
        return;
      }
      if (checkDraw()) {
        result = 3;
        score  = 0;
        state  = GAME_OVER;  // ← Match nul
        return;
      }
      currentPlayer = 1;
    }

    // ── Tour du joueur ────────────────────────────────────────
    if (currentPlayer == 1 && state == IN_PROGRESS) {
      bool changed = false;

      if (buttons.upPressed    && cursorY > 0) { cursorY--; changed = true; }
      else if (buttons.downPressed  && cursorY < 2) { cursorY++; changed = true; }
      else if (buttons.leftPressed  && cursorX > 0) { cursorX--; changed = true; }
      else if (buttons.rightPressed && cursorX < 2) { cursorX++; changed = true; }
      else if (buttons.aPressed) {
        if (board[cursorY][cursorX] == 0) {
          board[cursorY][cursorX] = 1;
          changed = true;

          result = checkWinner();
          if (result != 0) {
            // result == 1 forcément ici (le joueur vient de jouer)
            score = 1;
            state = VICTORY;   // ← VICTORY, pas GAME_OVER
            needsRedraw = true;
            return;
          }
          if (checkDraw()) {
            result = 3;
            score  = 0;
            state  = GAME_OVER;
            needsRedraw = true;
            return;
          }
          currentPlayer = 2;
          waitingForAI  = true;
          lastMoveTime  = millis();
        }
      }

      if (changed) needsRedraw = true;
    }
  }

  void render() override {
    if (!needsRedraw) return;
    needsRedraw = false;

    // ── Écran de victoire joueur ──────────────────────────────
    if (state == VICTORY) {
      drawVictoryScreen();
      return;
    }

    // ── Écran de défaite / nul ────────────────────────────────
    if (state == GAME_OVER) {
      drawEndScreen();
      return;
    }

    // ── Partie en cours ───────────────────────────────────────
    screen->fillScreen(TFT_BLACK);
    displayScore();

    screen->setTextSize(1);
    if (currentPlayer == 1 && !waitingForAI) {
      screen->setTextColor(TFT_RED, TFT_BLACK);
      screen->setCursor(5, 4);
      screen->print("A vous (X)");
    } else {
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setCursor(5, 4);
      screen->print("IA reflechit...");
    }

    // Lignes de grille
    screen->drawLine(gridX + cellSize,     gridY, gridX + cellSize,     gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX + cellSize * 2, gridY, gridX + cellSize * 2, gridY + cellSize * 3, TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize,     gridX + cellSize * 3, gridY + cellSize,     TFT_WHITE);
    screen->drawLine(gridX, gridY + cellSize * 2, gridX + cellSize * 3, gridY + cellSize * 2, TFT_WHITE);

    // Cellules
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        int cx = gridX + col * cellSize + cellSize / 2;
        int cy = gridY + row * cellSize + cellSize / 2;

        // Curseur
        if (col == cursorX && row == cursorY &&
            state == IN_PROGRESS && currentPlayer == 1 && !waitingForAI) {
          screen->drawRect(
            gridX + col * cellSize + 2,
            gridY + row * cellSize + 2,
            cellSize - 4, cellSize - 4,
            TFT_YELLOW);
        }

        if (board[row][col] == 1) {
          int m = 8;
          screen->drawLine(
            gridX + col * cellSize + m,          gridY + row * cellSize + m,
            gridX + col * cellSize + cellSize - m, gridY + row * cellSize + cellSize - m,
            TFT_RED);
          screen->drawLine(
            gridX + col * cellSize + cellSize - m, gridY + row * cellSize + m,
            gridX + col * cellSize + m,            gridY + row * cellSize + cellSize - m,
            TFT_RED);
        } else if (board[row][col] == 2) {
          screen->drawCircle(cx, cy, cellSize / 2 - 6, TFT_CYAN);
        }
      }
    }
  }

  virtual ~MorpionGame() {}
};

#endif