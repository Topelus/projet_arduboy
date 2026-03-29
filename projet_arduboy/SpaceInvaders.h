#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H

#include "Game.h"

//  SPACE INVADERS — Détruis les envahisseurs avant qu'ils t'atteignent
//  Boutons : GAUCHE / DROITE pour déplacer le vaisseau
//            A pour tirer
//  Game Over : si les envahisseurs atteignent le bas ou si tu perds toutes tes vies

class SpaceInvadersGame : public Game {
  private:

    // ── Dimensions écran ──
    static const int SW = 240;
    static const int SH = 135;

    // ── Vaisseau joueur ──
    int   shipX;
    const int shipY   = SH - 14;
    const int shipW   = 14;
    const int shipH   =  8;
    const int shipSpd =  3;

    // ── Balle joueur ──
    int  bulletX, bulletY;
    bool bulletActive;
    const int bulletH   = 5;
    const int bulletSpd = 5;

    // ── Envahisseurs ──
    static const int INV_COLS = 8;
    static const int INV_ROWS = 3;
    static const int INV_W    = 14;
    static const int INV_H    =  8;
    static const int INV_GAP_X = 8;
    static const int INV_GAP_Y = 10;
    static const int INV_OFF_X = 12;
    static const int INV_OFF_Y = 18;

    bool  invAlive[INV_ROWS][INV_COLS];
    int   invCount;       // envahisseurs restants
    int   invDirX;        // direction horizontale (+1 ou -1)
    float invX;           // décalage X global (float pour vitesse variable)
    int   invY;           // décalage Y global
    float invSpeed;       // vitesse horizontale actuelle
    int   invDropped;     // nombre de descentes effectuées

    // ── Bombes ennemies ──
    static const int BOMB_MAX = 3;
    int  bombX[BOMB_MAX], bombY[BOMB_MAX];
    bool bombActive[BOMB_MAX];
    const int bombSpd = 2;

    // ── Boucliers ──
    static const int SHIELD_COUNT = 3;
    static const int SHIELD_W     = 20;
    static const int SHIELD_H     = 10;
    int shieldHP[SHIELD_COUNT];       // points de vie de chaque bouclier (max 3)
    const int shieldY = SH - 30;

    // ── Vies joueur ──
    int lives;
    const int MAX_LIVES = 3;

    // ── Timing ──
    unsigned long lastUpdate;
    unsigned long lastInvMove;
    unsigned long lastBomb;
    const int updateInterval = 16;   // ~60fps
    int invMoveInterval;             // ms entre chaque mouvement des envahisseurs

    // ── Rendu ──
    bool needsRedraw;
    bool firstDraw;

    // ── Animation envahisseurs ──
    bool invFrame;                   // alterne entre 2 frames d'animation
    unsigned long lastAnimFrame;

    // ────────────────────────────────────────────
    //  Dessiner un envahisseur (2 frames)
    //  Rangée 0 = pieuvre, 1 = crabe, 2 = seiche
    // ────────────────────────────────────────────
    void drawInvader(int x, int y, int row, uint16_t color) {
      screen->fillRect(x, y, INV_W, INV_H, TFT_BLACK); // effacer d'abord

      if (color == TFT_BLACK) return;  // juste effacer

      if (row == 0) {
        // Seiche (rangée haute) — simple et compacte
        screen->fillRect(x + 4, y,     6,  2, color);
        screen->fillRect(x + 2, y + 2, 10, 3, color);
        screen->fillRect(x,     y + 5, 14, 2, color);
        if (invFrame) {
          screen->fillRect(x,     y + 7, 2, 1, color);
          screen->fillRect(x + 12, y + 7, 2, 1, color);
        } else {
          screen->fillRect(x + 2, y + 7, 2, 1, color);
          screen->fillRect(x + 10, y + 7, 2, 1, color);
        }
      } else if (row == 1) {
        // Crabe (rangée milieu)
        screen->fillRect(x + 3, y,     8,  2, color);
        screen->fillRect(x + 1, y + 2, 12, 4, color);
        screen->fillRect(x + 3, y + 6, 8,  2, color);
        if (invFrame) {
          screen->fillRect(x,      y + 4, 1, 3, color);
          screen->fillRect(x + 13, y + 4, 1, 3, color);
        } else {
          screen->fillRect(x,      y + 5, 1, 2, color);
          screen->fillRect(x + 13, y + 5, 1, 2, color);
        }
      } else {
        // Pieuvre (rangée basse)
        screen->fillRect(x + 2, y,     10, 3, color);
        screen->fillRect(x,     y + 3, 14, 3, color);
        screen->fillRect(x + 2, y + 6, 4,  2, color);
        screen->fillRect(x + 8, y + 6, 4,  2, color);
        if (invFrame) {
          screen->fillRect(x,      y + 6, 2, 2, color);
          screen->fillRect(x + 12, y + 6, 2, 2, color);
        }
      }
    }

