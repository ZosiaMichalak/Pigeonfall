#include "Enemy.h"
#include <ctime> // For rand() support

// Constructor: Initializes enemy stats, red rectangle shape, and HP bar colors/sizes
Enemy::Enemy(float x, float y) : GameObject(x, y) {
    maxHp = 3;
    hp = maxHp;
    isHit = false;
    hitTimer = 0.f;

    // Set main visual properties of the enemy
    shape.setSize(sf::Vector2f(40.f, 40.f));
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(position);

    // Health bar background setup (gray rectangle)
    hpBarBack.setSize(sf::Vector2f(40.f, 5.f));
    hpBarBack.setFillColor(sf::Color(50, 50, 50));

    // Health bar foreground setup (green rectangle)
    hpBarFront.setSize(sf::Vector2f(40.f, 5.f));
    hpBarFront.setFillColor(sf::Color::Green);
}

// Resets health status and teleports the enemy to a random location inside the window
void Enemy::respawn() {
    hp = maxHp;
    isHit = false;
    hitTimer = 0.f;

    // Restore full green health bar and default red color
    hpBarFront.setSize(sf::Vector2f(40.f, 5.f));
    shape.setFillColor(sf::Color::Red);

    // Randomize new coordinates using standard modulo limits
    position.x = static_cast<float>(rand() % 1100 + 50);
    position.y = static_cast<float>(rand() % 600 + 50);
    
    shape.setPosition(position);
}

// Inflicts damage, handles death-to-respawn shift, and scales the health bar width
void Enemy::takeDamage(int damage) {
    // Only apply damage if the enemy is not currently in its temporary invulnerability window
    if (!isHit) {
        hp -= damage;
        isHit = true;
        hitTimer = 0.2f; // Set duration for the invulnerability/hit-flash state

        if (hp <= 0) {
            // Respawn instantly instead of deleting the object from memory
            respawn();
        } else {
            // Scale the green health bar length proportionally to remaining HP
            float hpPercent = static_cast<float>(hp) / static_cast<float>(maxHp);
            hpBarFront.setSize(sf::Vector2f(40.f * hpPercent, 5.f));
        }
    }
}

// Updates hit timers, alternates flashing colors, and anchors the UI to the enemy's coordinates
void Enemy::update(float dt, sf::RenderWindow& window) {
    if (isHit) {
        hitTimer -= dt;
        if (hitTimer <= 0.f) {
            isHit = false;
            shape.setFillColor(sf::Color::Red); // Return to default color
        } else {
            shape.setFillColor(sf::Color::White); // Flash white to visually indicate damage taken
        }
    }

    // Keep the health bars hovering directly above the enemy shape
    hpBarBack.setPosition(position.x, position.y - 12.f);
    hpBarFront.setPosition(position.x, position.y - 12.f);
    
    shape.setPosition(position); // Apply current coordinates to the sprite shape
}

// Draws the main enemy body and its health bars if active
void Enemy::draw(sf::RenderWindow& window) {
    if (active) {
        window.draw(shape);
        window.draw(hpBarBack);
        window.draw(hpBarFront);
    }
}