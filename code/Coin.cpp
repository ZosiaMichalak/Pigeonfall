#include "Coin.h"
#include <cmath>
#include <iostream>

sf::Texture Coin::texture;

void Coin::loadTexture() {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("assets/coin.png"))
            std::cerr << "[Warning] assets/coin.png not found – using fallback.\n";
        else
            texture.setSmooth(false);
    }
}

Coin::Coin(float x, float y)
    : GameObject(x, y),
      hasTexture(texture.getSize().x > 0),
      currentFrame(0), animTimer(0.f),
      bobTimer(0.f), bobOffset(0.f)
{
    if (hasTexture) {
        sprite.setTexture(texture);
        // Frame (0,0) to start
        sprite.setTextureRect(sf::IntRect(0, 0, FRAME_W, FRAME_H));
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);
        sprite.setPosition(position);
    } else {
        fallback.setSize({6.f, 6.f});
        fallback.setFillColor(sf::Color(255, 210, 30));
        fallback.setOrigin(3.f, 3.f);
        fallback.setPosition(position);
    }
}

void Coin::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;

    // Advance animation
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

    // Bob
    bobTimer += dt;
    bobOffset = std::sin(bobTimer * 5.f) * 2.f;

    sf::Vector2f drawPos(position.x, position.y + bobOffset);
    if (hasTexture)
        sprite.setPosition(drawPos);
    else
        fallback.setPosition(drawPos);
}

void Coin::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (hasTexture)
        window.draw(sprite);
    else
        window.draw(fallback);
}

sf::FloatRect Coin::getBounds() const {
    return { position.x - 10.f, position.y - 10.f, 20.f, 20.f };
}
