/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "HUD.h"
#include "Player.h"
#include <algorithm>
#include <string>
#include <cmath>

static constexpr float BAR_Y = 195.f;
static constexpr float BAR_H = 30.f;

// Helper function rounding coordinates to the nearest pixel to prevent blurriness.
static inline float px(float v) { return std::floor(v + 0.5f); }

// Configures font texture settings to ensure sharp rendering for specific sizes.
static void sharpFont(sf::Font& font) {
    for (unsigned sz : {9u, 12u, 13u, 14u, 16u, 18u, 20u, 24u})
        const_cast<sf::Texture&>(font.getTexture(sz)).setSmooth(false);
}

// Constructor: sets up text colors, native font sizes, and loads the spinning coin texture.
HUD::HUD(sf::Font& font) : font(font), coinIconFrame(0), coinIconTimer(0.f) {
    sharpFont(font);
    
    // Set standard size 16 (matches native m5x7 resolution)
    roomText.setFont(font);
    roomText.setCharacterSize(16); 
    roomText.setFillColor(sf::Color(200, 200, 200));

    coinText.setFont(font);
    coinText.setCharacterSize(16);
    coinText.setFillColor(sf::Color(255, 210, 30));

    hasCoinIcon = coinIconTexture.loadFromFile("assets/coin.png");
    if (hasCoinIcon) {
        coinIconTexture.setSmooth(false);
        coinIcon.setTexture(coinIconTexture);
        coinIcon.setTextureRect(sf::IntRect(0, 0, COIN_FRAME_W, COIN_FRAME_H));
    }
}

