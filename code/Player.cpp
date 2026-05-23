#include "Player.h"
#include <cmath>
#include <algorithm>
#include <iostream>

static constexpr float PI = 3.14159265f;

// ── Statics ───────────────────────────────────────────────────────────────────
int                          Player::persistentXP               = 0;
int                          Player::persistentLevel            = 1;
int                          Player::persistentSkillPoints      = 0;
std::array<int, SKILL_COUNT> Player::persistentUpgrades         = {0,0,0,0,0,0};
bool                         Player::persistentSecondChanceUsed = false;

// ── Constructor ───────────────────────────────────────────────────────────────
Player::Player(float x, float y) : GameObject(x, y) {
    hasIdleTexture   = textureIdle.loadFromFile("assets/player_idle.png");
    hasWalkTexture   = textureWalk.loadFromFile("assets/player_walk.png");
    hasAttackTexture = textureAttack.loadFromFile("assets/player_attack.png");

    xp            = persistentXP;
    level         = persistentLevel;
    skillPoints   = persistentSkillPoints;
    upgradeLevels = persistentUpgrades;
    xpToNextLevel = 10;

    isDead             = false;
    isInvincible       = false;
    invincibilityTimer = 0.f;
    facingLeft         = false;
    baseSpeed          = 150.f;

    applySkillStats();
    hp = maxHp;

    frameWidth     = 32;
    frameHeight    = 32;
    animationTimer = 0.f;
    frameDuration  = 0.15f;
    currentColumn  = 0;
    maxColumns     = 3;
    sheetCols      = 2;
    currentAnim    = AnimState::IDLE;

    if (hasIdleTexture) {
        sprite.setTexture(textureIdle);
        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
        currentFrame = sf::IntRect(0, 0, frameWidth, frameHeight);
        sprite.setTextureRect(currentFrame);
        sprite.setPosition(position);
        applyFacingScale();
    } else {
        fallbackShape.setSize({10.f, 10.f});
        fallbackShape.setFillColor(sf::Color(0, 120, 255));
        fallbackShape.setOrigin(5.f, 5.f);
        fallbackShape.setPosition(position);
    }

    isDashing         = false;
    dashDuration      = 0.2f;
    dashCooldownTimer = 0.f;
    dashTimer         = 0.f;

    isAttacking         = false;
    attackTimer         = 0.f;
    attackDuration      = 0.16f;
    attackCooldownTimer = 0.f;
    attackCooldownMax   = 0.35f;
    attackAngle         = 0.f;

    // Sword hitbox — visible white-ish rectangle, rotated toward mouse
    swordHitbox.setSize(sf::Vector2f(26.f, 18.f));
    swordHitbox.setFillColor(sf::Color(255, 255, 200, 130));
    swordHitbox.setOrigin(0.f, 9.f);
}

// ── Skills ────────────────────────────────────────────────────────────────────
void Player::applySkillStats() {
    speed  = baseSpeed + persistentUpgrades[SK_SPEED] * 20.f;
    maxHp  = 5         + persistentUpgrades[SK_HEALTH];

    attackDamage = 1 + persistentUpgrades[SK_ATTACK];

    float atkMod = std::max(0.1f, 1.f - persistentUpgrades[SK_ATK_SPEED] * 0.15f);
    attackCooldownMax = 0.35f * atkMod;

    float dshMod = std::max(0.1f, 1.f - persistentUpgrades[SK_DASH_CD] * 0.15f);
    dashCooldown = 1.0f * dshMod;
}

void Player::addXP(int amount) {
    xp += amount;
    while (xp >= xpToNextLevel) {
        xp -= xpToNextLevel;
        level++;
        skillPoints++;
        xpToNextLevel = static_cast<int>(xpToNextLevel * 1.5f);
        hp = maxHp;
    }
    persistentXP          = xp;
    persistentLevel       = level;
    persistentSkillPoints = skillPoints;
}

bool Player::canBuySkill(int id) const {
    return skillPoints > 0 && persistentUpgrades[id] < SKILL_DEFS[id].maxLevel;
}

void Player::buySkill(int id) {
    if (!canBuySkill(id)) return;
    skillPoints--;
    persistentSkillPoints = skillPoints;
    persistentUpgrades[id]++;
    upgradeLevels = persistentUpgrades;
    applySkillStats();
    hp = std::min(hp + 1, maxHp);
}

// ── Damage / death ────────────────────────────────────────────────────────────
void Player::takeDamage(int amount) {
    if (isInvincible || isDashing) return;
    hp -= amount;
    isInvincible       = true;
    invincibilityTimer = 1.0f;
    if (hp <= 0) { hp = 0; isDead = true; }
}

bool Player::consumeSecondChance() {
    if (persistentUpgrades[SK_SECOND_CHANCE] > 0 && !persistentSecondChanceUsed) {
        hp = maxHp;
        isDead = false;
        persistentSecondChanceUsed = true;
        isInvincible       = true;
        invincibilityTimer = 3.0f;
        std::cout << "[Player] Second Chance triggered!\n";
        return true;
    }
    return false;
}

