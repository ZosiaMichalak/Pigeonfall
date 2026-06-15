#include "BulletEnemy.h"
#include "Bullet.h"
#include "DifficultySettings.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Sprite sheet layout (all frames 51x32):
//   bulletEnemy_idle : 1 col x 3 rows  => 3 frames
//   bulletEnemy_walk : 2 cols x 3 rows => 6 frames

#include "BulletEnemy.h"
#include "Bullet.h"
#include "DifficultySettings.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Sprite sheet layout (all frames 51x32):
//   bulletEnemy_idle : 1 col x 3 rows  => 3 frames
//   bulletEnemy_walk : 2 cols x 3 rows => 6 frames

// Constructor: Initializes difficulty-scaled statistics, textures, tiers, colors, and initial states.
BulletEnemy::BulletEnemy(float x, float y, int tier) : Enemy(x, y) {
    const DifficultySettings& diff = ActiveDifficulty::settings;

    // ── Stats (difficulty-scaled) ─────────────────────────────────────────────
    // Health increases with tier and scales with difficulty settings
    maxHp = static_cast<int>(std::round((2 + tier) * diff.enemyHpMult));
    if (maxHp < 1) maxHp = 1;
    hp = maxHp;

    // Movement speeds scale with difficulty
    moveSpeed   = (40.f + tier * 4.f) * diff.enemySpeedMult;
    strafeSpeed = 0.f;
    shootRange  = 140.f + tier * 10.f;

    // Shooting cooldown scales and gets randomized slightly to desynchronize enemy firing cycles
    shootCooldown = std::max(0.6f, (2.2f - tier * 0.2f) * diff.shootCooldownMult);
    shootTimer    = shootCooldown * (static_cast<float>(rand()) / RAND_MAX);

    // Store bullet speed and accuracy multipliers for projectile generation
    bulletSpeedMult_ = diff.bulletSpeedMult;
    bulletSpreadMult_ = diff.bulletSpreadMult;

    strafeSign = (rand() % 2 == 0) ? 1 : -1;
    state      = BulletEnemyState::CHASE;

    // Calculate unique red-hued tint based on tier level
    int r = std::min(255, 200 + tier * 15);
    int g = std::max(0,    20 - tier * 5);
    baseColor = sf::Color(r, g, 20);

    // Load visual assets
    bool idleOk = texIdle.loadFromFile("assets/bulletEnemy_idle.png");
    bool walkOk = texWalk.loadFromFile("assets/bulletEnemy_walk.png");

    if (idleOk || walkOk) {
        hasSprite = true;
        sprite.setOrigin(frameW / 2.f, frameH / 2.f);
        setSheet(false);          // Begin with idle animation state
    }

    // Set colors of fallback shapes and health bars
    shape.setFillColor(baseColor);
    hpBarFront.setFillColor(baseColor);

    if (hasSprite) tickAnim(0.f);
}

// Configures anim state (frames, durations, texture) depending on whether the enemy is moving.
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

