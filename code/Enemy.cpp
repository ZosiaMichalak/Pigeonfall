#include "Enemy.h"

Enemy::Enemy(float x, float y) : GameObject(x, y) {
    maxHp    = 2;
    hp       = maxHp;
    isHit    = false;
    hitTimer = 0.f;
    moveSpeed = 40.f;

    shape.setSize(sf::Vector2f(9.f, 9.f));
    shape.setFillColor(sf::Color::Red);
    shape.setOrigin(4.5f, 4.5f);
    shape.setPosition(position);
    
    // NAPRAWA: Zabezpieczenie pozycji początkowej dla sprite'a
    sprite.setPosition(position);

    hpBarBack.setSize(sf::Vector2f(9.f, 1.5f));
    hpBarBack.setFillColor(sf::Color(50, 50, 50));
    hpBarBack.setOrigin(4.5f, 0.75f);
    // NAPRAWA: Ustawienie pozycji paska HP w momencie spawnu
    hpBarBack.setPosition(position.x, position.y - 10.f);

    hpBarFront.setSize(sf::Vector2f(9.f, 1.5f));
    hpBarFront.setFillColor(sf::Color::Red);
    hpBarFront.setOrigin(4.5f, 0.75f);
    // NAPRAWA: Ustawienie pozycji paska HP w momencie spawnu
    hpBarFront.setPosition(position.x, position.y - 10.f);
}

void Enemy::tickAnim(float dt) {
    animTimer += dt;
    if (animTimer >= frameDur) {
        animTimer -= frameDur;
        animCol = (animCol + 1) % animMaxCols;
    }
    int col = animCol % animSheetCols;
    int row = animCol / animSheetCols;
    sprite.setTextureRect(sf::IntRect(col * frameW, row * frameH, frameW, frameH));
    sprite.setPosition(position);
    // Mirror horizontally when facing left
    sprite.setScale(facingLeft ? 1.f : -1.f, 1.f);
}

void Enemy::takeDamage(int damage) {
    if (isSpawning) return;  // immune during spawn animation
    if (!isHit) {
        hp -= damage;
        isHit    = true;
        hitTimer = 0.2f;
        if (hp <= 0) {
            destroy();
        } else {
            float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
            hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
        }
    }
}

void Enemy::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (hasSprite) {
        window.draw(sprite);
    } else {
        window.draw(shape);
    }
    window.draw(hpBarBack);
    window.draw(hpBarFront);
}