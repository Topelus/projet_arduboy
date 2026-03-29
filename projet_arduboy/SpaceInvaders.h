// ============================================================
//  SpaceInvaders.h — Space Invaders Infini avec descente progressive
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
    static const int INV_GAP_Y   = 10;  // Espace vertical entre lignes
    static const int INV_ORIG_X  = 12;
    static const int INV_ORIG_Y  = 12;
    static const int INV_STEP    = 4;
    static const int INV_DROP    = 8;   // Descente plus douce
    static const int INV_MARGIN  = 4;

    static const int INTERVAL_MAX = 500;
    static const int INTERVAL_MIN = 80;

    static const int PLAYER_Y    = SCREEN_H - 14;
    static const int PLAYER_W    = 16;
    static const int PLAYER_H    = 8;
    static const int PLAYER_STEP = 4;

    static const int MAX_BULLETS  = 2;
    static const int BULLET_SPEED = 5;
    static const int BULLET_W     = 2;
    static const int BULLET_H     = 6;

    static const int MAX_BOMBS    = 3;
    static const int BOMB_SPEED   = 2;
    static const int BOMB_W       = 2;
    static const int BOMB_H       = 6;

    static const int MAX_LIVES    = 3;
    static const int FRAME_MS     = 16;

    struct Invader { int x, y; bool alive; uint8_t type; };
    struct Bullet  { int x, y; bool active; };
    struct Bomb    { int x, y; bool active; };

    Invader       invaders[INV_ROWS][INV_COLS];
    Bullet        bullets[MAX_BULLETS];
    Bomb          bombs[MAX_BOMBS];

    int           playerX;
    int           lives;
    int           direction;
    int           waveCount;
    unsigned long lastMoveTime;
    unsigned long lastFrameTime;
    unsigned long lastBombTime;
    int           moveInterval;
    int           buzzerPin;

    bool          shootRequested;
    bool          needsFullRedraw;

    static const uint16_t COL_BG      = TFT_BLACK;
    static const uint16_t COL_PLAYER  = TFT_GREEN;
    static const uint16_t COL_BULLET  = TFT_WHITE;
    static const uint16_t COL_BOMB    = TFT_RED;
    static const uint16_t COL_SCORE   = TFT_WHITE;
    static const uint16_t COL_LIVES   = TFT_RED;

    uint16_t invaderColor(uint8_t type) {
      switch (type % 3) {
        case 0: return TFT_RED;
        case 1: return TFT_CYAN;
        default: return TFT_MAGENTA;
      }
    }

    void playImperialMarch() {
      int notes[] = { 392, 392, 392, 311, 466, 392, 311, 466, 392 };
      int durations[] = { 500, 500, 500, 350, 150, 500, 350, 150, 1000 };
      for (int i = 0; i < 9; i++) {
        tone(buzzerPin, notes[i], durations[i]);
        delay(durations[i] + 50);
      }
      noTone(buzzerPin);
    }

    void drawInvader(int x, int y, uint8_t type, uint16_t col) {
      if (col == TFT_BLACK) {
        screen->fillRect(x, y, INV_W, INV_H, COL_BG);
        return;
      }
      switch (type % 3) {
        case 0:
          screen->fillRect(x + 2, y,     12, 3,  col);
          screen->fillRect(x,     y + 3,  16, 4,  col);
          screen->fillRect(x + 2, y + 7,  4,  3,  col);
          screen->fillRect(x + 10,y + 7,  4,  3,  col);
          screen->fillRect(x,     y + 2,  2,  2,  col);
          screen->fillRect(x + 14,y + 2,  2,  2,  col);
          break;
        case 1:
          screen->fillRect(x + 3, y,     10, 3,  col);
          screen->fillRect(x + 1, y + 3,  14, 4,  col);
          screen->fillRect(x,     y + 7,  4,  3,  col);
          screen->fillRect(x + 6, y + 7,  4,  3,  col);
          screen->fillRect(x + 12,y + 7,  4,  3,  col);
          break;
        default:
          screen->fillRect(x + 4, y,      8,  3,  col);
          screen->fillRect(x + 2, y + 3,  12, 4,  col);
          screen->fillRect(x,     y + 7,  6,  3,  col);
          screen->fillRect(x + 10,y + 7,  6,  3,  col);
          break;
      }
    }

    void eraseInvader(int x, int y) {
      screen->fillRect(x - 2, y - 2, INV_W + 4, INV_H + 4, COL_BG);
    }

    void drawPlayer(int x, uint16_t col) {
      screen->fillRect(x,         PLAYER_Y + 4, PLAYER_W,     4, col);
      screen->fillRect(x + 5,     PLAYER_Y,     6,            4, col);
      screen->fillRect(x + 7,     PLAYER_Y - 3, 2,            3, col);
    }

    void drawHUD() {
      screen->fillRect(0, SCREEN_H - 12, SCREEN_W, 12, COL_BG);
      screen->drawFastHLine(0, SCREEN_H - 13, SCREEN_W, 0x4208);

      screen->setTextColor(COL_SCORE, COL_BG);
      screen->setTextSize(1);
      screen->setCursor(2, SCREEN_H - 10);
      screen->print("Score:");
      screen->print(score);

      screen->setCursor(90, SCREEN_H - 10);
      screen->print("Vague:");
      screen->print(waveCount);

      screen->setCursor(160, SCREEN_H - 10);
      screen->setTextColor(COL_LIVES, COL_BG);
      screen->print("Vies:");
      for (int i = 0; i < lives; i++) {
        screen->fillRect(195 + i * 10, SCREEN_H - 10, 6, 6, COL_LIVES);
      }
    }

    void updateInterval() {
      int speedBonus = waveCount * 20;
      moveInterval = max(INTERVAL_MIN, INTERVAL_MAX - speedBonus);
    }

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

    void fireBomb() {
      int aliveList[INV_ROWS * INV_COLS][2];
      int aliveCount = 0;
      
      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          if (invaders[r][c].alive) {
            aliveList[aliveCount][0] = r;
            aliveList[aliveCount][1] = c;
            aliveCount++;
          }
        }
      }
      
      if (aliveCount == 0) return;
      
      int pick = random(0, aliveCount);
      int r = aliveList[pick][0];
      int c = aliveList[pick][1];
      
      for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bombs[i].active) {
          bombs[i].active = true;
          bombs[i].x      = invaders[r][c].x + INV_W / 2 - BOMB_W / 2;
          bombs[i].y      = invaders[r][c].y + INV_H;
          lastBombTime    = millis();
          return;
        }
      }
    }

    // Respawn une ligne en haut avec Y différent selon la rangée
    void respawnRow(int row) {
      uint8_t newType = (waveCount + row) % 3;
      
      for (int c = 0; c < INV_COLS; c++) {
        invaders[row][c].x     = INV_ORIG_X + c * (INV_W + INV_GAP_X);
        // Position Y décalée selon la rangée pour éviter la superposition
        invaders[row][c].y     = INV_ORIG_Y + row * (INV_H + INV_GAP_Y);
        invaders[row][c].alive = true;
        invaders[row][c].type  = newType;
      }
      
      // Redessiner cette ligne
      for (int c = 0; c < INV_COLS; c++) {
        drawInvader(invaders[row][c].x, invaders[row][c].y, 
                    invaders[row][c].type, invaderColor(invaders[row][c].type));
      }
    }

    void moveFormation() {
      unsigned long now = millis();
      if (now - lastMoveTime < (unsigned long)moveInterval) return;
      lastMoveTime = now;

      // Vérifier rebond
      bool bounce = false;
      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          if (!invaders[r][c].alive) continue;
          int nx = invaders[r][c].x + direction * INV_STEP;
          if (nx < INV_MARGIN || nx + INV_W > SCREEN_W - INV_MARGIN) {
            bounce = true;
            break;
          }
        }
        if (bounce) break;
      }

      if (bounce) {
        direction = -direction;
        
        // Vérifier d'abord quelles lignes vont toucher le bas APRES la descente
        bool rowWillHitBottom[INV_ROWS] = {false};
        int rowsToRespawn = 0;
        
        for (int r = 0; r < INV_ROWS; r++) {
          // Trouver le Y le plus bas de cette ligne
          int lowestY = 0;
          bool hasAlive = false;
          for (int c = 0; c < INV_COLS; c++) {
            if (invaders[r][c].alive) {
              hasAlive = true;
              if (invaders[r][c].y > lowestY) lowestY = invaders[r][c].y;
            }
          }
          
          if (hasAlive && lowestY + INV_H + INV_DROP >= PLAYER_Y - 10) {
            rowWillHitBottom[r] = true;
            rowsToRespawn++;
          }
        }
        
        // Si des lignes vont toucher le bas
        if (rowsToRespawn > 0) {
          waveCount++;
          updateInterval();
          score += rowsToRespawn * 50;  // Bonus par ligne
          
          // Effacer les lignes qui touchent et les respawn
          for (int r = 0; r < INV_ROWS; r++) {
            if (rowWillHitBottom[r]) {
              // Effacer complètement la ligne
              for (int c = 0; c < INV_COLS; c++) {
                if (invaders[r][c].alive) {
                  eraseInvader(invaders[r][c].x, invaders[r][c].y);
                }
              }
              // Respawn immédiatement en haut
              respawnRow(r);
            }
          }
        }
        
        // Descendre les lignes qui ne touchent pas le bas
        for (int r = 0; r < INV_ROWS; r++) {
          if (rowWillHitBottom[r]) continue;  // Cette ligne a été respawnée
          
          for (int c = 0; c < INV_COLS; c++) {
            if (!invaders[r][c].alive) continue;
            eraseInvader(invaders[r][c].x, invaders[r][c].y);
            invaders[r][c].y += INV_DROP;
          }
        }
        
      } else {
        // Déplacement horizontal normal
        for (int r = 0; r < INV_ROWS; r++) {
          for (int c = 0; c < INV_COLS; c++) {
            if (!invaders[r][c].alive) continue;
            eraseInvader(invaders[r][c].x, invaders[r][c].y);
            invaders[r][c].x += direction * INV_STEP;
          }
        }
      }

      // Redessiner tout
      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          if (!invaders[r][c].alive) continue;
          drawInvader(invaders[r][c].x, invaders[r][c].y, invaders[r][c].type,
                      invaderColor(invaders[r][c].type));
        }
      }
    }

    void updateBullets() {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        screen->fillRect(bullets[i].x, bullets[i].y, BULLET_W, BULLET_H, COL_BG);
        bullets[i].y -= BULLET_SPEED;

        if (bullets[i].y < 0) {
          bullets[i].active = false;
          continue;
        }

        bool hit = false;
        for (int r = 0; r < INV_ROWS && !hit; r++) {
          for (int c = 0; c < INV_COLS && !hit; c++) {
            if (!invaders[r][c].alive) continue;
            if (bullets[i].x + BULLET_W > invaders[r][c].x &&
                bullets[i].x < invaders[r][c].x + INV_W    &&
                bullets[i].y < invaders[r][c].y + INV_H    &&
                bullets[i].y + BULLET_H > invaders[r][c].y) {
              invaders[r][c].alive = false;
              eraseInvader(invaders[r][c].x, invaders[r][c].y);
              bullets[i].active = false;
              score += (invaders[r][c].type == 0) ? 30 :
                       (invaders[r][c].type == 1) ? 20 : 10;
              drawHUD();
              hit = true;
              break;
            }
          }
        }
        if (!hit) {
          screen->fillRect(bullets[i].x, bullets[i].y, BULLET_W, BULLET_H, COL_BULLET);
        }
      }
    }

    void updateBombs() {
      for (int i = 0; i < MAX_BOMBS; i++) {
        if (!bombs[i].active) continue;
        screen->fillRect(bombs[i].x, bombs[i].y, BOMB_W, BOMB_H, COL_BG);
        bombs[i].y += BOMB_SPEED;

        if (bombs[i].y > SCREEN_H) {
          bombs[i].active = false;
          continue;
        }

        if (bombs[i].x + BOMB_W > playerX &&
            bombs[i].x < playerX + PLAYER_W &&
            bombs[i].y + BOMB_H > PLAYER_Y &&
            bombs[i].y < PLAYER_Y + PLAYER_H) {
          bombs[i].active = false;
          lives--;
          drawHUD();
          screen->fillRect(playerX, PLAYER_Y - 3, PLAYER_W, PLAYER_H + 3, TFT_RED);
          delay(120);
          screen->fillRect(playerX, PLAYER_Y - 3, PLAYER_W, PLAYER_H + 3, COL_BG);
          drawPlayer(playerX, COL_PLAYER);
          if (lives <= 0) { state = GAME_OVER; return; }
          continue;
        }

        screen->fillRect(bombs[i].x, bombs[i].y, BOMB_W, BOMB_H, COL_BOMB);
      }
    }

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
        screen->fillRect(px, PLAYER_Y - 3, PLAYER_W, PLAYER_H + 3, COL_BG);
        drawPlayer(playerX, COL_PLAYER);
      }
    }

    void stepGame(Buttons buttons) {
      if (shootRequested) { fireBullet(); shootRequested = false; }

      movePlayer(buttons);
      moveFormation();
      updateBullets();
      if (state == GAME_OVER) return;
      updateBombs();
      if (state == GAME_OVER) return;

      unsigned long now = millis();
      int bombDelay = max(400, moveInterval * 2);
      if (now - lastBombTime > (unsigned long)bombDelay) fireBomb();
    }

  public:
    SpaceInvadersGame(TFT_eSPI* display, int buzzer = 17)
      : Game(display), buzzerPin(buzzer),
        playerX(SCREEN_W / 2 - PLAYER_W / 2),
        lives(MAX_LIVES),
        direction(1),
        waveCount(0),
        lastMoveTime(0), lastFrameTime(0), lastBombTime(0),
        moveInterval(INTERVAL_MAX),
        shootRequested(false), needsFullRedraw(true) {
      for (int r = 0; r < INV_ROWS; r++)
        for (int c = 0; c < INV_COLS; c++)
          invaders[r][c].alive = false;
      for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
      for (int i = 0; i < MAX_BOMBS;   i++) bombs[i].active   = false;
    }

    void forceRedraw() override {
      needsFullRedraw = true;
    }

    void init() override {
      playImperialMarch();

      // Initialiser les 3 lignes avec espacement vertical
      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          invaders[r][c].x     = INV_ORIG_X + c * (INV_W + INV_GAP_X);
          invaders[r][c].y     = INV_ORIG_Y + r * (INV_H + INV_GAP_Y);
          invaders[r][c].alive = true;
          invaders[r][c].type  = r % 3;
        }
      }
      
      for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
      for (int i = 0; i < MAX_BOMBS;   i++) bombs[i].active   = false;

      playerX       = SCREEN_W / 2 - PLAYER_W / 2;
      lives         = MAX_LIVES;
      waveCount     = 0;
      direction     = 1;
      score         = 0;
      state         = IN_PROGRESS;
      moveInterval  = INTERVAL_MAX;
      shootRequested = false;
      lastMoveTime  = millis();
      lastFrameTime = millis();
      lastBombTime  = millis();
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

      for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
          if (!invaders[r][c].alive) continue;
          drawInvader(invaders[r][c].x, invaders[r][c].y,
                      invaders[r][c].type, invaderColor(invaders[r][c].type));
        }
      }

      drawPlayer(playerX, COL_PLAYER);
      drawHUD();
    }

    virtual ~SpaceInvadersGame() {}
};

#endif