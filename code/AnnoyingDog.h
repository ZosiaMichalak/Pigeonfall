#ifndef ANNOYING_DOG_H
#define ANNOYING_DOG_H

#include "GameObject.h"
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// Spawned by the "Annoying Dog" item. Ignores prop collisions, rushes the
// nearest enemy, explodes on contact. The explosion instantly kills the direct
// target and halves HP of all enemies in the blast radius.
class AnnoyingDog : public GameObject {
public:
    AnnoyingDog(float x, float y);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    // Call each frame from Game::update. Returns true if an explosion occurred
    // (so Game can apply area damage).
    bool tryExplode(std::vector<std::unique_ptr<GameObject>>& objects,
                    std::vector<std::unique_ptr<GameObject>>& spawnQueue);

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }

private:
    sf::CircleShape shape;
    sf::Vector2f    velocity;

    // Explosion flash
    bool  exploding;
    float explodeTimer;
    sf::CircleShape explosionShape;

    static constexpr float SPEED       = 150.f;
    static constexpr float HIT_DIST    = 10.f;
    static constexpr float BLAST_RADIUS = 40.f;
};

#endif
