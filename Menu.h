#ifndef MENU_H
#define MENU_H
 
#include <TFT_eSPI.h>
#include <pgmspace.h>
#include "Game.h"
 
// ─────────────────────────────────────────────
// BITMAPS 16x16 en PROGMEM (1 bit par pixel)
// 1 = pixel allumé  |  0 = fond transparent
// Chaque ligne = 2 octets = 16 pixels
// ─────────────────────────────────────────────
 
// Snake : corps sinueux + pomme
static const uint8_t PROGMEM icon_snake[] = {
  0b00000000, 0b00000000,
  0b00000001, 0b11100000,
  0b00000010, 0b00100000,
  0b00000010, 0b10100000,
  0b00000001, 0b11000000,
  0b00000111, 0b10000000,
  0b00001000, 0b00000000,
  0b00011111, 0b11000000,
  0b00100000, 0b01000000,
  0b00100000, 0b01000000,
  0b00011111, 0b11000000,
  0b00000000, 0b00000000,
  0b00111100, 0b00000000,
  0b01000010, 0b00000000,
  0b01000010, 0b00000000,
  0b00111100, 0b00000000,
};
 
// Morpion : grille 3x3 avec X et O
static const uint8_t PROGMEM icon_morpion[] = {
  0b10001000, 0b10000000,
  0b10001000, 0b10000000,
  0b11111111, 0b11000000,
  0b10001000, 0b10000000,
  0b10001000, 0b10000000,
  0b11111111, 0b11000000,
  0b10001000, 0b10000000,
  0b10001000, 0b10000000,
  0b00000000, 0b00000000,
  0b10100010, 0b10000000,
  0b01000001, 0b00000000,
  0b00100000, 0b00000000,  // centre X
  0b01000001, 0b00000000,
  0b10100010, 0b10000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};
 
// Pong : 2 raquettes + balle
static const uint8_t PROGMEM icon_pong[] = {
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000011, 0b10000011,
  0b11000111, 0b11000011,
  0b11000111, 0b11000011,
  0b11000011, 0b10000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
  0b11000000, 0b00000011,
};
 
// Dino Run : dinosaure + sol
static const uint8_t PROGMEM icon_dino[] = {
  0b00000011, 0b11000000,
  0b00000101, 0b11000000,
  0b00000111, 0b11000000,
  0b00000111, 0b10000000,
  0b00011111, 0b10000000,
  0b00111111, 0b00000000,
  0b00111110, 0b00000000,
  0b00011100, 0b00000000,
  0b00001100, 0b00000000,
  0b00001100, 0b00000000,
  0b00001101, 0b00000000,
  0b00001100, 0b10000000,
  0b00001000, 0b10000000,
  0b11111111, 0b11111111,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
};
 
// Space Invaders : alien classique
static const uint8_t PROGMEM icon_space[] = {
  0b00100000, 0b00100000,
  0b00010000, 0b01000000,
  0b00111111, 0b11100000,
  0b01101101, 0b10110000,
  0b11111111, 0b11111000,
  0b10111111, 0b11101000,
  0b10100000, 0b00101000,
  0b00011011, 0b01100000,
  0b00000000, 0b00000000,
  0b00000011, 0b11000000,
  0b00001111, 0b11110000,
  0b00011111, 0b11111000,
  0b00011011, 0b01111000,
  0b00011111, 0b11111000,
  0b00001001, 0b10110000,
  0b00000000, 0b00000000,
};
 
// Breakout : briques + balle + raquette
static const uint8_t PROGMEM icon_breakout[] = {
  0b01110111, 0b01110000,
  0b01110111, 0b01110000,
  0b00000000, 0b00000000,
  0b11101110, 0b11100000,
  0b11101110, 0b11100000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000110, 0b00000000,
  0b00001111, 0b00000000,
  0b00000110, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b01111111, 0b11100000,
  0b01111111, 0b11100000,
};

// FIX: Memory — grille de cartes 2x3 avec un symbole "?" au centre
static const uint8_t PROGMEM icon_memory[] = {
  0b11011011, 0b01100000,
  0b10011001, 0b00100000,
  0b10011001, 0b00100000,
  0b11011011, 0b01100000,
  0b00000000, 0b00000000,
  0b11011011, 0b01100000,
  0b10011001, 0b00100000,
  0b10111001, 0b11100000,
  0b10011001, 0b00100000,
  0b11011011, 0b01100000,
  0b00000000, 0b00000000,
  0b11011011, 0b01100000,
  0b10011001, 0b00100000,
  0b10011001, 0b00100000,
  0b11011011, 0b01100000,
  0b00000000, 0b00000000,
};
 
// ─────────────────────────────────────────────
// Structure MenuItem
// ─────────────────────────────────────────────
struct MenuItem {
  const char*    name;
  int            gameId;
  const uint8_t* icon;
  uint16_t       color;
};
 
// ─────────────────────────────────────────────
// Classe Menu
// ─────────────────────────────────────────────
class Menu {
private:
  TFT_eSPI* screen;
  MenuItem  items[10];
  int       itemCount;
  int       selectedIndex;
  int       scrollOffset;
  bool      needsRedraw;
 
