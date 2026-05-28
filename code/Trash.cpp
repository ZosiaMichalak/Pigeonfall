#include "Trash.h"
#include <iostream>

// Inicjalizacja statycznej tekstury
sf::Texture Trash::texture;

// Współczynnik skali – dostosuj go według potrzeb
static constexpr float SCALE = 0.6f;

void Trash::loadTexture() {
    // Ładujemy tylko jeśli tekstura jest pusta
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
    // Skalujemy grafikę
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Trash::draw(sf::RenderWindow& window) {
    window.draw(sprite);    
}

sf::FloatRect Trash::getBounds() const {
    // Oryginalne wymiary: 22.f szerokości, 28.f wysokości, offset 5.f/4.f
    // Mnożymy przez SCALE, aby hitbox zawsze pasował do przeskalowanej grafiki
    float width  = 22.f * SCALE;
    float height = 28.f * SCALE;
    
    float offsetX = 5.f * SCALE;
    float offsetY = 4.f * SCALE;
    
    return { position.x + offsetX, position.y + offsetY, width, height };
}