#ifndef VENDING_H
#define VENDING_H
#include "Prop.h"

class Vending : public Prop {
public:
    Vending(sf::Vector2f pos);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    static void loadTexture();
private:
    static sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f position;
};
#endif