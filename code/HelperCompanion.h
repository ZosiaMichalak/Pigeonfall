#ifndef HELPER_COMPANION_H
#define HELPER_COMPANION_H

#include "GameObject.h"
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// States representing the companion's active modes.
enum class DuoState { HUNTING, EXIT };

/*
    A friendly companion summoned to help the player by chasing down
    and destroying nearby enemies (or dealing heavy damage to bosses).
    After its hunt duration expires, it leaves the stage.
    Inherits from GameObject.
*/
class HelperCompanion : public GameObject {
public:
    // Constructor: Configures companion's starting position, shapes, and loads textures.
    HelperCompanion(float x, float y);

    // Updates position, timers, state transitions, and boundaries.
    void update(float dt, sf::RenderWindow& window) override;

    // Renders the companion sprite or fallback shape.
    void draw(sf::RenderWindow& window) override;

    // Scans enemies, moves towards them, and inflicts damage when touching.
    void tryHitEnemy(float dt,
                     std::vector<std::unique_ptr<GameObject>>& objects,
                     std::vector<std::unique_ptr<GameObject>>& spawnQueue);

    // Returns the tight visual boundary bounding box of the companion.
    sf::FloatRect getBounds() const;

private:
    sf::CircleShape shape;        // Fallback circular shape (in case sprite texture fails to load)
    sf::Vector2f    velocity;     // Current movement direction and speed vector

    sf::Texture texHit;           // Spritesheet for attacking animation
    sf::Texture texWalk;          // Spritesheet for standard running animation
    sf::Sprite  sprite;           // Visual representation sprite using textures
    bool        hasSprite = false;// True if visual assets loaded successfully
    bool        facingLeft = false;// True if moving left (flips sprite scale)

    DuoState state      = DuoState::HUNTING; // Current companion AI state
    float    huntTimer  = 10.f;              // Remaining duration of the hunt phase
    float    exitTimer  = 2.f;               // Remaining duration of the exit/leave phase

    Enemy*   bossTarget     = nullptr;       // Active boss target pointer (if focusing a boss)
    float    bossFocusTimer = 0.f;           // Duration tracking lock-on focus on a boss

    int   animFrame     = 0;                 // Current animation frame index
    float animTimer     = 0.f;               // Time accumulator for animation frames
    int   hitFrames     = 5;                 // Total frames in the hit spritesheet
    int   walkFrames    = 3;                 // Total frames in the walk spritesheet
    int   hitSheetCols  = 2;                 // Column layout count for the hit sheet
    int   walkSheetCols = 2;                 // Column layout count for the walk sheet

    static constexpr int   FRAME_W        = 16;     // Frame width in pixels
    static constexpr int   FRAME_H        = 16;     // Frame height in pixels
    static constexpr float SPRITE_SCALE   = 2.f;    // Visual scaling multiplier
    static constexpr float HUNT_DURATION  = 10.f;   // Default summoning duration
    static constexpr float EXIT_DURATION  = 2.f;    // Leaving duration
    static constexpr float HIT_FRAME_DUR  = 0.08f;  // Delay per attack frame
    static constexpr float WALK_FRAME_DUR = 0.10f;  // Delay per movement frame
    static constexpr float SPEED          = 175.f;   // Base movement speed
    static constexpr float HIT_DIST       = 12.f;    // Distance threshold to trigger contact damage
    static constexpr float BOSS_FOCUS_DUR = 2.f;    // Delay lock-on to boss targets
    static constexpr int   BOSS_HIT_DMG   = 30;     // Flat damage dealt to boss enemies

    // Sets viewport rect texture mapping coordinate variables
    void setAnimFrame(sf::Texture& tex, int frame, int sheetCols);
    
    // Updates horizontal scale factor for left/right facing sprite mirroring
    void applyFacingScale();
    
    // Cycles attacking animation frames
    void tickHitAnim(float dt);
    
    // Cycles normal running animation frames
    void tickWalkAnim(float dt);
    
    // Returns total frames count in spritesheet textures based on size
    int  countSheetFrames(const sf::Texture& tex) const;
    
    // Calculates damage values depending on target tier/boss status
    int  damageFor(Enemy* e) const;
    
    // Helper function to spawn reward coins when an enemy is defeated by the companion
    void spawnCoinsForKill(Enemy* e, sf::Vector2f at,
                           std::vector<std::unique_ptr<GameObject>>& spawnQueue);
                           
    // Targeting: locates the best enemy target to pursue (prioritizing non-bosses)
    Enemy* findTarget(std::vector<std::unique_ptr<GameObject>>& objects, bool skipBoss) const;
};

#endif

