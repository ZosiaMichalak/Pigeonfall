/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <SFML/Graphics.hpp>

// Base class for all interactive entities, players, enemies, and props in the game world.
class GameObject {
protected:
    sf::Vector2f position; // 2D position of the game object in world space
    bool active;           // Lifecycle flag: true = active in world, false = marked for cleanup

public:
    // Constructor: initializes object position and sets active state to true
    GameObject(float x, float y) : position(x, y), active(true) {} 
    virtual ~GameObject() = default; 

    // Update object state; called once per frame with time elapsed (dt)
    virtual void update(float dt, sf::RenderWindow& window) = 0; 
    
    // Draw the object on the window screen; called once per frame
    virtual void draw(sf::RenderWindow& window) = 0; 

    // Checks if the object is still active in the game loop
    bool isActive() const { return active; } 
    
    // Marks the object for destruction and removal from the active object list
    void destroy() { active = false; } 
};

#endif