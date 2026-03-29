// ============================================================
//  SpaceInvaders.h — Space Invaders 3 Lignes Descendantes
//  GAUCHE/DROITE : déplacer le canon
//  A             : tirer
//  Score         : points par envahisseur détruit
//  Game Over     : vie = 0 OU ligne touche le joueur
//
//  LOGIQUE :
//   - 3 lignes descendent continuellement (mouvement latéral + descente)
//   - Chaque ligne est indépendante (pas de formation rigide)
//   - Quand une ligne franchit PLAYER_Y → Game Over
//   - Quand toute une ligne est détruite par les balles → nouvelle ligne
//     générée en haut avec un nouveau type
// ============================================================
#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H

#include "Game.h"

class SpaceInvadersGame : public Game {
  private:

    static const int SCREEN_W = 240;
    static const int SCREEN_H = 135;

    static const int INV_COLS    = 8;
    static const int INV_ROWS    = 3;
    static const int INV_W       = 16;
    static const int INV_H       = 10;
    static const int INV_GAP_X   = 6;
    // Espacement vertical entre les lignes (centre à centre)
    static const int INV_STRIDE_Y = 22;
    static const int INV_ORIG_X  = 12;
    static const int INV_STEP    = 3;       // Pas horizontal par tick
    static const int INV_MARGIN  = 4;
    static const int INV_DESCENT = 1;       // Pixels de descente par tick (continu)

    static const int INTERVAL_MAX = 160;    // ms entre chaque tick de mouvement
    static const int INTERVAL_MIN =  60;

    static const int PLAYER_Y    = SCREEN_H - 16;
    static const int PLAYER_W    = 16;
    static const int PLAYER_H    = 8;
    static const int PLAYER_STEP = 4;

    static const int MAX_BULLETS  = 3;
    static const int BULLET_SPEED = 5;
    static const int BULLET_W     = 2;
    static const int BULLET_H     = 6;

    static const int MAX_BOMBS    = 4;
    static const int BOMB_SPEED   = 2;
    static const int BOMB_W       = 2;
    static const int BOMB_H       = 6;

    static const int MAX_LIVES    = 3;
    static const int FRAME_MS     = 16;

    // ── Structure d'une ligne ──────────────────────────────────
    // Chaque ligne a son propre Y, sa propre direction horizontale,
    // et ses cellules vivantes/mortes.
    struct Row {
      int     y;                    // Y du haut de la ligne
      int     direction;            // +1 ou -1
      bool    alive[INV_COLS];      // cellules vivantes
      uint8_t type;                 // type visuel (0,1,2)
      int     x[INV_COLS];         // X de chaque cellule
    };

    struct Bullet { int x, y; bool active; };
    struct Bomb   { int x, y; bool active; };

    Row    rows[INV_ROWS];
    Bullet bullets[MAX_BULLETS];
    Bomb   bombs[MAX_BOMBS];

    int           playerX;
    int           lives;
    int           waveCount;
    unsigned long lastMoveTime;
    unsigned long lastFrameTime;
    unsigned long lastBombTime;
    int           moveInterval;
    int           buzzerPin;

    bool          shootRequested;
    bool          needsFullRedraw;

    static const uint16_t COL_BG     = TFT_BLACK;
    static const uint16_t COL_PLAYER = TFT_GREEN;
    static const uint16_t COL_BULLET = TFT_WHITE;
    static const uint16_t COL_BOMB   = TFT_RED;
    static const uint16_t COL_SCORE  = TFT_WHITE;
    static const uint16_t COL_LIVES  = TFT_RED;

    // ── Couleur par type ──────────────────────────────────────
    uint16_t invaderColor(uint8_t type) {
      switch (type % 3) {
        case 0:  return TFT_RED;
        case 1:  return TFT_CYAN;
        default: return TFT_MAGENTA;
      }
    }

    // ── Dessin d'un envahisseur ───────────────────────────────
    void drawInvader(int x, int y, uint8_t type, uint16_t col) {
      if (y + INV_H < 14 || y > SCREEN_H) return;

      if (col == COL_BG) {
        screen->fillRect(x - 1, y - 1, INV_W + 2, INV_H + 2, COL_BG);
        return;
      }
      switch (type % 3) {
        case 0:
          screen->fillRect(x + 2,  y,      12, 3, col);
          screen->fillRect(x,      y + 3,  16, 4, col);
          screen->fillRect(x + 2,  y + 7,   4, 3, col);
          screen->fillRect(x + 10, y + 7,   4, 3, col);
          screen->fillRect(x,      y + 2,   2, 2, col);
          screen->fillRect(x + 14, y + 2,   2, 2, col);
          break;
        case 1:
          screen->fillRect(x + 3,  y,      10, 3, col);
          screen->fillRect(x + 1,  y + 3,  14, 4, col);
          screen->fillRect(x,      y + 7,   4, 3, col);
          screen->fillRect(x + 6,  y + 7,   4, 3, col);
          screen->fillRect(x + 12, y + 7,   4, 3, col);
          break;
        default:
          screen->fillRect(x + 4,  y,       8, 3, col);
          screen->fillRect(x + 2,  y + 3,  12, 4, col);
          screen->fillRect(x,      y + 7,   6, 3, col);
          screen->fillRect(x + 10, y + 7,   6, 3, col);
          break;
      }
    }

