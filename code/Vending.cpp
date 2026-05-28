#include "Vending.h"

// Inicjalizacja statycznej tekstury
sf::Texture Vending::texture;

// Współczynnik skali – dostosuj go, aby pasował do innych obiektów
static constexpr float SCALE = 0.5f;

Vending::Vending(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    // Skalujemy grafikę
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Vending::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/vending.png")) {
            // Możesz dodać obsługę błędu, np. std::cerr
        } else {
            texture.setSmooth(false);
        }
    }
}

void Vending::draw(sf::RenderWindow& window) { 
    window.draw(sprite);   
}

sf::FloatRect Vending::getBounds() const {
    // Oryginalne wymiary: 52.f szerokości, 67.f wysokości, offset 3.f/1.f
    // Mnożymy przez SCALE, aby hitbox pasował do przeskalowanego sprite'a
    float width  = 52.f * SCALE;
    float height = 67.f * SCALE;
    
    float offsetX = 3.f * SCALE;
    float offsetY = 1.f * SCALE;
    
    return { position.x + offsetX, position.y + offsetY, width, height };
}