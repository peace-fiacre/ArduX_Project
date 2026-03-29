// Le fichier Menu.h

#ifndef MENU_H
#define MENU_H

#include <TFT_eSPI.h>
#include "Game.h"

#ifndef BUZZER_PIN
  #define BUZZER_PIN 17
#endif

//Structure d'un jeu dans le menu
struct MenuItem {
  String name;
  String description;
  String emoji;
  int gameId;
};

class Menu {
  private:

    TFT_eSPI* screen;

    MenuItem items[10];
    int itemCount;

    int selectedIndex;  // index global du jeu sélectionné (0 à itemCount-1)
    int scrollOffset;   // index du premier jeu affiché à l'écran

    bool needsRedraw;

    // Nombre de jeux visibles simultanément
    // Calcul : titre=20px + 4×24px + bas=15px = 131px ≤ 135px ✅
    const int VISIBLE = 4;

  public:

    // Constructeur
    Menu(TFT_eSPI* display) {
      screen        = display;
      itemCount     = 0;
      selectedIndex = 0;
      scrollOffset  = 0;
      needsRedraw   = true;
    }

    // Ajouter un jeu à la liste
    void addGame(String name, String description, String emoji, int gameId) {
      if (itemCount < 10) {
        items[itemCount].name        = name;
        items[itemCount].description = description;
        items[itemCount].emoji       = emoji;
        items[itemCount].gameId      = gameId;
        itemCount++;
      }
    }

    // Update — navigation + scroll automatique
    int update(Buttons buttons) {
      unsigned long now = millis();
      static unsigned long lastNavigation = 0;

      // ── Navigation BAS ──
      if (buttons.downPressed && (now - lastNavigation > 200)) {
        selectedIndex++;
        if (selectedIndex >= itemCount) {
          selectedIndex = 0;
          scrollOffset  = 0;
        } else if (selectedIndex >= scrollOffset + VISIBLE) {
          scrollOffset = selectedIndex - VISIBLE + 1;
        }

        // Bip navigation bas — ton aigu court
        tone(BUZZER_PIN, 900, 30);

        needsRedraw    = true;
        lastNavigation = now;
      }

      // ── Navigation HAUT ──
      if (buttons.upPressed && (now - lastNavigation > 200)) {
        selectedIndex--;
        if (selectedIndex < 0) {
          selectedIndex = itemCount - 1;
          scrollOffset  = max(0, itemCount - VISIBLE);
        } else if (selectedIndex < scrollOffset) {
          scrollOffset = selectedIndex;
        }

        // Bip navigation haut — ton aigu court
        tone(BUZZER_PIN, 900, 30);

        needsRedraw    = true;
        lastNavigation = now;
      }

      return -1;
    }

    // Retourner l'ID du jeu sélectionné
    int getSelectedId() {
      return items[selectedIndex].gameId;
    }

    // Render — affiche VISIBLE jeux à la fois
    void render() {
      if (!needsRedraw) return;
      needsRedraw = false;

      screen->fillScreen(TFT_BLACK);

      // Titre
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(2);
      screen->setCursor(50, 2);
      screen->print("ARDUBOY");

      // Indicateur scroll HAUT
      if (scrollOffset > 0) {
        screen->setTextColor(TFT_WHITE, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(112, 18);
        screen->print("^ plus haut");
      }

      // Liste des jeux visibles
      for (int i = 0; i < VISIBLE; i++) {
        int idx = scrollOffset + i;
        if (idx >= itemCount) break;

        int y = 26 + (i * 24);

        if (idx == selectedIndex) {
          screen->fillRect(0, y, 240, 22, TFT_DARKGREY);
          screen->setTextColor(TFT_CYAN, TFT_DARKGREY);
          screen->setTextSize(1);
          screen->setCursor(2, y + 7);
          screen->print("=>");
          screen->setTextSize(2);
          screen->setTextColor(TFT_WHITE, TFT_DARKGREY);
          screen->setCursor(18, y + 3);
          screen->print(items[idx].name);
        } else {
          screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
          screen->setTextSize(2);
          screen->setCursor(18, y + 3);
          screen->print(items[idx].name);
        }
      }

      // Indicateur scroll BAS
      if (scrollOffset + VISIBLE < itemCount) {
        screen->setTextColor(TFT_WHITE, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(112, 122);
        screen->print("v plus bas");
      }

      // Compteur de position
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
      screen->print("[A] Jouer [^/v] Naviguer");
    }

    // Forcer un redessin complet du menu
    void forceRedraw() {
      needsRedraw = true;
    }
};

#endif
