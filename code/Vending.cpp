#include "Vending.h"

sf::Texture Vending::texture;

Vending::Vending(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setPosition(position);
}

void Vending::loadTexture() {
    texture.loadFromFile("assets/vending.png");
}

void Vending::draw(sf::RenderWindow& window) { window.draw(sprite); }

sf::FloatRect Vending::getBounds() const {
    return { position.x, position.y, 52.f, 67.f };
}