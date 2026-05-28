#ifndef CAR_H
#define CAR_H

#include "Prop.h"
#include <SFML/Graphics.hpp>

enum class CarType { CAR1, CAR2, CAR3 };

class Car : public Prop {
public:
    explicit Car(sf::Vector2f pos, CarType type = CarType::CAR1);
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    static void loadTextures();

private:
    static sf::Texture textureCar1;
    static sf::Texture textureCar2;
    static sf::Texture textureCar3;

    sf::Sprite   sprite;
    sf::Vector2f position;
    CarType      carType;
};

#endif
