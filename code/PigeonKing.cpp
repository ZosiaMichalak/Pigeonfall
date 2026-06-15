#include "PigeonKing.h"
#include "Bullet.h"
#include "BulletEnemy.h"
#include "DashEnemy.h"
#include "DifficultySettings.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static constexpr float PI = 3.14159265f;

static constexpr float SCALE            = 1.5f;   // Boss sprite scaling factor
static constexpr float ORBIT_SPEED      = 1.1f;   // Orbital path rotation speed
static constexpr float ORBIT_RADIUS     = 75.f;   // Orbital path circle radius
static constexpr float DIVE_WINDUP      = 1.0f;   // telegraph hover windup duration
static constexpr float DIVE_SPEED       = 480.f;  // Lunge velocity during divebomb
static constexpr float DIVE_DURATION    = 0.35f;  // Dive attack duration
static constexpr float DIVE_RECOVER     = 0.45f;  // Recovery pause duration on ground
static constexpr float DIVE_CD_MIN      = 1.8f;   // Minimum dive cooldown spacing
static constexpr float DIVE_CD_MAX      = 3.2f;   // Maximum dive cooldown spacing
static constexpr float FEATHER_SPEED    = 135.f;  // Velocity of fired feather bullet projectiles
static constexpr float LANDING_DURATION = 0.6f;   // Transition landing phase duration
static constexpr int   BOSS_HP          = 120;    // Legacy fallback base health

// Sprite sheets are 51x32 per frame (same as normal enemies), scaled x1.5
static constexpr int   FRAME_W = 51;
static constexpr int   FRAME_H = 32;

