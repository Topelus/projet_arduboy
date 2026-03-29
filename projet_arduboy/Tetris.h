#ifndef TETRIS_H
#define TETRIS_H

#include "Game.h"

//  TETRIS — Aligne les lignes pour marquer des points
//  Boutons : GAUCHE / DROITE pour déplacer
//            HAUT pour rotation
//            BAS pour accélérer la chute
//            A pour rotation (alternative)
//  Game Over : si une pièce dépasse le haut de la grille

class TetrisGame : public Game {
  private:

    // ── Grille de jeu ──
    static const int COLS   = 10;   // colonnes
    static const int ROWS   = 18;   // rangées visibles
    static const int CELL   =  7;   // taille d'une cellule en pixels

    // Origine de la grille (centrée horizontalement)
    const int gridX = (240 - COLS * CELL) / 2;  // = 85px
    const int gridY = 135 - ROWS * CELL;         // = 9px

    uint8_t board[ROWS][COLS];  // 0 = vide, 1-7 = couleur de pièce

    // ── Pièces Tetromino ──
    // 7 pièces, 4 rotations, forme 4×4
    // Encodées en bits (ligne par ligne, 4 bits par ligne)
    static const uint8_t PIECES[7][4][4];

    // Couleurs des 7 pièces
    const uint16_t pieceColors[8] = {
      TFT_BLACK,    // 0 = vide
      TFT_CYAN,     // I
      TFT_BLUE,     // J
      TFT_ORANGE,   // L
      TFT_YELLOW,   // O
      TFT_GREEN,    // S
      TFT_MAGENTA,  // T
      TFT_RED       // Z
    };

    // ── Pièce courante ──
    int pieceType;    // 0-6
    int pieceRot;     // 0-3
    int pieceX;       // colonne (grille)
    int pieceY;       // rangée  (grille)

    // ── Pièce suivante ──
    int nextType;

    // ── Timing ──
    unsigned long lastFall;     // dernier déplacement vers le bas
    unsigned long lastInput;    // anti-rebond navigation
    int  fallInterval;          // ms entre chaque chute (diminue avec le niveau)

    const int FALL_BASE  = 500;  // ms au niveau 1
    const int FALL_MIN   =  80;  // ms au niveau max
    const int INPUT_DELAY = 120; // ms anti-rebond boutons

    // ── Niveau ──
    int level;
    int linesCleared;

    // ── Rendu ──
    bool needsRedraw;     // redessin complet demandé
    bool firstDraw;

    // ──────────────────────────────────────────────
    //  Forme d'une pièce : retourne true si la
    //  cellule (row, col) de la pièce est pleine
    // ──────────────────────────────────────────────
    bool cellFilled(int type, int rot, int row, int col) {
      return (PIECES[type][rot][row] >> (3 - col)) & 1;
    }

