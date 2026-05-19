#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"

// Enemy class inheriting from the abstract GameObject base class
class Enemy : public GameObject {
protected:
    sf::RectangleShape shape;       // Visual rectangle shape of the enemy
    
    // Enemy UI elements
    sf::RectangleShape hpBarBack;   // Background rectangle of the health bar (gray)
    sf::RectangleShape hpBarFront;  // Foreground rectangle representing current health (green)
    
    // Enemy stats and state variables
    int hp;
    int maxHp;
    bool isHit;       // Flag tracking if the enemy is currently in a hit-stun state
    float hitTimer;   // Timer tracking the duration of the hit flash effect

public:
    // Constructor to set starting coordinates
    Enemy(float x, float y);
    
    // Virtual destructor ensuring safe memory cleanup via base pointers
    virtual ~Enemy() = default;

    // Overridden base class methods for game loop integration
    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;
    
    // Core combat methods
    void takeDamage(int damage);
    
    // Getter to check if the enemy is currently invulnerable/flashing after being hit
    bool getIsHit() const { return isHit; }

    // Getters for collision boundaries and respawn logic
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
    void respawn();
};

#endif