// Draws the main bottom HUD layout: renders HP bar, level, XP progress, current room number, gold coins count, and the active skills toggle hint.
void HUD::render(sf::RenderWindow& window, Player* player, int roomId, int coins) {
    sharpFont(font);
    
    // 1. Draw HUD panel background rectangle
    sf::RectangleShape bar({400.f, BAR_H});
    bar.setFillColor(sf::Color(10, 12, 22));
    bar.setPosition(0.f, px(BAR_Y));
    window.draw(bar);

    // Draw top separator border line
    sf::RectangleShape line({400.f, 1.f});
    line.setFillColor(sf::Color(40, 45, 60));
    line.setPosition(0.f, px(BAR_Y));
    window.draw(line);

    float rightSideX = 330.f;

    // 2. Render Player stats (HP Bar, Second-Chance indicator, Level, XP bar)
    if (player && player->isActive()) {
        Player* p = player;

        // Health Bar (HP) background
        float hpPct = (p->getMaxHp() > 0) ? std::min(1.f, static_cast<float>(p->getHp()) / p->getMaxHp()) : 0.f;

        sf::RectangleShape hpBack({80.f, 8.f});
        hpBack.setFillColor(sf::Color(60, 20, 20));
        hpBack.setPosition(10.f, px(BAR_Y + 4.f));
        window.draw(hpBack);

        // Fill HP Bar according to percentage remaining
        if (hpPct > 0.f) {
            sf::RectangleShape hpFill({px(80.f * hpPct), 8.f});
            hpFill.setFillColor(sf::Color(220, 40, 40));
            hpFill.setPosition(10.f, px(BAR_Y + 4.f));
            window.draw(hpFill);
        }

        // Draw HP value text over the center of the bar
        sf::Text hpText(std::to_string(p->getHp()) + "/" + std::to_string(p->getMaxHp()), font, 16);
        hpText.setScale(0.5f, 0.5f);
        hpText.setFillColor(sf::Color::White);
        
        sf::FloatRect hpTextBounds = hpText.getGlobalBounds();
        float hpTextX = px(10.f + (80.f - hpTextBounds.width) / 2.f);
        float hpTextY = px(BAR_Y + 2.f);
        hpText.setPosition(hpTextX, hpTextY);
        window.draw(hpText);

        // Render second-chance skill / totem dots to track revive availability
        {
            float dotX = 96.f; 
            float dotY = px(BAR_Y + 4.f);
            float dotR = 3.f;

            bool hasSkillSC = (p->getUpgradeLevel(SK_SECOND_CHANCE) > 0);
            if (hasSkillSC) {
                sf::CircleShape dot(dotR);
                dot.setOrigin(dotR, dotR);
                dot.setPosition(dotX + dotR, dotY + dotR);
                // Gray out dot if the second chance revive has been consumed
                dot.setFillColor(p->isSecondChanceUsed()
                    ? sf::Color(90, 90, 90) : sf::Color(255, 210, 30));
                dot.setOutlineThickness(0.5f);
                dot.setOutlineColor(sf::Color(180, 140, 0));
                window.draw(dot);
                dotX += dotR * 2.f + 3.f;
            }

            // Draw a dot for each extra totem revive charge held
            int totemCharges = p->getTotemCharges();
            for (int t = 0; t < totemCharges; ++t) {
                sf::CircleShape dot(dotR);
                dot.setOrigin(dotR, dotR);
                dot.setPosition(dotX + dotR, dotY + dotR);
                dot.setFillColor(sf::Color(255, 210, 30));
                dot.setOutlineThickness(0.5f);
                dot.setOutlineColor(sf::Color(180, 140, 0));
                window.draw(dot);
                dotX += dotR * 2.f + 3.f;
            }
        }

        // Render player level (LVL) label
        sf::Text lvlText("LVL: " + std::to_string(p->getLevel()), font, 16);
        lvlText.setScale(0.75f, 0.75f);
        lvlText.setFillColor(sf::Color(200, 200, 200));
        lvlText.setPosition(10.f, px(BAR_Y + 16.f));
        window.draw(lvlText);

        // Experience (XP) bar rendering
        float xpStartX = px(lvlText.getPosition().x + lvlText.getGlobalBounds().width + 10.f);
        float xpWidth  = px(rightSideX - xpStartX - 15.f);

        if (xpWidth > 20.f) {
            sf::RectangleShape xpBack({xpWidth, 5.f});
            xpBack.setFillColor(sf::Color(22, 18, 40));
            xpBack.setPosition(xpStartX, px(BAR_Y + 22.f));
            window.draw(xpBack);

            float xpPct = (p->getXPToNext() > 0)
                        ? std::min(1.f, static_cast<float>(p->getXP()) / p->getXPToNext()) : 0.f;

            if (xpPct > 0.f) {
                sf::RectangleShape xpFill({px(xpWidth * xpPct), 5.f});
                xpFill.setFillColor(sf::Color(50, 220, 100));
                xpFill.setPosition(xpStartX, px(BAR_Y + 22.f));
                window.draw(xpFill);
            }
        }
    }

    // 3. Render current room number and gold coins count
    roomText.setString("ROOM: " + std::to_string(roomId + 1));
    roomText.setPosition(px(rightSideX), px(BAR_Y - 2.f)); 
    window.draw(roomText);

    coinText.setString(std::to_string(coins));
    if (hasCoinIcon) {
        int tx = (coinIconFrame % COIN_SHEET_COLS) * COIN_FRAME_W;
        int ty = (coinIconFrame / COIN_SHEET_COLS) * COIN_FRAME_H;
        coinIcon.setTextureRect(sf::IntRect(tx, ty, COIN_FRAME_W, COIN_FRAME_H));
        
        coinIcon.setPosition(px(rightSideX), px(BAR_Y + 18.f));
        coinText.setPosition(px(rightSideX + 12.f), px(BAR_Y + 10.f));
        
        window.draw(coinIcon);
    } else {
        coinText.setPosition(px(rightSideX), px(BAR_Y + 14.f));
    }
    window.draw(coinText);

    // 4. Render control hint for active skill tree toggle
    if (player && player->isActive()) {
        bool hasPoints = player->getSkillPoints() > 0;
        sf::Text skillsHint("[Tab] Skills", font, 16);
        skillsHint.setScale(0.75f, 0.75f);
        // Highlight in yellow if player has unspent skill points
        skillsHint.setFillColor(hasPoints ? sf::Color(255, 210, 30) : sf::Color(120, 120, 120));
        skillsHint.setPosition(px(150.f), px(BAR_Y + 1.f));
        window.draw(skillsHint);
    }
}

// Draw skill tree hint (Unused: code has been relocated directly into HUD::render).
void HUD::renderSkillsHint(sf::RenderWindow& /*window*/, Player* /*player*/) {
}