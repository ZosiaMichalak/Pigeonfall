#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"
#include <vector>
#include <memory>

enum class EnemyState { CHASE, STRAFE, RETREAT };

class Enemy : public GameObject {
protected:
    sf::RectangleShape shape;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    int hp;
    int maxHp;
    bool isHit;
    float hitTimer;

    EnemyState state;
    float moveSpeed;
    float strafeSpeed;
    float shootRange;
    float shootCooldown;
    float shootTimer;
    int strafeSign;

public:
    Enemy(float x, float y);
    virtual ~Enemy() = default;

    void update(float dt, sf::RenderWindow& window) override;
    void updateAI(float dt, sf::Vector2f playerPos,
                  std::vector<std::unique_ptr<GameObject>>& spawnQueue);

    void draw(sf::RenderWindow& window) override;
    void takeDamage(int damage);

    bool getIsHit() const { return isHit; }
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
};

#endif