  // FIX: VISIBLE passe de 4 à 5 pour que Memory soit visible plus tôt
  //      ITEM_H réduit de 24 à 21 pour tenir dans les 135px d'écran
  static const int VISIBLE = 5;
  static const int ITEM_H  = 21;
  static const int LIST_Y  = 26;
  static const int ICON_X  = 2;
  static const int ICON_W  = 16;
  static const int ICON_H  = 16;
  static const int NAME_X  = 36;
 
  // Dessin bitmap 16x16 depuis PROGMEM pixel par pixel
  void drawIcon(const uint8_t* bmp, int x, int y, uint16_t fg, uint16_t bg) {
    for (int row = 0; row < ICON_H; row++) {
      uint8_t  b0   = pgm_read_byte(bmp + row * 2);
      uint8_t  b1   = pgm_read_byte(bmp + row * 2 + 1);
      uint16_t bits = ((uint16_t)b0 << 8) | b1;
      for (int col = 0; col < ICON_W; col++) {
        uint16_t c = (bits & (0x8000 >> col)) ? fg : bg;
        screen->drawPixel(x + col, y + row, c);
      }
    }
  }
 
public:
  Menu(TFT_eSPI* display) {
    screen        = display;
    itemCount     = 0;
    selectedIndex = 0;
    scrollOffset  = 0;
    needsRedraw   = true;
  }
 
  // Ajouter un jeu au menu
  void addGame(const char* name, int gameId, const uint8_t* icon, uint16_t color) {
    if (itemCount < 10) {
      items[itemCount++] = { name, gameId, icon, color };
    }
  }
 
  // Mise à jour navigation (à appeler dans loop)
  int update(Buttons buttons) {
    unsigned long now = millis();
    static unsigned long lastNav = 0;
    if (now - lastNav < 200) return -1;
 
    if (buttons.downPressed) {
      lastNav = now;
      selectedIndex++;
      if (selectedIndex >= itemCount) {
        selectedIndex = 0;
        scrollOffset  = 0;
      } else if (selectedIndex >= scrollOffset + VISIBLE) {
        scrollOffset = selectedIndex - VISIBLE + 1;
      }
      needsRedraw = true;
    }
 
    if (buttons.upPressed) {
      lastNav = now;
      selectedIndex--;
      if (selectedIndex < 0) {
        selectedIndex = itemCount - 1;
        scrollOffset  = max(0, itemCount - VISIBLE);
      } else if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
      }
      needsRedraw = true;
    }
 
    return -1;
  }
 
  int getSelectedId() {
    return items[selectedIndex].gameId;
  }
 
  // Rendu du menu (ne redessine que si nécessaire)
  void render() {
    if (!needsRedraw) return;
    needsRedraw = false;
 
    screen->fillScreen(TFT_BLACK);
 
    // Titre
    screen->setTextColor(TFT_CYAN, TFT_BLACK);
    screen->setTextSize(2);
    screen->setCursor(50, 2);
    screen->print("ARDUBOY");
 
    // Flèche scroll haut
    if (scrollOffset > 0) {
      // FIX: couleur plus visible (TFT_YELLOW au lieu de TFT_WHITE sur noir)
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(75, 18);
      screen->print("^^  plus haut  ^^");
    }
 
    // Liste des items
    for (int i = 0; i < VISIBLE; i++) {
      int idx   = scrollOffset + i;
      if (idx >= itemCount) break;
 
      int y     = LIST_Y + i * ITEM_H;
      int iconY = y + (ITEM_H - ICON_H) / 2;
 
      if (idx == selectedIndex) {
        screen->fillRect(0, y, 240, ITEM_H, TFT_DARKGREY);
 
        drawIcon(items[idx].icon, ICON_X, iconY, items[idx].color, TFT_DARKGREY);
 
        screen->setTextColor(TFT_CYAN, TFT_DARKGREY);
        screen->setTextSize(1);
        screen->setCursor(20, y + 7);
        screen->print(">");
 
        screen->setTextSize(2);
        screen->setTextColor(TFT_WHITE, TFT_DARKGREY);
        screen->setCursor(NAME_X, y + 3);
        screen->print(items[idx].name);
 
      } else {
        screen->fillRect(0, y, 240, ITEM_H, TFT_BLACK);
 
        drawIcon(items[idx].icon, ICON_X, iconY, TFT_DARKGREY, TFT_BLACK);
 
        screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        screen->setTextSize(2);
        screen->setCursor(NAME_X, y + 3);
        screen->print(items[idx].name);
      }
    }
 
    // FIX: flèche scroll bas — couleur TFT_YELLOW bien visible
    if (scrollOffset + VISIBLE < itemCount) {
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(75, LIST_Y + VISIBLE * ITEM_H + 1);
      screen->print("vv  plus bas  vv");
    }
 
    // Compteur position
    screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
    screen->setTextSize(1);
    screen->setCursor(195, 126);
    screen->print(selectedIndex + 1);
    screen->print("/");
    screen->print(itemCount);
 
    // Aide boutons
    screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
    screen->setTextSize(1);
    screen->setCursor(5, 126);
    screen->print("[A] Jouer [^v] Nav");
  }
 
  void forceRedraw() {
    needsRedraw = true;
  }
};
 
#endif