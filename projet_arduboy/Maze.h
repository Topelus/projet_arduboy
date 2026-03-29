// ============================================================
//  Maze.h — Labyrinthe infini avec compte à rebours
// ============================================================
#ifndef MAZE_H
#define MAZE_H

#include <Arduino.h>
#include "Game.h"

class MazeGame : public Game {
  private:
    enum {
      MAZE_WIDTH  = 15,
      MAZE_HEIGHT = 8,
      CELL_SZ = 15,
      ORIG_X = 7,
      ORIG_Y = 7,
      EXIT_COL = MAZE_WIDTH - 1,
      EXIT_ROW = MAZE_HEIGHT - 1,
      MOVE_DELAY = 150,  // Légèrement réduit pour plus de réactivité
      TIME_LIMIT = 120000
    };

    uint8_t walls[MAZE_HEIGHT][MAZE_WIDTH];
    bool    visited[MAZE_HEIGHT][MAZE_WIDTH];
    int     playerCol, playerRow;
    int     prevCol, prevRow;

    struct MCell { int8_t c, r; };
    MCell   dfsStack[MAZE_WIDTH * MAZE_HEIGHT];
    int     dfsTop;

    unsigned long startTime, lastMoveTime;
    bool needsFirstDraw;
    int level;
    int timeLeft;
    unsigned long lastTimerUpdate;
    bool timerRunning;

    void generateMaze();
    void drawMaze();
    void drawExit();
    void drawPlayer(int col, int row, uint16_t color);
    void restoreCell(int col, int row);
    bool canMove(int col, int row, int dir);
    void drawTimer();
    void nextLevel();

  public:
    MazeGame(TFT_eSPI* display);
    void init() override;
    void update(Buttons buttons) override;
    void render() override;
    void forceRedraw() override;
    virtual ~MazeGame() {}
};

inline void MazeGame::generateMaze() {
  for (int r = 0; r < MAZE_HEIGHT; r++)
    for (int c = 0; c < MAZE_WIDTH; c++) {
      walls[r][c]   = 0b1111;
      visited[r][c] = false;
    }
  dfsTop = 0;
  dfsStack[dfsTop++] = {0, 0};
  visited[0][0] = true;

  const int dc[4] = { 0,  1,  0, -1 };
  const int dr[4] = {-1,  0,  1,  0 };
  const int opp[4] = { 2,  3,  0,  1 };

  while (dfsTop > 0) {
    MCell cur = dfsStack[dfsTop - 1];
    int dirs[4] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--) {
      int j = random(0, i + 1);
      int t = dirs[i]; dirs[i] = dirs[j]; dirs[j] = t;
    }
    bool pushed = false;
    for (int di = 0; di < 4; di++) {
      int d = dirs[di];
      int nc = cur.c + dc[d], nr = cur.r + dr[d];
      if (nc < 0 || nc >= MAZE_WIDTH || nr < 0 || nr >= MAZE_HEIGHT) continue;
      if (visited[nr][nc]) continue;
      walls[cur.r][cur.c] &= ~(1 << d);
      walls[nr][nc]       &= ~(1 << opp[d]);
      visited[nr][nc]      = true;
      dfsStack[dfsTop++]   = {(int8_t)nc, (int8_t)nr};
      pushed = true;
      break;
    }
    if (!pushed) dfsTop--;
  }
}

inline void MazeGame::drawMaze() {
  screen->fillScreen(TFT_BLACK);
  for (int r = 0; r < MAZE_HEIGHT; r++) {
    for (int c = 0; c < MAZE_WIDTH; c++) {
      int px = ORIG_X + c * CELL_SZ, py = ORIG_Y + r * CELL_SZ;
      uint8_t w = walls[r][c];
      if (w & 1) screen->drawLine(px, py, px + CELL_SZ, py, TFT_WHITE);
      if (w & 2) screen->drawLine(px + CELL_SZ, py, px + CELL_SZ, py + CELL_SZ, TFT_WHITE);
      if (w & 4) screen->drawLine(px, py + CELL_SZ, px + CELL_SZ, py + CELL_SZ, TFT_WHITE);
      if (w & 8) screen->drawLine(px, py, px, py + CELL_SZ, TFT_WHITE);
    }
  }
  drawExit();
}

inline void MazeGame::drawExit() {
  int ex = ORIG_X + EXIT_COL * CELL_SZ + 3;
  int ey = ORIG_Y + EXIT_ROW * CELL_SZ + 3;
  screen->fillRect(ex, ey, CELL_SZ - 6, CELL_SZ - 6, TFT_YELLOW);
  screen->setTextColor(TFT_BLACK, TFT_YELLOW);
  screen->setTextSize(1);
  screen->setCursor(ex + 1, ey + 3);
  screen->print("S");
}

inline void MazeGame::drawPlayer(int col, int row, uint16_t color) {
  int px = ORIG_X + col * CELL_SZ + CELL_SZ / 2;
  int py = ORIG_Y + row * CELL_SZ + CELL_SZ / 2;
  screen->fillCircle(px, py, 4, color);
}