    // ── Initialise une ligne en haut de l'écran ───────────────
    // startY < 14 : peut être négatif (hors écran, entrée progressive)
    void initRow(int rowIdx, int startY, uint8_t type, int dir = 1) {
      rows[rowIdx].y         = startY;
      rows[rowIdx].direction = dir;
      rows[rowIdx].type      = type;
      for (int c = 0; c < INV_COLS; c++) {
        rows[rowIdx].x[c]     = INV_ORIG_X + c * (INV_W + INV_GAP_X);
        rows[rowIdx].alive[c] = true;
      }
    }

    // ── Ligne entièrement détruite ? ──────────────────────────
    bool rowEmpty(int rowIdx) {
      for (int c = 0; c < INV_COLS; c++)
        if (rows[rowIdx].alive[c]) return false;
      return true;
    }

    // ── Respawn une ligne détruite (en haut, hors écran) ─────
    void respawnRow(int rowIdx) {
      waveCount++;
      score += 50;
      updateInterval();
      // Direction alternée pour varier
      int dir = (waveCount % 2 == 0) ? 1 : -1;
      initRow(rowIdx, -INV_H - 4, waveCount % 3, dir);
      drawHUD();
    }

    // ── Effacer visuellement toute une ligne ──────────────────
    void eraseRow(int rowIdx) {
      for (int c = 0; c < INV_COLS; c++) {
        drawInvader(rows[rowIdx].x[c], rows[rowIdx].y,
                    rows[rowIdx].type, COL_BG);
      }
    }

    // ── Dessin joueur ─────────────────────────────────────────
    void drawPlayer(int x, uint16_t col) {
      int py = PLAYER_Y;
      screen->fillRect(x,     py + 4, PLAYER_W, 4, col);
      screen->fillRect(x + 5, py,     6,         4, col);
      screen->fillRect(x + 7, py - 3, 2,         3, col);
    }

    void erasePlayer(int x) {
      int py = PLAYER_Y;
      screen->fillRect(x - 2, py - 4, PLAYER_W + 4, PLAYER_H + 8, COL_BG);
    }

    // ── HUD ───────────────────────────────────────────────────
    void drawHUD() {
      screen->fillRect(0, 0, SCREEN_W, 12, COL_BG);
      screen->drawFastHLine(0, 12, SCREEN_W, 0x4208);

      screen->setTextColor(COL_SCORE, COL_BG);
      screen->setTextSize(1);
      screen->setCursor(2, 2);
      screen->print("Score:");
      screen->print(score);

      screen->setCursor(90, 2);
      screen->print("Vague:");
      screen->print(waveCount);

      screen->setCursor(160, 2);
      screen->setTextColor(COL_LIVES, COL_BG);
      screen->print("Vies:");
      for (int i = 0; i < lives; i++)
        screen->fillRect(195 + i * 10, 2, 6, 6, COL_LIVES);
    }

    void updateInterval() {
      int bonus    = waveCount * 8;
      moveInterval = max(INTERVAL_MIN, INTERVAL_MAX - bonus);
    }

