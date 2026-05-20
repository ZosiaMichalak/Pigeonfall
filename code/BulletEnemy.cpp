#include "BulletEnemy.h"
#include "Bullet.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

BulletEnemy::BulletEnemy(float x, float y, int tier) : Enemy(x, y) {
    maxHp = 2 + tier;
    hp    = maxHp;

    moveSpeed   = 40.f + tier * 4.f;
    strafeSpeed = 0.f;
    shootRange  = 140.f + tier * 10.f;

    shootCooldown = std::max(1.2f, 2.2f - tier * 0.2f);
    shootTimer    = shootCooldown * (static_cast<float>(rand()) / RAND_MAX);

    strafeSign = (rand() % 2 == 0) ? 1 : -1;
    state      = BulletEnemyState::CHASE;

    int r = std::min(255, 200 + tier * 15);
    int g = std::max(0,    20 - tier * 5);
    shape.setFillColor(sf::Color(r, g, 20));
    hpBarFront.setFillColor(sf::Color(r, g, 20));
}

void BulletEnemy::updateAI(float dt, sf::Vector2f playerPos,
                            std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    if (!isActive()) return;

    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            int tier = maxHp - 2;
            int r = std::min(255, 200 + tier * 15);
            int g = std::max(0,    20 - tier * 5);
            shape.setFillColor(sf::Color(r, g, 20));
        } else {
            shape.setFillColor(sf::Color::White);
        }
    }

    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist < 70.f) {
        state = BulletEnemyState::RETREAT;
    } else if (state == BulletEnemyState::RETREAT && dist >= 100.f) {
        state = BulletEnemyState::STRAFE;
    } else if (state == BulletEnemyState::CHASE && dist <= shootRange) {
        state = BulletEnemyState::STRAFE;
    } else if (state == BulletEnemyState::STRAFE && dist > shootRange * 1.2f) {
        state = BulletEnemyState::CHASE;
    }

    sf::Vector2f desiredMove(0.f, 0.f);
    if (state == BulletEnemyState::RETREAT) {
        if (dist > 0.f) desiredMove = (-toPlayer / dist) * moveSpeed;
    } else if (state == BulletEnemyState::CHASE) {
        if (dist > 0.f) desiredMove = (toPlayer / dist) * moveSpeed;
    }

    float margin = 35.f, push = 50.f;
    if (position.x < margin)         desiredMove.x += (margin - position.x)          / margin * push;
    if (position.x > 400.f - margin) desiredMove.x -= (position.x - (400.f - margin))/ margin * push;
    if (position.y < margin)         desiredMove.y += (margin - position.y)          / margin * push;

    if (position.y > 195.f - margin) desiredMove.y -= (position.y - (195.f - margin))/ margin * push;

    position += desiredMove * dt;

    if (position.x < 10.f)  position.x = 10.f;
    if (position.x > 390.f) position.x = 390.f;
    if (position.y < 10.f)  position.y = 10.f;

    if (position.y > 185.f) position.y = 185.f;

    if ((state == BulletEnemyState::STRAFE || state == BulletEnemyState::RETREAT) && dist >= 60.f) {
        shootTimer -= dt;
        if (shootTimer <= 0.f) {
            shootTimer = shootCooldown;
            sf::Vector2f baseDir = playerPos - position;
            float spread = std::max(4.f, 15.f - (maxHp - 2) * 3.f);
            float ox = (static_cast<float>(rand()) / RAND_MAX) * spread * 2.f - spread;
            float oy = (static_cast<float>(rand()) / RAND_MAX) * spread * 2.f - spread;
            spawnQueue.push_back(
                std::make_unique<Bullet>(position.x, position.y,
                                         baseDir + sf::Vector2f(ox, oy), 100.f, true));
        }
    }

    shape.setPosition(position);
    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarBack.setPosition (position.x,                         position.y - 8.f);
    hpBarFront.setPosition(position.x - (4.5f * (1.f - pct)), position.y - 8.f);
    hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
}