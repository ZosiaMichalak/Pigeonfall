/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef PROP_H
#define PROP_H

#include <SFML/Graphics.hpp>

// Base class for non-animated interactive props and obstacles in the room layout.
class Prop {
public:
    virtual ~Prop() = default;

    // Draw the prop to the render window
    virtual void draw(sf::RenderWindow& window) = 0;

    // Get bounding box of the prop for collision detection
    virtual sf::FloatRect getBounds() const = 0;
};

#endif