    // ──────────────────────────────────────────────
    //  Vérifier si la pièce peut se placer en (px, py)
    // ──────────────────────────────────────────────
    bool canPlace(int type, int rot, int px, int py) {
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          if (!cellFilled(type, rot, r, c)) continue;
          int gx = px + c;
          int gy = py + r;
          if (gx < 0 || gx >= COLS) return false;
          if (gy >= ROWS)           return false;
          if (gy >= 0 && board[gy][gx] != 0) return false;
        }
      }
      return true;
    }

    // ──────────────────────────────────────────────
    //  Coller la pièce courante dans la grille
    // ──────────────────────────────────────────────
    void lockPiece() {
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          if (!cellFilled(pieceType, pieceRot, r, c)) continue;
          int gy = pieceY + r;
          int gx = pieceX + c;
          if (gy >= 0 && gy < ROWS && gx >= 0 && gx < COLS) {
            board[gy][gx] = pieceType + 1;  // 1-7
          }
        }
      }
    }

    // ──────────────────────────────────────────────
    //  Effacer les lignes complètes
    //  Retourne le nombre de lignes effacées
    // ──────────────────────────────────────────────
    int clearLines() {
      int cleared = 0;
      for (int r = ROWS - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < COLS; c++) {
          if (board[r][c] == 0) { full = false; break; }
        }
        if (full) {
          cleared++;
          // Faire descendre toutes les rangées au-dessus
          for (int rr = r; rr > 0; rr--) {
            for (int c = 0; c < COLS; c++) {
              board[rr][c] = board[rr - 1][c];
            }
          }
          // Vider la rangée du haut
          for (int c = 0; c < COLS; c++) board[0][c] = 0;
          r++;  // re-vérifier la même rangée
        }
      }
      return cleared;
    }

    // ──────────────────────────────────────────────
    //  Piocher une nouvelle pièce aléatoire
    // ──────────────────────────────────────────────
    void spawnPiece() {
      pieceType = nextType;
      nextType  = random(0, 7);
      pieceRot  = 0;
      pieceX    = COLS / 2 - 2;  // centré
      pieceY    = -1;             // légèrement au-dessus

      // Game Over si impossible de placer
      if (!canPlace(pieceType, pieceRot, pieceX, pieceY)) {
        state = GAME_OVER;
      }
    }

    // ──────────────────────────────────────────────
    //  Calculer l'intervalle de chute selon le niveau
    // ──────────────────────────────────────────────
    int getFallInterval() {
      int interval = FALL_BASE - (level - 1) * 45;
      return max(interval, FALL_MIN);
    }

    // ──────────────────────────────────────────────
    //  Dessiner une cellule de la grille
    // ──────────────────────────────────────────────
    void drawCell(int gx, int gy, uint16_t color) {
      int px = gridX + gx * CELL;
      int py = gridY + gy * CELL;
      screen->fillRect(px + 1, py + 1, CELL - 1, CELL - 1, color);
    }

    // ──────────────────────────────────────────────
    //  Dessiner / effacer la pièce courante
    // ──────────────────────────────────────────────
    void drawPiece(int type, int rot, int px, int py, uint16_t color) {
      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          if (!cellFilled(type, rot, r, c)) continue;
          int gy = py + r;
          int gx = px + c;
          if (gy >= 0 && gy < ROWS && gx >= 0 && gx < COLS) {
            drawCell(gx, gy, color);
          }
        }
      }
    }

    // ──────────────────────────────────────────────
    //  Dessiner la grille complète (fond + toutes les cellules)
    // ──────────────────────────────────────────────
    void drawBoard() {
      // Fond de la grille
      screen->fillRect(gridX, gridY, COLS * CELL, ROWS * CELL, TFT_BLACK);
      // Bordure
      screen->drawRect(gridX - 1, gridY - 1, COLS * CELL + 2, ROWS * CELL + 2, TFT_DARKGREY);

      // Cellules posées
      for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
          if (board[r][c] != 0) {
            drawCell(c, r, pieceColors[board[r][c]]);
          }
        }
      }
    }

    // ──────────────────────────────────────────────
    //  Afficher le panneau "NEXT" + score + niveau
    // ──────────────────────────────────────────────
    void drawPanel() {
      int panelX = gridX + COLS * CELL + 6;  // à droite de la grille

      // Effacer le panneau
      screen->fillRect(panelX, 0, 240 - panelX, 135, TFT_BLACK);

      // NEXT
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(panelX, 4);
      screen->print("NEXT");

      // Aperçu pièce suivante (zone 4×4 cellules réduite)
      const int previewCell = 5;
      int previewX = panelX;
      int previewY = 14;
      screen->fillRect(previewX, previewY, 4 * previewCell + 2, 4 * previewCell + 2, TFT_BLACK);
      screen->drawRect(previewX - 1, previewY - 1, 4 * previewCell + 2, 4 * previewCell + 2, TFT_DARKGREY);

      for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
          if (cellFilled(nextType, 0, r, c)) {
            screen->fillRect(
              previewX + c * previewCell + 1,
              previewY + r * previewCell + 1,
              previewCell - 1, previewCell - 1,
              pieceColors[nextType + 1]
            );
          }
        }
      }

      // Score
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(panelX, 42);
      screen->print("SCORE");
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(panelX, 52);
      screen->print(score);

      // Niveau
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setCursor(panelX, 68);
      screen->print("NIV.");
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(panelX, 78);
      screen->print(level);

      // Lignes
      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setCursor(panelX, 94);
      screen->print("LGN.");
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(panelX, 104);
      screen->print(linesCleared);
    }

  public:
    TetrisGame(TFT_eSPI* display) : Game(display) {}

    //  Initialisation
    void init() override {
      // Vider la grille
      for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
          board[r][c] = 0;

      score        = 0;
      level        = 1;
      linesCleared = 0;
      state        = IN_PROGRESS;
      firstDraw    = true;
      needsRedraw  = true;
      lastFall     = millis();
      lastInput    = 0;
      nextType     = random(0, 7);

      spawnPiece();
    }

    //  Update
    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      unsigned long now = millis();

      // ── Entrées joueur ──
      if (now - lastInput > (unsigned long)INPUT_DELAY) {

        // Déplacement gauche
        if (buttons.leftPressed || buttons.left) {
          if (canPlace(pieceType, pieceRot, pieceX - 1, pieceY)) {
            drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
            pieceX--;
            drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
          }
          lastInput = now;
        }

        // Déplacement droite
        else if (buttons.rightPressed || buttons.right) {
          if (canPlace(pieceType, pieceRot, pieceX + 1, pieceY)) {
            drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
            pieceX++;
            drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
          }
          lastInput = now;
        }

        // Rotation (HAUT ou A)
        else if (buttons.upPressed || buttons.aPressed) {
          int newRot = (pieceRot + 1) % 4;
          // Wall kick : essayer décalage si bloqué
          if (canPlace(pieceType, newRot, pieceX, pieceY)) {
            drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
            pieceRot = newRot;
            drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
          } else if (canPlace(pieceType, newRot, pieceX + 1, pieceY)) {
            drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
            pieceX++;
            pieceRot = newRot;
            drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
          } else if (canPlace(pieceType, newRot, pieceX - 1, pieceY)) {
            drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
            pieceX--;
            pieceRot = newRot;
            drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
          }
          lastInput = now;
        }

        // Chute rapide (BAS)
        else if (buttons.down) {
          if (canPlace(pieceType, pieceRot, pieceX, pieceY + 1)) {
            drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
            pieceY++;
            drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
            score++;  // bonus de rapidité
          }
          lastInput = now;
        }
      }

      // ── Chute automatique ──
      if (now - lastFall >= (unsigned long)getFallInterval()) {
        lastFall = now;

        if (canPlace(pieceType, pieceRot, pieceX, pieceY + 1)) {
          // Effacer + descendre + redessiner
          drawPiece(pieceType, pieceRot, pieceX, pieceY, TFT_BLACK);
          pieceY++;
          drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
        } else {
          // Poser la pièce
          lockPiece();

          // Effacer les lignes complètes
          int lines = clearLines();
          if (lines > 0) {
            linesCleared += lines;

            // Scoring classique Tetris
            const int pts[5] = {0, 100, 300, 500, 800};
            score += pts[min(lines, 4)] * level;

            // Monter de niveau tous les 10 lignes
            level = linesCleared / 10 + 1;

            // Redessin complet après effacement de lignes
            needsRedraw = true;
          }

          // Piocher la pièce suivante
          spawnPiece();
          drawPanel();
        }
      }

      // Redessin complet si demandé
      if (needsRedraw) {
        needsRedraw = false;
        drawBoard();
        drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
        drawPanel();
      }
    }

    //  Render — le rendu est fait directement dans update()
    //  pour éviter le double-buffering, on fait juste le
    //  premier dessin complet ici
    void render() override {
      if (!firstDraw) return;
      firstDraw = false;

      screen->fillScreen(TFT_BLACK);
      drawBoard();
      drawPiece(pieceType, pieceRot, pieceX, pieceY, pieceColors[pieceType + 1]);
      drawPanel();
    }

    virtual ~TetrisGame() {}
};

