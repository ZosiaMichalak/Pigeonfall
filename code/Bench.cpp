#include "Bench.h"
#include <iostream>

sf::Texture Bench::texture;

// Sprite: 32x32 px — uniform scale keeps aspect ratio intact
// 1.0 renders bench as 32x32 game-world px
static constexpr float SCALE = 1.0f;

void Bench::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/bench.png"))
            std::cerr << "[Error] Failed to load assets/bench.png\n";
        else
            texture.setSmooth(false);
    }
}

Bench::Bench(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Bench::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Bench::getBounds() const {
    float width  = 28.f * SCALE;
    float height = 18.f * SCALE;

    float offsetX = 2.f * SCALE;
    float offsetY = 10.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
