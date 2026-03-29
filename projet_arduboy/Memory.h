// ============================================================
//  Memory.h — Jeu de paires (Memory)
//  HAUT/BAS/GAUCHE/DROITE : déplacer le curseur
//  A    : retourner une carte
//  Score : paires trouvées × 10
//  Game Over : toutes les paires trouvées
// ============================================================
#ifndef MEMORY_H
#define MEMORY_H

#include "Game.h"

class MemoryGame : public Game {
private:

  // ── Grille 4×4 = 16 cartes = 8 paires ────────────────
  static const int GRID_COLS = 4;
  static const int GRID_ROWS = 4;
  static const int CARD_COUNT = GRID_COLS * GRID_ROWS;

  // ── Taille des cartes ─────────────────────────────────
  // Écran 240×135 : 4 cols × 56px = 224 → marge 8px de chaque côté
  //                 4 rows × 30px = 120 → marge 7px haut et 8px bas
  static const int CARD_W = 54;
  static const int CARD_H = 29;
  static const int CARD_GAP_X = 2;
  static const int CARD_GAP_Y = 2;
  static const int ORIG_X = 7;
  static const int ORIG_Y = 7;

  // ── Données des cartes ────────────────────────────────
  uint8_t cardValue[CARD_COUNT];  // 0..7 (8 paires)
  bool cardFaceUp[CARD_COUNT];    // true = visible
  bool cardMatched[CARD_COUNT];   // true = paire trouvée

  // ── État du jeu ───────────────────────────────────────
  int firstFlipped;  // index de la 1ère carte retournée (-1 si aucune)
  int pairsFound;

  // ── Curseur ───────────────────────────────────────────
  int cursorCol, cursorRow;

  // ── Temporisation pour montrer les 2 cartes ───────────
  unsigned long flipTime;
  bool waitingFlip;  // attend qu'on retourne 2 cartes non-correspondantes

  bool needsFullRedraw;

  // ── Couleurs par valeur (8 couleurs distinctes) ───────
  uint16_t symbolColor(uint8_t val) {
    const uint16_t COLORS[8] = {
      TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW,
      TFT_CYAN, TFT_MAGENTA, TFT_ORANGE, TFT_WHITE
    };
    return COLORS[val & 7];
  }

  // ── Symboles par valeur (dessinés sur la carte) ───────
  void drawSymbol(int px, int py, uint8_t val) {
    int cx = px + CARD_W / 2;
    int cy = py + CARD_H / 2;
    uint16_t col = symbolColor(val);
    switch (val) {
      case 0: screen->fillCircle(cx, cy, 9, col); break;
      case 1: screen->fillTriangle(cx, cy - 9, cx - 8, cy + 8, cx + 8, cy + 8, col); break;
      case 2: screen->fillRect(cx - 8, cy - 8, 16, 16, col); break;
      case 3:  // Losange
        screen->fillTriangle(cx, cy - 10, cx - 8, cy, cx, cy + 10, col);
        screen->fillTriangle(cx, cy - 10, cx + 8, cy, cx, cy + 10, col);
        break;
      case 4:  // Croix
        screen->fillRect(cx - 9, cy - 3, 18, 6, col);
        screen->fillRect(cx - 3, cy - 9, 6, 18, col);
        break;
      case 5:  // Étoile (5 points approchée)
        screen->fillTriangle(cx, cy - 10, cx - 6, cy + 4, cx + 6, cy + 4, col);
        screen->fillTriangle(cx, cy + 11, cx - 6, cy - 3, cx + 6, cy - 3, col);
        break;
      case 6:  // Cœur simplifié
        screen->fillCircle(cx - 4, cy - 2, 5, col);
        screen->fillCircle(cx + 4, cy - 2, 5, col);
        screen->fillTriangle(cx - 9, cy, cx + 9, cy, cx, cy + 11, col);
        break;
      case 7:  // Hexagone
        screen->fillTriangle(cx, cy - 9, cx - 8, cy - 4, cx + 8, cy - 4, col);
        screen->fillRect(cx - 8, cy - 4, 16, 8, col);
        screen->fillTriangle(cx - 8, cy + 4, cx + 8, cy + 4, cx, cy + 9, col);
        break;
    }
  }

  // ── Index de la carte ─────────────────────────────────
  int cardIndex(int col, int row) {
    return row * GRID_COLS + col;
  }

  // ── Position pixel d'une carte ────────────────────────
  int cardPX(int col) {
    return ORIG_X + col * (CARD_W + CARD_GAP_X);
  }
  int cardPY(int row) {
    return ORIG_Y + row * (CARD_H + CARD_GAP_Y);
  }

