# ArduX — Console de Jeux Embarquée

Console de jeux rétro DIY construite sur **ESP32 TTGO T-Display** dans le cadre des **Arduino Days 2026** (SCOP, Bénin). Le projet implémente 6 jeux classiques avec une architecture orientée objet, une machine à états et un rendu optimisé.

---

## Matériel

| Composant | Détails |
|-----------|---------|
| Carte | ESP32 TTGO T-Display |
| Écran | LCD IPS ST7789 — 135 × 240 px |
| Boutons | 6 boutons poussoirs (UP / DOWN / LEFT / RIGHT / A / B) |
| Audio | Buzzer passif |
| Mémoire | EEPROM émulée en flash (sauvegarde du meilleur score) |

---

## Jeux disponibles

| # | Jeu | Description |
|---|-----|-------------|
| 0 | 🐍 Snake | Mange les pommes sans te mordre |
| 1 | ❌ Morpion | 2 joueurs — alignez 3 symboles |
| 2 | 🏓 Pong | Bat l'IA en 5 points |
| 3 | 🧱 Breakout | Casse toutes les briques |
| 4 | 👾 Space Invaders | Détruit les envahisseurs avant qu'ils descendent |
| 5 | 🦕 Dino Run | Saute les cactus le plus longtemps possible |

---

## Architecture

Le projet repose sur trois couches séparées :
```
COUCHE MATÉRIEL   →   Boutons, écran TFT, buzzer, EEPROM
COUCHE CONSOLE    →   Machine à états, menu scrollable, game over
COUCHE JEUX       →   Chaque jeu est une classe C++ autonome
```

### Machine à états de la console
```
STARTING → MENU → PLAYING → GAMEOVER → PLAYING (rejouer)
                                     → MENU (retour)
```

### Contrat de chaque jeu

Chaque jeu hérite de la classe abstraite `Game` et implémente :
```cpp
void init()                    // initialisation / reset
void update(Buttons buttons)   // logique (entrées + physique)
void render()                  // affichage optimisé
bool isGameOver()              // signale la fin de partie
```

### Optimisations appliquées

- **Timing non bloquant** — `millis()` au lieu de `delay()`, chaque composant a son propre timer
- **Rendu différentiel** — on efface uniquement les pixels qui ont changé, pas tout l'écran
- **Drapeau `needsRedraw`** — les jeux statiques (Morpion) ne redessinent que sur événement
- **Fronts montants** — les entrées utilisent `pressed` (vrai une seule fois par appui) pour éviter la répétition

---

## Structure des fichiers
```
ArduX_Project/
├── arduX.ino          # Point d'entrée — setup(), loop(), machine à états
├── Game.h             # Classe abstraite de base + struct Buttons
├── Menu.h             # Menu scrollable avec navigation
├── Snake.h            # Jeu Snake
├── Morpion.h          # Jeu Morpion (Tic-Tac-Toe)
├── Pong.h             # Jeu Pong avec IA
├── Breakout.h         # Jeu Breakout
├── SpaceInvaders.h    # Jeu Space Invaders
└── DinoRun.h          # Jeu Dino Run
```

---

## Installation

### Prérequis

- Arduino IDE 2.x
- Carte : **ESP32** (via le gestionnaire de cartes Arduino)
- Bibliothèque : **TFT_eSPI** (via le gestionnaire de bibliothèques)

### Configuration TFT_eSPI

Dans le fichier `User_Setup.h` de la bibliothèque TFT_eSPI, activer le driver :
```cpp
#define ST7789_DRIVER
#define TFT_WIDTH  135
#define TFT_HEIGHT 240
```

### Flash

1. Cloner le repo
```bash
git clone https://github.com/peace-fiacre/ArduX_Project.git
```
2. Ouvrir `arduX.ino` dans Arduino IDE
3. Sélectionner la carte **ESP32 Dev Module**
4. Téléverser

---

## Broches

| Bouton | GPIO |
|--------|------|
| UP | 25 |
| DOWN | 26 |
| LEFT | 27 |
| RIGHT | 32 |
| A | 33 |
| B | 15 |
| Buzzer | 17 |

---

## Auteurs

Projet réalisé par la team ArduX lors des **Arduino Days 2026** — SCOP, Bénin.

---

## Licence

Open source — libre d'utilisation et de modification.
