#ifndef PIGEON_KING_H
#define PIGEON_KING_H

#include "Enemy.h"
#include <SFML/Graphics.hpp>

// Action phases for the boss state machine.
enum class PigeonPhase {
    SPAWNING,     // Initial one-shot entrance animation before moving
    FLYING,       // Phase 1: Orbits high in the air, dive-bombs, shoots feathers (vulnerable)
    LANDING,      // Transition state landing on the ground
    WAVE_BARRAGE, // Phase 2: Fires horizontal/vertical shockwaves
    VULNERABLE,   // Phase 2: Staggered/stunned on the ground, vulnerable to all item damage
    RECOVER       // Delay cycle recovery before taking off/initiating waves again
};

// Flight-specific states during the FLYING phase.
enum class FlyingState {
    CIRCLE,              // Orbiting coordinates above the player
    DIVEBOMB_WINDUP,     // Stationary hover telegraphing a dive charge
    DIVEBOMB_LUNGE,      // High-velocity lunge directly towards locked-in player coordinates
    DIVEBOMB_RECOVER     // Recovery shake-off on the ground after lunging
};

// Boss difficulty tiers determining stat settings and speed parameters.
enum class PigeonBossTier {
    WEAK,   // Encountered in Room 25 (mid-boss)
    STRONG  // Encountered in Room 50 (final boss of the run)
};

// ── Wave hazard zone ─────────────────────────────────────────────────────────
// Represents a danger zone sweep overlay mapping damage regions to the screen.
struct WaveZone {
    sf::FloatRect rect;         // Covered bounding box area of the hazard
    float warnTimer  = 0.f;     // Remaining warning indicator duration (blinks orange)
    float activeTimer = 0.f;    // Remaining active damage duration (burns orange/red)
    float dmgTimer   = 0.f;     // Time since last damage tick inside this zone
    bool  active     = false;   // True if currently active and inflicting damage ticks
    bool  done       = false;   // True if the hazard has concluded and should be cleaned up
    bool  buffDmgApplied = false; // Flag to apply monster buff triggers once per wave zone

    sf::RectangleShape shape;   // Rendered shape visual block
};

/*
    The Pigeon King: The boss entity.
    Operates in two primary phases: flying (dive bombs, orbits, projectile bursts)
    and wave barrage (screenspace hazard zones, stagger windows).
    Inherits from the base Enemy class.
*/
class PigeonKing : public Enemy {
public:
    // Constructor: Configures stats, health bars, difficulty scaling based on tier.
    explicit PigeonKing(float x, float y, PigeonBossTier tier = PigeonBossTier::WEAK);

    // AI logic machine: Updates state phases, movements, attacks, animations, and shockwave zones.
    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) override;

    // Renders the boss, HP overlays, shadows, and warning lines.
    void draw(sf::RenderWindow& window) override;

    // Returns a custom bounding hitbox for Pigeon King overlaps.
    sf::FloatRect getBounds() const {
        return { position.x - 18.f, position.y - 14.f, 36.f, 28.f };
    }

    // Overrides base class boss identity check to true.
    bool isBoss()          const override { return true; }
    
    // Limits item damage to only apply when boss is in a vulnerable staggering phase.
    bool canTakeItemDamage() const override { return isVulnerable(); }
    
    // Checks if the boss is susceptible to incoming player attacks/damage.
    bool isVulnerable()    const {
        return phase == PigeonPhase::FLYING   ||
               phase == PigeonPhase::VULNERABLE;
    }
    
    // Defeat state helpers
    bool isDefeated()      const { return !isActive() && deathHandled; }
    void markDeathHandled()      { deathHandled = true; }

    // Returns a reference to the active screenspace hazard zones
    std::vector<WaveZone>& getWaveZones() { return waveZones; }

