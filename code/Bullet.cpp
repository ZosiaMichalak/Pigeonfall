/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Bullet.h"
#include <cmath>

static constexpr float ROOM_LEFT   =   0.f;
static constexpr float ROOM_RIGHT  = 400.f;
static constexpr float ROOM_TOP    =   0.f;
static constexpr float ROOM_BOTTOM = 225.f;

// Constructor: Initializes velocity vector, fallback shape colors, and attempts to load sprite texture.
Bullet::Bullet(float x, float y, sf::Vector2f direction, float speed, bool fromEnemy)
    : GameObject(x, y), fromEnemy(fromEnemy), hasSprite(false)
{
    // Normalize direction vector
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f) direction /= len;
    velocity = direction * speed;

    // Set up fallback rendering shape
    shape.setRadius(3.f);
    shape.setOrigin(3.f, 3.f);
    shape.setPosition(position);
    shape.setFillColor(sf::Color(255, 160, 30)); 
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(255, 80, 0));

    // Try loading sprite asset
    if (tex.loadFromFile("assets/Bullet.png")) {
        tex.setSmooth(false);
        hasSprite = true;
        sprite.setTexture(tex);
        sprite.setOrigin(2.f, 2.f);
        sprite.setScale(2.f, 2.f);
        sprite.setPosition(position);
    }
}

// Moves bullet position. Destroys the bullet if it flies off-screen.
void Bullet::update(float dt, sf::RenderWindow&) {
    position += velocity * dt;
    shape.setPosition(position);
    if (hasSprite) sprite.setPosition(position);

    // Off-screen boundary checks
    if (position.x < ROOM_LEFT  || position.x > ROOM_RIGHT ||
        position.y < ROOM_TOP   || position.y > ROOM_BOTTOM)
        destroy();
}

// Renders either the pixel-art sprite or the fallback circle vector.
void Bullet::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (hasSprite)
        window.draw(sprite);
    else
        window.draw(shape);
}

// Alters bullet flight path and ownership (triggered upon successful player deflect).
void Bullet::deflect(sf::Vector2f direction, float speed) {
    fromEnemy = false; // Player is now the owner of the projectile
    
    // Normalize and scale new velocity
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f) direction /= len;
    velocity = direction * speed;

    // Change fallback shapes color to green to indicate friendly projectile status
    shape.setFillColor(sf::Color(0, 255, 100));
    shape.setOutlineColor(sf::Color(0, 180, 50));
}