  // ── Dessiner une carte ────────────────────────────────
  void drawCard(int col, int row) {
    int idx = cardIndex(col, row);
    int px = cardPX(col), py = cardPY(row);
    bool sel = (col == cursorCol && row == cursorRow);

    if (cardMatched[idx]) {
      // Paire trouvée : fond vert foncé
      screen->fillRect(px, py, CARD_W, CARD_H, 0x03E0);
      drawSymbol(px, py, cardValue[idx]);
      if (sel) screen->drawRect(px, py, CARD_W, CARD_H, TFT_YELLOW);
      else screen->drawRect(px, py, CARD_W, CARD_H, 0x0180);
    } else if (cardFaceUp[idx]) {
      // Carte retournée : fond blanc
      screen->fillRect(px, py, CARD_W, CARD_H, 0x2945);
      drawSymbol(px, py, cardValue[idx]);
      if (sel) screen->drawRect(px, py, CARD_W, CARD_H, TFT_YELLOW);
      else screen->drawRect(px, py, CARD_W, CARD_H, TFT_WHITE);
    } else {
      // Carte face cachée : fond bleu
      screen->fillRect(px, py, CARD_W, CARD_H, 0x1082);
      // Motif de dos de carte
      screen->drawRect(px + 3, py + 3, CARD_W - 6, CARD_H - 6, 0x294A);
      if (sel) screen->drawRect(px, py, CARD_W, CARD_H, TFT_YELLOW);
      else screen->drawRect(px, py, CARD_W, CARD_H, 0x4208);
    }
  }

  // ── Mélanger les cartes (Fisher-Yates) ────────────────
  void shuffleCards() {
    // Assigner les paires
    for (int i = 0; i < CARD_COUNT; i++)
      cardValue[i] = i / 2;  // 0,0,1,1,2,2,...,7,7

    // Mélanger
    for (int i = CARD_COUNT - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t tmp = cardValue[i];
      cardValue[i] = cardValue[j];
      cardValue[j] = tmp;
    }
  }

public:
  // ✅ Constructeur corrigé : initialise tous les membres
  MemoryGame(TFT_eSPI* display)
    : Game(display),
      firstFlipped(-1), pairsFound(0),
      cursorCol(0), cursorRow(0),
      flipTime(0), waitingFlip(false),
      needsFullRedraw(true) {
    for (int i = 0; i < CARD_COUNT; i++) {
      cardValue[i] = 0;
      cardFaceUp[i] = false;
      cardMatched[i] = false;
    }
  }
  void forceRedraw() override {
    needsFullRedraw = true;
  }

  void init() override {
    shuffleCards();
    for (int i = 0; i < CARD_COUNT; i++) {
      cardFaceUp[i] = false;
      cardMatched[i] = false;
    }
    cursorCol = 0;
    cursorRow = 0;
    firstFlipped = -1;
    pairsFound = 0;
    score = 0;
    state = IN_PROGRESS;
    waitingFlip = false;
    needsFullRedraw = true;
  }

  void update(Buttons buttons) override {
    if (state == GAME_OVER) return;

    // ── Attente après 2 cartes non-correspondantes ─────
    if (waitingFlip) {
      if (millis() - flipTime >= 800) {
        // Retourner les 2 cartes
        for (int i = 0; i < CARD_COUNT; i++) {
          if (cardFaceUp[i] && !cardMatched[i]) {
            cardFaceUp[i] = false;
            drawCard(i % GRID_COLS, i / GRID_COLS);
          }
        }
        firstFlipped = -1;
        waitingFlip = false;
      }
      return;
    }

    // ── Navigation ────────────────────────────────────
    bool moved = false;
    int pc = cursorCol, pr = cursorRow;

    if (buttons.upPressed && cursorRow > 0) {
      cursorRow--;
      moved = true;
    }
    if (buttons.downPressed && cursorRow < GRID_ROWS - 1) {
      cursorRow++;
      moved = true;
    }
    if (buttons.leftPressed && cursorCol > 0) {
      cursorCol--;
      moved = true;
    }
    if (buttons.rightPressed && cursorCol < GRID_COLS - 1) {
      cursorCol++;
      moved = true;
    }

    if (moved) {
      drawCard(pc, pr);                // redessiner ancienne case (enlever curseur)
      drawCard(cursorCol, cursorRow);  // dessiner nouvelle case (avec curseur)
    }

    // ── Retourner une carte ────────────────────────────
    if (buttons.aPressed) {
      int idx = cardIndex(cursorCol, cursorRow);
      if (cardFaceUp[idx] || cardMatched[idx]) return;  // déjà visible

      cardFaceUp[idx] = true;
      drawCard(cursorCol, cursorRow);

      if (firstFlipped == -1) {
        // Première carte de la paire
        firstFlipped = idx;
      } else {
        // Deuxième carte
        if (cardValue[firstFlipped] == cardValue[idx]) {
          // Paire trouvée !
          cardMatched[firstFlipped] = true;
          cardMatched[idx] = true;
          pairsFound++;
          score += 10;

          // Redessiner les 2 cartes en vert
          drawCard(firstFlipped % GRID_COLS, firstFlipped / GRID_COLS);
          drawCard(cursorCol, cursorRow);

          firstFlipped = -1;

          // Mise à jour score
          screen->fillRect(0, 0, 240, 6, TFT_BLACK);
          screen->setTextColor(TFT_WHITE, TFT_BLACK);
          screen->setTextSize(1);
          // (affiché dans render initial, pas besoin de répéter)

          if (pairsFound == CARD_COUNT / 2) {
            state = GAME_OVER;
          }
        } else {
          // Pas une paire → attendre avant de retourner
          waitingFlip = true;
          flipTime = millis();
        }
      }
    }
  }

  void render() override {
    if (!needsFullRedraw) return;
    needsFullRedraw = false;
    screen->fillScreen(TFT_BLACK);

    for (int r = 0; r < GRID_ROWS; r++)
      for (int c = 0; c < GRID_COLS; c++)
        drawCard(c, r);
  }

  virtual ~MemoryGame() {}
};

#endif