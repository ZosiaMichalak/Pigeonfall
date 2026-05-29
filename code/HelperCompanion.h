#ifndef HELPER_COMPANION_H
#define HELPER_COMPANION_H

#include "GameObject.h"
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// Spawned by the "Duo" item. Flies above props (no collision), homes on enemies,
// and one-hits them on contact. Disappears when no enemies remain.
class HelperCompanion : public GameObject {
public:
    HelperCompanion(float x, float y);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    // Returns true and destroys itself + target when it reaches an enemy.
    // Call this from Game::update after normal updates.
    bool tryHitEnemy(std::vector<std::unique_ptr<GameObject>>& objects);

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }

private:
    sf::CircleShape shape;
    sf::Vector2f    velocity;

    float bobTimer;
    float lifetime; // auto-despawn after 20 s if no enemies
};

#endif
