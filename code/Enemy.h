#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"
#include <vector>
#include <memory>

/*
    Base class representing a generic hostile enemy in the game.
    Inherits from GameObject. Provides shared functionality for health tracking,
    damage reception, animation updates, rendering, and simple physics.
    Specific enemy types must inherit from this class and implement updateAI().
*/
class Enemy : public GameObject {
protected:
    sf::RectangleShape shape;       // Fallback shape used when no sprite texture is loaded
    sf::RectangleShape hpBarBack;   // Background bar for health display (usually dark gray/black)
    sf::RectangleShape hpBarFront;  // Foreground bar for health display (usually red, scales with HP)

    sf::Sprite  sprite;             // Sprite used for rendering enemy animation frames
    bool        hasSprite  = false; // True if a valid sprite texture has been loaded
    bool        facingLeft = false; // True if facing left, used to scale/flip the sprite horizontally

    // Shared animation helpers
    int   animCol      = 0;         // Current active column index in the sprite sheet
    int   animMaxCols  = 1;         // Maximum number of columns (total frames) in current loop
    int   animSheetCols = 1;        // Total columns per row in the actual sprite sheet layout
    int   frameW       = 51;        // Width of a single animation frame in pixels
    int   frameH       = 32;        // Height of a single animation frame in pixels
    float animTimer    = 0.f;       // Accumulated time since last animation frame switch
    float frameDur     = 0.18f;     // Duration in seconds that each frame is displayed

    // Advances the animation frame based on elapsed time and updates sprite texture rect.
    void tickAnim(float dt);

    int   hp;                       // Current health points
    int   maxHp;                    // Maximum health points
    bool  isHit;                    // True if recently damaged and currently in hit recovery
    float hitTimer;                 // Remaining duration of the hit recovery state in seconds
    bool  isSpawning = false;       // True if currently spawning (immune to damage)

    float moveSpeed;                // Base movement speed of the enemy

public:
    // Constructor: Configures starting coordinates, HP values, fallback shapes, and HP bars.
    Enemy(float x, float y);

    // Virtual destructor.
    virtual ~Enemy() = default;

    // Overridden update method from GameObject (AI logic is handled in updateAI instead).
    void update(float dt, sf::RenderWindow& window) override {}

    // Pure virtual method for custom enemy AI behavior (movement, state machines, player tracking).
    virtual void updateAI(float dt, sf::Vector2f playerPos,
                          std::vector<std::unique_ptr<GameObject>>& spawnQueue) = 0;

    // Draws the enemy sprite (or fallback shape) and health bars.
    void draw(sf::RenderWindow& window) override;

    // Inflicts damage on the enemy unless immune (spawning or in hit recovery).
    void takeDamage(int damage);

    // Checks if the enemy is currently in hit recovery.
    bool          getIsHit()     const { return isHit; }

    // Checks if the enemy is currently spawning.
    bool          getIsSpawning()const { return isSpawning; }

    // Sets the spawning state of the enemy.
    void          setSpawning(bool v)  { isSpawning = v; }

    // Gets current health points.
    int           getHp()     const { return hp; }

    // Gets maximum health points.
    int           getMaxHp()  const { return maxHp; }

    // Returns true if this enemy is a boss (e.g. PigeonKing).
    virtual bool  isBoss()    const { return false; }

    // Returns true if the enemy can be damaged by passive items/companions.
    virtual bool  canTakeItemDamage() const { return true; }

    // Returns a tight 10x10 hitbox centered at the enemy's position for player collisions.
    sf::FloatRect getBounds() const {
        constexpr float W = 10.f, H = 10.f;
        return { position.x - W / 2.f, position.y - H / 2.f, W, H };
    }

    // Gets the global bounds of the fallback shape.
    sf::FloatRect getShapeBounds() const { return shape.getGlobalBounds(); }

    // Adjusts the enemy position by a delta offset (separating overlapping enemies).
    void          nudgePosition(sf::Vector2f delta) { position += delta; shape.setPosition(position); }
};

#endif


