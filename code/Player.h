#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

// Deklaracja wyprzedzająca struktury SaveData
struct SaveData;

enum class AnimState { IDLE, WALK, DASH, ATTACK, DIE };

// ── Skill tree ────────────────────────────────────────────────────────────────
struct SkillDef {
    std::string name;
    int         maxLevel;
};

static constexpr int SKILL_COUNT = 6;
enum SkillID {
    SK_SPEED         = 0,
    SK_HEALTH        = 1,
    SK_ATTACK        = 2,
    SK_ATK_SPEED     = 3,
    SK_DASH_CD       = 4,
    SK_SECOND_CHANCE = 5
};

static const SkillDef SKILL_DEFS[SKILL_COUNT] = {
    { "SPEED",       5 },
    { "HEALTH",      5 },
    { "ATTACK",      3 },
    { "ATK SPEED",   4 },
    { "DASH CD",     4 },
    { "2ND CHANCE",  1 }
};

// ── Player ────────────────────────────────────────────────────────────────────
class Player : public GameObject {
private:
    // Statyczne dane trwałe między uruchomieniami/pokojami
    static int                          persistentXP;
    static int                          persistentLevel;
    static int                          persistentSkillPoints;
    static int                          persistentXpToNext;
    static std::array<int, SKILL_COUNT> persistentUpgrades;
    static bool                         persistentSecondChanceUsed;
    static int                          persistentTotemCharges;
    static bool                         persistentTotemBoughtThisRun;

    // Dane bieżącej instancji gracza
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
    bool  facingLeft;
    float baseSpeed;
    float speed;
    float animationTimer;
    float frameDuration;
    AnimState currentAnim;
    sf::Vector2f dashDir;
    float dashTimer;
    float dashDuration;
    float monsterBuffBaseSpeed;
    bool  monsterOneHitKill;
    float attackTimer;
    float attackDuration;
    float attackAngle;
    int   slashFrameWidth;
    int   slashFrameHeight;

    // ── Combo tracking ────────────────────────────────────────────────────────
    int   comboCount;            // 0..3 — enemy hits in current combo (damage)
    int   comboSwingCount;       // 0..3 — consecutive swings (hits or air)
    float comboWindowTimer;      // time left before combo chain resets
    float comboLockoutTimer;     // 0.1s pause after 3 swings in a row
    bool  hitConnectedThisSwing; // true once per attack swing when an enemy is hit

    // Tekstury i sprite'y SFML
    // ── Held-item overlay textures ────────────────────────────────────────────
    // Loaded lazily when the held item changes
    sf::Texture textureHeldIdle;
    sf::Texture textureHeldWalk;
    bool        hasHeldIdle  = false;
    bool        hasHeldWalk  = false;
    std::string loadedHeldItem;
    int         monsterVariant = 0;
    bool        monsterWalkLocked = false;

    // Dedicated monster-mode animation (player_monsterMode.png)
    sf::Texture textureMonsterMode;
    bool        hasMonsterModeTexture = false;

    void loadHeldTextures(const std::string& item);

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

    sf::RectangleShape swordHitbox;

    // Prywatne metody pomocnicze
    void applyFacingScale();
    void applySkillStats();
    void setAnim(AnimState anim);
    void updateAttack(float dt, sf::RenderWindow& window);

public:
    Player(float x, float y);

    void applyLoadedSave(const SaveData& sd);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;
    
    void startDash(sf::Vector2f moveDir);

    // Gettery i metody pomocnicze
    sf::FloatRect getSwordBounds() const { return swordHitbox.getGlobalBounds(); }
    int           getComboHitDamage() const { return attackDamage; }
    // Called when sword actually hits an enemy; returns damage for this hit
    int           registerHit();
    int           getComboCount() const { return comboCount; }
    bool          hasHitThisSwing() const { return hitConnectedThisSwing; }

    float getAttackCooldownTimer() const { return attackCooldownTimer; }
    float getAttackCooldownMax()   const { return attackCooldownMax; }
    float getDashCooldownTimer()   const { return dashCooldownTimer; }
    float getDashCooldownMax()     const { return dashCooldown; }

    sf::FloatRect getBounds()                    const;
    void          setPosition(const sf::Vector2f& p);

    int  getHp()                 const { return hp; }
    int  getMaxHp()              const { return maxHp; }
    void addXP(int amount);
    int  getXP()                 const { return xp; }
    int  getXPToNext()           const { return xpToNextLevel; }
    int  getLevel()              const { return level; }
    int  getSkillPoints()        const { return skillPoints; }
    int  getUpgradeLevel(int id) const { return upgradeLevels[id]; }
    bool canBuySkill(int id)     const;
    void buySkill(int id);
    bool isSecondChanceUsed()    const { return persistentSecondChanceUsed; }
    int  getTotemCharges()       const { return persistentTotemCharges; }
    bool hasTotemThisRun()       const { return persistentTotemBoughtThisRun; }
    void markTotemPurchased();
    void addTotemCharge();

    void  applyMonsterBuff();
    void  assignRandomMonsterEnergyVariant();
    int   getMonsterEnergyVariant() const { return monsterVariant; }
    void  setHeldItem(const std::string& item); // call whenever heldItem changes
    void  healFull();
    bool  hasMonsterBuff()       const { return monsterBuffTimer > 0.f; }
    
    bool isDeadNow() const { return isDead; }
    void startDeathAnimation();
    void updateDeathAnimation(float dt);
    bool isAttackingNow() const { return isAttacking; }
    bool isDashingNow() const { return isDashing; }
    bool isMonsterOneHit() const { return monsterOneHitKill; }
    void takeDamage(int amount);
    bool consumeSecondChance();
    static void resetRunStats();
    // Clears XP, skills, and level — call before new game or when loading a save slot
    static void resetProgression();
};

#endif // PLAYER_H