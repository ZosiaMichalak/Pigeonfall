#ifndef FLOWERS_H
#define FLOWERS_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

class Flowers : public Prop {
public:
    explicit Flowers(sf::Vector2f pos);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    static void loadTexture();

private:
    static sf::Texture texture;
    sf::Sprite   sprite;
    sf::Vector2f position;
};

#endif
