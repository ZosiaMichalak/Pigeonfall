#ifndef DASH_ENEMY_H
#define DASH_ENEMY_H

#include "Enemy.h"

// AI states representing the behavioral pattern of the DashEnemy.
enum class DashEnemyState { STALK, WIND_UP, DASH, RECOVER };

/*
    Fast-moving combat enemy that circles (stalks) the player.
    Periodically charges/dashes straight towards the player at high velocity.
    During wind-up, it flashes to telegraph the charge.
    Inherits from the base Enemy class.
*/
class DashEnemy : public Enemy {
private:
    DashEnemyState dashState;         // Current AI behavior state

    float        dashSpeed;           // Velocity multiplier during the charge/dash
    sf::Vector2f dashDirection;       // Vector direction of the active dash charge
    float        dashTimer;           // Accumulated timer for the active dash phase
    float        dashDuration;        // Total duration of a dash movement

    float windUpTimer;                // Timer tracking wind-up winddown
    float windUpDuration;             // Total wind-up duration before launching the dash

    float stalkTimer;                 // Timer tracking how long the enemy circles the player
    float stalkDuration;              // Total duration of the stalking phase
    float orbitAngle;                 // Current orbital angle relative to the player
    float orbitRadius;                // Distance to maintain from the player while stalking
    int   orbitSign;                  // Direction multiplier for orbital movement (1 = CW, -1 = CCW)

    float recoverTimer;               // Timer tracking recovery stun/exhaustion duration after a dash
    float recoverDuration;            // Total duration of the recover state

    float dashCooldownTimer;          // Timer tracking internal cooldown before another dash can begin
    float dashCooldownDuration;       // Total cooldown duration between dash charges

    sf::Color baseColor;              // Visual color filter applied to the sprites (based on tier)

    // Per-state textures loaded once at instantiation
    sf::Texture texIdle;              // Animation sheet for stationary/recover states
    sf::Texture texWalk;              // Animation sheet for standard orbit/stalking movement
    sf::Texture texLoading;           // Animation sheet for wind-up phase
    sf::Texture texDash;              // Animation sheet for active high-speed charge

    DashEnemyState prevSheetState  = DashEnemyState::RECOVER; // Cache to force spritesheet swap detection
    bool           dashJustStarted = false;                  // Flag indicating a dash was initiated on the current frame

    // Swaps animation sheet to match the current AI state and updates layout variables
    void setSheet(DashEnemyState s);

public:
    // Constructor: Configures stats, difficulty multipliers, textures, and initial state based on tier.
    explicit DashEnemy(float x, float y, int tier = 0);

    // AI logic routine: Updates the state machine (movement patterns, timers, dash checks).
    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) override;

    // Checks if the enemy is currently in the active high-speed dash state
    bool isDashingNow()    const { return dashState == DashEnemyState::DASH; }
    
    // Checks if the dash just started on this frame (used for playing audio effects once)
    bool justStartedDash() const { return dashJustStarted; }
};

#endif

