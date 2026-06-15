/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

// Enum representing the active animation state of the player character.
enum class AnimState { IDLE, WALK, DASH, ATTACK, DIE };

// Structure containing information about a single skill tree upgrade node.
struct SkillDef {
    std::string name;     // Name of the skill (e.g. "SPEED")
    int         maxLevel; // Max upgrade tier allowed
};

static constexpr int SKILL_COUNT = 6;

// Identifiers for individual skills in the upgrades array.
enum SkillID {
    SK_SPEED         = 0,
    SK_HEALTH        = 1,
    SK_ATTACK        = 2,
    SK_ATK_SPEED     = 3,
    SK_DASH_CD       = 4,
    SK_SECOND_CHANCE = 5
};

// Skill parameters definition catalogue.
static const SkillDef SKILL_DEFS[SKILL_COUNT] = {
    { "SPEED",       5 },
    { "HEALTH",      5 },
    { "ATTACK",      3 },
    { "ATK SPEED",   4 },
    { "DASH CD",     4 },
    { "2ND CHANCE",  1 }
};

// Player class representing the main playable character: manages player movement, sword slashes, dash mechanics, leveling, and held items.
class Player : public GameObject {
private:
    // Static player statistics preserved across room changes
    static int                          persistentXP;
    static int                          persistentLevel;
    static int                          persistentSkillPoints;
    static int                          persistentXpToNext;
    static std::array<int, SKILL_COUNT> persistentUpgrades;
    static bool                         persistentSecondChanceUsed;
    static int                          persistentTotemCharges;
    static bool                         persistentTotemBoughtThisRun;

    // Local stats of the current player instance
    int xp;
    int level;
    int skillPoints;
    int xpToNextLevel;
    std::array<int, SKILL_COUNT> upgradeLevels;
    
    int hp;
    int maxHp;
    int attackDamage;
    
    float attackCooldownTimer;
    float attackCooldownMax;
    float dashCooldownTimer;
    float dashCooldown;
    float monsterBuffTimer;
    
    bool isInvincible;
    float invincibilityTimer;
    bool isAttacking;
    bool isDashing;
    
    bool  isDead;
    bool  facingLeft;             // Direction scale helper
    float baseSpeed;
    float speed;
    float animationTimer;
    float frameDuration;
    AnimState currentAnim;
    sf::Vector2f dashDir;         // Movement direction during dash
    float dashTimer;
    float dashDuration;
    float monsterBuffBaseSpeed;
    bool  monsterOneHitKill;
    float attackTimer;
    float attackDuration;
    float attackAngle;
    int   slashFrameWidth;
    int   slashFrameHeight;

    // Combat combo variables
    int   comboCount;            // Current hit index in slash chain (0..3)
    int   comboSwingCount;       // Number of swings made in rapid succession
    float comboWindowTimer;      // Time remaining before combo damage resets
    float comboLockoutTimer;     // Short pause period after finishing a 3-swing chain
    bool  hitConnectedThisSwing; // Ensures damage is applied only once per sword swing

    // Held item graphics and overlays
    sf::Texture textureHeldIdle;
    sf::Texture textureHeldWalk;
    bool        hasHeldIdle  = false;
    bool        hasHeldWalk  = false;
    std::string loadedHeldItem;
    int         monsterVariant = 0;   // Chosen variant flavor of the Monster Energy item
    bool        monsterWalkLocked = false;

    // Monster power-up state sprites sheet
    sf::Texture textureMonsterMode;
    bool        hasMonsterModeTexture = false;

    // Preloads overlay sprites when switching active items
    void loadHeldTextures(const std::string& item);

    // Core character sprite resource sheets
    sf::Texture textureIdle;
    sf::Texture textureWalk;
    sf::Texture textureAttack;
    sf::Texture textureDash;
    sf::Texture textureDie;
    sf::Texture slashTexture;
    sf::Sprite sprite;
    sf::Sprite slashSprite;
    sf::IntRect currentFrame;
    sf::RectangleShape fallbackShape;

