#include "Enemy.h"
#include "Bullet.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(float x, float y) : GameObject(x, y) {
    maxHp = 2; 
    hp = maxHp;
    isHit = false;
    hitTimer = 0.f;

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
    state = EnemyState::CHASE;
    moveSpeed = 40.f;       
    strafeSpeed = 0.f;  
    shootRange = 140.f;      
    
    shootCooldown = 2.2f; 
    shootTimer = shootCooldown * (static_cast<float>(rand()) / RAND_MAX);
    
    strafeSign = (rand() % 2 == 0) ? 1 : -1;
}

void Enemy::takeDamage(int damage) {
    if (!isHit) {
        hp -= damage;
        isHit = true;
        hitTimer = 0.2f;

        if (hp <= 0) {
            destroy();
        } else {
            float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
            hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
        }
    }
}

void Enemy::update(float, sf::RenderWindow& ) {}

void Enemy::updateAI(float dt, sf::Vector2f playerPos,
                     std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    if (!isActive()) return;

    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            shape.setFillColor(sf::Color::Red);
        } else {
            shape.setFillColor(sf::Color::White);
        }
    }

    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist < 70.f) {
        state = EnemyState::RETREAT;
    } 
    else if (state == EnemyState::RETREAT && dist >= 100.f) {
        state = EnemyState::STRAFE;
    } 
    else if (state == EnemyState::CHASE && dist <= shootRange) {
        state = EnemyState::STRAFE;
    } else if (state == EnemyState::STRAFE && dist > shootRange * 1.2f) {
        state = EnemyState::CHASE; 
    }

    sf::Vector2f desiredMove(0.f, 0.f);

    if (state == EnemyState::RETREAT) {
        if (dist > 0.f) {
            desiredMove = (-toPlayer / dist) * moveSpeed; 
        }
    } 
    else if (state == EnemyState::CHASE) {
        if (dist > 0.f)
            desiredMove = (toPlayer / dist) * moveSpeed;
    } 

    sf::Vector2f avoidWallForce(0.f, 0.f);
    float margin = 35.f;        
    float pushStrength = 50.f;  

    if (position.x < margin) avoidWallForce.x += (margin - position.x) / margin * pushStrength;
    if (position.x > 400.f - margin) avoidWallForce.x -= (position.x - (400.f - margin)) / margin * pushStrength;
    if (position.y < margin) avoidWallForce.y += (margin - position.y) / margin * pushStrength;
    if (position.y > 225.f - margin) avoidWallForce.y -= (position.y - (225.f - margin)) / margin * pushStrength;

    desiredMove += avoidWallForce;
    position += desiredMove * dt;

    if (position.x < 10.f)  position.x = 10.f;
    if (position.x > 390.f) position.x = 390.f;
    if (position.y < 10.f)  position.y = 10.f;
    if (position.y > 215.f) position.y = 215.f;

    if ((state == EnemyState::STRAFE || state == EnemyState::RETREAT) && dist >= 60.f) {
        shootTimer -= dt;
        if (shootTimer <= 0.f) {
            shootTimer = shootCooldown;
            sf::Vector2f baseDirection = playerPos - position;
            float offsetX = (static_cast<float>(rand()) / RAND_MAX) * 30.f - 15.f;
            float offsetY = (static_cast<float>(rand()) / RAND_MAX) * 30.f - 15.f;
            
            sf::Vector2f inaccurateDirection = baseDirection + sf::Vector2f(offsetX, offsetY);

            spawnQueue.push_back(
                std::make_unique<Bullet>(position.x, position.y, inaccurateDirection, 100.f, true));
        }
    }

    shape.setPosition(position);
    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarBack.setPosition(position.x, position.y - 8.f);
    hpBarFront.setPosition(position.x - (4.5f * (1.f - pct)), position.y - 8.f);
}

void Enemy::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    window.draw(shape);
    window.draw(hpBarBack);
    window.draw(hpBarFront);
}