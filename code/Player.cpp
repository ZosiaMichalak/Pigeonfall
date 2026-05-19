#include "Player.h"
#include <cmath>

Player::Player(float x, float y) : GameObject(x, y) {
    hasIdleTexture = textureIdle.loadFromFile("assets/player_idle.png");
    hasWalkTexture = textureWalk.loadFromFile("assets/player_walk.png");

    if (!hasIdleTexture)
        std::cerr << "[BLAD SFML] Nie mozna znalezc: assets/player_idle.png !" << std::endl;
    if (!hasWalkTexture)
        std::cerr << "[BLAD SFML] Nie mozna znalezc: assets/player_walk.png !" << std::endl;

    maxHp = 5;
    hp = maxHp;
    isInvincible = false;
    invincibilityTimer = 0.f;
    facingLeft = false; 

    hpBarBack.setSize(sf::Vector2f(10.f, 1.5f));
    hpBarBack.setFillColor(sf::Color(50, 50, 50));
    hpBarBack.setOrigin(5.f, 0.75f);

    hpBarFront.setSize(sf::Vector2f(10.f, 1.5f));
    hpBarFront.setFillColor(sf::Color(50, 220, 80));
    hpBarFront.setOrigin(5.f, 0.75f);

    frameWidth = 32;
    frameHeight = 32;
    animationTimer = 0.f;
    frameDuration = 0.15f; 
    currentColumn = 0;
    maxColumns = 3;

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
    dashDuration = 0.2f;
    dashCooldown = 1.0f;
    dashCooldownTimer = 0.f;

    isAttacking = false;
    attackDuration = 0.15f;
    attackCooldown = 0.4f;
    attackCooldownTimer = 0.f;

    swordHitbox.setSize(sf::Vector2f(36.f, 32.f));
    swordHitbox.setFillColor(sf::Color(255, 255, 255, 130)); 
    swordHitbox.setOrigin(0.f, 16.f); 
}

void Player::applyFacingScale() {
    constexpr float BASE = 0.9f;
    sprite.setScale(facingLeft ? BASE : -BASE, BASE);
}

sf::FloatRect Player::getBounds() const {
    if (hasIdleTexture || hasWalkTexture)
        return sprite.getGlobalBounds();
    return fallbackShape.getGlobalBounds();
}

void Player::setPosition(const sf::Vector2f& newPos) {
    position = newPos;
    if (hasIdleTexture || hasWalkTexture)
        sprite.setPosition(position);
    else
        fallbackShape.setPosition(position);
}

void Player::takeDamage(int amount) {
    if (!isInvincible && !isDashing) {
        hp -= amount;
        isInvincible = true;
        invincibilityTimer = 1.0f;
        if (hp < 0) hp = 0;
    }
}

void Player::updateAttack(float dt, sf::RenderWindow& window) {
    if (attackCooldownTimer > 0.f) attackCooldownTimer -= dt;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !isAttacking && attackCooldownTimer <= 0.f) {
        isAttacking = true;
        attackTimer = attackDuration;
        attackCooldownTimer = attackCooldown;

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f playerCenter = position;
        sf::Vector2f delta = mousePos - playerCenter;

        attackAngle = std::atan2(delta.y, delta.x) * 180.f / 3.14159f;
        swordHitbox.setRotation(attackAngle);
    }

    if (isAttacking) {
        attackTimer -= dt;
        if (attackTimer <= 0.f) isAttacking = false;

        float rad = attackAngle * 3.14159265f / 180.f;
        float offsetX = std::cos(rad) * 12.f;
        float offsetY = std::sin(rad) * 12.f;

        swordHitbox.setPosition(position.x + offsetX, position.y + offsetY);
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

    sf::Texture const* prevTexture = sprite.getTexture();

    if (isDashing) {
        if (hasWalkTexture) {
            sprite.setTexture(textureWalk);
            maxColumns = 6;
            frameDuration = 0.05f; 
        }
        position += dashDir * (speed * 3.f) * dt;

        if (dashDir.x < 0.f) { facingLeft = true;  applyFacingScale(); }
        else if (dashDir.x > 0.f) { facingLeft = false; applyFacingScale(); }

        dashTimer -= dt;
        if (dashTimer <= 0.f) isDashing = false;

    } else {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { moveDir.x -= 1.f; facingLeft = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { moveDir.x += 1.f; facingLeft = false; }

        if (moveDir.x != 0 || moveDir.y != 0) {
            if (hasWalkTexture) { 
                sprite.setTexture(textureWalk); 
                maxColumns = 6; 
                frameDuration = 0.08f; 
            }

            float length = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
            moveDir /= length;
            position += moveDir * speed * dt;

            applyFacingScale();
        } else {
            if (hasIdleTexture) { 
                sprite.setTexture(textureIdle); 
                maxColumns = 3; 
                frameDuration = 0.15f; 
            }
            applyFacingScale(); 
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            startDash(moveDir);
    }

    if (position.x < 12.f)  position.x = 12.f;
    if (position.x > 388.f) position.x = 388.f;
    if (position.y < 12.f)  position.y = 12.f;
    if (position.y > 213.f) position.y = 213.f;

    if ((hasIdleTexture || hasWalkTexture) && prevTexture != sprite.getTexture()) {
        currentColumn  = 0;
        animationTimer = 0.f;
    }

    if (hasIdleTexture || hasWalkTexture) {
        animationTimer += dt;
        if (animationTimer >= frameDuration) {
            animationTimer = 0.f;
            currentColumn  = (currentColumn + 1) % maxColumns;
        }

        currentFrame.left = (currentColumn % 2) * frameWidth;
        currentFrame.top = (currentColumn / 2) * frameHeight;
        sprite.setTextureRect(currentFrame);
        sprite.setPosition(position);
    } else {
        fallbackShape.setPosition(position);
    }

    hpBarBack.setPosition(position.x, position.y - 10.f);
    hpBarFront.setPosition(position.x - (5.f * (1.f - hpPercent)), position.y - 10.f);
}

void Player::startDash(sf::Vector2f moveDir) {
    if (!isDashing && dashCooldownTimer <= 0.f) {
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
        dashDir = (moveDir.x == 0 && moveDir.y == 0) ? sf::Vector2f(1.f, 0.f) : moveDir;
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (hasIdleTexture || hasWalkTexture)
        window.draw(sprite);
    else
        window.draw(fallbackShape);

    if (isAttacking) window.draw(swordHitbox);

    window.draw(hpBarBack);
    window.draw(hpBarFront);
}