    // ── Tir joueur ────────────────────────────────────────────
    void fireBullet() {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
          bullets[i].active = true;
          bullets[i].x      = playerX + PLAYER_W / 2 - BULLET_W / 2;
          bullets[i].y      = PLAYER_Y - BULLET_H;
          return;
        }
      }
    }

    // ── Bombe envahisseur ─────────────────────────────────────
    void fireBomb() {
      // Choisir une cellule vivante au hasard parmi les lignes visibles
      int candidates[INV_ROWS * INV_COLS][2];
      int count = 0;
      for (int r = 0; r < INV_ROWS; r++) {
        if (rows[r].y < 14) continue;          // pas encore visible
        if (rows[r].y > PLAYER_Y - 12) continue; // trop proche du joueur
        for (int c = 0; c < INV_COLS; c++) {
          if (rows[r].alive[c]) {
            candidates[count][0] = r;
            candidates[count][1] = c;
            count++;
          }
        }
      }
      if (count == 0) return;

      int pick = random(0, count);
      int r    = candidates[pick][0];
      int c    = candidates[pick][1];

      for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bombs[i].active) {
          bombs[i].active = true;
          bombs[i].x      = rows[r].x[c] + INV_W / 2 - BOMB_W / 2;
          bombs[i].y      = rows[r].y + INV_H;
          lastBombTime    = millis();
          return;
        }
      }
    }

    // ── Déplacement de la formation ───────────────────────────
    // Chaque ligne bouge indépendamment :
    //  - latéralement, rebondit sur les murs
    //  - descend de INV_DESCENT pixels à chaque tick
    void moveFormation() {
      unsigned long now = millis();
      if (now - lastMoveTime < (unsigned long)moveInterval) return;
      lastMoveTime = now;

      for (int r = 0; r < INV_ROWS; r++) {

        // ── Effacer la ligne à son ancienne position ──────────
        eraseRow(r);

        // ── Calculer la nouvelle position horizontale ─────────
        // Trouver les bords gauche et droit des cellules vivantes
        int leftX  = SCREEN_W;
        int rightX = 0;
        bool hasAlive = false;
        for (int c = 0; c < INV_COLS; c++) {
          if (!rows[r].alive[c]) continue;
          hasAlive = true;
          if (rows[r].x[c] < leftX)              leftX  = rows[r].x[c];
          if (rows[r].x[c] + INV_W > rightX)     rightX = rows[r].x[c] + INV_W;
        }

        if (hasAlive) {
          int newLeft  = leftX  + rows[r].direction * INV_STEP;
          int newRight = rightX + rows[r].direction * INV_STEP;

          // Rebond sur les murs → inverse uniquement la direction (pas de descente ici,
          // la descente est continue et indépendante du rebond)
          if (newLeft < INV_MARGIN || newRight > SCREEN_W - INV_MARGIN) {
            rows[r].direction = -rows[r].direction;
          } else {
            for (int c = 0; c < INV_COLS; c++)
              rows[r].x[c] += rows[r].direction * INV_STEP;
          }
        }

        // ── Descente continue ─────────────────────────────────
        rows[r].y += INV_DESCENT;

        // ── Vérifier Game Over : ligne touche le joueur ───────
        if (rows[r].y + INV_H >= PLAYER_Y && hasAlive) {
          // Au moins une cellule vivante arrive sur le joueur
          for (int c = 0; c < INV_COLS; c++) {
            if (!rows[r].alive[c]) continue;
            if (rows[r].x[c] + INV_W > playerX &&
                rows[r].x[c]         < playerX + PLAYER_W) {
              // Contact direct avec le canon → Game Over
              state = GAME_OVER;
              return;
            }
          }
          // La ligne est passée sous le joueur sans le toucher
          // (possible si le joueur s'est écarté) → recycler la ligne
          if (rows[r].y > SCREEN_H) {
            respawnRow(r);
          }
        }

        // ── Ligne sortie par le bas (détruite ou évitée) ──────
        if (rows[r].y > SCREEN_H) {
          respawnRow(r);
          continue;
        }

        // ── Redessiner la ligne ───────────────────────────────
        for (int c = 0; c < INV_COLS; c++) {
          if (!rows[r].alive[c]) continue;
          drawInvader(rows[r].x[c], rows[r].y,
                      rows[r].type, invaderColor(rows[r].type));
        }
      }
    }

    // ── Mise à jour des balles du joueur ──────────────────────
    void updateBullets() {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        // Effacer ancienne position
        screen->fillRect(bullets[i].x, bullets[i].y, BULLET_W, BULLET_H, COL_BG);
        bullets[i].y -= BULLET_SPEED;

        // Sortie par le haut
        if (bullets[i].y < 14) { bullets[i].active = false; continue; }

        // Collision avec les envahisseurs
        bool hit = false;
        for (int r = 0; r < INV_ROWS && !hit; r++) {
          for (int c = 0; c < INV_COLS && !hit; c++) {
            if (!rows[r].alive[c]) continue;
            if (bullets[i].x + BULLET_W > rows[r].x[c]         &&
                bullets[i].x            < rows[r].x[c] + INV_W  &&
                bullets[i].y            < rows[r].y    + INV_H  &&
                bullets[i].y + BULLET_H > rows[r].y) {

              // Cellule touchée
              drawInvader(rows[r].x[c], rows[r].y, rows[r].type, COL_BG);
              rows[r].alive[c]  = false;
              bullets[i].active = false;
              score += (rows[r].type == 0) ? 30 :
                       (rows[r].type == 1) ? 20 : 10;
              drawHUD();
              hit = true;

              // Si toute la ligne est détruite → nouvelle ligne en haut
              if (rowEmpty(r)) respawnRow(r);
            }
          }
        }

        if (!hit)
          screen->fillRect(bullets[i].x, bullets[i].y, BULLET_W, BULLET_H, COL_BULLET);
      }
    }

    // ── Mise à jour des bombes ennemies ───────────────────────
    void updateBombs() {
      for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bombs[i].active) continue;

        screen->fillRect(bombs[i].x, bombs[i].y, BOMB_W, BOMB_H, COL_BG);
        bombs[i].y += BOMB_SPEED;

        if (bombs[i].y > SCREEN_H) { bombs[i].active = false; continue; }

        // Collision avec le joueur
        if (bombs[i].x + BOMB_W > playerX             &&
            bombs[i].x          < playerX + PLAYER_W  &&
            bombs[i].y + BOMB_H > PLAYER_Y            &&
            bombs[i].y          < PLAYER_Y + PLAYER_H) {

          bombs[i].active = false;
          lives--;
          drawHUD();
          erasePlayer(playerX);
          screen->fillRect(playerX, PLAYER_Y - 3, PLAYER_W, PLAYER_H + 6, TFT_RED);
          delay(120);
          erasePlayer(playerX);
          drawPlayer(playerX, COL_PLAYER);
          if (lives <= 0) { state = GAME_OVER; return; }
          continue;
        }

        screen->fillRect(bombs[i].x, bombs[i].y, BOMB_W, BOMB_H, COL_BOMB);
      }
    }

    // ── Déplacement du joueur ─────────────────────────────────
    void movePlayer(Buttons buttons) {
      int px = playerX;
      if (buttons.leftPressed  && playerX > 0)
        playerX -= PLAYER_STEP;
      if (buttons.rightPressed && playerX + PLAYER_W < SCREEN_W)
        playerX += PLAYER_STEP;
      if (buttons.left  && !buttons.leftPressed  && playerX > 0)
        playerX -= PLAYER_STEP;
      if (buttons.right && !buttons.rightPressed && playerX + PLAYER_W < SCREEN_W)
        playerX += PLAYER_STEP;

      if (px != playerX) {
        erasePlayer(px);
        drawPlayer(playerX, COL_PLAYER);
      }
    }

    // ── Étape principale ──────────────────────────────────────
    void stepGame(Buttons buttons) {
      if (shootRequested) { fireBullet(); shootRequested = false; }

      movePlayer(buttons);
      moveFormation();
      if (state == GAME_OVER) return;

      updateBullets();
      if (state == GAME_OVER) return;

      updateBombs();
      if (state == GAME_OVER) return;

      unsigned long now      = millis();
      int           bombDelay = max(400, moveInterval * 3);
      if (now - lastBombTime > (unsigned long)bombDelay) fireBomb();
    }

  public:
    SpaceInvadersGame(TFT_eSPI* display, int buzzer = 17)
      : Game(display), buzzerPin(buzzer),
        playerX(SCREEN_W / 2 - PLAYER_W / 2),
        lives(MAX_LIVES),
        waveCount(0),
        lastMoveTime(0), lastFrameTime(0), lastBombTime(0),
        moveInterval(INTERVAL_MAX),
        shootRequested(false), needsFullRedraw(true) {
      for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
      for (int i = 0; i < MAX_BOMBS;   i++) bombs[i].active   = false;
    }

    void forceRedraw() override { needsFullRedraw = true; }

    void init() override {
      // 3 lignes initiales espacées verticalement, déjà sur l'écran
      // (ligne 0 = la plus haute, ligne 2 = la plus basse)
      for (int r = 0; r < INV_ROWS; r++) {
        // Y initial : répartir les 3 lignes entre 20 et ~70
        int startY = 20 + r * (INV_H + INV_STRIDE_Y);
        // Alterner la direction pour chaque ligne
        int dir    = (r % 2 == 0) ? 1 : -1;
        initRow(r, startY, r % 3, dir);
      }

      for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
      for (int i = 0; i < MAX_BOMBS;   i++) bombs[i].active   = false;

      playerX      = SCREEN_W / 2 - PLAYER_W / 2;
      lives        = MAX_LIVES;
      waveCount    = 0;
      score        = 0;
      state        = IN_PROGRESS;
      moveInterval = INTERVAL_MAX;
      shootRequested  = false;
      lastMoveTime    = millis();
      lastFrameTime   = millis();
      lastBombTime    = millis();
      needsFullRedraw = true;
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      if (buttons.aPressed) shootRequested = true;

      unsigned long now = millis();
      if (now - lastFrameTime < (unsigned long)FRAME_MS) return;
      lastFrameTime = now;

      stepGame(buttons);
    }

    void render() override {
      if (!needsFullRedraw) return;
      needsFullRedraw = false;

      screen->fillScreen(COL_BG);
      screen->drawFastHLine(0, 13, SCREEN_W, 0x4208);

      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          if (!rows[r].alive[c]) continue;
          drawInvader(rows[r].x[c], rows[r].y,
                      rows[r].type, invaderColor(rows[r].type));
        }
      }

      drawPlayer(playerX, COL_PLAYER);
      drawHUD();
    }

    virtual ~SpaceInvadersGame() {}
};

#endif