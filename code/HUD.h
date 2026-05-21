#ifndef HUD_H
#define HUD_H

#include <SFML/Graphics.hpp>
#include <string>

class Player;

class HUD {
public:
    explicit HUD(sf::Font& font);
    void render(sf::RenderWindow& window, Player* player, int roomId);
    void renderSkillsHint(sf::RenderWindow& window, Player* player);

private:
    sf::Font& font;
    sf::Text  roomText; // Będzie używany w prawym górnym rogu

    float drawCooldownBox(sf::RenderWindow& window, float bx, float by, 
                          const std::string& label, float progress, 
                          sf::Color fillReady, sf::Color fillWait);
};

#endif