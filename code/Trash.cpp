#include "Trash.h"
#include <iostream>

// Inicjalizacja statycznej tekstury
sf::Texture Trash::texture;

void Trash::loadTexture() {
    // Ładujemy tylko jeśli tekstura jest pusta
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/trash.png")) {
            std::cerr << "[Error] Failed to load assets/trash.png" << std::endl;
        }
    }
}

Trash::Trash(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setPosition(position);
}

void Trash::draw(sf::RenderWindow& window) {
    window.draw(sprite);    
}

sf::FloatRect Trash::getBounds() const {
    return { position.x + 5.f, position.y + 4.f, 22.f, 28.f };
}