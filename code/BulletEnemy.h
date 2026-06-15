#ifndef BULLET_ENEMY_H
#define BULLET_ENEMY_H

#include "Enemy.h"

// States representing the BulletEnemy's behavioral routine.
enum class BulletEnemyState { CHASE, STRAFE, RETREAT };

/*
    Ranged combat enemy that keeps distance from the player.
    Depending on the distance to the player, it will chase, strafe, or retreat.
    Fires bullets at the player at regular intervals with configurable spread.
    Inherits from the base Enemy class.
*/
class BulletEnemy : public Enemy {
private:
    BulletEnemyState state;       // Current behavioral AI state

    float strafeSpeed;            // Speed used when circling/strafing around the player
    float shootRange;             // Maximum range at which this enemy can fire
    float shootCooldown;          // Time between successive bullet shots
    float shootTimer;             // Timer tracking cooldown between shots
    int   strafeSign;             // Direction multiplier for strafing (1 = clockwise, -1 = counter-clockwise)

    // Difficulty-baked per-instance multipliers (set in constructor)
    float bulletSpeedMult_  = 1.f; // Speed multiplier applied to fired projectiles
    float bulletSpreadMult_ = 1.f; // Accuracy/spread angle multiplier

    sf::Texture texIdle;          // Idle animation texture spritesheet
    sf::Texture texWalk;          // Walking animation texture spritesheet

    sf::Color baseColor;          // Visual color filter applied to the sprites (based on tier)

    // Swaps animation spritesheets and configures columns/rows accordingly
    void setSheet(bool walking);

public:
    // Constructor: Configures stats, difficulty multipliers, textures, and starting state based on tier.
    explicit BulletEnemy(float x, float y, int tier = 0);

    // AI routine: Decides movement behavior (chase, strafe, retreat) and spawns bullets when ready.
    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) override;
};

#endif

