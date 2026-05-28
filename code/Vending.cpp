#include "Vending.h"
#include <iostream>

sf::Texture Vending::texture;

// Sprite: 60x70 px — uniform scale keeps aspect ratio intact
// 0.75 renders vending machine as 45x52 game-world px (tall prominent prop)
static constexpr float SCALE = 0.75f;

void Vending::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/vending.png")) {
            std::cerr << "[Error] Failed to load assets/vending.png" << std::endl;
        } else {
            texture.setSmooth(false);
        }
    }
}

Vending::Vending(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Vending::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Vending::getBounds() const {
    float width  = 52.f * SCALE;
    float height = 67.f * SCALE;

    float offsetX = 3.f * SCALE;
    float offsetY = 1.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
