#ifndef PIGEON_KING_H
#define PIGEON_KING_H

#include "Enemy.h"
#include <SFML/Graphics.hpp>

enum class PigeonPhase {
    SPAWNING,     // one-shot spawn animation before anything else
    FLYING,       // phase 1 — orbits, dive-bombs, shoots feathers (always hittable)
    LANDING,      // brief transition to phase 2
    WAVE_BARRAGE, // phase 2 — fires bullet waves
    VULNERABLE,   // phase 2 — boss staggers, takes damage
    RECOVER       // brief pause before next barrage cycle
};

enum class FlyingState {
    CIRCLE,
    DIVEBOMB_WINDUP,
    DIVEBOMB_LUNGE,
    DIVEBOMB_RECOVER
};

enum class PigeonBossTier {
    WEAK,   // room 25
    STRONG  // room 50 — final boss
};

// ── Wave hazard zone ─────────────────────────────────────────────────────────
struct WaveZone {
    sf::FloatRect rect;         // half-arena rectangle
    float warnTimer  = 0.f;     // counts down; zone is warning while > 0
    float activeTimer = 0.f;    // counts down after warn; zone is active while > 0
    float dmgTimer   = 0.f;     // how long player has been inside this frame cycle
    bool  active     = false;   // true = deal damage, false = just warning
    bool  done       = false;   // remove when both timers expired
    bool  buffDmgApplied = false; // monster buff: only one tick per wave zone

    sf::RectangleShape shape;   // rendered overlay
};

class PigeonKing : public Enemy {
public:
    explicit PigeonKing(float x, float y, PigeonBossTier tier = PigeonBossTier::WEAK);

    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) override;

    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getBounds() const {
        return { position.x - 18.f, position.y - 14.f, 36.f, 28.f };
    }

    bool isBoss()          const override { return true; }
    bool canTakeItemDamage() const override { return isVulnerable(); }
    // Phase 1: always hittable. Phase 2: only during VULNERABLE.
    bool isVulnerable()    const {
        return phase == PigeonPhase::FLYING   ||
               phase == PigeonPhase::VULNERABLE;
    }
    bool isDefeated()      const { return !isActive() && deathHandled; }
    void markDeathHandled()      { deathHandled = true; }

    // Damage zones — Game::update iterates these to apply player damage
    std::vector<WaveZone>& getWaveZones() { return waveZones; }

private:
    PigeonPhase  phase      = PigeonPhase::SPAWNING;
    FlyingState  flyState   = FlyingState::CIRCLE;
    bool         deathHandled = false;

    // ── Spawn animation ───────────────────────────────────────────────────────
    float spawnAnimTimer  = 0.f;   // counts up while spawn sheet plays
    float spawnAnimDur    = 0.f;   // total duration of spawn anim (frames * frameDur)
    bool  spawnDone       = false;

    // ── Phase 1: flying / orbit ───────────────────────────────────────────────
    float        orbitAngle  = 0.f;
    sf::Vector2f orbitCenter { 200.f, 100.f };
    float        diveWindupTimer  = 0.f;
    sf::Vector2f diveTarget;
    float        diveLungeTimer   = 0.f;
    sf::Vector2f diveLungeDir;
    float        diveRecoverTimer = 0.f;
    float        diveCD           = 0.f;
    float        shootTimer       = 0.f;
    float        landingTimer     = 0.6f;
    sf::CircleShape shadow;

    // ── Phase 2: wave barrage ─────────────────────────────────────────────────
    int   wavesRemaining    = 0;
    float waveTimer         = 0.f;
    float vulnerableTimer   = 0.f;
    float recoverTimer      = 0.f;
    bool  waveFromLeft      = true;
    int   barrageCount      = 0;
    bool  showWarnLine      = false;   // kept for legacy; unused
    float warnLineY         = 0.f;
    float warnTimer         = 0.f;

    static constexpr float WARN_DURATION     = 0.32f;
    static constexpr float WAVE_INTERVAL     = 1.1f;
    static constexpr float WAVE_BULLET_SPD   = 130.f;
    static constexpr float RECOVER_DUR       = 1.0f;

    float diveSpeedMult    = 1.f;
    int   wavesPerBarrage  = 5;
    float vulnerableDur    = 5.f;
    float warnDuration     = 0.55f;
    float wavePaceScale    = 1.f;   // lower = faster waves each barrage cycle

    // ── Textures ──────────────────────────────────────────────────────────────
    sf::Texture texIdle;
    sf::Texture texWalk;
    sf::Texture texSpawn;
    bool        flyingUsingWalkSheet = false; // tracks which sheet is active in FLYING
    sf::Color   baseColor { 80, 60, 100 };

    std::vector<WaveZone> waveZones;

    // Boss HP bar
    sf::RectangleShape bossBarBack;
    sf::RectangleShape bossBarFront;
    sf::RectangleShape warnLine;

    // Helpers
    void setSheet(sf::Texture& tex, int totalFrames, int sheetCols, float dur);
    void shootFeathers(sf::Vector2f playerPos,
                       std::vector<std::unique_ptr<GameObject>>& spawnQueue);
};

#endif
