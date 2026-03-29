#include <TFT_eSPI.h>
#include <EEPROM.h>
#include "Game.h"
#include "Menu.h"

// ==== Jeux ====
#include "Snake.h"
#include "Morpion.h"
#include "Pong.h"
#include "Breakout.h"
#include "FlappyBird.h"
#include "Tetris.h"
#include "SpaceInvaders.h"
#include "DinoRun.h"
#include "Maze.h"
#include "Memory.h"

// ==== Broches ====
#define BTN_UP 25
#define BTN_DOWN 26
#define BTN_LEFT 27
#define BTN_RIGHT 32
#define BTN_A 13
#define BTN_B 15
#define BUZZER_PIN 17

// ==== États de la console ====
enum stateConsole {
  STARTING,
  MENU,
  PLAYING,
  PAUSED,  // <--- Nouvel état
  GAMEOVER
};

// ==== Variables globales ====
TFT_eSPI screen;
Menu menu(&screen);
stateConsole consoleState = STARTING;
Game* currentGame = nullptr;
int currentGameId = -1;

// EEPROM : une adresse par jeu (0..6)
const int EEPROM_SIZE = 16;
const int HIGHSCORE_ADDR[] = { 0, 1, 2, 3, 4, 5, 6 };

Buttons buttons;

// ==== Lecture boutons (debouncing par front) ====
void readButtons() {
  static bool lastUp, lastDown, lastLeft, lastRight, lastA, lastB;
  bool nowUp = !digitalRead(BTN_UP);
  bool nowDown = !digitalRead(BTN_DOWN);
  bool nowLeft = !digitalRead(BTN_LEFT);
  bool nowRight = !digitalRead(BTN_RIGHT);
  bool nowA = !digitalRead(BTN_A);
  bool nowB = !digitalRead(BTN_B);

  buttons.upPressed = nowUp && !lastUp;
  buttons.downPressed = nowDown && !lastDown;
  buttons.leftPressed = nowLeft && !lastLeft;
  buttons.rightPressed = nowRight && !lastRight;
  buttons.aPressed = nowA && !lastA;
  buttons.bPressed = nowB && !lastB;

  buttons.up = nowUp;
  buttons.down = nowDown;
  buttons.left = nowLeft;
  buttons.right = nowRight;
  buttons.a = nowA;
  buttons.b = nowB;

  lastUp = nowUp;
  lastDown = nowDown;
  lastLeft = nowLeft;
  lastRight = nowRight;
  lastA = nowA;
  lastB = nowB;
}

// ==== High score par jeu ====
int getHighScore(int gameId) {
  if (gameId < 0 || gameId >= 7) return 0;
  return EEPROM.read(HIGHSCORE_ADDR[gameId]);
}

void setHighScore(int gameId, int score) {
  if (gameId < 0 || gameId >= 7) return;
  if (score > getHighScore(gameId)) {
    EEPROM.write(HIGHSCORE_ADDR[gameId], score);
    EEPROM.commit();
  }
}

// ==== Écran de démarrage ====
void showBoot() {
  screen.fillScreen(TFT_BLACK);
  screen.drawRect(10, 10, 220, 80, TFT_CYAN);
  screen.drawRect(12, 12, 216, 76, TFT_CYAN);
  screen.setTextColor(TFT_CYAN, TFT_BLACK);
  screen.setTextSize(3);
  screen.setCursor(30, 35);
  screen.print(" ARDUBOY ");
  screen.setTextSize(1);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setCursor(55, 60);
  screen.print("UNE CONSOLE DE JEUX");

  // Mélodie de démarrage (ascendante)
  tone(BUZZER_PIN, 523, 100);
  delay(100);  // Do
  tone(BUZZER_PIN, 659, 100);
  delay(100);  // Mi
  tone(BUZZER_PIN, 784, 100);
  delay(100);  // Sol
  tone(BUZZER_PIN, 1047, 200);
  delay(200);  // Do (octave supérieure)
  noTone(BUZZER_PIN);

  screen.drawRect(20, 100, 200, 15, TFT_WHITE);
  for (int i = 0; i <= 100; i += 2) {
    int largeur = (i * 196) / 100;
    screen.fillRect(22, 102, largeur, 10, TFT_GREEN);
    delay(30);
  }
  delay(500);
  consoleState = MENU;
}

