#ifndef ANNOYING_DOG_H
#define ANNOYING_DOG_H

#include "GameObject.h"
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

enum class DogState { WALK, WINDUP, EXPLODING };

// Companion/Item entity that walks towards the nearest enemy, winds up, and explodes dealing massive damage.
class AnnoyingDog : public GameObject {
public:
    AnnoyingDog(float x, float y);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    bool tryExplode(std::vector<std::unique_ptr<GameObject>>& objects,
                    std::vector<std::unique_ptr<GameObject>>& spawnQueue);

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
    void nudgePosition(sf::Vector2f delta) { position += delta; shape.setPosition(position); }

private:
    // Fallback shape (used if textures fail to load)
    sf::CircleShape shape;
    sf::Vector2f    velocity;

    // Sprite animation
    sf::Texture  texWalk;
    sf::Texture  texBoom;
    sf::Sprite   sprite;
    bool         hasSprite = false;
    bool         facingLeft = false;

    int   animFrame    = 0;
    float animTimer    = 0.f;

    static constexpr int   FRAME_W      = 32;
    static constexpr int   FRAME_H      = 32;
    static constexpr int   WALK_FRAMES  = 5;
    static constexpr int   BOOM_FRAMES  = 7;
    static constexpr float WALK_DUR     = 0.10f;   // per frame
    static constexpr float BOOM_DUR     = 1.f / 7.f; // spread 7 frames over ~1 s

    // State machine
    DogState state       = DogState::WALK;
    float    windupTimer = 0.f;
    static constexpr float WINDUP_DURATION = 1.0f;

    // Explosion flash overlay
    sf::CircleShape explosionShape;

    static constexpr float SPEED        = 150.f;
    static constexpr float HIT_DIST     = 18.f;
    static constexpr float BLAST_RADIUS = 65.f;

    void setFrame(sf::Texture& tex, int col, int row = 0);
    void tickWalkAnim(float dt);
    void tickBoomAnim(float dt);
};

#endif
