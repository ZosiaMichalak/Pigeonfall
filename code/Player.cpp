#include "Player.h"
#include <cmath>
#include <algorithm>
#include <iostream>

static constexpr float PI = 3.14159265f;

// ── Statics ───────────────────────────────────────────────────────────────────
int                          Player::persistentXP               = 0;
int                          Player::persistentLevel            = 1;
int                          Player::persistentSkillPoints      = 0;
int                          Player::persistentXpToNext         = 10;
std::array<int, SKILL_COUNT> Player::persistentUpgrades         = {0,0,0,0,0,0};
bool                         Player::persistentSecondChanceUsed = false;
int                          Player::persistentTotemCharges     = 0;

// ── Constructor ───────────────────────────────────────────────────────────────
Player::Player(float x, float y) : GameObject(x, y) {
    hasIdleTexture   = textureIdle.loadFromFile("assets/player_idle.png");
    hasWalkTexture   = textureWalk.loadFromFile("assets/player_walk.png");
    hasAttackTexture = textureAttack.loadFromFile("assets/player_attack.png");
    hasDashTexture   = textureDash.loadFromFile("assets/player_dash.png");
    
    // Wczytanie slasha
    hasSlashTexture = slashTexture.loadFromFile("assets/slash.png");
    if (hasSlashTexture) {
        slashSprite.setTexture(slashTexture);
        slashMaxFrames = 6;
        slashCols = 2;
        slashFrameWidth = 32;
        slashFrameHeight = 32;
        // Origin na środku lewej krawędzi (oryginalne ustawienie)
        slashSprite.setOrigin(0.f, slashFrameHeight / 2.f); 
    }

    xp            = persistentXP;
    level         = persistentLevel;
    skillPoints   = persistentSkillPoints;
    upgradeLevels = persistentUpgrades;
    xpToNextLevel = persistentXpToNext;

    isDead             = false;
    isInvincible       = false;
    invincibilityTimer = 0.f;
    facingLeft         = false;
    baseSpeed          = 150.f;

    applySkillStats();
    hp = maxHp;

    // Start w stanie IDLE
    frameWidth     = 51;
    frameHeight    = 32;
    animationTimer = 0.f;
    frameDuration  = 0.18f;
    currentColumn  = 0;
    maxColumns     = 3;
    sheetCols      = 1;
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

    monsterBuffTimer    = 0.f;
    monsterBuffBaseSpeed = 0.f;
    monsterOneHitKill   = false;

    isAttacking         = false;
    attackTimer         = 0.f;
    attackDuration      = 0.32f;
    attackCooldownTimer = 0.f;
    attackCooldownMax   = 0.35f;
    attackAngle         = 0.f;

    // Niewidzialny hitbox
    swordHitbox.setSize(sf::Vector2f(26.f, 18.f));
    swordHitbox.setFillColor(sf::Color(255, 255, 200, 130));
    swordHitbox.setOrigin(0.f, 9.f);
}

// ── Animation state switch ────────────────────────────────────────────────────
void Player::setAnim(AnimState anim) {
    if (currentAnim == anim) return;
    currentAnim    = anim;
    currentColumn  = 0;
    animationTimer = 0.f;

    switch (anim) {
    case AnimState::IDLE:
        if (!hasIdleTexture) return;
        sprite.setTexture(textureIdle);
        frameWidth = 51; frameHeight = 32;
        maxColumns = 3;  
        sheetCols  = 1; // 1 kolumna (pionowy pasek)
        frameDuration = 0.18f;
        break;

    case AnimState::WALK:
        if (!hasWalkTexture) { setAnim(AnimState::IDLE); return; }
        sprite.setTexture(textureWalk);
        frameWidth = 51; frameHeight = 32;
        maxColumns = 6;  
        sheetCols  = 2; // 2 kolumny
        frameDuration = 0.1f;
        break;

    case AnimState::DASH:
        if (!hasDashTexture) return;
        sprite.setTexture(textureDash);
        frameWidth = 51; frameHeight = 32;
        maxColumns = 8;  
        sheetCols  = 2; // 2 kolumny
        frameDuration = dashDuration / 8.f;
        break;

    case AnimState::ATTACK:
        if (!hasAttackTexture) return;
        sprite.setTexture(textureAttack);
        frameWidth = 51; frameHeight = 32;
        maxColumns = 5;  
        sheetCols  = 2; // 2 kolumny
        frameDuration = attackDuration / 5.f;
        break;
    }

    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
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
        // No HP restore on level-up
    }
    persistentXP          = xp;
    persistentLevel       = level;
    persistentSkillPoints = skillPoints;
    persistentXpToNext    = xpToNextLevel;
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

void Player::healFull() {
    hp = maxHp;
}

void Player::applyMonsterBuff() {
    monsterBuffTimer  = 10.f;
    monsterOneHitKill = true;
    speed = baseSpeed + persistentUpgrades[SK_SPEED] * 20.f + 200.f; // ultra fast
    hp = std::min(hp + 1, maxHp); // heal 1 HP
}

void Player::addTotemCharge() {
    persistentTotemCharges++;
}

bool Player::consumeSecondChance() {
    if (persistentUpgrades[SK_SECOND_CHANCE] > 0 && !persistentSecondChanceUsed) {
        hp = maxHp;
        isDead = false;
        persistentSecondChanceUsed = true;
        isInvincible       = true;
        invincibilityTimer = 3.0f;
        std::cout << "[Player] Second Chance (skill) triggered!\n";
        return true;
    }
    if (persistentTotemCharges > 0) {
        hp = maxHp;
        isDead = false;
        persistentTotemCharges--;
        isInvincible       = true;
        invincibilityTimer = 3.0f;
        std::cout << "[Player] Second Chance (totem) triggered!\n";
        return true;
    }
    return false;
}

