/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Vending.h"
#include <iostream>

sf::Texture Vending::texture;

// Sprite: 60x70 px — uniform scale keeps aspect ratio intact
// 0.75 renders vending machine as 45x52 game-world px (tall prominent prop)
static constexpr float SCALE = 0.75f;

// Loads the shared vending machine texture file if not already loaded.
void Vending::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/vending.png")) {
            std::cerr << "[Error] Failed to load assets/vending.png" << std::endl;
        } else {
            texture.setSmooth(false); // Disable smoothing to preserve pixel art aesthetic
        }
    }
}

// Constructor: Configures sprite with shared texture, position, and uniform scale.
Vending::Vending(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

// Renders the vending machine sprite.
void Vending::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

// Computes the collision bounds rectangle, adjusted for scaling and minor offsets.
sf::FloatRect Vending::getBounds() const {
    float width  = 52.f * SCALE;
    float height = 67.f * SCALE;

    float offsetX = 3.f * SCALE;
    float offsetY = 1.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
