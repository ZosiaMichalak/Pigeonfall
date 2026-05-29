#ifndef DASH_ENEMY_H
#define DASH_ENEMY_H

#include "Enemy.h"

enum class DashEnemyState { STALK, WIND_UP, DASH, RECOVER };

class DashEnemy : public Enemy {
private:
    DashEnemyState dashState;

    float        dashSpeed;
    sf::Vector2f dashDirection;
    float        dashTimer;
    float        dashDuration;

    float windUpTimer;
    float windUpDuration;

    float stalkTimer;
    float stalkDuration;
    float orbitAngle;
    float orbitRadius;
    int   orbitSign;

    float recoverTimer;
    float recoverDuration;

    float dashCooldownTimer;
    float dashCooldownDuration;

    sf::Color baseColor;

    // Per-state textures
    sf::Texture texIdle;
    sf::Texture texWalk;
    sf::Texture texLoading;   // wind-up
    sf::Texture texDash;

    DashEnemyState prevSheetState = DashEnemyState::RECOVER; // forces first switch

    void setSheet(DashEnemyState s);

public:
    explicit DashEnemy(float x, float y, int tier = 0);

    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) override;

    bool isDashingNow() const { return dashState == DashEnemyState::DASH; }
};

#endif
