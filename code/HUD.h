/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef HUD_H
#define HUD_H

#include <SFML/Graphics.hpp>
#include <string>

class Player;

// Heads-Up Display (HUD) class that renders health, level, xp progression, room index, coins, and controls hint on the screen bar.
class HUD {
public:
    // Constructor: links the hud render elements to the active font resource
    explicit HUD(sf::Font& font);

    // Renders the full HUD layout (bottom screen panel)
    void render(sf::RenderWindow& window, Player* player, int roomId, int coins);

    // Render helper for skill points availability notification
    void renderSkillsHint(sf::RenderWindow& window, Player* player);

private:
    sf::Font& font;
    sf::Text  roomText;
    sf::Text  coinText;

    // Coin icon spinning animation state
    sf::Texture coinIconTexture;
    sf::Sprite  coinIcon;
    bool        hasCoinIcon;
    int         coinIconFrame;
    float       coinIconTimer;

    // Animation dimensions mapping
    static constexpr int   COIN_FRAME_W    = 8;
    static constexpr int   COIN_FRAME_H    = 8;
    static constexpr int   COIN_SHEET_COLS = 3;
    static constexpr int   COIN_FRAMES     = 8;   
    static constexpr float COIN_FRAME_DUR  = 0.10f;
};

#endif