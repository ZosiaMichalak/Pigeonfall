#include "Player.h"
#include <cmath>

void Player::setAnim(AnimState next, int frames, float dur,
                     sf::Texture& tex, int cols, int rows)
{
    if (currentAnim == next) return;

    currentAnim    = next;
    maxColumns     = frames;
    frameDuration  = dur;
    sheetCols      = cols;
    currentColumn  = 0;
    animationTimer = 0.f;

    sprite.setTexture(tex);
    sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    applyFacingScale();
    (void)rows; 
}


Player::Player(float x, float y) : GameObject(x, y) {
    hasIdleTexture   = textureIdle.loadFromFile("assets/player_idle.png");
    hasWalkTexture   = textureWalk.loadFromFile("assets/player_walk.png");
    hasAttackTexture = textureAttack.loadFromFile("assets/player_attack.png");

    if (!hasIdleTexture)
        std::cerr << "[BLAD] assets/player_idle.png not found\n";
    if (!hasWalkTexture)
        std::cerr << "[BLAD] assets/player_walk.png not found\n";
    if (!hasAttackTexture)
        std::cerr << "[BLAD] assets/player_attack.png not found\n";

    maxHp = 5; hp = maxHp;
    isInvincible = false; invincibilityTimer = 0.f;
    facingLeft   = false;

    hpBarBack.setSize(sf::Vector2f(10.f, 1.5f));
    hpBarBack.setFillColor(sf::Color(50, 50, 50));
    hpBarBack.setOrigin(5.f, 0.75f);

    hpBarFront.setSize(sf::Vector2f(10.f, 1.5f));
    hpBarFront.setFillColor(sf::Color(50, 220, 80));
    hpBarFront.setOrigin(5.f, 0.75f);

    frameWidth  = 32;
    frameHeight = 32;
    animationTimer = 0.f;
    frameDuration  = 0.15f;
    currentColumn  = 0;
    maxColumns     = 3;
    sheetCols      = 2;
    currentAnim    = AnimState::IDLE;

    if (hasIdleTexture) {
        sprite.setTexture(textureIdle);
        currentFrame = sf::IntRect(0, 0, frameWidth, frameHeight);
        sprite.setTextureRect(currentFrame);
        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
        applyFacingScale();
    } else {
        fallbackShape.setSize(sf::Vector2f(10.f, 10.f));
        fallbackShape.setFillColor(sf::Color::Blue);
        fallbackShape.setOrigin(5.f, 5.f);
    }

    speed = 150.f;

    isDashing = false;
    dashDuration = 0.2f; dashCooldown = 1.0f; dashCooldownTimer = 0.f;

    isAttacking = false;
    attackDuration = 0.20f; 
    attackCooldown = 0.4f; attackCooldownTimer = 0.f;
    attackAngle = 0.f; attackTimer = 0.f;

    swordHitbox.setSize(sf::Vector2f(36.f, 32.f));
    swordHitbox.setFillColor(sf::Color(255, 255, 255, 0)); 
    swordHitbox.setOrigin(0.f, 16.f);
}

void Player::applyFacingScale() {
    constexpr float BASE = 0.9f;
    sprite.setScale(facingLeft ? BASE : -BASE, BASE);
}

sf::FloatRect Player::getBounds() const {
    if (hasIdleTexture || hasWalkTexture || hasAttackTexture)
        return sprite.getGlobalBounds();
    return fallbackShape.getGlobalBounds();
}

void Player::setPosition(const sf::Vector2f& newPos) {
    position = newPos;
    if (hasIdleTexture || hasWalkTexture || hasAttackTexture)
        sprite.setPosition(position);
    else
        fallbackShape.setPosition(position);
}

void Player::takeDamage(int amount) {
    if (!isInvincible && !isDashing) {
        hp -= amount;
        isInvincible = true; invincibilityTimer = 1.0f;
        if (hp < 0) hp = 0;
    }
}

void Player::updateAttack(float dt, sf::RenderWindow& window) {
    if (attackCooldownTimer > 0.f) attackCooldownTimer -= dt;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)
        && !isAttacking
        && attackCooldownTimer <= 0.f)
    {
        isAttacking         = true;
        attackTimer         = attackDuration;
        currentColumn       = 0; 
        attackCooldownTimer = attackCooldown;

        sf::Vector2f delta = window.mapPixelToCoords(
            sf::Mouse::getPosition(window)) - position;

        attackAngle = std::atan2(delta.y, delta.x) * 180.f / 3.14159f;
        swordHitbox.setRotation(attackAngle);
        facingLeft = (delta.x < 0.f);
    }

    if (isAttacking) {
        attackTimer -= dt;
        if (attackTimer <= 0.f) isAttacking = false;

        float rad = attackAngle * 3.14159265f / 180.f;
        swordHitbox.setPosition(
            position.x + std::cos(rad) * 12.f,
            position.y + std::sin(rad) * 12.f);
    }
}

