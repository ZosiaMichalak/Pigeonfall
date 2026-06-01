#include "PigeonKing.h"
#include "Bullet.h"
#include "BulletEnemy.h"
#include "DashEnemy.h"
#include "DifficultySettings.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static constexpr float PI = 3.14159265f;

static constexpr float SCALE            = 1.5f;
static constexpr float ORBIT_SPEED      = 1.1f;
static constexpr float ORBIT_RADIUS     = 75.f;
static constexpr float DIVE_WINDUP      = 1.0f;
static constexpr float DIVE_SPEED       = 480.f;
static constexpr float DIVE_DURATION    = 0.35f;
static constexpr float DIVE_RECOVER     = 0.45f;
static constexpr float DIVE_CD_MIN      = 1.8f;
static constexpr float DIVE_CD_MAX      = 3.2f;
static constexpr float FEATHER_SPEED    = 135.f;
static constexpr float LANDING_DURATION = 0.6f;
static constexpr int   BOSS_HP          = 120;

// Sprite sheets are 51x32 per frame (same as normal enemies), scaled x1.5
static constexpr int   FRAME_W = 51;
static constexpr int   FRAME_H = 32;

// ── Constructor ───────────────────────────────────────────────────────────────
PigeonKing::PigeonKing(float x, float y, PigeonBossTier tier) : Enemy(x, y) {
    const DifficultySettings& diff = ActiveDifficulty::settings;

    int baseHp = (tier == PigeonBossTier::STRONG) ? 380 : 170;
    maxHp = static_cast<int>(baseHp * diff.bossHpMult);
    hp    = maxHp;

    if (tier == PigeonBossTier::STRONG) {
        wavesPerBarrage = 10;
        vulnerableDur   = 1.6f;
        diveSpeedMult   = 1.65f * diff.bossDiveSpeedMult;
        warnDuration    = WARN_DURATION * diff.bossWarnMult;
    } else {
        wavesPerBarrage = 7;
        vulnerableDur   = 2.4f;
        diveSpeedMult   = 1.25f * diff.bossDiveSpeedMult;
        warnDuration    = WARN_DURATION * diff.bossWarnMult;
    }
    vulnerableDur *= diff.bossVulnerableMult;

    moveSpeed = 0.f;

    // Override base class frame size for tickAnim
    frameW = FRAME_W;
    frameH = FRAME_H;

    shadow.setRadius(22.f);
    shadow.setOrigin(22.f, 22.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    shadow.setOutlineThickness(2.f);
    shadow.setOutlineColor(sf::Color(200, 30, 30, 160));

    bossBarBack.setSize({180.f, 8.f});
    bossBarBack.setFillColor(sf::Color(50, 20, 50));
    bossBarBack.setOutlineThickness(1.f);
    bossBarBack.setOutlineColor(sf::Color(120, 60, 120));
    bossBarBack.setPosition(110.f, 6.f);

    bossBarFront.setSize({180.f, 8.f});
    bossBarFront.setFillColor(sf::Color(200, 60, 220));
    bossBarFront.setPosition(110.f, 6.f);

    warnLine.setSize({400.f, 6.f});
    warnLine.setFillColor(sf::Color(255, 60, 60, 140));
    warnLine.setOutlineThickness(1.f);
    warnLine.setOutlineColor(sf::Color(255, 20, 20, 200));

    diveCD = DIVE_CD_MIN +
        (static_cast<float>(std::rand()) / RAND_MAX) * 2.f;

    shape.setSize({36.f, 28.f});
    shape.setOrigin(18.f, 14.f);
    shape.setFillColor(baseColor);
    hpBarFront.setFillColor(sf::Color(200, 60, 220));

    // Load textures
    texIdle.loadFromFile("assets/pigeonKing_idle.png");
    texWalk.loadFromFile("assets/pigeonKing_walk.png");
    texSpawn.loadFromFile("assets/pigeonKing_spawn.png");

    if (texSpawn.getSize().x > 0 || texIdle.getSize().x > 0 || texWalk.getSize().x > 0) {
        hasSprite = true;
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);
        sprite.setScale(SCALE, SCALE);
    }

    // Start with spawn animation if available, else skip straight to flying
    if (hasSprite && texSpawn.getSize().x > 0) {
        int spawnFrames = static_cast<int>(texSpawn.getSize().x / FRAME_W)
                        * static_cast<int>(texSpawn.getSize().y / FRAME_H);
        spawnFrames = std::max(1, spawnFrames);
        // Compress spawn anim into exactly 0.5 s
        float spawnFrameDur = 0.5f / static_cast<float>(spawnFrames);
        setSheet(texSpawn, spawnFrames, std::max(1, static_cast<int>(texSpawn.getSize().x / FRAME_W)), spawnFrameDur);
        spawnAnimDur  = 0.5f;
        spawnAnimTimer = 0.f;
        phase = PigeonPhase::SPAWNING;
    } else {
        phase = PigeonPhase::FLYING;
        if (hasSprite && texIdle.getSize().x > 0)
            setSheet(texIdle, 3, 1, 0.18f);
    }

    if (hasSprite) tickAnim(0.f);
}

