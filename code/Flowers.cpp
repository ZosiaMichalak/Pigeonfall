#include "Flowers.h"
#include <iostream>

sf::Texture Flowers::texture;

// Sprite: 32x32 px — uniform scale keeps aspect ratio intact
// 1.0 renders flowers as 32x32 game-world px (small ground decoration)
static constexpr float SCALE = 1.0f;

void Flowers::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/flowers.png"))
            std::cerr << "[Error] Failed to load assets/flowers.png\n";
        else
            texture.setSmooth(false);
    }
}

Flowers::Flowers(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Flowers::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Flowers::getBounds() const {
    float width  = 24.f * SCALE;
    float height = 16.f * SCALE;

    float offsetX = 4.f * SCALE;
    float offsetY = 14.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
