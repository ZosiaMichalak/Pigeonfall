/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef COIN_H
#define COIN_H

#include "GameObject.h"
#include <SFML/Graphics.hpp>

// Represents a collectible gold coin dropped by defeated enemies that animates and bobs up and down.
class Coin : public GameObject {
public:
    // Static loader that preloads the coin texture sheet once
    static void loadTexture();

    // Constructor: sets initial coordinates of the coin
    Coin(float x, float y);

    // Updates coin animation frames and bobs the coin vertically
    void update(float dt, sf::RenderWindow& window) override;

    // Draws the coin sprite or fallback shape
    void draw(sf::RenderWindow& window) override;

    // Retrieves current coin bounding box for pickup checks
    sf::FloatRect getBounds() const;

private:
    static sf::Texture texture;   // Shared texture resource

    sf::Sprite sprite;            // Sprite used for rendering
    bool       hasTexture;        // Tracks if texture successfully loaded

    // Spritesheet details: 3 cols x 3 rows of 8x8 frames, 8 active frames (last cell empty)
    static constexpr int   SHEET_COLS   = 3;
    static constexpr int   FRAME_W      = 8;
    static constexpr int   FRAME_H      = 8;
    static constexpr int   FRAME_COUNT  = 8;   // 9th cell is blank
    static constexpr float FRAME_DUR    = 0.10f;

    int   currentFrame;           // Index of current animation frame
    float animTimer;              // Timer to pace animation swaps

    // Hover bobbing variables
    float bobTimer;
    float bobOffset;

    sf::RectangleShape fallback;  // Fallback box if texture load fails
};

#endif