// ──────────────────────────────────────────────────────────────
//  Définition statique des 7 pièces × 4 rotations
//  Chaque rangée = 4 bits (MSB à gauche)
//  Exemple : 0b1111 = ████  /  0b0110 = _██_
// ──────────────────────────────────────────────────────────────
const uint8_t TetrisGame::PIECES[7][4][4] = {
  // ── I ──
  {
    { 0b0000, 0b1111, 0b0000, 0b0000 },
    { 0b0010, 0b0010, 0b0010, 0b0010 },
    { 0b0000, 0b1111, 0b0000, 0b0000 },
    { 0b0010, 0b0010, 0b0010, 0b0010 },
  },
  // ── J ──
  {
    { 0b1000, 0b1110, 0b0000, 0b0000 },
    { 0b0110, 0b0100, 0b0100, 0b0000 },
    { 0b0000, 0b1110, 0b0010, 0b0000 },
    { 0b0100, 0b0100, 0b1100, 0b0000 },
  },
  // ── L ──
  {
    { 0b0010, 0b1110, 0b0000, 0b0000 },
    { 0b0100, 0b0100, 0b0110, 0b0000 },
    { 0b0000, 0b1110, 0b1000, 0b0000 },
    { 0b1100, 0b0100, 0b0100, 0b0000 },
  },
  // ── O ──
  {
    { 0b0110, 0b0110, 0b0000, 0b0000 },
    { 0b0110, 0b0110, 0b0000, 0b0000 },
    { 0b0110, 0b0110, 0b0000, 0b0000 },
    { 0b0110, 0b0110, 0b0000, 0b0000 },
  },
  // ── S ──
  {
    { 0b0110, 0b1100, 0b0000, 0b0000 },
    { 0b0100, 0b0110, 0b0010, 0b0000 },
    { 0b0110, 0b1100, 0b0000, 0b0000 },
    { 0b0100, 0b0110, 0b0010, 0b0000 },
  },
  // ── T ──
  {
    { 0b0100, 0b1110, 0b0000, 0b0000 },
    { 0b0100, 0b0110, 0b0100, 0b0000 },
    { 0b0000, 0b1110, 0b0100, 0b0000 },
    { 0b0100, 0b1100, 0b0100, 0b0000 },
  },
  // ── Z ──
  {
    { 0b1100, 0b0110, 0b0000, 0b0000 },
    { 0b0010, 0b0110, 0b0100, 0b0000 },
    { 0b1100, 0b0110, 0b0000, 0b0000 },
    { 0b0010, 0b0110, 0b0100, 0b0000 },
  },
};

#endif