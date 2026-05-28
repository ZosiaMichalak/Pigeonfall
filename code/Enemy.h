#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"
#include <vector>
#include <memory>

class Enemy : public GameObject {
protected:
    sf::RectangleShape shape;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    int   hp;
    int   maxHp;
    bool  isHit;
    float hitTimer;

    float moveSpeed;

public:
    Enemy(float x, float y);
    virtual ~Enemy() = default;

    void update(float dt, sf::RenderWindow& window) override {}

    virtual void updateAI(float dt, sf::Vector2f playerPos,
                          std::vector<std::unique_ptr<GameObject>>& spawnQueue) = 0;

    void draw(sf::RenderWindow& window) override;
    void takeDamage(int damage);

    bool          getIsHit()  const { return isHit; }
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
    void          nudgePosition(sf::Vector2f delta) { position += delta; shape.setPosition(position); }
};

#endif