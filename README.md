# 🎮 Projet Arduboy - Plateforme de Jeux Multi-Genre

Console de jeux portable basée sur **Arduino** avec écran TFT, système de menu intégré et **10 jeux complets**. Architecture modulaire permettant l'ajout facile de nouveaux jeux, système de pause, et high scores persistants par jeu.

## 📋 Table des matières

- [Vue d'ensemble](#vue-densemble)
- [Architecture](#architecture)
- [Matériel requis](#matériel-requis)
- [Installation](#installation)
- [Structure du code](#structure-du-code)
- [Jeux inclus](#jeux-inclus)
- [Système de contrôle](#système-de-contrôle)
- [Guide de développement](#guide-de-développement)
- [Dépannage](#dépannage)

---

## 🎯 Vue d'ensemble

Ce projet transforme une carte Arduino en console de jeux portative complète. Il offre :

- **Interface utilisateur** : Menu de sélection avec scroll automatique et compteur
- **10 jeux complets** : Classiques rétro + jeux modernes optimisés pour TFT
- **Persistance des données** : High scores sauvegardés par jeu en EEPROM
- **Système de pause** : Pause le jeu et reprendre facilement (B = Pause)
- **Système audio** : Buzzer PWM avec mélodie de démarrage
- **Architecture extensible** : Framework objet pour ajouter nouveaux jeux
- **Optimisation graphique** : Rendu partiel et gestion mémoire efficace

### Caractéristiques techniques

| Propriété | Valeur |
|-----------|--------|
| **Plateforme** | Arduino (ESP32 compatible) |
| **Écran** | TFT_eSPI 240×135 pixels |
| **Boutons** | 6 (Directionnels + A/B) |
| **Audio** | Buzzer PWM sur GPIO 17 |
| **Stockage** | EEPROM (scores) |
| **Fréquence** | ~60 FPS |

---

## 🏗️ Architecture

### Machine à états de la console

```
┌─────────┐
│STARTING │  (Écran de démarrage + son)
└────┬────┘
     ↓
┌─────────┐        ┌──────────┐
│  MENU   │◄──────►│ PLAYING  │
└────┬────┘        └────┬─────        ┌────────┐
     └────────────┤ GAMEOVER │        │ PAUSED │
                  └──────────┘        └────┬───┘
                                           ↓
                                      (B = Pause)
     └────────────┤ GAMEOVER │
                  └──────────┘
```

### Diagramme des classes

```
Game (classe abstraite)
├── SnakeGame
├── MorpionGame
├── PongGame
├── BreakoutGame
├── FlappyBirdGame
├── TetrisGame
├── SpaceInvadersGame
├── DinoRunGame          ← NOUVEAU
├── MazeGame             ← NOUVEAU
└── MemoryGame           ← NOUVEAU

Structures supports
├── Buttons (états des boutons)
├── MenuItem (entrées du menu)
└── Autres structures (Segment, Cactus, Cloud, MCell, etc.)
```

---

## 🔧 Matériel requis

### Composants principaux

- **Carte ESP 32** : ESP32 TTGO T-Display
- **Écran TFT** : Module SPI 135×240 (utilisant TFT_eSPI)
- **Buzzer** : Buzzer passif PWM
- **Boutons** : 6 boutons poussoir avec pull-up

### Brochage

| Composant | GPIO |
|-----------|------|
| BTN_UP | 25 |
| BTN_DOWN | 26 |
| BTN_LEFT | 27 |
| BTN_RIGHT | 32 |
| BTN_A | 13 |
| BTN_B | 15 |
| BUZZER_PIN | 17 |
| SPI (TFT) | VSPI (21,22,23) |

### Schéma de connexion

```
TFT_eSPI
├── CS → GPIO 5
├── DC → GPIO 2
├── RST → GPIO 4
├── MOSI → GPIO 23
├── CLK → GPIO 18
└── MISO → GPIO 19

Boutons (tous en INPUT_PULLUP)
├── BTN_UP → GPIO 25
├── BTN_DOWN → GPIO 26
├── BTN_LEFT → GPIO 27
├── BTN_RIGHT → GPIO 32
├── BTN_A → GPIO 13
└── BTN_B → GPIO 15

Buzzer
└── GPIO 17 → Cathode (Anode → +3.3V)
```

---

## 📦 Installation

### 1. Environnement Arduino IDE

```bash
# Installer Arduino IDE (https://www.arduino.cc/en/software)

# Ajouter la carte ESP32 :
# Fichier → Préférences
# URL de gestionnaire de cartes supplémentaires :
# https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

# Outils → Gestionnaire de cartes
# Chercher et installer "esp32" by Espressif Systems
```

### 2. Bibliothèques requises

Dans Arduino IDE, aller à **Sketch** → **Inclure une bibliothèque** → **Gérer les bibliothèques** :

```
- TFT_eSPI (Bodmer)          # Contrôle écran TFT
- EEPROM (ESP32 built-in)    # Stockage persistant
```

### 3. Configuration TFT_eSPI

Éditer `libraries/TFT_eSPI/User_Setup.h` :

```cpp
#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 135

#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_MISO 19

#define SPI_FREQUENCY  40000000  // 40 MHz SPI
```

### 4. Téléversement

```bash
# Dans Arduino IDE :
# 1. Sélectionner Outils → Carte → ESP32 Dev Module
# 2. Sélectionner le port COM approprié
# 3. Vérifier → Téléverser
```

---

## 📁 Structure du code

### Fichiers principaux

```
projet_arduboy/
├── projet_arduboy.ino      # Point d'entrée principal
├── Game.h                  # Classe abstraite Game
├── Menu.h                  # Système de menu avec scroll
├── Buttons (embedded)      # Structure des boutons
│
├── [JEUX - 7 CLASSIQUES]
├── Snake.h                 # Jeu du serpent
├── Morpion.h              # Tic-tac-toe vs IA
├── Pong.h                 # Pong vs IA
├── Breakout.h             # Casse-briques
├── FlappyBird.h           # Oiseaux et tuyaux
├── Tetris.h               # Tetrominos
├── SpaceInvaders.h        # Envahisseurs spatiaux
│
├── [JEUX - 3 NOUVEAUX] ✨
├── DinoRun.h              # Endless runner avec dino
├── Maze.h                 # Labyrinthe procédural
├── Memory.h               # Jeu de paires avec symboles
│
└── README.md              # Cette documentation
```

### Architecture logicielle

#### Classe `Game` (abstraite)

```cpp
class Game {
protected:
    TFT_eSPI* screen;      // Pointeur vers l'écran
    int score;             // Score du jeu
    stateGame state;       // État (START, IN_PROGRESS, GAME_OVER)

public:
    virtual void init();                // Initialiser le jeu
    virtual void update(Buttons btn);   // Mettre à jour la logique
    virtual void render();              // Dessiner à l'écran
    
    bool isGameOver();                  // Vérifier fin de partie
    int getScore();                     // Obtenir le score
    void displayScore();                // Afficher le score en haut
};
```

#### Classe `Menu`

```cpp
class Menu {
private:
    MenuItem items[10];      // Max 10 jeux
    int selectedIndex;       // Index du jeu sélectionné
    int scrollOffset;        // Premier jeu affiché
    const int VISIBLE = 4;   // Jeux visibles simultanément

public:
    void addGame(...);       // Ajouter un jeu
    int update(Buttons);     // Navigation + retour ID si sélectionné
    void render();           // Dessiner le menu
    int getSelectedId();     // Obtenir l'ID du jeu sélectionné
};
```

#### Machine à états principale (loop)

```cpp
enum stateConsole { STARTING, MENU, PLAYING, GAMEOVER };

void loop() {
    readButtons();           // Lire les états des boutons
    
    switch(consoleState) {
        case STARTING:       // Écran de démarrage (1 fois)
        case MENU:           // Menu de sélection
        case PLAYING:        // Jeu en cours
        case GAMEOVER:       // Écran fin de partie
    }
}
```

### Système de boutons

Les boutons utilisent la détection de **fronts montants** pour éviter les répétitions :

```cpp
struct Buttons {
    // États actuels (appui continu)
    bool up, down, left, right, a, b;
    
    // Fronts montants (true UNE SEULE FOIS au premier appui)
    bool upPressed, downPressed, leftPressed, etc.
};
```

**Avantage** : Chaque action requiert un nouvel appui, évitant les déplacements involontaires.

### Système de High Scores

Chaque jeu possède son propre high score en EEPROM :

```cpp
// Adresses EEPROM (1 byte par jeu)
HIGHSCORE_ADDR[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
//                  Snake, Morpion, Pong, Breakout, Flappy, Tetris, Space, Dino, Maze, Memory

int getHighScore(int gameId);
void setHighScore(int gameId, int score);
```

**Mécanisme** :
- Score sauvegardé automatiquement au Game Over
- Le meilleur score persiste entre les redémarrages
- Seules les 3 adresses EEPROM utilisées (sur 7 jeux initiaux)

---

## 🎮 Jeux inclus (10 jeux)

### Jeux Classiques (7 jeux)

#### 1. **Snake** - Mange des pommes

| Aspect | Détails |
|--------|---------|
| **Contrôles** | ↑↓←→ pour diriger |
| **Objectif** | Manger 50+ pommes |
| **Fin** | Collision avec soi-même ou murs |
| **Score** | Croît avec les pommes mangées |
| **Grille** | 30×13 cellules (8px) |
| **Vitesse** | 150 ms entre déplacements |

**Bugs corrigés** :
- Tableau de 200 segments (au lieu de 100) pour grilles grandes
- File de direction pour éviter les demi-tours immédiats
- Vérification que les pommes ne spawning pas sur le serpent

---

### 2. **Morpion** - Tic-tac-toe 2 joueurs

| Aspect | Détails |
|--------|---------|
| **Contrôles** | ↑↓←→ pour curser, A pour jouer |
| **Joueurs** | J1 (X) vs J2 (O) alternés |
| **Victoire** | 3 alignés horizontalement/verticalement/diagonal |
| **Grille** | 3×3 ; cases 36×36 px |
| **Score** | +10 points pour victoire |

---

### 3. **Pong** - Solo vs IA

| Aspect | Détails |
|--------|---------|
| **Contrôles** | ↑↓ pour raquette joueur |
| **IA** | Suit la balle à vitesse réduite |
| **Victoire** | Premier à 5 points gagne |
| **Balle** | Rebondit, accélère légèrement |
| **Rendu optimisé** | Seules les parties mobiles redessinnées |

**Vitesse**:
- Joueur : 3 px/frame
- IA : 2 px/frame (plus facile)
- Balle : 2-3 px/frame (vecteur)

---

### 4. **Breakout** - Casse-briques

| Aspect | Détails |
|--------|---------|
| **Contrôles** | ←→ pour raquette |
| **Grille** | 10×5 briques (22×8 px) |
| **Couleurs** | Rouge → Orange → Jaune → Vert → Cyan |
| **Vies** | 3 vies max |
| **Fin** | Soit casser toutes les briques, soit perdre 3 vies |
| **Score** | +1 point par brique cassée |

**Physique** :
- Rebond selon angle de frappe
- Accélération progressive
- Positions précises en float

---

### 5. **Flappy Bird** - Évite les tuyaux

| Aspect | Détails |
|--------|---------|
| **Contrôles** | A pour battre les ailes (sauter) |
| **Obstacles** | 3 tuyaux simultanés |
| **Espacement** | 90 px entre tuyaux |
| **Gravité** | 0.35 px/frame² |
| **Saut** | Force de -3.2 px/frame |
| **Gap tuyau** | 36 px (variable) |
| **Vitesse tuyaux** | 2 px/frame |

**Écrans spéciaux** :
- Écran d'accueil flottant (oiseau monte/descend)
- Démarrage au premier appui de A
- Offset visuel séparé pour smooth flottaison

---

### 6. **Tetris** - Aligne les lignes

| Aspect | Détails |
|--------|---------|
| **Grille** | 10 colonnes × 18 rangées |
| **Pièces** | 7 tétrominos (I, J, L, O, S, T, Z) |
| **Rotations** | 4 états par pièce |
| **Contrôles** | ←→ déplacer, ↑ ou A pour rotation, ↓ accélérer chute |
| **Cellule** | 7×7 pixels |
| **Timing** | Chute ralentie au départ (500 ms), accélère avec niveau |

**Niveaux** :
- Niveau 1 : 500 ms entre chutes
- Augmente : -60 ms par ligne complète
- Max : 80 ms (fin accélération)

**Logique spéciale** :
- Encodage des pièces en bits (4 rotations × 4 lignes)
- Détection collision robuste
- Suppression de lignes avec décalage

---

### 7. **Space Invaders** - Détruit les envahisseurs

| Aspect | Détails |
|--------|---------|
| **Grille** | 8 colonnes × 3 rangées d'ennemis |
| **Vaisseau** | Joueur en bas, shoot avec A |
| **Ennemis** | 3 types différents (pieuvre, crabe, seiche) |
| **Bombes IA** | Max 3 simultanées, vitesse 2 px/frame |
| **Boucliers** | 3 boucliers; 3 PV chacun |
| **Vies** | 3 vies max |
| **Score** | +10 par ennemi détruit |
| **Victoire** | Tous les ennemis détruits |
| **Fin** | Ennemis atteignent le bas ou 0 vies |

**Mouvements**:
- Ennemis avancent ensemble horizontalement
- Descendent quand touchent les bords
- Accélération progressive
- Animation 2 frames (jambes alternées)

---

### 8. **Dino Run** - Endless Runner

| Aspect | Détails |
|--------|---------|
| **Contrôles** | A pour sauter ; B pour quitter |
| **Obstacles** | Cactus de hauteur variable |
| **Vitesse** | Augmente progressivement (3→8 px/frame) |
| **Score** | +1 par frame logique (60 fps) |
| **Collision** | Dino vs cactus débordement écran |
| **Animation** | Dino anime ses jambes (2 frames) |
| **Physique** | Virgule fixe (×100) pour précision |
| **Sol** | Mouvements parallax, nuages |

**Caractéristiques** :
- Timing frame : 16 ms (~60 FPS) sans `delay()`
- Jump force : 700 (virgule fixe)
- Gravity : 55 /frame
- Génération procédurale des cactus

---

### 9. **Maze** - Labyrinthe Procédural

| Aspect | Détails |
|--------|---------|
| **Grille** | 15×8 cellules (15 px chacune) |
| **Génération** | DFS (Depth-First Search) procédural |
| **Joueur** | Position cursor, mouvements ↑↓←→ |
| **Sortie** | Coin inférieur droit toujours accessible |
| **Objectif** | Atteindre la sortie avant le temps limite |
| **Temps limite** | 120 secondes (120000 ms) |
| **Niveaux** | Augmentent avec chaque sortie trouvée |
| **Score** | Temps restant × 10 |

**Algorithme de génération** :
- Murs initialisés à 0b1111 (tous fermés)
- DFS avec visitation aléatoire des voisins
- Removal des murs entre cellules visitées
- Garantit un chemin unique vers la sortie

---

### 10. **Memory** - Jeu de Paires

| Aspect | Détails |
|--------|---------|
| **Grille** | 4×4 cartes (54×29 px) |
| **Paires** | 8 paires (0-7) de symboles |
| **Symboles** | 8 formes géométriques distinctes |
| **Couleurs** | Couleur unique par paire |
| **Mécanisme** | Retourner 2 cartes, matcher = +10 points |
| **Curseur** | ↑↓←→ pour naviguer, A pour retourner |
| **États carte** | Face cachée (bleu), visible (gris), matchée (vert) |
| **Victoire** | Toutes les paires trouvées |
| **Score** | Paires trouvées × 10 |

**Symboles** :
- 0: Cercle (RED)
- 1: Triangle (GREEN)
- 2: Carré (BLUE)
- 3: Losange (YELLOW)
- 4: Croix (CYAN)
- 5: Étoile (MAGENTA)
- 6: Cœur (ORANGE)
- 7: Hexagone (WHITE)

**Mélange** : Algortihme Fisher-Yates pour distribution uniforme

---

## ⌨️ Système de contrôle

### Mapping des boutons

```
              [UP]
              │ 25
       [LEFT] ┼ [RIGHT]
        27    │  32
              ↓
            [DOWN]
             26

            [B]  [A]
             15   13
```

### Actions par jeu

| Jeu | Contrôles |
|-----|-----------|
| **Snake** | ↑↓←→ diriger ; B annuler |
| **Morpion** | ↑↓←→ curser ; A jouer ; B annuler |
| **Pong** | ↑↓ raquette ; B annuler |
| **Breakout** | ←→ raquette ; B annuler |
| **Flappy** | A sauter ; B annuler |
| **Tetris** | ←→ déplacer ; ↑/A rotation ; ↓ accélérer ; B pause |
| **Space** | ←→ vaisseau ; A tirer ; B pause |
| **Dino Run** | A sauter ; B quitter |
| **Maze** | ↑↓←→ se déplacer ; B quitter |
| **Memory** | ↑↓←→ curser ; A retourner ; B quitter |

### Menu

- **↑↓** : Navigation avec scroll automatique et indicateurs
- **A** : Sélectionner jeu
- **Compteur** : Position actuelle / Total (ex: "5/10")

### Système de Pause

Appuyer sur **B** pendant un jeu affiche l'écran de pause :

```
        PAUSE
   [A] Continuer
   [B] Quitter
```

- **A** : Reprendre le jeu
- **B** : Retour au menu

---

## 🛠️ Guide de développement

### Ajouter un nouveau jeu

#### Étape 1 : Créer la classe

```cpp
// MyGame.h
#ifndef MYGAME_H
#define MYGAME_H

#include "Game.h"

class MyGame : public Game {
private:
    // Variables de jeu
    int playerX, playerY;
    
public:
    MyGame(TFT_eSPI* display) : Game(display) {}
    
    void init() override {
        playerX = 120;
        playerY = 67;
        score = 0;
        state = IN_PROGRESS;
    }
    
    void update(Buttons buttons) override {
        if (buttons.left) playerX -= 2;
        if (buttons.right) playerX += 2;
        
        // Vérifier fin
        if (condition_gameover) {
            state = GAME_OVER;
        }
    }
    
    void render() override {
        screen->fillScreen(TFT_BLACK);
        screen->drawRect(playerX, playerY, 10, 10, TFT_WHITE);
        displayScore();
    }
};

#endif
```

#### Étape 2 : Inclure dans le .ino

```cpp
#include "MyGame.h"

void setup() {
    // ...
    menu.addGame("My Game", "Descriptif", "🎮", 10);  // ID = 10
    // ...
}

void launchGame(int gameId) {
    switch(gameId) {
        case 10: currentGame = new MyGame(&screen); break;
        // ...
    }
    if (currentGame) currentGame->init();
}
```

### Optimisations de performance

#### Rendu partiel

Au lieu de redessiner tout l'écran, ne redessiner que les parties changed :

```cpp
void render() override {
    // Effacer l'ancienne position
    screen->drawRect(prevX, prevY, 10, 10, TFT_BLACK);
    
    // Dessiner la nouvelle
    screen->drawRect(playerX, playerY, 10, 10, TFT_WHITE);
    
    prevX = playerX;
    prevY = playerY;
}
```

#### Timing des updates

Utiliser des délais pour contrôler la fréquence :

```cpp
unsigned long lastUpdate = 0;
const int updateInterval = 16;  // ~60 FPS

if (millis() - lastUpdate < updateInterval) return;
lastUpdate = millis();

// Logique de jeu
```

#### Codage efficace des données

Pour Tetris, utiliser des bits pour stocker les formes :

```cpp
static const uint8_t PIECES[7][4][4];  // 4 bits par ligne

bool cellFilled(int type, int rot, int row, int col) {
    return (PIECES[type][rot][row] >> (3 - col)) & 1;
}
```

---

## 🐛 Dépannage

### L'écran ne s'affiche pas

**Vérification** :
1. Connexions SPI correctes ?
   - CS, DC, RST, MOSI, CLK sur GPIO corrects
2. Configuration TFT_eSPI.h correcte ?
3. Bibliothèque TFT_eSPI installée et à jour ?

**Solution** :
```cpp
// Dans le moniteur série
Serial.begin(115200);
Serial.println("Écran testé");
screen.init();
screen.fillScreen(TFT_RED);  // Doit afficher rouge
```

### Les boutons ne répondent pas

**Vérification** :
1. Boutons branchés en INPUT_PULLUP ? (Avec résistances)
2. Pins GPIO correctes dans le code ?

**Debug** :
```cpp
// Dans readButtons()
if (buttons.upPressed) Serial.println("UP PRESSED");
```

### Stuttering / ralentissements

**Causes possibles** :
- Rendu complet à chaque frame (au lieu de partiel)
- Délai() trop long
- TFT_SPI fréquence trop basse

**Solutions** :
- Activer rendu optimisé (variables `prev*`)
- Augmenter SPI_FREQUENCY à 40 MHz
- Réduire appels `fillRect()` inutiles

### Buzzer silencieux

**Vérification** :
1. GPIO 17 correct pour tone() ?
2. Buzzer branché entre GPIO 17 et GND ?

**Test** :
```cpp
tone(BUZZER_PIN, 440, 500);  // La4 pendant 500ms
```

### Jeu crashe / redémarre

**Causes** :
- Débordement de pile (pile trop personnages snake)
- Accès mémoire invalide
- Exception non gérée

**Solutions** :
- Augmenter la pile : Menu Outils → Taille de pile
- Vérifier tailles d'arrays
- Utiliser Serial.println() pour déboguer

---

## 📊 Performances

### Consommation mémoire

| Composant | Approx. |
|-----------|---------|
| Binaire Arduino | ~300 KB |
| Heap utilisé | ~50-80 KB |
| EEPROM (scores) | 16 bytes |

### Fréquence d'images

- **Menu** : 30-60 FPS (rendu complet)
- **Jeux** : 55-65 FPS (rendu optimisé)
- **Écran de démarrage** : Animations 2D lisses

---

## 📝 Notes de développement

### État du Projet

✅ **Complété et Production Ready**

- Tous les 10 jeux implémentés et testés
- Système de menus et haute scores fonctionnel
- Système de pause intégré
- Optimisations graphiques appliquées
- Documentation technique complète

### Jeux implémentés

| Jeu | Statut | Complétude |
|-----|--------|-----------|
| Snake | ✅ Production | 100% |
| Morpion | ✅ Production | 100% |
| Pong | ✅ Production | 100% |
| Breakout | ✅ Production | 100% |
| Flappy Bird | ✅ Production | 100% |
| Tetris | ✅ Production | 100% |
| Space Invaders | ✅ Production | 100% |
| Dino Run | ✅ Production | 100% |
| Maze | ✅ Production | 100% |
| Memory | ✅ Production | 100% |

### Bugs connus et correctifs appliqués

| Bug | Cause | Fix | Status |
|-----|-------|-----|--------|
| Snake très court crash | Tableau 100 segments insuffisant | Augmenté à 200 | ✅ Corrigé |
| Demi-tour immédiat Snake | Pas de file d'attente direction | Implémenté queue | ✅ Corrigé |
| Pomme sur serpent | Spawn aléatoire non vérifié | Vérification collision | ✅ Corrigé |
| Flappy saccadé | Float position mélangée | Offset visuel séparé | ✅ Corrigé |
| Tetris lent | Vérifications collision redondantes | Caching positions | ✅ Corrigé |

### Améliorations depuis V1.0

**V1.1 - Ajout des 3 nouveaux jeux** :
- ✨ DinoRun : Endless runner avec parallax
- ✨ Maze : Labyrinthe procédural DFS
- ✨ Memory : Jeu de mémoire avec symboles vectoriels
- 🎮 Système de pause global (touche B)
- 💾 High scores par jeu (10 adresses EEPROM)
- 🎨 Menu amélioré avec compteur de position
- 🐛 Tous les bugs critiques corrigés

---

## 📚 Ressources

### Bibliothèques utilisées

- **TFT_eSPI** : https://github.com/Bodmer/TFT_eSPI
- **Arduino Core ESP32** : https://github.com/espressif/arduino-esp32

### Documentations de référence

- [ESP32 GPIO](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32_pinout.html)
- [TFT_eSPI Wiki](https://github.com/Bodmer/TFT_eSPI/wiki)
- [Arduino Tone()](https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/)

---

## 📄 Licence

Projet personnel éducatif. Libre d'utilisation et modification.

---

## ✨ Idées d'Améliorations Futures

- [ ] Animations de transition entre jeux
- [ ] Système de niveaux progressifs
- [ ] Éditeur de palette de couleurs
- [ ] Sons variés par jeu (SFX library)
- [ ] BT Multijoueur réseau (PvP en ligne)
- [ ] Sauvegarde de profil joueur
- [ ] Système d'achievements/badges
- [ ] Leaderboard ligne (SD card)
- [ ] Éditeur de niveaux
- [ ] Thèmes visuels personnalisés

---

## 📄 Licence

Projet personnel éducatif. Libre d'utilisation et modification pour usages non commerciaux.

---

**Dernière mise à jour** : Mars 2026  
**Version** : 1.1 - Complet avec 10 jeux  
**Statut** : ✅ Production Ready
