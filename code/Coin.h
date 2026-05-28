#ifndef COIN_H
#define COIN_H

#include "GameObject.h"
#include <SFML/Graphics.hpp>

class Coin : public GameObject {
public:
    static void loadTexture();

    Coin(float x, float y);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    sf::FloatRect getBounds() const;

private:
    static sf::Texture texture;

    sf::Sprite sprite;
    bool       hasTexture;

    // Spritesheet: 3 cols x 3 rows of 8x8 frames, 8 active frames (last cell empty)
    static constexpr int   SHEET_COLS   = 3;
    static constexpr int   FRAME_W      = 8;
    static constexpr int   FRAME_H      = 8;
    static constexpr int   FRAME_COUNT  = 8;   // 9th cell is blank
    static constexpr float FRAME_DUR    = 0.10f;

    int   currentFrame;
    float animTimer;

    // Gentle bob
    float bobTimer;
    float bobOffset;

    sf::RectangleShape fallback;
};

#endif
