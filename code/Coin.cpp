/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "Coin.h"
#include <cmath>
#include <iostream>

sf::Texture Coin::texture;

// Loads the shared coin sprite sheet texture file if not already loaded.
void Coin::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/coin.png"))
            std::cerr << "[Warning] assets/coin.png not found – using fallback.\n";
        else
            texture.setSmooth(false); // Disable smoothing to preserve pixel art style
    }
}

// Constructor: Configures animation state, sets origins, and defines the fallback vector box.
Coin::Coin(float x, float y)
    : GameObject(x, y),
      hasTexture(texture.getSize().x > 0),
      currentFrame(0), animTimer(0.f),
      bobTimer(0.f), bobOffset(0.f)
{
    if (hasTexture) {
        sprite.setTexture(texture);
        // Display frame (0,0) initially
        sprite.setTextureRect(sf::IntRect(0, 0, FRAME_W, FRAME_H));
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);
        sprite.setPosition(position);
    } else {
        // Fallback drawing shape setup
        fallback.setSize({6.f, 6.f});
        fallback.setFillColor(sf::Color(255, 210, 30));
        fallback.setOrigin(3.f, 3.f);
        fallback.setPosition(position);
    }
}

// Animates coin rotation and applies a sinusoidal vertical bobbing offset.
void Coin::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;

    // Advance animation frame on timer tick
    animTimer += dt;
    if (animTimer >= FRAME_DUR) {
        animTimer -= FRAME_DUR;
        currentFrame = (currentFrame + 1) % FRAME_COUNT;

        if (hasTexture) {
            int col = currentFrame % SHEET_COLS;
            int row = currentFrame / SHEET_COLS;
            sprite.setTextureRect(sf::IntRect(col * FRAME_W, row * FRAME_H, FRAME_W, FRAME_H));
        }
    }

    // Gentle bobbing calculation
    bobTimer += dt;
    bobOffset = std::sin(bobTimer * 5.f) * 2.f;

    sf::Vector2f drawPos(position.x, position.y + bobOffset);
    if (hasTexture)
        sprite.setPosition(drawPos);
    else
        fallback.setPosition(drawPos);
}

// Renders either the animated sprite or the fallback box.
void Coin::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (hasTexture)
        window.draw(sprite);
    else
        window.draw(fallback);
}

// Computes the collision bounds rectangle (expanded slightly for easier pickup).
sf::FloatRect Coin::getBounds() const {
    return { position.x - 10.f, position.y - 10.f, 20.f, 20.f };
}
