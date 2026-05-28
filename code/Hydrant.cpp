#include "Hydrant.h"
#include <iostream>

sf::Texture Hydrant::texture;

// Współczynnik skali – dostosuj go według potrzeb
static constexpr float SCALE = 1.3f;

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
    // Skalowanie grafiki
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Hydrant::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Hydrant::getBounds() const {
    // Oryginał: 15.f szerokości, 20.f wysokości, offset 8.f/10.f
    float width  = 15.f * SCALE;
    float height = 20.f * SCALE;
    
    float offsetX = 8.f * SCALE;
    float offsetY = 10.f * SCALE;
    
    return { position.x + offsetX, position.y + offsetY, width, height };
}