    // Spritesheet dimension parameters
    int currentColumn;
    int maxColumns;
    int sheetCols;
    int frameWidth;
    int frameHeight;
    
    bool hasIdleTexture;
    bool hasWalkTexture;
    bool hasAttackTexture;
    bool hasDashTexture;
    bool hasDieTexture;
    bool hasSlashTexture;

    float deathAnimTimer = 0.f;
    int   dieMaxFrames   = 6;
    static constexpr float DEATH_ANIM_DURATION = 2.f;
    
    int slashMaxFrames;
    int slashCols;

    sf::RectangleShape swordHitbox; // Custom rectangular outline to test sword collisions

    // Math/Logic helper overrides
    void applyFacingScale();
    void applySkillStats();
    void setAnim(AnimState anim);
    void updateAttack(float dt, sf::RenderWindow& window);

public:
    // Constructor: loads sprite sheets, resets stats, and positions character
    Player(float x, float y);

    // Updates animation clocks, checks inputs, applies movement vectors, and handles slash combos each frame
    void update(float dt, sf::RenderWindow& window) override;
    
    // Renders the character, active weapon overlay, and slash sword effects
    void draw(sf::RenderWindow& window) override;
    
    // Initiates dash movement
    void startDash(sf::Vector2f moveDir);

    // Collision boundaries getters
    sf::FloatRect getSwordBounds() const { return swordHitbox.getGlobalBounds(); }
    int           getComboHitDamage() const { return attackDamage; }
    
    // Registers a hit during active swing, modifying combo chains and returning scaled damage
    int           registerHit();
    int           getComboCount() const { return comboCount; }
    bool          hasHitThisSwing() const { return hitConnectedThisSwing; }

    float getAttackCooldownTimer() const { return attackCooldownTimer; }
    float getAttackCooldownMax()   const { return attackCooldownMax; }
    float getDashCooldownTimer()   const { return dashCooldownTimer; }
    float getDashCooldownMax()     const { return dashCooldown; }

    sf::FloatRect getBounds()                    const;
    void          setPosition(const sf::Vector2f& p);

    // Stat getters
    int  getHp()                 const { return hp; }
    int  getMaxHp()              const { return maxHp; }
    void addXP(int amount);
    int  getXP()                 const { return xp; }
    int  getXPToNext()           const { return xpToNextLevel; }
    int  getLevel()              const { return level; }
    int  getSkillPoints()        const { return skillPoints; }
    int  getUpgradeLevel(int id) const { return upgradeLevels[id]; }
    
    // Upgrades tree logic check/buy
    bool canBuySkill(int id)     const;
    void buySkill(int id);
    bool isSecondChanceUsed()    const { return persistentSecondChanceUsed; }
    int  getTotemCharges()       const { return persistentTotemCharges; }
    bool hasTotemThisRun()       const { return persistentTotemBoughtThisRun; }
    void markTotemPurchased();
    void addTotemCharge();

    // Consumable item actions
    void  applyMonsterBuff();
    void  assignRandomMonsterEnergyVariant();
    int   getMonsterEnergyVariant() const { return monsterVariant; }
    void  setHeldItem(const std::string& item);
    void  healFull();
    bool  hasMonsterBuff()       const { return monsterBuffTimer > 0.f; }
    
    // Death sequence controls
    bool isDeadNow() const { return isDead; }
    void startDeathAnimation();
    void updateDeathAnimation(float dt);
    
    bool isAttackingNow() const { return isAttacking; }
    bool isDashingNow() const { return isDashing; }
    bool isMonsterOneHit() const { return monsterOneHitKill; }
    
    // Applies damage and updates invincibility cooldowns
    void takeDamage(int amount);
    
    // Attempts to trigger second chance revive, returning true if successful
    bool consumeSecondChance();
    
    // Resets active run-bound items (totems, coins, items)
    static void resetRunStats();
    
    // Clears permanent level progression data (called before starting a new game)
    static void resetProgression();
};

#endif // PLAYER_H