#ifndef BENCH_H
#define BENCH_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

class Bench : public Prop {
public:
    explicit Bench(sf::Vector2f pos);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    static void loadTexture();

private:
    static sf::Texture texture;
    sf::Sprite   sprite;
    sf::Vector2f position;
};

#endif