// ==== Écran GAME OVER ====
/ ==== Écran GAME OVER / VICTOIRE ====
void showGameOver() {
  int lastScore = currentGame->getScore();
  int high = getHighScore(currentGameId);
  if (lastScore > high) {
    setHighScore(currentGameId, lastScore);
    high = lastScore;
  }

  bool isVictory = currentGame->isVictory();

  screen.fillScreen(TFT_BLACK);
  
  if (isVictory) {
    // Écran de VICTOIRE (vert)
    screen.setTextColor(TFT_GREEN, TFT_BLACK);
    screen.setTextSize(3);
    screen.setCursor(40, 20);
    screen.print("VICTOIRE!");
    
    // Son de victoire
    tone(BUZZER_PIN, 523, 200); delay(200);
    tone(BUZZER_PIN, 659, 200); delay(200);
    tone(BUZZER_PIN, 784, 400); delay(400);
    noTone(BUZZER_PIN);
  } else {
    // Écran de GAME OVER (rouge)
    screen.setTextColor(TFT_RED, TFT_BLACK);
    screen.setTextSize(3);
    screen.setCursor(30, 20);
    screen.print("GAME OVER");
  }
  
  // Affichage du score
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setTextSize(2);
  screen.setCursor(20, 60);
  screen.print("Score: ");
  screen.print(lastScore);
  
  screen.setTextColor(TFT_YELLOW, TFT_BLACK);
  screen.setCursor(20, 85);
  screen.print("Record: ");
  screen.print(high);
  
  // Instructions
  screen.setTextColor(TFT_CYAN, TFT_BLACK);
  screen.setTextSize(1);
  screen.setCursor(20, 115);
  screen.print("[A] Rejouer  [B] Menu");
}

  screen.fillScreen(TFT_BLACK);
  screen.setTextColor(TFT_RED, TFT_BLACK);
  screen.setTextSize(3);
  screen.setCursor(30, 15);
  screen.print("GAME OVER !");
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setTextSize(2);
  screen.setCursor(20, 55);
  screen.print("SCORE  : ");
  screen.print(lastScore);
  screen.setTextColor(TFT_YELLOW, TFT_BLACK);
  screen.setCursor(25, 75);
  screen.print("MEILLEUR: ");
  screen.print(high);
  screen.setTextColor(TFT_GREEN, TFT_BLACK);
  screen.setTextSize(1);
  screen.setCursor(20, 110);
  screen.print("[A] Rejouer  [B] Menu");
}

// ==== Écran PAUSE ====
void showPauseScreen() {
  screen.fillScreen(TFT_BLACK);
  screen.setTextColor(TFT_ORANGE, TFT_BLACK);
  screen.setTextSize(3);
  screen.setCursor(30, 30);
  screen.print("PAUSE");
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setTextSize(1);
  screen.setCursor(35, 70);
  screen.print("[A] Continuer");
  screen.setCursor(35, 90);
  screen.print("[B] Quitter");
}

// ==== Lancement d’un jeu ====
void launchGame(int gameId) {
  if (currentGame) delete currentGame;
  currentGame = nullptr;
  currentGameId = gameId;

  switch (gameId) {
    case 0: currentGame = new SnakeGame(&screen); break;
    case 1: currentGame = new MorpionGame(&screen); break;
    case 2: currentGame = new PongGame(&screen); break;
    case 3: currentGame = new BreakoutGame(&screen); break;
    case 4: currentGame = new FlappyBirdGame(&screen); break;
    case 5: currentGame = new TetrisGame(&screen); break;
    case 6: currentGame = new SpaceInvadersGame(&screen); break;
    case 7: currentGame = new DinoRunGame(&screen); break;
    case 8: currentGame = new MazeGame(&screen); break;
    case 9: currentGame = new MemoryGame(&screen); break;
    default:
      consoleState = MENU;
      menu.forceRedraw();
      return;
  }

  if (currentGame) {
    currentGame->init();
    consoleState = PLAYING;
  } else {
    consoleState = MENU;
  }
}

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  screen.init();
  screen.setRotation(1);
  screen.fillScreen(TFT_BLACK);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  EEPROM.begin(EEPROM_SIZE);

  menu.addGame("Snake", "Mange les pommes!", "", 0);
  menu.addGame("Morpion", "Joueur vs IA", "", 1);
  menu.addGame("Pong", "Bat l'IA en 5 points!", "", 2);
  menu.addGame("Breakout", "Casse toutes les briques!", "", 3);
  menu.addGame("Flappy Bird", "Evite les tuyaux!", "", 4);
  menu.addGame("Tetris", "Aligne les lignes!", "", 5);
  menu.addGame("Space Invaders", "Detruit les envahisseurs!", "", 6);
  menu.addGame("Dino Run", "Saute les cactus!", "", 7);
  menu.addGame("Labyrinthe", "Trouve la sortie!", "", 8);
  menu.addGame("Memory", "Trouve les paires!", "", 9);

  showBoot();
}

// ==== LOOP ====
void loop() {
  readButtons();

  switch (consoleState) {
    case STARTING:
      break;

    case MENU:
      menu.update(buttons);
      menu.render();
      if (buttons.aPressed) {
        int id = menu.getSelectedId();
        launchGame(id);
      }
      break;

    case PLAYING:
      if (currentGame) {
        // Vérifier si l'utilisateur veut mettre en pause (touche B)
        if (buttons.bPressed) {
          consoleState = PAUSED;
          showPauseScreen();  // affiche immédiatement l'écran de pause
          break;
        }
        currentGame->update(buttons);
        currentGame->render();
        if (currentGame->isGameOver()) {
          consoleState = GAMEOVER;
        }
      } else {
        consoleState = MENU;
        menu.forceRedraw();
      }
      break;

    case PAUSED:
      if (buttons.aPressed) {
        currentGame->forceRedraw();  // ← force le jeu à tout redessiner
        consoleState = PLAYING;
      } else if (buttons.bPressed) {
        consoleState = MENU;
        menu.forceRedraw();
      }
      break;

    case GAMEOVER:
      {
        static bool gameOverDrawn = false;
        if (!gameOverDrawn) {
          showGameOver();
          gameOverDrawn = true;
        }
        if (buttons.aPressed) {
          gameOverDrawn = false;
          launchGame(currentGameId);
        }
        if (buttons.bPressed) {
          gameOverDrawn = false;
          consoleState = MENU;
          menu.forceRedraw();
        }
      }
      break;
  }

  buttons.clearPressed();
  delay(10);
}