#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <iostream>

enum class AnimState { IDLE, WALK, DASH, ATTACK };

class Player : public GameObject {
private:
    sf::Sprite   sprite;
    sf::Texture  textureIdle;
    sf::Texture  textureWalk;
    sf::Texture  textureAttack;

    sf::RectangleShape swordHitbox;
    sf::RectangleShape fallbackShape;
    float speed;

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

    void setAnim(AnimState next, int frames, float dur,
                 sf::Texture& tex, int cols, int rows);

    bool isDashing;
    float dashTimer, dashDuration, dashCooldownTimer, dashCooldown;
    sf::Vector2f dashDir;

    bool  isAttacking;
    float attackTimer, attackDuration, attackCooldownTimer, attackAngle;

    // Stałe i zmienne combo przeniesione wyżej, aby metody inline mogły z nich korzystać
    static constexpr float ATTACK_COOLDOWN_NORMAL = 0.35f;
    static constexpr float ATTACK_COOLDOWN_COMBO   = 0.70f;
    static constexpr float COMBO_WINDOW            = 0.55f;
    static constexpr int   COMBO_FINISHER_HIT      = 3;

    int   comboCount;      
    float comboWindowTimer; 
    bool  comboWindowOpen; 
    int   pendingDamage;   

    int   hp, maxHp;
    bool  isInvincible;
    float invincibilityTimer;
    sf::RectangleShape hpBarBack, hpBarFront;

    void updateAttack(float dt, sf::RenderWindow& window);
    int sheetCols;

public:
    Player(float x, float y);
    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getSwordBounds() const { return swordHitbox.getGlobalBounds(); }
    bool isAttackingNow() const          { return isAttacking; }
    void startDash(sf::Vector2f moveDir);
    bool isDashingNow()   const          { return isDashing; }
    sf::FloatRect getBounds() const;
    int  getHp()          const          { return hp; }

    float getAttackCooldownTimer() const { return attackCooldownTimer; }
    float getAttackCooldownMax() const   { return comboCount >= 2 ? ATTACK_COOLDOWN_COMBO : ATTACK_COOLDOWN_NORMAL; }
    float getDashCooldownTimer() const   { return dashCooldownTimer; }
    float getDashCooldownMax() const     { return dashCooldown; }
    bool isDashOnCooldown() const        { return dashCooldownTimer > 0.f; }
    bool isAttackOnCooldown() const      { return attackCooldownTimer > 0.f; }

    int  getComboHitDamage() const       { return pendingDamage; }
    bool isComboFinisher()   const       { return isAttacking && comboCount == COMBO_FINISHER_HIT; }

    void takeDamage(int amount);
    void resetHp() { hp = maxHp; isInvincible = false; invincibilityTimer = 0.f; }
    void setPosition(const sf::Vector2f& newPos);
};

#endif