void Player::resetRunStats() {
    persistentSecondChanceUsed = false;
    persistentTotemCharges     = 0;
    // XP, level, skillpoints and upgrades intentionally persist across deaths
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

//Hitbox
sf::FloatRect Player::getBounds() const {
    constexpr float W = 16.f, H = 14.f;
    if (hasIdleTexture)
        return { position.x - W / 2.f, position.y - H / 2.f, W, H };
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

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f delta    = mousePos - position;
        attackAngle = std::atan2(delta.y, delta.x) * 180.f / PI;
        facingLeft  = (delta.x < 0.f);

        setAnim(AnimState::ATTACK);
    }

    if (isAttacking) {
        attackTimer -= dt;

        float rad = attackAngle * PI / 180.f;

        // ZAWSZE aktualizuj niewidzialny hitbox do kolizji
        swordHitbox.setRotation(attackAngle);
        swordHitbox.setPosition(position.x + std::cos(rad) * 6.f,
                                position.y + std::sin(rad) * 6.f);

        if (hasSlashTexture) {
            slashSprite.setRotation(attackAngle); 
            slashSprite.setPosition(position.x + std::cos(rad) * 6.f,
                                    position.y + std::sin(rad) * 6.f);

            // ─── NOWA LOGIKA PING-PONG ───────────────────────────────────────
            float timeElapsed = attackDuration - attackTimer;
            
            // Dla 6 klatek mamy 11 kroków: do przodu (6) i z powrotem (5)
            int totalAnimSteps = slashMaxFrames * 2 - 1; 
            float timePerFrame = attackDuration / totalAnimSteps;
            
            int animStep = static_cast<int>(timeElapsed / timePerFrame);
            if (animStep >= totalAnimSteps) animStep = totalAnimSteps - 1;
            if (animStep < 0) animStep = 0;

            int currentSlashFrame = animStep;
            
            // Jeśli minęliśmy połowę animacji, zaczynamy odliczać wstecz
            if (animStep >= slashMaxFrames) {
                currentSlashFrame = (slashMaxFrames - 1) - (animStep - slashMaxFrames + 1);
            }
            // ─────────────────────────────────────────────────────────────────

            // Wycinanie odpowiedniej klatki z siatki
            int col = currentSlashFrame % slashCols;
            int row = currentSlashFrame / slashCols;

            slashSprite.setTextureRect(sf::IntRect(
                col * slashFrameWidth, 
                row * slashFrameHeight, 
                slashFrameWidth, 
                slashFrameHeight
            ));

            if (attackAngle > 90.f || attackAngle < -90.f) {
                slashSprite.setScale(1.f, -1.f);
            } else {
                slashSprite.setScale(1.f, 1.f);
            }
        }

        if (attackTimer <= 0.f) {
            isAttacking = false;
            setAnim(AnimState::IDLE);
        }
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void Player::update(float dt, sf::RenderWindow& window) {
    if (monsterBuffTimer > 0.f) {
        monsterBuffTimer -= dt;
        if (monsterBuffTimer <= 0.f) {
            // Buff expired – restore normal speed
            speed = baseSpeed + persistentUpgrades[SK_SPEED] * 20.f;
            monsterOneHitKill = false;
        }
    }

    updateAttack(dt, window);

    if (isInvincible) {
        invincibilityTimer -= dt;
        if (invincibilityTimer <= 0.f) isInvincible = false;
    }

    sf::Vector2f moveDir(0.f, 0.f);
    if (dashCooldownTimer > 0.f) dashCooldownTimer -= dt;

    if (isDashing) {
        if (currentAnim != AnimState::ATTACK)
            setAnim(AnimState::DASH);

        position  += dashDir * (speed * 3.f) * dt;
        dashTimer -= dt;

        if (dashTimer <= 0.f) {
            isDashing = false;
            if (currentAnim == AnimState::DASH)
                setAnim(AnimState::IDLE);
        }
    } else {
        bool movingX = false, movingY = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { moveDir.y -= 1.f; movingY = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { moveDir.y += 1.f; movingY = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { moveDir.x -= 1.f; facingLeft = true;  movingX = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { moveDir.x += 1.f; facingLeft = false; movingX = true; }

        bool moving = movingX || movingY;

        if (moving) {
            float len = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
            
            // Zabezpieczenie przed dzieleniem przez zero!
            if (len > 0.f) {
                position += (moveDir / len) * speed * dt;
            }

            if (currentAnim != AnimState::ATTACK && currentAnim != AnimState::DASH)
                setAnim(AnimState::WALK);
        } else {
            if (currentAnim != AnimState::ATTACK && currentAnim != AnimState::DASH)
                setAnim(AnimState::IDLE);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) startDash(moveDir);
    }

    // Border clamping
    if (position.x < 12.f)         position.x = 12.f;
    if (position.x > 388.f)        position.x = 388.f;
    if (position.y < 12.f)         position.y = 12.f;
    if (position.y > 225.f - 40.f) position.y = 225.f - 40.f;

    // ── Advance animation frame ───────────────────────────────────────────────
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
    // 1. Rysowanie gracza (Twój istniejący kod)
    if (hasIdleTexture) {
        bool visible = !isInvincible ||
                       (static_cast<int>(invincibilityTimer / 0.1f) % 2 == 0);
        if (visible) window.draw(sprite);
    } else {
        fallbackShape.setPosition(position);
        window.draw(fallbackShape);
    }

    if (isAttacking) {
        if (hasSlashTexture) {
            window.draw(slashSprite);
        }
    }

    
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