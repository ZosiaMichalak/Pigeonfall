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
#include "SaveSlotUI.h"
#include "Coin.h"
#include "HelperCompanion.h"
#include "AnnoyingDog.h"
#include "PigeonKing.h"
#include "SoundManager.h"

struct SaveData;

// SLOT_SELECT is shown after NEW_GAME or LOAD_GAME is chosen from the main menu
enum class AppState { MENU, SLOT_SELECT, PLAYING, DYING, GAME_OVER, VICTORY_PENDING, VICTORY };

class Game {
private:
    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;

    // ── App state ─────────────────────────────────────────────────────────────
    AppState                  appState;
    std::unique_ptr<MainMenu> mainMenu;
    SaveSlotUI                slotUI;

    // Which slot we are currently playing on (-1 = not set)
    int activeSlot = 0;
    // Remembers which menu action triggered the slot picker
    MenuAction pendingMenuAction = MenuAction::NONE;

    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<std::unique_ptr<Room>> rooms;
    int currentRoomIndex;
    int enemiesRemainingToSpawn;

    std::mt19937 rng;

    sf::RectangleShape doorShape;
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
    std::string heldItem;

    // ── Vending proximity ─────────────────────────────────────────────────────
    bool nearVending;

    // ── Music ─────────────────────────────────────────────────────────────────
    sf::Music music;
    int       musicVolume = 100;

    // ── Play time & deaths (this run) ─────────────────────────────────────────
    float playTime   = 0.f;
    int   deathCount = 0;

    static constexpr int BOSS_ROOM_WEAK   = 25;
    static constexpr int BOSS_ROOM_STRONG = 50;

    void applyMusicVolume();
    void applySFXVolume();

    SoundManager soundMgr;
    int   sfxVolume  = 70;
    float stepsTimer = 0.f;

    // ── Pause sub-menu (OPTIONS) ──────────────────────────────────────────────
    bool pauseInOptions = false;
    int  pauseOptSel    = 0;

    bool isPaused;
    int  pauseSel;

    bool isBossRoom = false;

    // ── Death / victory screen fades ──────────────────────────────────────────
    float fadeTimer;
    static constexpr float DEATH_ANIM_DURATION   = 2.f;
    static constexpr float DEATH_FADE_DURATION   = 1.2f;
    static constexpr float VICTORY_FADE_DURATION = 1.5f;
    int   gameOverSel;
    float gameOverTimer;
    std::unique_ptr<MainMenu> gameOverMenu;

    float victoryTimer = 0.f;
    int   victorySel   = 0;

    // ── Spawn effects ─────────────────────────────────────────────────────────
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

    void loadSpawnTextures();
    void updateSpawnEffects(float dt);
    void drawSpawnEffects();

    // ── E-button interact prompt ──────────────────────────────────────────────
    sf::Texture eButtonTexture;
    sf::Sprite  eButtonSprite;
    bool        eButtonLoaded  = false;
    float       eButtonFrame   = 0.f;    // 0 or 1
    float       eButtonAnim    = 0.f;    // counts up, flips at 0.5s
    float       eButtonAlpha   = 0.f;    // 0-255, smooth fade in/out
    bool        eButtonVisible = false;

    void loadEButton();
    void drawEButton(sf::Vector2f pos);

    // ── Floating damage numbers ───────────────────────────────────────────────
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

    void initWindow();
    void applyWindowMode();
    void applyLetterboxView();
    void refreshFontTextures();

    void spawnEnemy();
    void nextRoom();
    void resetRun(bool resetSessionTimer = true);
    bool isBossRoomIndex(int roomIndex) const;
    void configureBossRoom();
    void applyLoadedRoomState(const SaveData& sd);
    void buildRoomAt(int roomIndex, int layoutId);
    int  enemiesForRoom(int roomIndex) const;
    void rollVendingForPlayer(Player* player);
    void spawnPendingEnemiesAfterLoad();
    void syncHeldItemAfterMonsterBuff(Player* player);

    bool lastMonsterBuffActive = false;
    void drawScreenFadeOverlay(float elapsed, float fadeDelay, float fadeDuration);

    void drawPauseMenu();
    void drawGameOver();
    void drawVictory();

    // Slot-aware save / load
    void saveGame(int slot);
    void loadGame(int slot);

    // Legacy no-arg wrappers (use activeSlot)
    void saveGame() { saveGame(activeSlot); }
    void loadGame() { loadGame(activeSlot); }

    sf::FloatRect getClosestVendingBounds(sf::Vector2f playerCenter) const;

public:
    Game();
    void run();
    void update(float dt);
    void render();
};

#endif
