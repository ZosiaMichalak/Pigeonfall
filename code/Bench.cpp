#include "Bench.h"
#include <iostream>

// Definicja statycznej tekstury
sf::Texture Bench::texture;

// Współczynnik skali - zmień tę wartość, aby powiększyć lub pomniejszyć ławkę
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
    // Skalujemy grafikę
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Bench::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Bench::getBounds() const {
    // Bazowe wymiary hitboxa (zanim zostały przeskalowane)
    // Oryginalnie: 28.f szerokości, 18.f wysokości, offset 2.f/10.f
    
    float width  = 28.f * SCALE;
    float height = 18.f * SCALE;
    
    float offsetX = 2.f * SCALE;
    float offsetY = 10.f * SCALE;
    
    return { position.x + offsetX, position.y + offsetY, width, height };
}