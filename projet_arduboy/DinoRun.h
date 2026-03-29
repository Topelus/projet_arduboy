// ============================================================
//  DinoRun.h — Endless Runner (Dino Run)
//  A        : sauter
//  B        : pause (géré par la console)
//  Score    : +1 chaque frame logique
//  Game Over: collision avec un cactus
// ============================================================
#ifndef DINORUN_H
#define DINORUN_H

#include "Game.h"

class DinoRunGame : public Game {
  private:

    // ── Écran 240×135 ─────────────────────────────────────
    static const int SCREEN_W = 240;
    static const int SCREEN_H = 135;

    // ── Sol ───────────────────────────────────────────────
    static const int GROUND_Y    = 110;
    static const int DINO_X      = 30;
    static const int DINO_W      = 16;
    static const int DINO_H      = 20;
    static const int DINO_GROUND = GROUND_Y - DINO_H;

    // ── Physique (virgule fixe ×100) ──────────────────────
    static const int JUMP_FORCE = 700;
    static const int GRAVITY    = 55;

    // ── Obstacles ─────────────────────────────────────────
    static const int MAX_CACTUS    = 3;
    static const int CACTUS_W      = 10;
    static const int CACTUS_H_MIN  = 14;
    static const int CACTUS_H_MAX  = 26;
    static const int SPAWN_DELAY   = 1800;

    // ── Vitesse ───────────────────────────────────────────
    static const int SPEED_INIT = 3;
    static const int SPEED_MAX  = 8;
    static const int SPEED_STEP = 100;

    // ── Nuages ────────────────────────────────────────────
    static const int MAX_CLOUDS = 3;

    // ── Timing frame ──────────────────────────────────────
    static const int FRAME_MS = 16; // ~60 fps, sans delay()

    // ── Structures ────────────────────────────────────────
    struct Cactus { int x, h; bool active; };
    struct Cloud  { int x, y; bool active; };

    // ── État ──────────────────────────────────────────────
    int           dinoY, velY;
    bool          onGround;
    bool          jumpRequested;   // ← FLAG SAUT : mémorise aPressed entre frames
    uint8_t       animFrame, animTick;
    Cactus        cactus[MAX_CACTUS];
    Cloud         clouds[MAX_CLOUDS];
    unsigned long lastSpawnTime;
    unsigned long lastFrameTime;
    int           speed;
    int           groundOffset;
    bool          needsFullRedraw;

    // ── Couleurs ──────────────────────────────────────────
    static const uint16_t COL_SKY    = 0x18DF;
    static const uint16_t COL_GROUND = 0x4A09;
    static const uint16_t COL_DINO   = TFT_GREEN;
    static const uint16_t COL_CACTUS = 0x04C0;
    static const uint16_t COL_CLOUD  = 0xC618;

    // ─────────────────────────────────────────────────────
    void drawDino(int y, uint8_t frame, uint16_t col) {
      int x = DINO_X;
      screen->fillRect(x,              y + 4, DINO_W,     DINO_H - 8, col);
      screen->fillRect(x + 4,          y,     DINO_W - 2, 8,          col);
      screen->fillRect(x + DINO_W - 4, y + 2, 2, 2, TFT_BLACK);
      screen->fillRect(x - 4,          y + 6, 6, 4, col);
      if (frame == 0) {
        screen->fillRect(x + 2, y + DINO_H - 6, 4, 6, col);
        screen->fillRect(x + 9, y + DINO_H - 4, 4, 4, col);
      } else {
        screen->fillRect(x + 2, y + DINO_H - 4, 4, 4, col);
        screen->fillRect(x + 9, y + DINO_H - 6, 4, 6, col);
      }
    }

    void drawCactus(const Cactus& c, uint16_t col) {
      int cy = GROUND_Y - c.h;
      screen->fillRect(c.x + 3, cy,     4,          c.h,     col);
      screen->fillRect(c.x,     cy + 4, 3,          c.h / 3, col);
      screen->fillRect(c.x,     cy + 2, CACTUS_W/2, 3,       col);
      screen->fillRect(c.x + 7, cy + 6, 3,          c.h / 3, col);
      screen->fillRect(c.x + 5, cy + 4, CACTUS_W/2, 3,       col);
    }

    void drawCloud(const Cloud& cl, uint16_t col) {
      screen->fillRect(cl.x,      cl.y + 4, 28, 8,  col);
      screen->fillRect(cl.x + 4,  cl.y,     12, 12, col);
      screen->fillRect(cl.x + 14, cl.y + 2, 8,  10, col);
    }

    void drawGround() {
      screen->drawFastHLine(0, GROUND_Y,     SCREEN_W, COL_GROUND);
      screen->drawFastHLine(0, GROUND_Y + 1, SCREEN_W, COL_GROUND);
      for (int x = -(groundOffset % 20); x < SCREEN_W; x += 20) {
        screen->fillRect(x,     GROUND_Y + 3, 3, 2, COL_GROUND);
        screen->fillRect(x + 9, GROUND_Y + 4, 2, 2, COL_GROUND);
      }
    }

    void drawScore() {
      screen->fillRect(160, 2, 78, 10, COL_SKY);
      screen->setTextColor(TFT_WHITE, COL_SKY);
      screen->setTextSize(1);
      screen->setCursor(162, 3);
      screen->print("Score:");
      screen->print(score);
    }

    void spawnCactus() {
      for (int i = 0; i < MAX_CACTUS; i++) {
        if (!cactus[i].active) {
          cactus[i].active = true;
          cactus[i].x      = SCREEN_W;
          cactus[i].h      = CACTUS_H_MIN + (int)random(0, CACTUS_H_MAX - CACTUS_H_MIN + 1);
          lastSpawnTime    = millis();
          return;
        }
      }
    }

