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

    hpBarBack.setSize(sf::Vector2f(9.f, 1.5f));
    hpBarBack.setFillColor(sf::Color(50, 50, 50));
    hpBarBack.setOrigin(4.5f, 0.75f);

    hpBarFront.setSize(sf::Vector2f(9.f, 1.5f));
    hpBarFront.setFillColor(sf::Color::Red);
    hpBarFront.setOrigin(4.5f, 0.75f);
}

void Enemy::takeDamage(int damage) {
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
    window.draw(shape);
    window.draw(hpBarBack);
    window.draw(hpBarFront);
}
