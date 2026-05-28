#include "Trash.h"
#include <iostream>

sf::Texture Trash::texture;

// Sprite: 32x32 px — uniform scale keeps aspect ratio intact
// 1.0 renders trash as 32x32 game-world px (small street prop)
static constexpr float SCALE = 1.0f;

void Trash::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/trash.png")) {
            std::cerr << "[Error] Failed to load assets/trash.png" << std::endl;
        } else {
            texture.setSmooth(false);
        }
    }
}

Trash::Trash(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Trash::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Trash::getBounds() const {
    float width  = 22.f * SCALE;
    float height = 28.f * SCALE;

    float offsetX = 5.f * SCALE;
    float offsetY = 4.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