    // ────────────────────────────────────────────
    //  Dessiner le vaisseau joueur
    // ────────────────────────────────────────────
    void drawShip(int x, uint16_t color) {
      screen->fillRect(x - shipW/2, shipY + 3, shipW,    shipH - 3, color);
      screen->fillRect(x - 2,       shipY,     4,        4,         color);
      // Canon
      screen->fillRect(x - 1, shipY - 2, 2, 3, color);
    }

    // ────────────────────────────────────────────
    //  Dessiner un bouclier
    // ────────────────────────────────────────────
    void drawShield(int idx, bool erase = false) {
      int sx = INV_OFF_X + idx * (SW / SHIELD_COUNT) + (SW / SHIELD_COUNT - SHIELD_W) / 2;
      int sy = shieldY;
      if (erase || shieldHP[idx] <= 0) {
        screen->fillRect(sx, sy, SHIELD_W, SHIELD_H, TFT_BLACK);
        return;
      }
      uint16_t col = (shieldHP[idx] == 3) ? TFT_GREEN :
                     (shieldHP[idx] == 2) ? TFT_YELLOW : TFT_RED;
      screen->fillRect(sx, sy, SHIELD_W, SHIELD_H, col);
      // Entailles selon dégâts
      if (shieldHP[idx] < 3) screen->fillRect(sx + 2, sy, 4, 3, TFT_BLACK);
      if (shieldHP[idx] < 2) screen->fillRect(sx + SHIELD_W - 6, sy + 3, 4, 3, TFT_BLACK);
    }

    // ────────────────────────────────────────────
    //  Dessiner le HUD
    // ────────────────────────────────────────────
    void drawHUD() {
      screen->fillRect(0, 0, SW, 14, TFT_BLACK);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 3);
      screen->print("SCO:");
      screen->print(score);

