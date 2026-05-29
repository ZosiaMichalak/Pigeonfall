#include "BulletEnemy.h"
#include "Bullet.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Sprite sheet layout (all frames 51x32):
//   bulletEnemy_idle : 1 col x 3 rows  => 3 frames
//   bulletEnemy_walk : 2 cols x 3 rows => 6 frames

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
    baseColor = sf::Color(r, g, 20);

    // Load textures
    bool idleOk = texIdle.loadFromFile("assets/bulletEnemy_idle.png");
    bool walkOk = texWalk.loadFromFile("assets/bulletEnemy_walk.png");

    if (idleOk || walkOk) {
        hasSprite = true;
        sprite.setOrigin(frameW / 2.f, frameH / 2.f);
        setSheet(false);          // start idle
    }

    // Fallback colour (used when no texture)
    shape.setFillColor(baseColor);
    hpBarFront.setFillColor(baseColor);

    // NAPRAWA: Zaktualizuj grafikę (wycięcie z klatki i pozycję) przed 1. klatką!
    if (hasSprite) tickAnim(0.f); 
}

void BulletEnemy::setSheet(bool walking) {
    if (walking) {
        if (texWalk.getSize().x == 0) return;
        sprite.setTexture(texWalk);
        animMaxCols  = 6;
        animSheetCols = 2;
        frameDur     = 0.10f;
    } else {
        if (texIdle.getSize().x == 0) return;
        sprite.setTexture(texIdle);
        animMaxCols  = 3;
        animSheetCols = 1;
        frameDur     = 0.18f;
    }
    animCol   = 0;
    animTimer = 0.f;
}

void BulletEnemy::updateAI(float dt, sf::Vector2f playerPos,
                            std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    if (!isActive()) return;

    // ── Hit flash ──────────────────────────────────────────────────────────────
    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            if (hasSprite)
                sprite.setColor(sf::Color::White);
            else
                shape.setFillColor(baseColor);
        } else {
            if (hasSprite)
                sprite.setColor(sf::Color(255, 100, 100));
            else
                shape.setFillColor(sf::Color::White);
        }
    }

    // ── State machine ──────────────────────────────────────────────────────────
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

    // ── Movement ───────────────────────────────────────────────────────────────
    sf::Vector2f desiredMove(0.f, 0.f);
    bool isMoving = false;

    if (state == BulletEnemyState::RETREAT) {
        if (dist > 0.f) { desiredMove = (-toPlayer / dist) * moveSpeed; isMoving = true; }
    } else if (state == BulletEnemyState::CHASE) {
        if (dist > 0.f) { desiredMove = (toPlayer / dist) * moveSpeed; isMoving = true; }
    }

    // Soft boundary push
    float margin = 35.f, push = 50.f;
    if (position.x < margin)         desiredMove.x += (margin - position.x)           / margin * push;
    if (position.x > 400.f - margin) desiredMove.x -= (position.x - (400.f - margin)) / margin * push;
    if (position.y < margin)         desiredMove.y += (margin - position.y)           / margin * push;
    if (position.y > 195.f - margin) desiredMove.y -= (position.y - (195.f - margin)) / margin * push;

    position += desiredMove * dt;

    // Hard clamp
    if (position.x < 10.f)  position.x = 10.f;
    if (position.x > 390.f) position.x = 390.f;
    if (position.y < 10.f)  position.y = 10.f;
    if (position.y > 185.f) position.y = 185.f;

    // ── Facing direction with deadzone to prevent rapid flipping ───────────────
    if (dist > 0.f) {
        if (toPlayer.x < -1.f) facingLeft = true;
        else if (toPlayer.x > 1.f) facingLeft = false;
    }

    // ── Animation sheet switch ─────────────────────────────────────────────────
    if (hasSprite) {
        bool currentlyWalking = (animMaxCols == 6);
        if (isMoving != currentlyWalking) {
            setSheet(isMoving);
        }
    }

    // ── Shooting ───────────────────────────────────────────────────────────────
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

    // ── Finalize visuals ───────────────────────────────────────────────────────
    if (hasSprite) {
        tickAnim(dt);
    } else {
        shape.setPosition(position);
    }

    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    
    // ── Fixed Health bar positioning ───────────────────────────────────────────
    hpBarBack.setPosition (position.x, position.y - 10.f);
    hpBarFront.setPosition(position.x, position.y - 10.f);
    hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
}