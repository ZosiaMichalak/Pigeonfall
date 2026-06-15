/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Trash.h"
#include <iostream>

sf::Texture Trash::texture;

// Sprite: 32x32 px — uniform scale keeps aspect ratio intact
// 1.0 renders trash as 32x32 game-world px (small street prop)
static constexpr float SCALE = 1.0f;

// Loads the shared trash can texture file if not already loaded.
void Trash::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/trash.png")) {
            std::cerr << "[Error] Failed to load assets/trash.png" << std::endl;
        } else {
            texture.setSmooth(false); // Disable smoothing to preserve pixel art style
        }
    }
}

// Constructor: Configures sprite with shared texture, position, and scale.
Trash::Trash(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

// Renders the trash can sprite.
void Trash::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

// Computes the collision bounds rectangle, adjusted for scaling and minor offsets.
sf::FloatRect Trash::getBounds() const {
    float width  = 22.f * SCALE;
    float height = 28.f * SCALE;

    float offsetX = 5.f * SCALE;
    float offsetY = 4.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
