#include "DashEnemy.h"
#include "DifficultySettings.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static constexpr float PI = 3.14159265f;

// Sprite sheet layouts (all frames 51x32):
//   dashEnemy_idle    : 1 col x 3 rows  =>  3 frames
//   dashEnemy_walk    : 2 cols x 3 rows =>  6 frames
//   dashEnemy_loading : 2 cols x 4 rows =>  8 frames
//   dashEnemy_dash    : 2 cols x 4 rows =>  8 frames

DashEnemy::DashEnemy(float x, float y, int tier) : Enemy(x, y) {
    const DifficultySettings& diff = ActiveDifficulty::settings;

    // ── Stats (difficulty-scaled) ─────────────────────────────────────────────
    maxHp = static_cast<int>(std::round((3 + tier) * diff.enemyHpMult));
    if (maxHp < 1) maxHp = 1;
    hp        = maxHp;
    moveSpeed = (30.f + tier * 5.f) * diff.enemySpeedMult;

    dashSpeed        = (420.f + tier * 30.f) * diff.dashSpeedMult;
    dashDuration     = 0.18f;
    dashTimer        = 0.f;
    dashDirection    = sf::Vector2f(0.f, 0.f);

    // Shorter wind-up = harder (less reaction time)
    windUpDuration   = std::max(0.10f, (0.35f - tier * 0.04f) * diff.dashWindupMult);
    windUpTimer      = 0.f;

    stalkDuration    = 1.4f;
    stalkTimer       = stalkDuration * (static_cast<float>(rand()) / RAND_MAX);

    orbitRadius      = 80.f;
    orbitAngle       = static_cast<float>(rand()) / RAND_MAX * 2.f * PI;
    orbitSign        = (rand() % 2 == 0) ? 1 : -1;

    recoverDuration      = 0.4f;
    recoverTimer         = 0.f;

    // Shorter cooldown = dashes more often (harder)
    dashCooldownDuration = std::max(0.5f, (1.6f - tier * 0.1f) * diff.dashCdMult);
    dashCooldownTimer    = dashCooldownDuration * (static_cast<float>(rand()) / RAND_MAX);

    dashState = DashEnemyState::STALK;

    int r = std::max(80,  180 - tier * 20);
    int b = std::min(255, 220 + tier * 10);
    baseColor = sf::Color(r, 40, b);

    // Load all four sheets
    texIdle.loadFromFile("assets/dashEnemy_idle.png");
    texWalk.loadFromFile("assets/dashEnemy_walk.png");
    texLoading.loadFromFile("assets/dashEnemy_loading.png");
    texDash.loadFromFile("assets/dashEnemy_dash.png");

    if (texIdle.getSize().x > 0) {
        hasSprite = true;
        sprite.setOrigin(frameW / 2.f, frameH / 2.f);
    }

    shape.setFillColor(baseColor);
    hpBarFront.setFillColor(sf::Color(r, 50, b));

    setSheet(dashState);
    if (hasSprite) tickAnim(0.f);
}

void DashEnemy::setSheet(DashEnemyState s) {
    if (s == prevSheetState) return;
    prevSheetState = s;
    animCol   = 0;
    animTimer = 0.f;

    switch (s) {
    case DashEnemyState::STALK:
        sprite.setTexture(texWalk);
        animMaxCols   = 6;
        animSheetCols = 2;
        frameDur      = 0.10f;
        break;
    case DashEnemyState::RECOVER:
        sprite.setTexture(texIdle);
        animMaxCols   = 3;
        animSheetCols = 1;
        frameDur      = 0.18f;
        break;
    case DashEnemyState::WIND_UP:
        sprite.setTexture(texLoading);
        animMaxCols   = 8;
        animSheetCols = 2;
        frameDur      = windUpDuration / 8.f;
        break;
    case DashEnemyState::DASH:
        sprite.setTexture(texDash);
        animMaxCols   = 8;
        animSheetCols = 2;
        frameDur      = dashDuration / 8.f;
        break;
    }
}

void DashEnemy::updateAI(float dt, sf::Vector2f playerPos,
                          std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    (void)spawnQueue;
    if (!isActive()) return;

    // ── Hit flash ──────────────────────────────────────────────────────────────
    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            if (hasSprite) sprite.setColor(sf::Color::White);
            else           shape.setFillColor(baseColor);
        } else {
            if (hasSprite) sprite.setColor(sf::Color(255, 100, 100));
            else           shape.setFillColor(sf::Color::White);
        }
    }

    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    sf::Vector2f toPlayerNorm = (dist > 0.f) ? toPlayer / dist : sf::Vector2f(1.f, 0.f);

    if (dashCooldownTimer > 0.f) dashCooldownTimer -= dt;

    if (dist > 0.f) facingLeft = (toPlayer.x < 0.f);

    // ── State machine ──────────────────────────────────────────────────────────
    switch (dashState) {

    case DashEnemyState::STALK: {
        setSheet(DashEnemyState::STALK);

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
        } else if (stalkTimer <= 0.f) {
            stalkTimer = stalkDuration;
        }
        break;
    }

    case DashEnemyState::WIND_UP: {
        setSheet(DashEnemyState::WIND_UP);

        dashJustStarted = false;
        windUpTimer -= dt;
        position += dashDirection * 15.f * dt;
        if (windUpTimer <= 0.f) {
            dashState       = DashEnemyState::DASH;
            dashTimer       = dashDuration;
            dashJustStarted = true;
        }
        break;
    }

    case DashEnemyState::DASH: {
        setSheet(DashEnemyState::DASH);

        if (dashTimer < dashDuration) dashJustStarted = false;
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
        setSheet(DashEnemyState::RECOVER);

        recoverTimer -= dt;
        if (recoverTimer <= 0.f) dashState = DashEnemyState::STALK;
        break;
    }
    }

    // Hard clamp
    if (position.x < 10.f)  position.x = 10.f;
    if (position.x > 390.f) position.x = 390.f;
    if (position.y < 10.f)  position.y = 10.f;
    if (position.y > 185.f) position.y = 185.f;

    if (hasSprite) {
        tickAnim(dt);
    } else {
        shape.setPosition(position);
    }

    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarBack.setPosition (position.x, position.y - 10.f);
    hpBarFront.setPosition(position.x, position.y - 10.f);
    hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
}