      // Vies
      screen->setCursor(160, 3);
      screen->print("VIES:");
      for (int i = 0; i < lives; i++) {
        screen->fillTriangle(
          192 + i * 12, shipY - 4,
          192 + i * 12 - 4, shipY + 2,
          192 + i * 12 + 4, shipY + 2,
          TFT_GREEN
        );
        // On réutilise un mini-triangle comme icône vaisseau dans le HUD
        screen->fillRect(192 + i * 12 - 2, 8, 4, 3, TFT_GREEN);
      }
    }

    // ────────────────────────────────────────────
    //  Position X d'un envahisseur
    // ────────────────────────────────────────────
    int invPosX(int col) {
      return INV_OFF_X + col * (INV_W + INV_GAP_X) + (int)invX;
    }

    int invPosY(int row) {
      return INV_OFF_Y + row * (INV_H + INV_GAP_Y) + invY;
    }

    // ────────────────────────────────────────────
    //  Trouver les limites gauche/droite des envahisseurs vivants
    // ────────────────────────────────────────────
    int leftmostCol() {
      for (int c = 0; c < INV_COLS; c++)
        for (int r = 0; r < INV_ROWS; r++)
          if (invAlive[r][c]) return c;
      return 0;
    }

    int rightmostCol() {
      for (int c = INV_COLS - 1; c >= 0; c--)
        for (int r = 0; r < INV_ROWS; r++)
          if (invAlive[r][c]) return c;
      return INV_COLS - 1;
    }

    // ────────────────────────────────────────────
    //  Lancer une bombe depuis un envahisseur aléatoire
    // ────────────────────────────────────────────
    void launchBomb() {
      // Trouver un slot libre
      int slot = -1;
      for (int i = 0; i < BOMB_MAX; i++) {
        if (!bombActive[i]) { slot = i; break; }
      }
      if (slot < 0) return;

      // Choisir un envahisseur vivant au hasard
      int attempts = 0;
      while (attempts < 20) {
        int rc = random(0, INV_ROWS);
        int cc = random(0, INV_COLS);
        if (invAlive[rc][cc]) {
          bombX[slot] = invPosX(cc) + INV_W / 2;
          bombY[slot] = invPosY(rc) + INV_H;
          bombActive[slot] = true;
          return;
        }
        attempts++;
      }
    }

    // ────────────────────────────────────────────
    //  Vérifier si les envahisseurs ont atteint le bas
    // ────────────────────────────────────────────
    bool invadersReachedBottom() {
      for (int r = INV_ROWS - 1; r >= 0; r--) {
        for (int c = 0; c < INV_COLS; c++) {
          if (invAlive[r][c]) {
            if (invPosY(r) + INV_H >= shieldY) return true;
          }
        }
      }
      return false;
    }

    // ────────────────────────────────────────────
    //  Redessiner tous les envahisseurs
    // ────────────────────────────────────────────
    void drawAllInvaders() {
      const uint16_t rowCols[INV_ROWS] = { TFT_CYAN, TFT_GREEN, TFT_RED };
      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          if (invAlive[r][c]) {
            drawInvader(invPosX(c), invPosY(r), r, rowCols[r]);
          } else {
            // Effacer la case
            screen->fillRect(invPosX(c) - 1, invPosY(r) - 1, INV_W + 2, INV_H + 2, TFT_BLACK);
          }
        }
      }
    }

  public:
    SpaceInvadersGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      shipX       = SW / 2;
      bulletActive = false;
      invDirX     = 1;
      invX        = 0;
      invY        = 0;
      invSpeed    = 1.5f;
      invDropped  = 0;
      invFrame    = false;
      lives       = MAX_LIVES;
      score       = 0;
      state       = IN_PROGRESS;
      firstDraw   = true;
      needsRedraw = true;
      lastUpdate  = 0;
      lastInvMove = 0;
      lastBomb    = 0;
      lastAnimFrame = 0;
      invMoveInterval = 600;

      // Initialiser les envahisseurs
      invCount = 0;
      for (int r = 0; r < INV_ROWS; r++)
        for (int c = 0; c < INV_COLS; c++) {
          invAlive[r][c] = true;
          invCount++;
        }

      // Initialiser les bombes
      for (int i = 0; i < BOMB_MAX; i++) bombActive[i] = false;

      // Initialiser les boucliers
      for (int i = 0; i < SHIELD_COUNT; i++) shieldHP[i] = 3;
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      unsigned long now = millis();
      if (now - lastUpdate < (unsigned long)updateInterval) return;
      lastUpdate = now;

      // ── Déplacement vaisseau ──
      int prevShipX = shipX;
      if (buttons.left)  shipX -= shipSpd;
      if (buttons.right) shipX += shipSpd;
      shipX = constrain(shipX, shipW / 2, SW - shipW / 2);

      if (prevShipX != shipX) {
        drawShip(prevShipX, TFT_BLACK);
        drawShip(shipX, TFT_GREEN);
      }

      // ── Tir joueur ──
      if (buttons.aPressed && !bulletActive) {
        bulletX = shipX;
        bulletY = shipY - 2;
        bulletActive = true;
      }

      // ── Mouvement balle joueur ──
      if (bulletActive) {
        screen->fillRect(bulletX - 1, bulletY, 2, bulletH, TFT_BLACK);
        bulletY -= bulletSpd;

        if (bulletY < 14) {
          bulletActive = false;
        } else {
          // Collision balle / envahisseurs
          bool hit = false;
          for (int r = 0; r < INV_ROWS && !hit; r++) {
            for (int c = 0; c < INV_COLS && !hit; c++) {
              if (!invAlive[r][c]) continue;
              int ix = invPosX(c);
              int iy = invPosY(r);
              if (bulletX >= ix && bulletX <= ix + INV_W &&
                  bulletY <= iy + INV_H && bulletY + bulletH >= iy) {
                // Tué !
                invAlive[r][c] = false;
                invCount--;
                screen->fillRect(ix - 1, iy - 1, INV_W + 2, INV_H + 2, TFT_BLACK);
                bulletActive = false;
                hit = true;

                // Points selon la rangée
                const int pts[INV_ROWS] = { 30, 20, 10 };
                score += pts[r];

                // Accélérer les envahisseurs
                invSpeed    = 1.5f + (float)(INV_ROWS * INV_COLS - invCount) * 0.08f;
                invMoveInterval = max(80, 600 - invCount * 18);

                drawHUD();

                // Victoire
                if (invCount <= 0) {
                  score += lives * 100;
                  state = GAME_OVER;
                  return;
                }
              }
            }
          }

          // Collision balle / boucliers
          if (bulletActive) {
            for (int i = 0; i < SHIELD_COUNT; i++) {
              if (shieldHP[i] <= 0) continue;
              int sx = INV_OFF_X + i * (SW / SHIELD_COUNT) + (SW / SHIELD_COUNT - SHIELD_W) / 2;
              if (bulletX >= sx && bulletX <= sx + SHIELD_W &&
                  bulletY <= shieldY + SHIELD_H && bulletY + bulletH >= shieldY) {
                shieldHP[i]--;
                drawShield(i);
                bulletActive = false;
                break;
              }
            }
          }

          if (bulletActive) {
            screen->fillRect(bulletX - 1, bulletY, 2, bulletH, TFT_WHITE);
          }
        }
      }

      // ── Mouvement des envahisseurs ──
      if (now - lastInvMove >= (unsigned long)invMoveInterval) {
        lastInvMove = now;

        // Alterner frame d'animation
        invFrame = !invFrame;
        if (now - lastAnimFrame > 300) {
          lastAnimFrame = now;
        }

        // Détecter si un bord est atteint
        int lx = invPosX(leftmostCol());
        int rx = invPosX(rightmostCol()) + INV_W;

        bool hitWall = (invDirX > 0 && rx + (int)invSpeed >= SW - 2) ||
                       (invDirX < 0 && lx - (int)invSpeed <= 2);

        if (hitWall) {
          invDirX = -invDirX;
          invY   += 6;  // descendre
          invDropped++;

          if (invadersReachedBottom()) {
            state = GAME_OVER;
            return;
          }
        } else {
          invX += invDirX * invSpeed;
        }

        drawAllInvaders();
      }

      // ── Bombes ennemies ──
      // Lancer une nouvelle bombe aléatoirement
      if (now - lastBomb > (unsigned long)(1200 - invDropped * 80)) {
        lastBomb = now;
        launchBomb();
      }

      for (int i = 0; i < BOMB_MAX; i++) {
        if (!bombActive[i]) continue;

        // Effacer
        screen->fillRect(bombX[i] - 1, bombY[i], 2, 5, TFT_BLACK);
        bombY[i] += bombSpd;

        if (bombY[i] > SH) {
          bombActive[i] = false;
          continue;
        }

        // Collision bombe / bouclier
        bool hitShield = false;
        for (int s = 0; s < SHIELD_COUNT; s++) {
          if (shieldHP[s] <= 0) continue;
          int sx = INV_OFF_X + s * (SW / SHIELD_COUNT) + (SW / SHIELD_COUNT - SHIELD_W) / 2;
          if (bombX[i] >= sx && bombX[i] <= sx + SHIELD_W &&
              bombY[i] >= shieldY && bombY[i] <= shieldY + SHIELD_H) {
            shieldHP[s]--;
            drawShield(s);
            bombActive[i] = false;
            hitShield = true;
            break;
          }
        }
        if (hitShield) continue;

        // Collision bombe / vaisseau
        if (bombY[i] >= shipY &&
            bombX[i] >= shipX - shipW / 2 &&
            bombX[i] <= shipX + shipW / 2) {
          bombActive[i] = false;
          lives--;
          drawShip(shipX, TFT_BLACK);

          // Flash rouge
          screen->fillRect(shipX - shipW/2 - 2, shipY - 2, shipW + 4, shipH + 6, TFT_RED);
          delay(120);
          screen->fillRect(shipX - shipW/2 - 2, shipY - 2, shipW + 4, shipH + 6, TFT_BLACK);

          drawHUD();

          if (lives <= 0) {
            state = GAME_OVER;
            return;
          }
          drawShip(shipX, TFT_GREEN);
          continue;
        }

        // Dessiner la bombe (zigzag)
        uint16_t bombColor = (bombY[i] / 3 % 2 == 0) ? TFT_YELLOW : TFT_RED;
        screen->fillRect(bombX[i] - 1, bombY[i], 2, 5, bombColor);
      }
    }

    void render() override {
      if (!firstDraw) return;
      firstDraw = false;

      screen->fillScreen(TFT_BLACK);

      // Fond étoilé
      for (int i = 0; i < 30; i++) {
        int sx = random(0, SW);
        int sy = random(14, SH - 15);
        screen->drawPixel(sx, sy, TFT_DARKGREY);
      }

      drawHUD();
      drawAllInvaders();
      drawShip(shipX, TFT_GREEN);
      for (int i = 0; i < SHIELD_COUNT; i++) drawShield(i);
    }

    virtual ~SpaceInvadersGame() {}
};

#endif