// Main AI Update: Handles hit-flash, state changes, movement boundaries, shooting, and animations.
void BulletEnemy::updateAI(float dt, sf::Vector2f playerPos,
                            std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    if (!isActive()) return;

    // ── Hit flash ──────────────────────────────────────────────────────────────
    // Flashes sprite red (or fallback shape white) when taking damage
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
    // Determine distance to player and shift states accordingly
    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist < 70.f) {
        state = BulletEnemyState::RETREAT; // Back away if player is too close
    } else if (state == BulletEnemyState::RETREAT && dist >= 100.f) {
        state = BulletEnemyState::STRAFE;  // Resume circle-strafing once safe
    } else if (state == BulletEnemyState::CHASE && dist <= shootRange) {
        state = BulletEnemyState::STRAFE;  // Stop and circle when close enough to shoot
    } else if (state == BulletEnemyState::STRAFE && dist > shootRange * 1.2f) {
        state = BulletEnemyState::CHASE;   // Pursue player if they move too far away
    }

    // ── Movement ───────────────────────────────────────────────────────────────
    sf::Vector2f desiredMove(0.f, 0.f);
    bool isMoving = false;

    if (state == BulletEnemyState::RETREAT) {
        // Back away faster if the player is extremely close (panic speed boost)
        float panicT     = 1.f - std::min(1.f, dist / 70.f);
        float retreatSpd = moveSpeed * (1.f + panicT * 1.5f);
        if (dist > 0.f) { desiredMove = (-toPlayer / dist) * retreatSpd; isMoving = true; }
    } else if (state == BulletEnemyState::CHASE) {
        // Run straight towards the player
        if (dist > 0.f) { desiredMove = (toPlayer / dist) * moveSpeed; isMoving = true; }
    }

    // Soft boundary push: gently push the enemy away from screen edges to prevent stuck behavior
    float margin = 35.f, push = 50.f;
    if (position.x < margin)         desiredMove.x += (margin - position.x)           / margin * push;
    if (position.x > 400.f - margin) desiredMove.x -= (position.x - (400.f - margin)) / margin * push;
    if (position.y < margin)         desiredMove.y += (margin - position.y)           / margin * push;
    if (position.y > 195.f - margin) desiredMove.y -= (position.y - (195.f - margin)) / margin * push;

    position += desiredMove * dt;

    // Hard boundary clamps: enforce hard limits on workspace/arena coordinates
    if (position.x < 10.f)  position.x = 10.f;
    if (position.x > 390.f) position.x = 390.f;
    if (position.y < 10.f)  position.y = 10.f;
    if (position.y > 185.f) position.y = 185.f;

    // ── Facing direction ───────────────────────────────────────────────────────
    // Flip sprite orientation horizontally to match player's relative direction
    if (dist > 0.f) {
        if (toPlayer.x < -1.f) facingLeft = true;
        else if (toPlayer.x > 1.f) facingLeft = false;
    }

    // ── Animation sheet switch ─────────────────────────────────────────────────
    // Swap spritesheets dynamically when transitioning between idle and moving
    if (hasSprite) {
        bool currentlyWalking = (animMaxCols == 6);
        if (isMoving != currentlyWalking) {
            setSheet(isMoving);
        }
    }

    // ── Shooting ───────────────────────────────────────────────────────────────
    // Fire a 3-bullet spread burst towards the player when in range and off cooldown
    if ((state == BulletEnemyState::STRAFE || state == BulletEnemyState::RETREAT) && dist >= 60.f) {
        shootTimer -= dt;
        if (shootTimer <= 0.f) {
            shootTimer = shootCooldown;
            sf::Vector2f baseDir = playerPos - position;
            float baseLen = std::sqrt(baseDir.x * baseDir.x + baseDir.y * baseDir.y);
            if (baseLen > 0.f) baseDir /= baseLen;

            float bulletSpd = 100.f * bulletSpeedMult_;
            static constexpr int   BURST_COUNT  = 3;
            static constexpr float BURST_SPREAD = 0.22f; // Angle spacing in radians

            for (int b = 0; b < BURST_COUNT; ++b) {
                // Calculate rotation offset for spread pattern
                float offset = (static_cast<float>(b) - 1.f) * BURST_SPREAD;
                float cosA = std::cos(offset);
                float sinA = std::sin(offset);
                sf::Vector2f dir(
                    baseDir.x * cosA - baseDir.y * sinA,
                    baseDir.x * sinA + baseDir.y * cosA);

                // Add minor random inaccuracy jitter
                float ox = (static_cast<float>(rand()) / RAND_MAX) * 4.f - 2.f;
                float oy = (static_cast<float>(rand()) / RAND_MAX) * 4.f - 2.f;

                // Spawn the bullet projectile into the game scene
                spawnQueue.push_back(
                    std::make_unique<Bullet>(position.x, position.y,
                                             dir + sf::Vector2f(ox, oy) * 0.02f,
                                             bulletSpd, true));
            }
        }
    }

    // ── Finalize visuals ───────────────────────────────────────────────────────
    // Update sprite/shape positions and refresh health bar positions/sizes
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