void Player::update(float dt, sf::RenderWindow& window) {

    updateAttack(dt, window);
    if (isInvincible) {
        invincibilityTimer -= dt;
        if (invincibilityTimer <= 0.f) {
            isInvincible = false;
            sprite.setColor(sf::Color::White);
        } else {
            sprite.setColor(sf::Color(255, 255, 255, 125));
        }
    }

    float hpPercent = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarFront.setSize(sf::Vector2f(10.f * hpPercent, 1.5f));

    sf::Vector2f moveDir(0.f, 0.f);
    if (dashCooldownTimer > 0.f) dashCooldownTimer -= dt;
    if (isDashing) {
        position += dashDir * (speed * 3.f) * dt;
        if (dashDir.x < 0.f)      { facingLeft = true; }
        else if (dashDir.x > 0.f) { facingLeft = false; }
        dashTimer -= dt;
        if (dashTimer <= 0.f) isDashing = false;
        applyFacingScale(); 

    } else {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { moveDir.x -= 1.f; facingLeft = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { moveDir.x += 1.f; facingLeft = false; }

        if (moveDir.x != 0 || moveDir.y != 0) {
            float len = std::sqrt(moveDir.x*moveDir.x + moveDir.y*moveDir.y);
            moveDir /= len;
            position += moveDir * speed * dt; 
            applyFacingScale(); 
        } else {
            applyFacingScale(); 
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            startDash(moveDir);
    }
    if (isAttacking) {
        if (hasAttackTexture)
            setAnim(AnimState::ATTACK, 5, 0.04f, textureAttack, 2, 3);
        else if (hasIdleTexture)
            setAnim(AnimState::IDLE,   3, 0.15f, textureIdle,   2, 2);

    } else if (isDashing) {
        if (hasWalkTexture)
            setAnim(AnimState::DASH, 6, 0.05f, textureWalk, 2, 3);
        else if (hasIdleTexture)
            setAnim(AnimState::IDLE, 3, 0.15f, textureIdle, 2, 2);

    } else if (moveDir.x != 0 || moveDir.y != 0) {
        if (hasWalkTexture)
            setAnim(AnimState::WALK, 6, 0.08f, textureWalk, 2, 3);
        else if (hasIdleTexture)
            setAnim(AnimState::IDLE, 3, 0.15f, textureIdle, 2, 2);

    } else {
        if (hasIdleTexture)
            setAnim(AnimState::IDLE, 3, 0.15f, textureIdle, 2, 2);
    }

    applyFacingScale();


    if (position.x < 12.f)  position.x = 12.f;
    if (position.x > 388.f) position.x = 388.f;
    if (position.y < 12.f)  position.y = 12.f;
    if (position.y > 213.f) position.y = 213.f;

    if (hasIdleTexture || hasWalkTexture || hasAttackTexture) {
        animationTimer += dt;
        if (animationTimer >= frameDuration) {
            animationTimer -= frameDuration;  
            currentColumn = (currentColumn + 1) % maxColumns;
        }

        currentFrame.left = (currentColumn % sheetCols) * frameWidth;
        currentFrame.top  = (currentColumn / sheetCols) * frameHeight;
        sprite.setTextureRect(currentFrame);
        sprite.setPosition(position);
    } else {
        fallbackShape.setPosition(position);
    }

    hpBarBack.setPosition (position.x,                          position.y - 10.f);
    hpBarFront.setPosition(position.x - (5.f * (1.f - hpPercent)), position.y - 10.f);
}

void Player::startDash(sf::Vector2f moveDir) {
    if (!isDashing && dashCooldownTimer <= 0.f) {
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
        dashDir = (moveDir.x == 0 && moveDir.y == 0)
                  ? (facingLeft ? sf::Vector2f(-1.f, 0.f) : sf::Vector2f(1.f, 0.f)) 
                  : moveDir;
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (hasIdleTexture || hasWalkTexture || hasAttackTexture)
        window.draw(sprite);
    else
        window.draw(fallbackShape);

    window.draw(hpBarBack);
    window.draw(hpBarFront);
}