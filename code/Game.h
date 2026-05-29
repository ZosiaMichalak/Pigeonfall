#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include <string>

#include "GameObject.h"
#include "Room.h"
#include "Enemy.h"
#include "BulletEnemy.h"
#include "DashEnemy.h"
#include "Player.h"
#include "HUD.h"
#include "SkillTreeUI.h"
#include "VendingUI.h"
#include "MainMenu.h"
#include "Coin.h"
#include "HelperCompanion.h"
#include "AnnoyingDog.h"

enum class AppState { MENU, PLAYING };

class Game {
private:
    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;

    // ── App state ─────────────────────────────────────────────────────────────
    AppState                  appState;
    std::unique_ptr<MainMenu> mainMenu;

    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<std::unique_ptr<Room>> rooms;
    int currentRoomIndex;
    int enemiesRemainingToSpawn;

    std::mt19937 rng;

    sf::RectangleShape doorShape;
    sf::Text           interactText;

    bool wasEPressed;
    bool wasFPressed;

    bool isFullscreen;
    bool wasF11Pressed;

    // Sub-systems
    HUD         hud;
    SkillTreeUI skillTree;
    VendingUI   vendingUI;

    bool wasMPressed;

    // ── Coins ─────────────────────────────────────────────────────────────────
    int totalCoins;

    // ── Held item ─────────────────────────────────────────────────────────────
    std::string heldItem;   // empty = nothing held

    // ── Vending proximity ─────────────────────────────────────────────────────
    bool nearVending;       // true when player is within interaction range

    bool isPaused;
    int  pauseSel; // Indeks wybranej opcji w menu pauzy

    void initWindow();
    void applyWindowMode();
    void applyLetterboxView();
    void refreshFontTextures();

    void spawnEnemy();
    void nextRoom();
    void resetRun();

    void drawPauseMenu();
    void saveGame();
    void loadGame();

    // Returns the closest vending-machine bounds in the current room, or an
    // empty rect if none exist.  Uses Room's prop list via getPropColliders()
    // — we store vending positions separately for proximity checks.
    sf::FloatRect getClosestVendingBounds(sf::Vector2f playerCenter) const;



public:
    Game();
    void run();
    void update(float dt);
    void render();
};

#endif
