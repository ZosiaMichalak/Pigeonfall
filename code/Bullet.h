/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef BULLET_H
#define BULLET_H

#include "GameObject.h"
#include <SFML/Graphics.hpp>

// Represents a projectile bullet in the game world, which can belong to enemies or be deflected by the player.
class Bullet : public GameObject {
private:
    sf::CircleShape shape;      // Fallback shape if texture is missing
    sf::Texture       tex;        // Texture resource for the bullet
    sf::Sprite        sprite;     // Sprite used for rendering
    bool              hasSprite = false;
    sf::Vector2f velocity;      // Velocity vector (direction * speed)
    bool fromEnemy;             // Tracks ownership (true = hurts player, false = hurts enemies)

public:
    // Constructor: sets initial coordinates, direction, speed, and ownership
    Bullet(float x, float y, sf::Vector2f direction, float speed, bool fromEnemy = true);

    // Updates bullet position and performs screen boundary checks
    void update(float dt, sf::RenderWindow& window) override;

    // Renders the bullet sprite or fallback shape
    void draw(sf::RenderWindow& window) override;

    // Retrieves current bullet bounding box for hit/deflection tests
    sf::FloatRect getBounds() const {
        if (hasSprite) return sprite.getGlobalBounds();
        return shape.getGlobalBounds();
    }
    
    // Checks if the bullet was shot by an enemy
    bool isFromEnemy() const { return fromEnemy; }

    // Reverses bullet ownership and redirects it towards a new path (used on sword deflects)
    void deflect(sf::Vector2f newDirection, float newSpeed);
};

#endif
