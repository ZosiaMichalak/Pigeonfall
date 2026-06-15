/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef CAR_H
#define CAR_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

// Types of cars available to spawn.
enum class CarType { CAR1, CAR2, CAR3 };

// Represents a parked car obstacle prop in room layouts.
class Car : public Prop {
public:
    // Constructor: sets coordinates and type variant of the car
    explicit Car(sf::Vector2f pos, CarType type = CarType::CAR1);

    // Draws the car sprite to the screen
    void draw(sf::RenderWindow& window) override;

    // Returns the collision bounding box of the car (tighter than full sprite bounds)
    sf::FloatRect getBounds() const override;

    // Static loader that preloads all car variant textures
    static void loadTextures();

private:
    static sf::Texture textureCar1; // Shared texture resources for variants
    static sf::Texture textureCar2;
    static sf::Texture textureCar3;

    sf::Sprite   sprite;            // Sprite used for rendering
    sf::Vector2f position;          // 2D position in world space
    CarType      carType;           // Mapped car type variant
};

#endif