// Constructor: Configures stats, health overlay UI, spritesheets, difficulty settings, and spawns.
PigeonKing::PigeonKing(float x, float y, PigeonBossTier tier) : Enemy(x, y) {
    const DifficultySettings& diff = ActiveDifficulty::settings;

    // Define base health pool values based on mid/final boss classifications
    int baseHp = (tier == PigeonBossTier::STRONG) ? 380 : 170;
    maxHp = static_cast<int>(baseHp * diff.bossHpMult);
    hp    = maxHp;

    // Scale combat features according to boss tiers and difficulty multipliers
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

    moveSpeed = 0.f; // Positional shifts handled inside updateAI

    // Configure override size properties for sprite animation ticks
    frameW = FRAME_W;
    frameH = FRAME_H;

    // Setup dive target shadow shape (rendered on the ground where the boss will land)
    shadow.setRadius(22.f);
    shadow.setOrigin(22.f, 22.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    shadow.setOutlineThickness(2.f);
    shadow.setOutlineColor(sf::Color(200, 30, 30, 160));

    // Setup main top-of-screen boss HP bar background container
    bossBarBack.setSize({180.f, 8.f});
    bossBarBack.setFillColor(sf::Color(50, 20, 50));
    bossBarBack.setOutlineThickness(1.f);
    bossBarBack.setOutlineColor(sf::Color(120, 60, 120));
    bossBarBack.setPosition(110.f, 6.f);

    // Setup main top-of-screen boss HP status fill line
    bossBarFront.setSize({180.f, 8.f});
    bossBarFront.setFillColor(sf::Color(200, 60, 220));
    bossBarFront.setPosition(110.f, 6.f);

    // Setup legacy warning line indicator
    warnLine.setSize({400.f, 6.f});
    warnLine.setFillColor(sf::Color(255, 60, 60, 140));
    warnLine.setOutlineThickness(1.f);
    warnLine.setOutlineColor(sf::Color(255, 20, 20, 200));

    // Desynchronize start of first dive lunge cycle
    diveCD = DIVE_CD_MIN +
        (static_cast<float>(std::rand()) / RAND_MAX) * 2.f;

    // Configure fallback shape properties
    shape.setSize({36.f, 28.f});
    shape.setOrigin(18.f, 14.f);
    shape.setFillColor(baseColor);
    hpBarFront.setFillColor(sf::Color(200, 60, 220));

    // Load visual sheet resources
    texIdle.loadFromFile("assets/pigeonKing_idle.png");
    texWalk.loadFromFile("assets/pigeonKing_walk.png");
    texSpawn.loadFromFile("assets/pigeonKing_spawn.png");

    if (texSpawn.getSize().x > 0 || texIdle.getSize().x > 0 || texWalk.getSize().x > 0) {
        hasSprite = true;
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);
        sprite.setScale(SCALE, SCALE);
    }

    // Assign entrance spawning animation sheet if loaded, otherwise start flying
    if (hasSprite && texSpawn.getSize().x > 0) {
        int spawnFrames = static_cast<int>(texSpawn.getSize().x / FRAME_W)
                        * static_cast<int>(texSpawn.getSize().y / FRAME_H);
        spawnFrames = std::max(1, spawnFrames);
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

// Configures anim state metrics for the current spritesheet assignment.
void PigeonKing::setSheet(sf::Texture& tex, int totalFrames, int sheetCols, float dur) {
    if (tex.getSize().x == 0) return;
    sprite.setTexture(tex);
    animMaxCols   = totalFrames;
    animSheetCols = sheetCols;
    frameDur      = dur;
    animCol       = 0;
    animTimer     = 0.f;
}

// Attack Helper: fires a fan of 5 feather projectiles spreading outward towards player coordinates.
void PigeonKing::shootFeathers(sf::Vector2f playerPos,
                                std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    sf::Vector2f base = playerPos - position;
    float baseAngle = std::atan2(base.y, base.x);
    float spread = PI / 4.f;
    int count = 5;
    
    // Spawn spread bullet entities with incremental angular offsets
    for (int i = 0; i < count; ++i) {
        float angle = baseAngle - spread / 2.f + spread * i / (count - 1);
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnQueue.push_back(std::make_unique<Bullet>(
            position.x, position.y, dir, FEATHER_SPEED, true));
    }
}

// Main AI Update Routine: handles phase transitions, AI movements, attacks, animations, and overlays.
void PigeonKing::updateAI(float dt, sf::Vector2f playerPos,
                           std::vector<std::unique_ptr<GameObject>>& spawnQueue)
{
    if (!isActive()) return;

    // ── Hit flash ─────────────────────────────────────────────────────────────
    // Flashes sprite red upon taking hit damage
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

    // Face player coordinates
    sf::Vector2f toPlayer = playerPos - position;
    float dist = std::sqrt(toPlayer.x*toPlayer.x + toPlayer.y*toPlayer.y);
    if (dist > 0.f) facingLeft = toPlayer.x < 0.f;

    // ── Main Phase Machine ───────────────────────────────────────────────────
    switch (phase) {

    case PigeonPhase::SPAWNING: {
        // Phase 0: SPAWNING - Play entrance layout sheet, transition when finished
        spawnAnimTimer += dt;
        if (spawnAnimTimer >= spawnAnimDur) {
            phase = PigeonPhase::FLYING;
            if (hasSprite && texIdle.getSize().x > 0)
                setSheet(texIdle, 3, 1, 0.18f);
        }
        break;
    }

    case PigeonPhase::FLYING: {
        // Phase 1: FLYING - Boss remains in the air, diving and firing feather fans

        // Dynamically toggle walking (diving charge) or idle (flight/orbit) textures
        if (hasSprite) {
            bool moving = (flyState == FlyingState::DIVEBOMB_LUNGE);
            sf::Texture& tex = moving ? texWalk : texIdle;
            bool needSwitch = (moving != flyingUsingWalkSheet);
            if (tex.getSize().x > 0 && needSwitch) {
                flyingUsingWalkSheet = moving;
                int frames = moving ? 6 : 3;
                int cols   = moving ? 2 : 1;
                float dur  = moving ? 0.10f : 0.18f;
                setSheet(tex, frames, cols, dur);
            }
        }

        // Shift to Phase 2 (Landing -> Barrages) when HP drops below 50%
        if (hp <= maxHp / 2) {
            phase        = PigeonPhase::LANDING;
            landingTimer = LANDING_DURATION;
            flyState     = FlyingState::CIRCLE;
            break;
        }

        // Flight Sub-State Machine
        switch (flyState) {
        case FlyingState::CIRCLE: {
            // Orbit player coordinates, shifting target slightly over time
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

            // Periodically fan shoot feathers
            shootTimer -= dt;
            if (shootTimer <= 0.f) {
                shootFeathers(playerPos, spawnQueue);
                shootTimer = 1.5f;
            }
            
            // Check if dive bomb CD expired to lock in targets and start windup
            diveCD -= dt;
            if (diveCD <= 0.f) {
                flyState        = FlyingState::DIVEBOMB_WINDUP;
                diveWindupTimer = DIVE_WINDUP;
                diveTarget = playerPos;
                shadow.setPosition(diveTarget);
            }
            break;
        }
        case FlyingState::DIVEBOMB_WINDUP: {
            // Windup: drift slowly toward player while locking down target coordinates
            sf::Vector2f toPlayer2 = playerPos - position;
            float d2 = std::sqrt(toPlayer2.x*toPlayer2.x + toPlayer2.y*toPlayer2.y);
            if (d2 > 0.f) position += (toPlayer2 / d2) * 30.f * dt;

            diveTarget = playerPos;
            shadow.setPosition(diveTarget);

            // Pulse shadow size to telegraph landing
            diveWindupTimer -= dt;
            float pulse = 1.f + 0.15f * std::sin(diveWindupTimer * 12.f);
            shadow.setScale(pulse, pulse);

            // Execute the charge when windup completes
            if (diveWindupTimer <= 0.f) {
                flyState       = FlyingState::DIVEBOMB_LUNGE;
                diveLungeTimer = DIVE_DURATION;
                sf::Vector2f d = diveTarget - position;
                float len = std::sqrt(d.x*d.x + d.y*d.y);
                diveLungeDir = (len > 0.f) ? d / len : sf::Vector2f(0.f, 1.f);
            }
            break;
        }
        case FlyingState::DIVEBOMB_LUNGE: {
            // High-speed lunge straight toward locked coordinates
            position += diveLungeDir * DIVE_SPEED * diveSpeedMult * dt;
            diveLungeTimer -= dt;
            
            // Recover and shoot a shock-feather fan upon landing
            if (diveLungeTimer <= 0.f) {
                flyState         = FlyingState::DIVEBOMB_RECOVER;
                diveRecoverTimer = DIVE_RECOVER;
                shootFeathers(playerPos, spawnQueue);
            }
            break;
        }
        case FlyingState::DIVEBOMB_RECOVER: {
            // Recovery: pause briefly, then return to orbit and spawn helper spawns
            diveRecoverTimer -= dt;
            if (diveRecoverTimer <= 0.f) {
                flyState    = FlyingState::CIRCLE;
                diveCD      = DIVE_CD_MIN +
                    (static_cast<float>(std::rand()) / RAND_MAX) * (DIVE_CD_MAX - DIVE_CD_MIN);
                orbitCenter = position;

                // Spawn 1-2 generic enemy minions to distract player
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

    case PigeonPhase::LANDING: {
        // Phase Landing Transition: return to mid-screen anchor coordinates
        landingTimer -= dt;
        sf::Vector2f centre(200.f, 95.f);
        sf::Vector2f d = centre - position;
        float len = std::sqrt(d.x*d.x + d.y*d.y);
        if (len > 2.f) position += (d / len) * 100.f * dt;

        // Initiate Wave Barrages when landing cycle completes
        if (landingTimer <= 0.f) {
            phase          = PigeonPhase::WAVE_BARRAGE;
            wavesRemaining = wavesPerBarrage;
            waveTimer      = 0.6f;
            waveFromLeft   = true;
            showWarnLine   = false;
            
            if (hasSprite && texWalk.getSize().x > 0)
                setSheet(texWalk, 6, 2, 0.14f);

            // Spawn generic helpers to shield the boss
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

    case PigeonPhase::WAVE_BARRAGE: {
        // Phase 2: WAVE BARRAGE - Fire hazard zones covering half-arena boundaries. Boss is immune.
        if (!isHit) {
            if (hasSprite) sprite.setColor(sf::Color::White);
            else           shape.setFillColor(baseColor);
        }

        // ── Tick existing zones ───────────────────────────────────────────────
        for (auto& z : waveZones) {
            if (z.done) continue;
            if (!z.active) {
                // Warning phase: blink hazard zone overlay
                z.warnTimer -= dt;
                sf::Uint8 alpha = static_cast<sf::Uint8>(
                    80.f + 80.f * std::abs(std::sin(z.warnTimer * 10.f)));
                z.shape.setFillColor(sf::Color(255, 80, 30, alpha));
                if (z.warnTimer <= 0.f) {
                    z.active = true;
                    z.shape.setFillColor(sf::Color(255, 30, 30, 110)); // Make red and active
                }
            } else {
                // Active phase: deal damage overlay ticks
                z.activeTimer -= dt;
                sf::Uint8 alpha = static_cast<sf::Uint8>(
                    90.f + 50.f * std::abs(std::sin(z.activeTimer * 6.f)));
                z.shape.setFillColor(sf::Color(255, 30, 30, alpha));
                if (z.activeTimer <= 0.f) z.done = true;
            }
        }
        
        // Clean up completed zone items
        waveZones.erase(
            std::remove_if(waveZones.begin(), waveZones.end(),
                           [](const WaveZone& z){ return z.done; }),
            waveZones.end());

        // ── Schedule next zone ────────────────────────────────────────────────────
        // Only count down to next wave when all previous zones are fully gone
        bool anyZoneAlive = !waveZones.empty();
        if (!anyZoneAlive) {
            waveTimer -= dt;
        }
        if (waveTimer <= 0.f && wavesRemaining > 0) {
            // Target the quadrant/half currently occupied by the player
            float dx = playerPos.x - 200.f;
            float dy = playerPos.y - 97.5f;
            int side;
            if (std::abs(dx) >= std::abs(dy))
                side = (playerPos.x < 200.f) ? 2 : 3;
            else
                side = (playerPos.y < 97.5f) ? 0 : 1;

            WaveZone z;
            switch (side) {
                case 0: z.rect = { 0.f,   0.f, 400.f,  97.f }; break; // Top half
                case 1: z.rect = { 0.f,  98.f, 400.f,  97.f }; break; // Bottom half
                case 2: z.rect = { 0.f,   0.f, 200.f, 195.f }; break; // Left half
                default: z.rect = { 200.f, 0.f, 200.f, 195.f }; break; // Right half
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
            waveTimer = 0.2f; // 0.2s gap AFTER previous zone fully disappears before next spawns

            // When all waves are discharged, stagger the boss into a vulnerable stun state
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

    case PigeonPhase::VULNERABLE: {
        // Phase Vulnerable: Boss staggers, flashes green, and is highly vulnerable to item damage
        vulnerableTimer -= dt;
        if (!isHit) {
            if (hasSprite) sprite.setColor(sf::Color(140, 255, 160));
            else           shape.setFillColor(sf::Color(100, 220, 120));
        }

        // Recover and increase future wave paces when stagger duration concludes
        if (vulnerableTimer <= 0.f) {
            if (hasSprite) sprite.setColor(sf::Color::White);
            else           shape.setFillColor(baseColor);
            phase        = PigeonPhase::RECOVER;
            recoverTimer = RECOVER_DUR;
            ++barrageCount;
            wavePaceScale = std::max(0.30f, wavePaceScale * 0.82f); // Future waves deploy faster
            if (hasSprite && texWalk.getSize().x > 0)
                setSheet(texWalk, 6, 2, 0.14f);
        }
        break;
    }

    case PigeonPhase::RECOVER: {
        // Phase Recovery: brief pause before executing the next wave barrage sequence
        recoverTimer -= dt;
        if (recoverTimer <= 0.f) {
            phase          = PigeonPhase::WAVE_BARRAGE;
            wavesRemaining = wavesPerBarrage + (barrageCount / 2); // Scales count with cycles
            waveTimer      = 0.6f;
            waveFromLeft   = (std::rand() % 2 == 0);
            showWarnLine   = false;

            // Spawn minions at start of new cycle
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
    // Keep coordinates within arena map boundary margins
    position.x = std::max(18.f, std::min(382.f, position.x));
    position.y = std::max(14.f, std::min(175.f, position.y));

    // ── Finalize sprite ───────────────────────────────────────────────────────
    if (hasSprite) tickAnim(dt);
    else           shape.setPosition(position);

    // Update HP bar dimensions and positions
    float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
    hpBarBack.setPosition (position.x, position.y - 18.f);
    hpBarFront.setPosition(position.x, position.y - 18.f);
    hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
    bossBarFront.setSize({180.f * pct, 8.f});
}

// Draws the boss, danger wave zones, target shadow projections, and health indicators.
void PigeonKing::draw(sf::RenderWindow& window) {
    if (!isActive()) return;

    // Draw active danger sweeps (hidden during stagger phases)
    if (phase != PigeonPhase::VULNERABLE) {
        for (const auto& z : waveZones)
            if (!z.done) window.draw(z.shape);
    }

    // Draw landing shadow target during divebomb preparation
    if (phase == PigeonPhase::FLYING && flyState == FlyingState::DIVEBOMB_WINDUP)
        window.draw(shadow);

    Enemy::draw(window);

    // Render global boss UI HP lines
    window.draw(bossBarBack);
    window.draw(bossBarFront);
}

