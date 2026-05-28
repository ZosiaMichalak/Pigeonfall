#include "Hydrant.h"
#include <iostream>

sf::Texture Hydrant::texture;

// Sprite: 31x32 px — uniform scale keeps aspect ratio intact
// 0.9 renders hydrant as ~28x29 game-world px (smaller than bench/trash but visible)
static constexpr float SCALE = 0.9f;

void Hydrant::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/Hydrant.png"))
            std::cerr << "[Error] Failed to load assets/Hydrant.png\n";
        else
            texture.setSmooth(false);
    }
}

Hydrant::Hydrant(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Hydrant::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Hydrant::getBounds() const {
    float width  = 15.f * SCALE;
    float height = 20.f * SCALE;

    float offsetX = 8.f * SCALE;
    float offsetY = 10.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
