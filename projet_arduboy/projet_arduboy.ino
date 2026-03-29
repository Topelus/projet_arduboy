#include <TFT_eSPI.h>
#include <EEPROM.h>

// Bibliothèques console
#include "Game.h"
#include "Menu.h"

//  Bibliothèques des jeux
#include "Snake.h"
#include "Morpion.h"
#include "Pong.h"
#include "Breakout.h"
#include "FlappyBird.h"
#include "Tetris.h"
#include "SpaceInvaders.h"
//#include "DinoRun.h"
//#include "Maze.h"
//#include "Memory.h"

//  Broches des boutons
#define BTN_UP 25
#define BTN_DOWN 26
#define BTN_LEFT 27
#define BTN_RIGHT 32
#define BTN_A 13
#define BTN_B 15
#define BUZZER_PIN 17

Buttons buttons;

//États de la console
enum stateConsole {
  STARTING,
  MENU,
  PLAYING,
  GAMEOVER
};

int currentGameId = -1;

// Variables globales
TFT_eSPI screen = TFT_eSPI();
Menu menu(&screen);
stateConsole consoleState = STARTING;
Game* currentGame = nullptr;
int highScore = 0;

//  Lecture des boutons
//  Calcule les fronts montants (pressed)
//  = vrai uniquement le frame où le bouton
//    vient d'être appuyé

void readButtons() {
  bool prevUp = buttons.up;
  bool prevDown = buttons.down;
  bool prevLeft = buttons.left;
  bool prevRight = buttons.right;
  bool prevA = buttons.a;
  bool prevB = buttons.b;

  buttons.up = !digitalRead(BTN_UP);
  buttons.down = !digitalRead(BTN_DOWN);
  buttons.left = !digitalRead(BTN_LEFT);
  buttons.right = !digitalRead(BTN_RIGHT);
  buttons.a = !digitalRead(BTN_A);
  buttons.b = !digitalRead(BTN_B);

  buttons.upPressed = buttons.up && !prevUp;
  buttons.downPressed = buttons.down && !prevDown;
  buttons.leftPressed = buttons.left && !prevLeft;
  buttons.rightPressed = buttons.right && !prevRight;
  buttons.aPressed = buttons.a && !prevA;
  buttons.bPressed = buttons.b && !prevB;

  // ── Debug Serial ──
  if (buttons.upPressed) Serial.println("BTN: UP");
  if (buttons.downPressed) Serial.println("BTN: DOWN");
  if (buttons.leftPressed) Serial.println("BTN: LEFT");
  if (buttons.rightPressed) Serial.println("BTN: RIGHT");
  if (buttons.aPressed) Serial.println("BTN: A");
  if (buttons.bPressed) Serial.println("BTN: B");
}


//  Écran de démarrage
//  Affiché une seule fois au boot
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

  // Son de démarrage
  tone(BUZZER_PIN, 523, 150);
  delay(150);
  tone(BUZZER_PIN, 659, 150);
  delay(150);
  tone(BUZZER_PIN, 784, 300);
  delay(300);

  // Barre de chargement
  screen.drawRect(20, 100, 200, 15, TFT_WHITE);
  for (int i = 0; i <= 100; i += 2) {
    int largeur = (i * 196) / 100;
    screen.fillRect(22, 102, largeur, 10, TFT_GREEN);
    delay(30);
  }

  delay(500);
  consoleState = MENU;
}


//  Écran GAME OVER
//  Affiche le score + meilleur score
//  Sauvegarde le meilleur score en EEPROM
//  Appelé UNE SEULE FOIS depuis case GAMEOVER

void showGameOver() {
  highScore = EEPROM.read(0);

  if (currentGame->getScore() > highScore) {
    highScore = currentGame->getScore();
    EEPROM.write(0, highScore);
    EEPROM.commit();
  }

  int lastScore = currentGame->getScore();

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
  screen.print(highScore);

  screen.setTextColor(TFT_GREEN, TFT_BLACK);
  screen.setTextSize(1);
  screen.setCursor(20, 110);
  screen.print("[A] Rejouer  [B] Menu");
}


//  Lancement d'un jeu
//  Supprime l'ancien jeu, crée le nouveau,
//  l'initialise et passe en état PLAYING

void launchGame(int gameId) {
  if (currentGame != nullptr) {
    delete currentGame;
    currentGame = nullptr;
  }

  switch (gameId) {
    case 0: currentGame = new SnakeGame(&screen); break;
    case 1: currentGame = new MorpionGame(&screen); break;
    case 2: currentGame = new PongGame(&screen); break;
    case 3: currentGame = new BreakoutGame(&screen); break;
    case 4: currentGame = new FlappyBirdGame(&screen); break;
    case 5: currentGame = new TetrisGame(&screen); break;
    case 6: currentGame = new SpaceInvadersGame(&screen); break;
      // case 7: currentGame = new DinoRunGame(&screen);       break;
      // case 8: currentGame = new MazeGame(&screen);          break;
      // case 9: currentGame = new MemoryGame(&screen);        break;
  }

  if (currentGame != nullptr) {
    currentGame->init();
    consoleState = PLAYING;
  }
}


//  SETUP
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

  EEPROM.begin(16);

  menu.addGame("Snake", "Mange les pommes!", "", 0);
  menu.addGame("Morpion", "2 joueurs - A pour jouer!", "", 1);
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


//  LOOP
//  Machine à états de la console :
//  STARTING → MENU → PLAYING → GAMEOVER → MENU
void loop() {
  readButtons();

  switch (consoleState) {

    // ── Démarrage ──
    case STARTING:
      break;

    // ── Menu principal ──
    case MENU:
      {
        menu.update(buttons);
        menu.render();

        if (buttons.aPressed) {
          currentGameId = menu.getSelectedId();
          launchGame(currentGameId);
        }
        break;
      }

    // ── Jeu en cours ──
    case PLAYING:
      {
        if (currentGame != nullptr) {
          currentGame->update(buttons);
          currentGame->render();
          if (currentGame->isGameOver()) {
            consoleState = GAMEOVER;
          }
        }
        break;
      }

    // ── Game Over ──
    case GAMEOVER:
      {
        static bool gameOverDrawn = false;

        if (!gameOverDrawn) {
          showGameOver();  // dessiné UNE SEULE FOIS
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
        break;
      }
  }

  delay(1);
}