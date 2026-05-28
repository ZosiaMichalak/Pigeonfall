#ifndef HYDRANT_H
#define HYDRANT_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

class Hydrant : public Prop {
public:
    explicit Hydrant(sf::Vector2f pos);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    static void loadTexture();

private:
    static sf::Texture texture;
    sf::Sprite   sprite;
    sf::Vector2f position;
};

#endif
