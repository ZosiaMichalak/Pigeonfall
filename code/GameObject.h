//Base Class

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <SFML/Graphics.hpp>

class GameObject {
protected:
    sf::Vector2f position; //Object position (x,y)
    bool active;           //Flag - if the object is active 

public:
    GameObject(float x, float y) : position(x, y), active(true) {} //Constructor
    virtual ~GameObject() = default; //Virtual Destructor - no memory leaks if object is removed

    virtual void update(float dt, sf::RenderWindow& window) = 0; //update method (virtual) - logic of an object
    virtual void draw(sf::RenderWindow& window) = 0; //draw method (virtual) - drawing sth 

    bool isActive() const { return active; } //getter
    void destroy() { active = false; } //safe destruction of an object (instead of delete)
};

#endif