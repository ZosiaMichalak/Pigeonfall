/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Car.h"
#include <iostream>

sf::Texture Car::textureCar1;
sf::Texture Car::textureCar2;
sf::Texture Car::textureCar3;

// Sprite: 40x32 px — uniform scale keeps aspect ratio intact
// 2.0 renders car as 80x64 game-world px (dominant street prop)
static constexpr float SCALE = 2.0f;

// Loads all shared car variant texture assets if they aren't loaded.
void Car::loadTextures() {
    auto load = [](sf::Texture& tex, const char* path) {
        if (tex.getSize().x == 0) {
            if (!tex.loadFromFile(path))
                std::cerr << "[Error] Failed to load " << path << "\n";
            else
                tex.setSmooth(false); // Disable smoothing to preserve pixel art style
        }
    };
    load(textureCar1, "assets/car1.png");
    load(textureCar2, "assets/car2.png");
    load(textureCar3, "assets/car3.png");
}

// Constructor: Configures sprite based on variant, sets scale, and positions it.
Car::Car(sf::Vector2f pos, CarType type) : position(pos), carType(type) {
    switch (carType) {
        case CarType::CAR1: sprite.setTexture(textureCar1); break;
        case CarType::CAR2: sprite.setTexture(textureCar2); break;
        case CarType::CAR3: sprite.setTexture(textureCar3); break;
    }
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

// Renders the car sprite.
void Car::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

// Computes the collision bounds rectangle (designed to be tighter than the sprite boundary).
sf::FloatRect Car::getBounds() const {
    // Hitbox is tighter than the full sprite (excludes bonnet overhang etc. to allow closer passing)
    float width  = 36.f * SCALE;
    float height = 18.f * SCALE;

    float offsetX = 2.f * SCALE;
    float offsetY = 8.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
