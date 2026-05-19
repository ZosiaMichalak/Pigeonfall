#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <iostream>

class Player : public GameObject {
private:
    sf::Sprite sprite;
    sf::Texture textureIdle;
    sf::Texture textureWalk;
    sf::RectangleShape swordHitbox;
    sf::RectangleShape fallbackShape;
    float speed;

    bool hasIdleTexture;
    bool hasWalkTexture;

    bool facingLeft;
    void applyFacingScale(); 

    sf::IntRect currentFrame;
    float animationTimer;
    float frameDuration;
    int frameWidth;
    int frameHeight;
    int currentColumn;
    int maxColumns;

    bool isDashing;
    float dashTimer, dashDuration, dashCooldownTimer, dashCooldown;
    sf::Vector2f dashDir;

    bool isAttacking;
    float attackTimer, attackDuration, attackCooldownTimer, attackCooldown, attackAngle;

    int hp;
    int maxHp;
    bool isInvincible;
    float invincibilityTimer;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    void updateAttack(float dt, sf::RenderWindow& window);

public:
    Player(float x, float y);
    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getSwordBounds() const { return swordHitbox.getGlobalBounds(); }
    bool isAttackingNow() const { return isAttacking; }
    void startDash(sf::Vector2f moveDir);
    bool isDashingNow() const { return isDashing; }
    sf::FloatRect getBounds() const;

    int getHp() const { return hp; }
    void takeDamage(int amount);
    void resetHp() { hp = maxHp; isInvincible = false; invincibilityTimer = 0.f; }
    void setPosition(const sf::Vector2f& newPos);
};

#endif
