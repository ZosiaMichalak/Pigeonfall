#ifndef BULLET_ENEMY_H
#define BULLET_ENEMY_H

#include "Enemy.h"

enum class BulletEnemyState { CHASE, STRAFE, RETREAT };

class BulletEnemy : public Enemy {
private:
    BulletEnemyState state;

    float strafeSpeed;
    float shootRange;
    float shootCooldown;
    float shootTimer;
    int   strafeSign;

public:

    explicit BulletEnemy(float x, float y, int tier = 0);

    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) override;
};

#endif