private:
    PigeonPhase  phase      = PigeonPhase::SPAWNING; // Current active state machine phase
    FlyingState  flyState   = FlyingState::CIRCLE;   // Sub-state while flying
    bool         deathHandled = false;               // True if defeat triggers (sounds/transitions) are complete

    // ── Spawn animation ───────────────────────────────────────────────────────
    float spawnAnimTimer  = 0.f;   // Time tracker for spawn entrance sequence
    float spawnAnimDur    = 0.f;   // Total duration of entrance sequence
    bool  spawnDone       = false; // True if spawn sequence finishes

    // ── Phase 1: flying / orbit ───────────────────────────────────────────────
    float        orbitAngle  = 0.f;          // Current orbital angle path
    sf::Vector2f orbitCenter { 200.f, 100.f };// Mid-screen anchor coordinate for orbital paths
    float        diveWindupTimer  = 0.f;     // Wind-up pause timer before charging
    sf::Vector2f diveTarget;                 // Target coordinate locked in at start of charge
    float        diveLungeTimer   = 0.f;     // Charge speed travel timer
    sf::Vector2f diveLungeDir;               // Direction vector of the dive charge
    float        diveRecoverTimer = 0.f;     // Recovery shake-off timer
    float        diveCD           = 0.f;     // Cooldown between dive bombs
    float        shootTimer       = 0.f;     // Cooldown timer between feather shots
    float        landingTimer     = 0.6f;    // Landing animation duration timer
    sf::CircleShape shadow;                  // Ground shadow projected under the boss while flying

    // ── Phase 2: wave barrage ─────────────────────────────────────────────────
    int   wavesRemaining    = 0;             // Shockwave zones left to fire in current barrage
    float waveTimer         = 0.f;           // Cooldown tracking between shockwaves
    float vulnerableTimer   = 0.f;           // Duration of ground stagger vulnerability
    float recoverTimer      = 0.f;           // Cooldown tracking for recovery stun phases
    bool  waveFromLeft      = true;          // Alternating side toggle for hazard placements
    int   barrageCount      = 0;             // Total barrage sequences executed
    bool  showWarnLine      = false;         // Legacy visual check
    float warnLineY         = 0.f;
    float warnTimer         = 0.f;

    static constexpr float WARN_DURATION     = 0.32f; // Warning blinking duration
    static constexpr float WAVE_INTERVAL     = 1.1f;  // Interval spacing between waves
    static constexpr float WAVE_BULLET_SPD   = 130.f; // Speed of bullets fired during waves
    static constexpr float RECOVER_DUR       = 1.0f;  // Recover duration

    float diveSpeedMult    = 1.f; // Speed factor for dive bombs
    int   wavesPerBarrage  = 5;   // Total waves generated per Phase 2 cycle
    float vulnerableDur    = 5.f; // Stagger stun duration
    float warnDuration     = 0.55f;// Zone warn blinking duration
    float wavePaceScale    = 1.f;  // Scales interval pacing for faster shockwaves

    // ── Textures ──────────────────────────────────────────────────────────────
    sf::Texture texIdle;                     // Idle state spritesheet
    sf::Texture texWalk;                     // Walk/Flight state spritesheet
    sf::Texture texSpawn;                    // Entrance/spawn state spritesheet
    bool        flyingUsingWalkSheet = false;// Toggle tracking active sheet during flight
    sf::Color   baseColor { 80, 60, 100 };   // Purple base color tint

    std::vector<WaveZone> waveZones;         // List of active screenspace hazard zones

    // Boss HP bar rendering overlay components
    sf::RectangleShape bossBarBack;          // Gray background container line
    sf::RectangleShape bossBarFront;         // Red health-scaled status line
    sf::RectangleShape warnLine;             // Blinking horizontal line for telegraphing

    // Helper: binds textures, defines grid dimensions, and duration constants
    void setSheet(sf::Texture& tex, int totalFrames, int sheetCols, float dur);
    
    // Attack Helper: fires feather projectiles outwards
    void shootFeathers(sf::Vector2f playerPos,
                       std::vector<std::unique_ptr<GameObject>>& spawnQueue);
};

#endif

