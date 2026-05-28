#ifndef PROP_H
#define PROP_H

#include <SFML/Graphics.hpp>

class Prop {
public:
    virtual ~Prop() = default;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual sf::FloatRect getBounds() const = 0;
};

#endif