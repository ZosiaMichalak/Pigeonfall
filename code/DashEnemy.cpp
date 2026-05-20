#include "DashEnemy.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static constexpr float PI = 3.14159265f;

DashEnemy::DashEnemy(float x, float y, int tier) : Enemy(x, y) {
    maxHp     = 3 + tier;
    hp        = maxHp;
    moveSpeed = 30.f + tier * 5.f;

    dashSpeed        = 420.f + tier * 30.f;
    dashDuration     = 0.18f;
    dashTimer        = 0.f;
    dashDirection    = sf::Vector2f(0.f, 0.f);

    windUpDuration   = std::max(0.18f, 0.35f - tier * 0.04f);
    windUpTimer      = 0.f;

    stalkDuration    = 1.4f;
    stalkTimer       = stalkDuration * (static_cast<float>(rand()) / RAND_MAX);

    orbitRadius      = 80.f;
    orbitAngle       = static_cast<float>(rand()) / RAND_MAX * 2.f * PI;
    orbitSign        = (rand() % 2 == 0) ? 1 : -1;

    recoverDuration      = 0.4f;
    recoverTimer         = 0.f;

    dashCooldownDuration = std::max(0.9f, 1.6f - tier * 0.1f);
    dashCooldownTimer    = dashCooldownDuration * (static_cast<float>(rand()) / RAND_MAX);

    dashState = DashEnemyState::STALK;

    int r = std::max(80,  180 - tier * 20);
    int b = std::min(255, 220 + tier * 10);
    baseColor = sf::Color(r, 40, b);

    shape.setFillColor(baseColor);
    hpBarFront.setFillColor(sf::Color(r, 50, b));
}

void DashEnemy::updateAI(float dt, sf::Vector2f playerPos,
                          std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    (void)spawnQueue;
    if (!isActive()) return;

    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            shape.setFillColor(
                dashState == DashEnemyState::WIND_UP
                    ? sf::Color(255, 255, 60) : baseColor);
        } else {
            shape.setFillColor(sf::Color::White);
        }
    }

    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    sf::Vector2f toPlayerNorm = (dist > 0.f) ? toPlayer / dist : sf::Vector2f(1.f, 0.f);

    if (dashCooldownTimer > 0.f) dashCooldownTimer -= dt;

    switch (dashState) {

    case DashEnemyState::STALK: {
        orbitAngle += orbitSign * 1.8f * dt;
        sf::Vector2f orbitTarget(
            playerPos.x + std::cos(orbitAngle) * orbitRadius,
            playerPos.y + std::sin(orbitAngle) * orbitRadius);
        sf::Vector2f toTarget = orbitTarget - position;
        float tLen = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
        if (tLen > 0.f) position += (toTarget / tLen) * moveSpeed * dt;

        stalkTimer -= dt;
        if (stalkTimer <= 0.f && dist < 130.f && dashCooldownTimer <= 0.f) {
            dashState     = DashEnemyState::WIND_UP;
            windUpTimer   = windUpDuration;
            dashDirection = toPlayerNorm;
            if (!isHit) shape.setFillColor(sf::Color(255, 255, 60));
        } else if (stalkTimer <= 0.f) {
            stalkTimer = stalkDuration;
        }
        break;
    }

    case DashEnemyState::WIND_UP: {
        windUpTimer -= dt;
        position += dashDirection * 15.f * dt;
        if (windUpTimer <= 0.f) {
            dashState = DashEnemyState::DASH;
            dashTimer = dashDuration;
            if (!isHit) shape.setFillColor(baseColor);
        }
        break;
    }

    case DashEnemyState::DASH: {
        position += dashDirection * dashSpeed * dt;
        dashTimer -= dt;
        if (dashTimer <= 0.f) {
            dashState         = DashEnemyState::RECOVER;
            recoverTimer      = recoverDuration;
            dashCooldownTimer = dashCooldownDuration;
            stalkTimer        = stalkDuration;
            sf::Vector2f fromPlayer = position - playerPos;
            orbitAngle = std::atan2(fromPlayer.y, fromPlayer.x);
        }
        break;
    }

    case DashEnemyState::RECOVER: {
        recoverTimer -= dt;
        if (recoverTimer <= 0.f) dashState = DashEnemyState::STALK;
        break;
    }
    }

    if (position.x < 10.f)  position.x = 10.f;
    if (position.x > 390.f) position.x = 390.f;
    if (position.y < 10.f)  position.y = 10.f;

    if (position.y > 185.f) position.y = 185.f;

    shape.setPosition(position);
    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarBack.setPosition (position.x,                         position.y - 8.f);
    hpBarFront.setPosition(position.x - (4.5f * (1.f - pct)), position.y - 8.f);
    hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
}