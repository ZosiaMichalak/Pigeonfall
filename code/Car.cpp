#include "Car.h"
#include <iostream>

sf::Texture Car::textureCar1;
sf::Texture Car::textureCar2;
sf::Texture Car::textureCar3;

// Sprite: 40x32 px — uniform scale keeps aspect ratio intact
// 2.0 renders car as 80x64 game-world px (dominant street prop)
static constexpr float SCALE = 2.0f;

void Car::loadTextures() {
    auto load = [](sf::Texture& tex, const char* path) {
        if (tex.getSize().x == 0) {
            if (!tex.loadFromFile(path))
                std::cerr << "[Error] Failed to load " << path << "\n";
            else
                tex.setSmooth(false);
        }
    };
    load(textureCar1, "assets/car1.png");
    load(textureCar2, "assets/car2.png");
    load(textureCar3, "assets/car3.png");
}

Car::Car(sf::Vector2f pos, CarType type) : position(pos), carType(type) {
    switch (carType) {
        case CarType::CAR1: sprite.setTexture(textureCar1); break;
        case CarType::CAR2: sprite.setTexture(textureCar2); break;
        case CarType::CAR3: sprite.setTexture(textureCar3); break;
    }
    sprite.setScale(SCALE, SCALE);
    sprite.setPosition(position);
}

void Car::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Car::getBounds() const {
    // Hitbox is tighter than the full sprite (excludes bonnet overhang etc.)
    float width  = 36.f * SCALE;
    float height = 18.f * SCALE;

    float offsetX = 2.f * SCALE;
    float offsetY = 8.f * SCALE;

    return { position.x + offsetX, position.y + offsetY, width, height };
}
