/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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
#include "SoundManager.h"
#include "SaveSlotUI.h"

// Enumeration of the different overall states the application can be in.
enum class AppState { MENU, PLAYING, DYING, GAME_OVER, VICTORY_PENDING, VICTORY, SLOT_SELECT };

// Core Game class managing the main game loop, level progression, entity lifetime, input handling, and screen states.
class Game {
private:
    sf::RenderWindow window;      // Main window object
    sf::Clock        clock;       // Clock used to compute frame delta time
    sf::Font         font;        // Primary font resource loaded for text rendering

    AppState                  appState;
    std::unique_ptr<MainMenu> mainMenu;
    MenuAction                pendingMenuAction = MenuAction::NONE;

    std::vector<std::unique_ptr<GameObject>> objects;            // All active entities (player, enemies, projectiles)
    std::vector<std::unique_ptr<Room>>       rooms;              // Visited rooms history
    int currentRoomIndex;                                        // Current active room index (0-based)
    int enemiesRemainingToSpawn;                                 // Number of enemies still waiting to be spawned in the room

    std::mt19937 rng;                                            // Random number generator

    // Door properties for room transition portal
    sf::RectangleShape doorShape;
    sf::Texture        doorTexture;
    sf::Sprite         doorSprite;
    sf::Text           interactText;

    // Input state flags to prevent multi-triggering
    bool wasEPressed;
    bool wasFPressed;

    bool isFullscreen;
    bool wasF11Pressed;

    // User Interface components
    HUD         hud;
    SkillTreeUI skillTree;
    VendingUI   vendingUI;
    DifficultySelectUI  slotUI;

    bool wasMPressed;             // Key press check for main menu toggles
    int totalCoins;               // Current run's collected gold coins balance
    std::string heldItem;         // Code tag of item currently held (e.g. "ANN_DOG")
    bool nearVending;             // Flag indicating player is within shop range

    // Audio players
    sf::Music music;
    int       musicVolume = 100;

    float playTime   = 0.f;       // Total time elapsed in the current play session
    int   deathCount = 0;         // Number of player deaths tracked

    static constexpr int BOSS_ROOM = 10; // Single boss room index — ends the run

    // Helper utilities to sync volume levels
    void applyMusicVolume();
    void applySFXVolume();

    SoundManager soundMgr;
    int   sfxVolume  = 70;
    float stepsTimer = 0.f;       // Paces player walking step audio cues

    bool pauseInOptions = false;  // Options menu pause state sub-flag
    int  pauseOptSel    = 0;

    bool isPaused;
    int  pauseSel;

    bool isBossRoom = false;      // Checked if current level hosts a boss fight

    // State counters for transitions and fading effects
    float fadeTimer;
    static constexpr float DEATH_ANIM_DURATION   = 2.f;
    static constexpr float DEATH_FADE_DURATION   = 1.2f;
    static constexpr float VICTORY_FADE_DURATION = 1.5f;
    int   gameOverSel;
    float gameOverTimer;
    std::unique_ptr<MainMenu> gameOverMenu;

    float victoryTimer = 0.f;
    int   victorySel   = 0;

    // Structure defining portal/spawn indicators when spawning enemies
    struct SpawnEffect {
        sf::Vector2f pos;
        float        timer;
        bool         isDash;
        int          tier;
        std::unique_ptr<GameObject> pendingObject;
        bool         isBossSpawn = false;
        static constexpr float SPAWN_ANIM_DUR      = 0.4f;
        static constexpr float BOSS_SPAWN_ANIM_DUR = 0.3f;
        float duration() const { return isBossSpawn ? BOSS_SPAWN_ANIM_DUR : SPAWN_ANIM_DUR; }
    };
    std::vector<SpawnEffect> pendingSpawns;

    sf::Texture spawnTexBullet;
    sf::Texture spawnTexDash;
    bool        spawnTexLoaded = false;

    // Spawning graphics helpers
    void loadSpawnTextures();
    void updateSpawnEffects(float dt);
    void drawSpawnEffects();

    // Context-sensitive prompt "E" indicator
    sf::Texture eButtonTexture;
    sf::Sprite  eButtonSprite;
    bool        eButtonLoaded  = false;
    float       eButtonFrame   = 0.f;
    float       eButtonAnim    = 0.f;
    float       eButtonAlpha   = 0.f;
    bool        eButtonVisible = false;

    void loadEButton();
    void drawEButton(sf::Vector2f pos);

    // Damage floating text numbers popups
    struct DamageNumber {
        sf::Text     text;
        sf::Vector2f velocity;
        float        lifetime;
        float        maxLifetime;
    };
    std::vector<DamageNumber> damageNumbers;

    void spawnDamageNumber(sf::Vector2f pos, int damage, bool isBig = false);
    void updateDamageNumbers(float dt);
    void drawDamageNumbers();

    // Internal initialization routines
    void initWindow();
    void applyWindowMode();
    void applyLetterboxView();
    void refreshFontTextures();

    // Room and game lifecycle managers
    void spawnEnemy();
    void nextRoom();
    void resetRun(bool resetSessionTimer = true);
    void startNewGame();
    bool isBossRoomIndex(int roomIndex) const;
    void configureBossRoom();
    void buildRoomAt(int roomIndex, int layoutId);
    int  enemiesForRoom(int roomIndex) const;
    void rollVendingForPlayer(Player* player);
    void syncHeldItemAfterMonsterBuff(Player* player);

    bool lastMonsterBuffActive = false;
    void drawScreenFadeOverlay(float elapsed, float fadeDelay, float fadeDuration);

    // Draw screens handlers
    void drawPauseMenu();
    void drawGameOver();
    void drawVictory();

    // Helper for shop/vending proximity checks
    sf::FloatRect getClosestVendingBounds(sf::Vector2f playerCenter) const;

public:
    // Game constructor: initializes systems, creates window, loads shared resources
    Game();
    
    // Starts the main application loop
    void run();
    
    // Updates all entities and game subsystems for the current frame
    void update(float dt);
    
    // Renders the current game screen frame
    void render();
};

#endif