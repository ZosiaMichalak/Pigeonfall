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
#include "PigeonKing.h"

enum class AppState { MENU, PLAYING, DYING, GAME_OVER };

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

    sf::RectangleShape doorShape;   // kept for collision bounds only (invisible)
    sf::Texture        doorTexture;
    sf::Sprite         doorSprite;
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
    int  pauseSel;

    bool isBossRoom = false;   // true when room 4 boss fight is active

    // ── Death fade & game-over screen ─────────────────────────────────────────
    float fadeTimer;          // counts up during DYING state
    static constexpr float FADE_DURATION = 1.2f;
    int   gameOverSel;        // always 0 for now (only "Reborn")
    float gameOverTimer;      // drives animations on game-over screen
    std::unique_ptr<MainMenu> gameOverMenu; // allocated once on GAME_OVER entry

    // ── Spawn effects ─────────────────────────────────────────────────────────
    struct SpawnEffect {
        sf::Vector2f pos;
        float        timer;
        bool         isDash;
        int          tier;
        std::unique_ptr<GameObject> pendingObject; // set for boss-spawned enemies
        bool         isBossSpawn = false;
        static constexpr float SPAWN_ANIM_DUR      = 0.4f;
        static constexpr float BOSS_SPAWN_ANIM_DUR = 0.3f;
        float duration() const { return isBossSpawn ? BOSS_SPAWN_ANIM_DUR : SPAWN_ANIM_DUR; }
    };
    std::vector<SpawnEffect> pendingSpawns;

    // Loaded once; shared across all spawn effects
    sf::Texture spawnTexBullet;
    sf::Texture spawnTexDash;
    bool        spawnTexLoaded = false;

    void loadSpawnTextures();
    void updateSpawnEffects(float dt);
    void drawSpawnEffects();

    // ── Floating damage numbers ───────────────────────────────────────────────
    struct DamageNumber {
        sf::Text     text;
        sf::Vector2f velocity;
        float        lifetime;   // seconds remaining
        float        maxLifetime;
    };
    std::vector<DamageNumber> damageNumbers;

    void spawnDamageNumber(sf::Vector2f pos, int damage, bool isBig = false);
    void updateDamageNumbers(float dt);
    void drawDamageNumbers();

    void initWindow();
    void applyWindowMode();
    void applyLetterboxView();
    void refreshFontTextures();

    void spawnEnemy();
    void nextRoom();
    void resetRun();

    void drawPauseMenu();
    void drawGameOver();
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