void PigeonKing::setSheet(sf::Texture& tex, int totalFrames, int sheetCols, float dur) {
    if (tex.getSize().x == 0) return;
    sprite.setTexture(tex);
    animMaxCols   = totalFrames;
    animSheetCols = sheetCols;
    frameDur      = dur;
    animCol       = 0;
    animTimer     = 0.f;
}

void PigeonKing::shootFeathers(sf::Vector2f playerPos,
                                std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    sf::Vector2f base = playerPos - position;
    float baseAngle = std::atan2(base.y, base.x);
    float spread = PI / 4.f;
    int count = 5;
    for (int i = 0; i < count; ++i) {
        float angle = baseAngle - spread / 2.f + spread * i / (count - 1);
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnQueue.push_back(std::make_unique<Bullet>(
            position.x, position.y, dir, FEATHER_SPEED, true));
    }
}

// ── updateAI ──────────────────────────────────────────────────────────────────
void PigeonKing::updateAI(float dt, sf::Vector2f playerPos,
                           std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    if (!isActive()) return;

    // ── Hit flash ─────────────────────────────────────────────────────────────
    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            if (hasSprite) sprite.setColor(sf::Color::White);
            else           shape.setFillColor(baseColor);
        } else {
            if (hasSprite) sprite.setColor(sf::Color(255, 80, 80));
            else           shape.setFillColor(sf::Color::White);
        }
    }

    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y);
    if (dist > 0.f) facingLeft = toPlayer.x < 0.f;

    switch (phase) {

    // ═══════════════════════════════════════════════════════════════════════
    case PigeonPhase::SPAWNING: {
        // Play spawn animation once, then transition to FLYING
        spawnAnimTimer += dt;
        if (spawnAnimTimer >= spawnAnimDur) {
            phase = PigeonPhase::FLYING;
            if (hasSprite && texIdle.getSize().x > 0)
                setSheet(texIdle, 3, 1, 0.18f);
        }
        break;
    }

    // ═══════════════════════════════════════════════════════════════════════
    case PigeonPhase::FLYING: {
        // Phase 1 — always hittable, no vulnerability guard needed

        // Switch to idle or walk sheet based on movement
        if (hasSprite) {
            bool moving = (flyState == FlyingState::DIVEBOMB_LUNGE);
            sf::Texture& tex = moving ? texWalk : texIdle;
            // Compare pointer to texture size as a proxy for "is this sheet already set"
            bool needSwitch = (moving != flyingUsingWalkSheet);
            if (tex.getSize().x > 0 && needSwitch) {
                flyingUsingWalkSheet = moving;
                int frames = moving ? 6 : 3;
                int cols   = moving ? 2 : 1;
                float dur  = moving ? 0.10f : 0.18f;
                setSheet(tex, frames, cols, dur);
            }
        }

        // Phase 1 → 2 transition at 50% HP
        if (hp <= maxHp / 2) {
            phase        = PigeonPhase::LANDING;
            landingTimer = LANDING_DURATION;
            flyState     = FlyingState::CIRCLE;
            break;
        }

        switch (flyState) {
        case FlyingState::CIRCLE: {
            // Walk toward a point orbiting the player — no teleport, actual movement
            orbitCenter.x += (playerPos.x - orbitCenter.x) * 1.5f * dt;
            orbitCenter.y += (playerPos.y - orbitCenter.y) * 1.5f * dt;
            orbitAngle += ORBIT_SPEED * dt;

            sf::Vector2f orbitTarget(
                orbitCenter.x + std::cos(orbitAngle) * ORBIT_RADIUS,
                orbitCenter.y + std::sin(orbitAngle) * ORBIT_RADIUS * 0.6f);

            sf::Vector2f toTarget = orbitTarget - position;
            float tLen = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
            float walkSpeed = 70.f;
            if (tLen > 0.f)
                position += (toTarget / tLen) * std::min(tLen / dt, walkSpeed) * dt;

            shootTimer -= dt;
            if (shootTimer <= 0.f) {
                shootFeathers(playerPos, spawnQueue);
                shootTimer = 1.5f;
            }
            diveCD -= dt;
            if (diveCD <= 0.f) {
                flyState        = FlyingState::DIVEBOMB_WINDUP;
                diveWindupTimer = DIVE_WINDUP;
                // Lock target to current player pos for accuracy
                diveTarget = playerPos;
                shadow.setPosition(diveTarget);
            }
            break;
        }
        case FlyingState::DIVEBOMB_WINDUP: {
            // Keep walking toward player during wind-up — no orbit snap
            sf::Vector2f toPlayer2 = playerPos - position;
            float d2 = std::sqrt(toPlayer2.x*toPlayer2.x + toPlayer2.y*toPlayer2.y);
            if (d2 > 0.f) position += (toPlayer2 / d2) * 30.f * dt;

            // Update target to track player during wind-up for accuracy
            diveTarget = playerPos;
            shadow.setPosition(diveTarget);

            diveWindupTimer -= dt;
            float pulse = 1.f + 0.15f * std::sin(diveWindupTimer * 12.f);
            shadow.setScale(pulse, pulse);

            if (diveWindupTimer <= 0.f) {
                flyState       = FlyingState::DIVEBOMB_LUNGE;
                diveLungeTimer = DIVE_DURATION;
                // Compute direction at the last moment for maximum accuracy
                sf::Vector2f d = diveTarget - position;
                float len = std::sqrt(d.x*d.x + d.y*d.y);
                diveLungeDir = (len > 0.f) ? d / len : sf::Vector2f(0.f, 1.f);
            }
            break;
        }
        case FlyingState::DIVEBOMB_LUNGE: {
            position += diveLungeDir * DIVE_SPEED * diveSpeedMult * dt;
            diveLungeTimer -= dt;
            if (diveLungeTimer <= 0.f) {
                flyState         = FlyingState::DIVEBOMB_RECOVER;
                diveRecoverTimer = DIVE_RECOVER;
                shootFeathers(playerPos, spawnQueue);
            }
            break;
        }
        case FlyingState::DIVEBOMB_RECOVER: {
            diveRecoverTimer -= dt;
            if (diveRecoverTimer <= 0.f) {
                flyState    = FlyingState::CIRCLE;
                diveCD      = DIVE_CD_MIN +
                    (static_cast<float>(std::rand()) / RAND_MAX) * (DIVE_CD_MAX - DIVE_CD_MIN);
                orbitCenter = position;

                // Spawn 1-2 enemies after each dive
                int enemyCount = 1 + std::rand() % 2;
                float spawnXs[] = { 40.f, 360.f, 200.f };
                float spawnYs[] = { 50.f, 150.f,  80.f };
                for (int ei = 0; ei < enemyCount; ++ei) {
                    float ex = spawnXs[ei % 3] + (static_cast<float>(std::rand()) / RAND_MAX) * 24.f - 12.f;
                    float ey = spawnYs[ei % 3] + (static_cast<float>(std::rand()) / RAND_MAX) * 24.f - 12.f;
                    if (std::rand() % 2 == 0)
                        spawnQueue.push_back(std::make_unique<BulletEnemy>(ex, ey, 0));
                    else
                        spawnQueue.push_back(std::make_unique<DashEnemy>(ex, ey, 0));
                }
            }
            break;
        }
        }
        break;
    }

    // ═══════════════════════════════════════════════════════════════════════
    case PigeonPhase::LANDING: {
        landingTimer -= dt;
        sf::Vector2f centre(200.f, 95.f);
        sf::Vector2f d = centre - position;
        float len = std::sqrt(d.x*d.x + d.y*d.y);
        if (len > 2.f) position += (d / len) * 100.f * dt;

        if (landingTimer <= 0.f) {
            phase          = PigeonPhase::WAVE_BARRAGE;
            wavesRemaining = wavesPerBarrage;
            waveTimer      = 0.6f;
            waveFromLeft   = true;
            showWarnLine   = false;
            // Switch to walk sheet for phase 2
            if (hasSprite && texWalk.getSize().x > 0)
                setSheet(texWalk, 6, 2, 0.14f);

            // Spawn 1-3 enemies at the start of each barrage cycle
            int enemyCount = 2 + std::rand() % 3;
            float spawnXs[] = { 40.f, 360.f, 200.f };
            float spawnYs[] = { 50.f,  50.f, 160.f };
            for (int ei = 0; ei < enemyCount; ++ei) {
                float ex = spawnXs[ei % 3] + (static_cast<float>(std::rand()) / RAND_MAX) * 20.f - 10.f;
                float ey = spawnYs[ei % 3] + (static_cast<float>(std::rand()) / RAND_MAX) * 20.f - 10.f;
                if (std::rand() % 2 == 0)
                    spawnQueue.push_back(std::make_unique<BulletEnemy>(ex, ey, 0));
                else
                    spawnQueue.push_back(std::make_unique<DashEnemy>(ex, ey, 0));
            }
        }
        break;
    }

    // ═══════════════════════════════════════════════════════════════════════
    case PigeonPhase::WAVE_BARRAGE: {
        if (!isHit) {
            if (hasSprite) sprite.setColor(sf::Color::White);
            else           shape.setFillColor(baseColor);
        }

        // ── Tick existing zones ───────────────────────────────────────────────
        for (auto& z : waveZones) {
            if (z.done) continue;
            if (!z.active) {
                // Warning phase — flicker the overlay
                z.warnTimer -= dt;
                sf::Uint8 alpha = static_cast<sf::Uint8>(
                    80.f + 80.f * std::abs(std::sin(z.warnTimer * 10.f)));
                z.shape.setFillColor(sf::Color(255, 80, 30, alpha));
                if (z.warnTimer <= 0.f) {
                    z.active = true;
                    z.shape.setFillColor(sf::Color(255, 30, 30, 110));
                }
            } else {
                // Active phase — zone is hot
                z.activeTimer -= dt;
                // Pulse intensity
                sf::Uint8 alpha = static_cast<sf::Uint8>(
                    90.f + 50.f * std::abs(std::sin(z.activeTimer * 6.f)));
                z.shape.setFillColor(sf::Color(255, 30, 30, alpha));
                if (z.activeTimer <= 0.f) z.done = true;
            }
        }
        // Remove finished zones
        waveZones.erase(
            std::remove_if(waveZones.begin(), waveZones.end(),
                           [](const WaveZone& z){ return z.done; }),
            waveZones.end());

        // ── Schedule next zone ────────────────────────────────────────────────────
        waveTimer -= dt;
        if (waveTimer <= 0.f && wavesRemaining > 0) {
            // Spawn on the arena half that currently contains the player
            float dx = playerPos.x - 200.f;
            float dy = playerPos.y - 97.5f;
            int side;
            if (std::abs(dx) >= std::abs(dy))
                side = (playerPos.x < 200.f) ? 2 : 3;
            else
                side = (playerPos.y < 97.5f) ? 0 : 1;

            WaveZone z;
            switch (side) {
                case 0: z.rect = { 0.f,   0.f, 400.f,  97.f }; break;
                case 1: z.rect = { 0.f,  98.f, 400.f,  97.f }; break;
                case 2: z.rect = { 0.f,   0.f, 200.f, 195.f }; break;
                default: z.rect = { 200.f, 0.f, 200.f, 195.f }; break;
            }
            z.shape.setSize({ z.rect.width, z.rect.height });
            z.shape.setPosition(z.rect.left, z.rect.top);
            z.shape.setFillColor(sf::Color(255, 80, 30, 80));
            z.warnTimer       = warnDuration;
            z.activeTimer     = 1.15f;
            z.dmgTimer        = 0.f;
            z.active          = false;
            z.done            = false;
            z.buffDmgApplied  = false;
            waveZones.push_back(std::move(z));

            --wavesRemaining;
            waveTimer = (warnDuration + 0.55f) * wavePaceScale;

            if (wavesRemaining <= 0) {
                waveZones.clear();
                phase           = PigeonPhase::VULNERABLE;
                vulnerableTimer = vulnerableDur;
                if (hasSprite && texIdle.getSize().x > 0)
                    setSheet(texIdle, 3, 1, 0.18f);
            }
        }
        break;
    }

    // ═══════════════════════════════════════════════════════════════════════
    case PigeonPhase::VULNERABLE: {
        vulnerableTimer -= dt;
        if (!isHit) {
            if (hasSprite) sprite.setColor(sf::Color(140, 255, 160));
            else           shape.setFillColor(sf::Color(100, 220, 120));
        }

        if (vulnerableTimer <= 0.f) {
            if (hasSprite) sprite.setColor(sf::Color::White);
            else           shape.setFillColor(baseColor);
            phase        = PigeonPhase::RECOVER;
            recoverTimer = RECOVER_DUR;
            ++barrageCount;
            wavePaceScale = std::max(0.30f, wavePaceScale * 0.82f);
            // Back to walk sheet
            if (hasSprite && texWalk.getSize().x > 0)
                setSheet(texWalk, 6, 2, 0.14f);
        }
        break;
    }

    // ═══════════════════════════════════════════════════════════════════════
    case PigeonPhase::RECOVER: {
        recoverTimer -= dt;
        if (recoverTimer <= 0.f) {
            phase          = PigeonPhase::WAVE_BARRAGE;
            wavesRemaining = wavesPerBarrage + (barrageCount / 2);
            waveTimer      = 0.6f;
            waveFromLeft   = (std::rand() % 2 == 0);
            showWarnLine   = false;

            // Spawn 1-3 enemies at the start of each barrage cycle
            int enemyCount = 2 + std::rand() % 3;
            float spawnXs[] = { 40.f, 360.f, 200.f };
            float spawnYs[] = { 50.f,  50.f, 160.f };
            for (int ei = 0; ei < enemyCount; ++ei) {
                float ex = spawnXs[ei % 3] + (static_cast<float>(std::rand()) / RAND_MAX) * 20.f - 10.f;
                float ey = spawnYs[ei % 3] + (static_cast<float>(std::rand()) / RAND_MAX) * 20.f - 10.f;
                if (std::rand() % 2 == 0)
                    spawnQueue.push_back(std::make_unique<BulletEnemy>(ex, ey, 0));
                else
                    spawnQueue.push_back(std::make_unique<DashEnemy>(ex, ey, 0));
            }
        }
        break;
    }
    }

    // ── Clamp ─────────────────────────────────────────────────────────────────
    position.x = std::max(18.f, std::min(382.f, position.x));
    position.y = std::max(14.f, std::min(175.f, position.y));

    // ── Finalize sprite ───────────────────────────────────────────────────────
    if (hasSprite) tickAnim(dt);
    else           shape.setPosition(position);

    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarBack.setPosition (position.x, position.y - 18.f);
    hpBarFront.setPosition(position.x, position.y - 18.f);
    hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
    bossBarFront.setSize({180.f * pct, 8.f});
}

void PigeonKing::draw(sf::RenderWindow& window) {
    if (!isActive()) return;

    if (phase != PigeonPhase::VULNERABLE) {
        for (const auto& z : waveZones)
            if (!z.done) window.draw(z.shape);
    }

    if (phase == PigeonPhase::FLYING && flyState == FlyingState::DIVEBOMB_WINDUP)
        window.draw(shadow);

    Enemy::draw(window);

    window.draw(bossBarBack);
    window.draw(bossBarFront);
}
