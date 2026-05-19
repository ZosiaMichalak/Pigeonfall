#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <SFML/Graphics.hpp>

class GameObject {
protected:
    sf::Vector2f position; 
    bool active;           

public:
    GameObject(float x, float y) : position(x, y), active(true) {} 
    virtual ~GameObject() = default; 

    virtual void update(float dt, sf::RenderWindow& window) = 0; 
    virtual void draw(sf::RenderWindow& window) = 0; 

    bool isActive() const { return active; } 
    void destroy() { active = false; } 
};

#endif