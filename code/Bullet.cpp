#include "Bullet.h"
#include <cmath>


static constexpr float ROOM_LEFT   =   0.f;
static constexpr float ROOM_RIGHT  = 400.f;
static constexpr float ROOM_TOP    =   0.f;
static constexpr float ROOM_BOTTOM = 225.f;

Bullet::Bullet(float x, float y, sf::Vector2f direction, float speed, bool fromEnemy)
    : GameObject(x, y), fromEnemy(fromEnemy)
{
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f) direction /= len;
    velocity = direction * speed;

    shape.setRadius(3.f);
    shape.setOrigin(3.f, 3.f);
    shape.setPosition(position);
    shape.setFillColor(sf::Color(255, 160, 30)); 
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(255, 80, 0));
}

void Bullet::update(float dt, sf::RenderWindow&) {
    position += velocity * dt;
    shape.setPosition(position);

    if (position.x < ROOM_LEFT  || position.x > ROOM_RIGHT ||
        position.y < ROOM_TOP   || position.y > ROOM_BOTTOM)
        destroy();
}

void Bullet::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    window.draw(shape);
}

void Bullet::deflect(sf::Vector2f direction, float speed) {
    fromEnemy = false; 
    
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f) direction /= len;
    velocity = direction * speed;

    shape.setFillColor(sf::Color(0, 255, 100));
    shape.setOutlineColor(sf::Color(0, 180, 50));
}