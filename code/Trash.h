#ifndef TRASH_H
#define TRASH_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

class Trash : public Prop {
public:
    Trash(sf::Vector2f pos);
    
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;

    // Metoda do wywołania przed tworzeniem pierwszego śmietnika
    static void loadTexture();

private:
    static sf::Texture texture;
    sf::Sprite sprite;
    sf::Vector2f position;
};

#endif