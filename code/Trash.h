/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef TRASH_H
#define TRASH_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

// Represents a street trash can obstacle prop in rooms.
class Trash : public Prop {
public:
    // Constructor: sets the world position of the trash can
    Trash(sf::Vector2f pos);
    
    // Draws the trash can sprite to the window screen
    void draw(sf::RenderWindow& window) override;

    // Returns the collision bounding box of the trash can
    sf::FloatRect getBounds() const override;

    // Static loader that preloads the trash can texture once for all instances
    static void loadTexture();

private:
    static sf::Texture texture;   // Shared texture resource
    sf::Sprite sprite;            // Sprite object for rendering
    sf::Vector2f position;        // 2D position in world space
};

#endif