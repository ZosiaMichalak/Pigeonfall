#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

enum class AnimState { IDLE, WALK, DASH, ATTACK };

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
    static int                          persistentXP;
    static int                          persistentLevel;
    static int                          persistentSkillPoints;
    static std::array<int, SKILL_COUNT> persistentUpgrades;
    static bool                         persistentSecondChanceUsed;

    sf::Sprite         sprite;
    sf::Texture        textureIdle;
    sf::Texture        textureWalk;
    sf::Texture        textureAttack;
    sf::RectangleShape fallbackShape;

    bool hasIdleTexture;
    bool hasWalkTexture;
    bool hasAttackTexture;
    bool facingLeft;

    void applyFacingScale();

    AnimState   currentAnim;
    sf::IntRect currentFrame;
    float animationTimer;
    float frameDuration;
    int   frameWidth;
    int   frameHeight;
    int   currentColumn;
    int   maxColumns;
    int   sheetCols;

    // Movement
    float speed;
    float baseSpeed;

    bool         isDashing;
    float        dashTimer;
    float        dashDuration;
    float        dashCooldownTimer;
    float        dashCooldown;
    sf::Vector2f dashDir;

    // Attack
    bool  isAttacking;
    float attackTimer;
    float attackDuration;
    float attackCooldownTimer;
    float attackCooldownMax;
    int   attackDamage;

    // Sword hitbox
    sf::RectangleShape swordHitbox;

    // Smudge attack effect — 2x2 spritesheet (4 frames, 228x192 each)
    sf::Texture textureSmudge;
    sf::Sprite  smudgeSprite;
    bool        hasSmudgeTexture;
    int         smudgeFrame;
    float       smudgeFrameTimer;
    int         smudgeFrameW;
    int         smudgeFrameH;
    float       smudgeScale;

    // HP
    int   hp;
    int   maxHp;
    bool  isInvincible;
    float invincibilityTimer;
    bool  isDead;

    // XP / Level
    int xp;
    int xpToNextLevel;
    int level;
    int skillPoints;

    std::array<int, SKILL_COUNT> upgradeLevels;

    void applySkillStats();
    void updateAttack(float dt);

public:
    Player(float x, float y);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    static void resetRunStats();

    void takeDamage(int amount);
    bool consumeSecondChance();
    bool isDeadNow()    const { return isDead; }
    bool isDashingNow() const { return isDashing; }

    bool          isAttackingNow()    const { return isAttacking; }
    sf::FloatRect getSwordBounds()    const { return swordHitbox.getGlobalBounds(); }
    int           getComboHitDamage() const { return attackDamage; }

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

    void startDash(sf::Vector2f moveDir);
};

#endif
