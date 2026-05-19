#ifndef BULLET_H
#define BULLET_H

#include "GameObject.h"

class Bullet : public GameObject {
private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool fromEnemy;  

public:
    Bullet(float x, float y, sf::Vector2f direction, float speed, bool fromEnemy = true);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
    bool isFromEnemy() const { return fromEnemy; }

    void deflect(sf::Vector2f newDirection, float newSpeed);
};

#endif