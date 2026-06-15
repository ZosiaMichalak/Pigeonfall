/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Bench.h"
#include <iostream>

sf::Texture Bench::texture;

// Sprite: 32x32 px — uniform scale keeps aspect ratio intact
// 1.0 renders bench as 32x32 game-world px
static constexpr float SCALE = 1.0f;

// Loads the shared bench texture file if not already loaded.
void Bench::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/bench.png"))
            std::cerr << "[Error] Failed to load assets/bench.png\n";
        else
            texture.setSmooth(false); // Disable smoothing to preserve pixel art style
    }
}

// Constructor: Configures sprite with shared texture, position, and scale.
Bench::Bench(sf::Vector2f pos) : position(pos) {
    sprite.setTexture(texture);
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

// Renders the bench sprite.
void Bench::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

// Computes the collision bounds rectangle, adjusted for scaling and minor offsets.
sf::FloatRect Bench::getBounds() const {
    float width  = 28.f * SCALE;
    float height = 18.f * SCALE;

    float offsetX = 2.f * SCALE;
    float offsetY = 10.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