    void spawnCloud() {
      for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!clouds[i].active) {
          clouds[i].active = true;
          clouds[i].x      = SCREEN_W;
          clouds[i].y      = 15 + (int)random(0, 30);
          return;
        }
      }
    }

    bool checkCollision(const Cactus& c) {
      int dx1 = DINO_X + 2,          dy1 = dinoY + 4;
      int dx2 = DINO_X + DINO_W - 2, dy2 = dinoY + DINO_H - 2;
      int cx1 = c.x,                 cy1 = GROUND_Y - c.h;
      int cx2 = c.x + CACTUS_W,      cy2 = GROUND_Y;
      return !(dx2 < cx1 || dx1 > cx2 || dy2 < cy1 || dy1 > cy2);
    }

    void stepGame() {
      // ── Saut : consomme le flag mémorisé ─────────────
      if (jumpRequested && onGround) {
        velY          = -JUMP_FORCE;
        onGround      = false;
      }
      jumpRequested = false; // toujours consommé après usage

      // ── Physique ──────────────────────────────────────
      int prevDinoY = dinoY;
      velY  += GRAVITY;
      dinoY += velY / 100;
      if (dinoY >= DINO_GROUND) {
        dinoY = DINO_GROUND; velY = 0; onGround = true;
      }

      // ── Animation pattes ──────────────────────────────
      if (onGround) {
        if (++animTick >= 6) { animTick = 0; animFrame ^= 1; }
      } else {
        animFrame = 0;
      }

      // ── Dino ──────────────────────────────────────────
      screen->fillRect(DINO_X - 4, prevDinoY, DINO_W + 8, DINO_H + 2, COL_SKY);
      drawDino(dinoY, animFrame, COL_DINO);

      // ── Nuages ────────────────────────────────────────
      for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!clouds[i].active) continue;
        drawCloud(clouds[i], COL_SKY);
        clouds[i].x -= max(1, speed / 2);
        if (clouds[i].x < -30) clouds[i].active = false;
        else                   drawCloud(clouds[i], COL_CLOUD);
      }
      if (random(0, 120) < 1) spawnCloud();

      // ── Sol ───────────────────────────────────────────
      groundOffset += speed;
      screen->fillRect(0, GROUND_Y + 2, SCREEN_W, 8, COL_SKY);
      drawGround();

      // ── Cactus ────────────────────────────────────────
      bool anyActive = false;
      for (int i = 0; i < MAX_CACTUS; i++) {
        if (!cactus[i].active) continue;
        anyActive = true;
        drawCactus(cactus[i], COL_SKY);
        cactus[i].x -= speed;
        if (cactus[i].x + CACTUS_W < 0) {
          cactus[i].active = false;
        } else {
          drawCactus(cactus[i], COL_CACTUS);
          if (checkCollision(cactus[i])) { state = GAME_OVER; return; }
        }
      }

      // ── Spawn cactus ──────────────────────────────────
      unsigned long now = millis();
      if (!anyActive || (now - lastSpawnTime > (unsigned long)SPAWN_DELAY && random(0, 100) < 5))
        spawnCactus();

      // ── Score + difficulté ────────────────────────────
      score++;
      if (score % SPEED_STEP == 0 && speed < SPEED_MAX) speed++;
      drawScore();
    }

  public:
    DinoRunGame(TFT_eSPI* display)
      : Game(display),
        dinoY(DINO_GROUND), velY(0), onGround(true),
        jumpRequested(false),
        animFrame(0), animTick(0),
        lastSpawnTime(0), lastFrameTime(0),
        speed(SPEED_INIT), groundOffset(0),
        needsFullRedraw(true) {
      for (int i = 0; i < MAX_CACTUS; i++) cactus[i].active = false;
      for (int i = 0; i < MAX_CLOUDS; i++) clouds[i].active = false;
    }

    void init() override {
      dinoY           = DINO_GROUND;
      velY            = 0;
      onGround        = true;
      jumpRequested   = false;
      animFrame       = 0;
      animTick        = 0;
      speed           = SPEED_INIT;
      score           = 0;
      groundOffset    = 0;
      state           = IN_PROGRESS;
      lastSpawnTime   = millis();
      lastFrameTime   = millis();
      needsFullRedraw = true;
      for (int i = 0; i < MAX_CACTUS; i++) cactus[i].active = false;
      for (int i = 0; i < MAX_CLOUDS; i++) clouds[i].active = false;
      spawnCloud();
    }

    // ── AJOUT : forceRedraw() ─────────────────────────────
    void forceRedraw() override {
      needsFullRedraw = true;
    }

    // ── update() ─────────────────────────────────────────
    // 1. On capture aPressed IMMÉDIATEMENT (avant tout return)
    // 2. On attend que FRAME_MS ms soient écoulées
    // 3. Seulement alors on exécute la frame logique
    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      // Capturer le saut dès que le bouton est pressé,
      // même si la frame logique n'est pas encore prête
      if (buttons.aPressed) jumpRequested = true;

      unsigned long now = millis();
      if (now - lastFrameTime < (unsigned long)FRAME_MS) return;
      lastFrameTime = now;

      stepGame();
    }

    // ── render() ─────────────────────────────────────────
    void render() override {
      if (!needsFullRedraw) return;
      needsFullRedraw = false;
      screen->fillScreen(COL_SKY);
      drawGround();
      drawDino(dinoY, animFrame, COL_DINO);
      drawScore();
    }

    virtual ~DinoRunGame() {}
};

#endif