void Player::resetRunStats() {
    persistentSecondChanceUsed = false;
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void Player::applyFacingScale() {
    sprite.setScale(facingLeft ? 1.f : -1.f, 1.f);
}

void Player::setPosition(const sf::Vector2f& newPos) {
    position = newPos;
    if (hasIdleTexture) sprite.setPosition(position);
    else                fallbackShape.setPosition(position);
}

sf::FloatRect Player::getBounds() const {
    if (hasIdleTexture) return sprite.getGlobalBounds();
    return fallbackShape.getGlobalBounds();
}

// ── Attack ────────────────────────────────────────────────────────────────────
void Player::updateAttack(float dt, sf::RenderWindow& window) {
    if (attackCooldownTimer > 0.f) attackCooldownTimer -= dt;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
        !isAttacking && attackCooldownTimer <= 0.f)
    {
        isAttacking         = true;
        attackTimer         = attackDuration;
        attackCooldownTimer = attackCooldownMax;

        // Calculate angle from player to mouse in world coordinates
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f delta    = mousePos - position;
        attackAngle = std::atan2(delta.y, delta.x) * 180.f / PI;

        // Update facing direction based on mouse side
        facingLeft = (delta.x < 0.f);

        // Switch to attack texture — only 2 frames, 3rd is empty
        if (hasAttackTexture) {
            currentAnim    = AnimState::ATTACK;
            sprite.setTexture(textureAttack);
            sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
            currentColumn  = 0;
            animationTimer = 0.f;
            maxColumns     = 2;
            sheetCols      = 3;
            frameDuration  = attackDuration / 2.f;
        }
    }

    if (isAttacking) {
        attackTimer -= dt;

        // Keep sword hitbox rotated toward the original attack angle
        swordHitbox.setRotation(attackAngle);
        float rad     = attackAngle * PI / 180.f;
        float offsetX = std::cos(rad) * 12.f;
        float offsetY = std::sin(rad) * 12.f;
        swordHitbox.setPosition(position.x + offsetX, position.y + offsetY);

        if (attackTimer <= 0.f) {
            isAttacking = false;
            // Return to idle texture
            if (hasIdleTexture) {
                currentAnim    = AnimState::IDLE;
                sprite.setTexture(textureIdle);
                sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
                currentColumn  = 0;
                animationTimer = 0.f;
                maxColumns     = 3;
                sheetCols      = 2;
                frameDuration  = 0.15f;
            }
        }
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void Player::update(float dt, sf::RenderWindow& window) {
    updateAttack(dt, window);

    if (isInvincible) {
        invincibilityTimer -= dt;
        if (invincibilityTimer <= 0.f) isInvincible = false;
    }

    sf::Vector2f moveDir(0.f, 0.f);
    if (dashCooldownTimer > 0.f) dashCooldownTimer -= dt;

    if (isDashing) {
        position  += dashDir * (speed * 3.f) * dt;
        dashTimer -= dt;
        if (dashTimer <= 0.f) isDashing = false;
    } else {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { moveDir.x -= 1.f; facingLeft = true;  }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { moveDir.x += 1.f; facingLeft = false; }

        if (moveDir.x != 0.f || moveDir.y != 0.f) {
            float len = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
            position += (moveDir / len) * speed * dt;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) startDash(moveDir);
    }

    if (position.x < 12.f)         position.x = 12.f;
    if (position.x > 388.f)        position.x = 388.f;
    if (position.y < 12.f)         position.y = 12.f;
    if (position.y > 225.f - 40.f) position.y = 225.f - 40.f;

    // Animation frame advance
    animationTimer += dt;
    if (animationTimer >= frameDuration) {
        animationTimer -= frameDuration;
        currentColumn = (currentColumn + 1) % maxColumns;
    }

    currentFrame = sf::IntRect(
        (currentColumn % sheetCols) * frameWidth,
        (currentColumn / sheetCols) * frameHeight,
        frameWidth, frameHeight);

    if (hasIdleTexture) {
        sprite.setTextureRect(currentFrame);
        sprite.setPosition(position);
        applyFacingScale();
    } else {
        fallbackShape.setPosition(position);
    }
}

// ── Draw ──────────────────────────────────────────────────────────────────────
void Player::draw(sf::RenderWindow& window) {
    if (hasIdleTexture) {
        bool visible = !isInvincible ||
                       (static_cast<int>(invincibilityTimer / 0.1f) % 2 == 0);
        if (visible) window.draw(sprite);
    } else {
        fallbackShape.setPosition(position);
        window.draw(fallbackShape);
    }

    // Sword hitbox drawn on top while attacking
    if (isAttacking) window.draw(swordHitbox);
}

// ── Dash ──────────────────────────────────────────────────────────────────────
void Player::startDash(sf::Vector2f moveDir) {
    if (isDashing || dashCooldownTimer > 0.f) return;
    isDashing         = true;
    dashTimer         = dashDuration;
    dashCooldownTimer = dashCooldown;
    dashDir = (moveDir.x == 0.f && moveDir.y == 0.f)
        ? (facingLeft ? sf::Vector2f(-1.f, 0.f) : sf::Vector2f(1.f, 0.f))
        : moveDir;
}
