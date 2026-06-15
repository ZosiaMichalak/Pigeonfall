/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Hydrant.h"
#include <iostream>

sf::Texture Hydrant::texture;

// Sprite: 31x32 px — uniform scale keeps aspect ratio intact
// 0.9 renders hydrant as ~28x29 game-world px (smaller than bench/trash but visible)
static constexpr float SCALE = 0.9f;

// Loads the shared hydrant texture file if not already loaded.
void Hydrant::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/Hydrant.png"))
            std::cerr << "[Error] Failed to load assets/Hydrant.png\n";
        else
            texture.setSmooth(false); // Disable smoothing to preserve pixel art style
    }
}

// Constructor: Configures sprite with shared texture, position, and scale.
Hydrant::Hydrant(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

// Renders the hydrant sprite.
void Hydrant::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

// Computes the collision bounds rectangle, adjusted for scaling and minor offsets.
sf::FloatRect Hydrant::getBounds() const {
    float width  = 15.f * SCALE;
    float height = 20.f * SCALE;

    float offsetX = 8.f * SCALE;
    float offsetY = 10.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
