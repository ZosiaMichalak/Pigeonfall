#ifndef HUD_H
#define HUD_H

#include <SFML/Graphics.hpp>
#include <string>

class Player;

class HUD {
public:
    explicit HUD(sf::Font& font);
    void render(sf::RenderWindow& window, Player* player, int roomId, int coins);
    void renderSkillsHint(sf::RenderWindow& window, Player* player);

private:
    sf::Font& font;
    sf::Text  roomText;
    sf::Text  coinText;

    // Coin icon animation (reuses the same coin spritesheet)
    sf::Texture coinIconTexture;
    sf::Sprite  coinIcon;
    bool        hasCoinIcon;
    int         coinIconFrame;
    float       coinIconTimer;

    static constexpr int   COIN_FRAME_W    = 8;
    static constexpr int   COIN_FRAME_H    = 8;
    static constexpr int   COIN_SHEET_COLS = 3;
    static constexpr int   COIN_FRAMES     = 8;   // 9th cell is blank
    static constexpr float COIN_FRAME_DUR  = 0.10f;

    float drawCooldownBox(sf::RenderWindow& window, float bx, float by,
                          const std::string& label, float progress,
                          sf::Color fillReady, sf::Color fillWait);
};

#endif