inline void MazeGame::restoreCell(int col, int row) {
  int px = ORIG_X + col * CELL_SZ + 1;
  int py = ORIG_Y + row * CELL_SZ + 1;
  screen->fillRect(px, py, CELL_SZ - 2, CELL_SZ - 2, TFT_BLACK);
  uint8_t w = walls[row][col];
  int bx = ORIG_X + col * CELL_SZ, by = ORIG_Y + row * CELL_SZ;
  if (w & 1) screen->drawLine(bx, by, bx + CELL_SZ, by, TFT_WHITE);
  if (w & 2) screen->drawLine(bx + CELL_SZ, by, bx + CELL_SZ, by + CELL_SZ, TFT_WHITE);
  if (w & 4) screen->drawLine(bx, by + CELL_SZ, bx + CELL_SZ, by + CELL_SZ, TFT_WHITE);
  if (w & 8) screen->drawLine(bx, by, bx, by + CELL_SZ, TFT_WHITE);
}

inline bool MazeGame::canMove(int col, int row, int dir) {
  if (row < 0 || row >= MAZE_HEIGHT || col < 0 || col >= MAZE_WIDTH) return false;
  return !(walls[row][col] & (1 << dir));
}

inline void MazeGame::drawTimer() {
  screen->fillRect(140, 2, 98, 14, TFT_BLACK);
  
  uint16_t color;
  if (timeLeft > 60) color = TFT_GREEN;
  else if (timeLeft > 30) color = TFT_YELLOW;
  else color = TFT_RED;
  
  screen->setTextColor(color, TFT_BLACK);
  screen->setTextSize(1);
  screen->setCursor(140, 3);
  
  int minutes = timeLeft / 60;
  int seconds = timeLeft % 60;
  
  screen->print("T:");
  if (minutes < 10) screen->print("0");
  screen->print(minutes);
  screen->print(":");
  if (seconds < 10) screen->print("0");
  screen->print(seconds);
  
  screen->setCursor(200, 3);
  screen->print("N:");
  screen->print(level);
}

inline void MazeGame::nextLevel() {
  score += timeLeft * 10;
  level++;
  
  generateMaze();
  playerCol = playerRow = prevCol = prevRow = 0;
  
  startTime = millis();
  lastTimerUpdate = millis();
  timeLeft = 120;
  
  needsFirstDraw = true;
}

inline MazeGame::MazeGame(TFT_eSPI* display) : Game(display) {
  static bool seeded = false;
  if (!seeded) {
    randomSeed(analogRead(0));
    seeded = true;
  }
}

inline void MazeGame::init() {
  level = 1;
  score = 0;
  timeLeft = 120;
  timerRunning = true;
  
  generateMaze();
  playerCol = playerRow = prevCol = prevRow = 0;
  state = IN_PROGRESS;
  startTime = millis();
  lastMoveTime = 0;  // IMPORTANT: initialisé à 0 pour permettre le premier mouvement
  lastTimerUpdate = millis();
  needsFirstDraw = true;
}

inline void MazeGame::update(Buttons buttons) {
  if (state == GAME_OVER) return;
  
  unsigned long now = millis();
  
  // Mise à jour du timer
  if (timerRunning && now - lastTimerUpdate >= 1000) {
    lastTimerUpdate = now;
    timeLeft--;
    
    if (timeLeft <= 0) {
      timeLeft = 0;
      state = GAME_OVER;
      return;
    }
    
    drawTimer();
  }
  
  // Déplacement avec anti-rebond temporel mais utilisant les états continus
  if (now - lastMoveTime < MOVE_DELAY) return;
  
  int nc = playerCol, nr = playerRow;
  bool moved = false;
  
  // Utilise les états continus des boutons
  if (buttons.up    && canMove(playerCol, playerRow, 0)) { nr--; moved = true; }
  if (buttons.right && canMove(playerCol, playerRow, 1)) { nc++; moved = true; }
  if (buttons.down  && canMove(playerCol, playerRow, 2)) { nr++; moved = true; }
  if (buttons.left  && canMove(playerCol, playerRow, 3)) { nc--; moved = true; }

  if (moved) {
    lastMoveTime = now;
    
    restoreCell(playerCol, playerRow);
    if (playerCol == EXIT_COL && playerRow == EXIT_ROW) drawExit();

    playerCol = nc; playerRow = nr;
    drawPlayer(playerCol, playerRow, TFT_CYAN);

    if (playerCol == EXIT_COL && playerRow == EXIT_ROW) {
      nextLevel();
    }
  }
}

inline void MazeGame::render() {
  if (!needsFirstDraw) return;
  needsFirstDraw = false;
  
  drawMaze();
  drawPlayer(playerCol, playerRow, TFT_CYAN);
  drawTimer();
  
  screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
  screen->setTextSize(1);
  screen->setCursor(2, 129);
  screen->print("Atteins S! Score:");
  screen->print(score);
}

inline void MazeGame::forceRedraw() {
  needsFirstDraw = true;
}

#endif