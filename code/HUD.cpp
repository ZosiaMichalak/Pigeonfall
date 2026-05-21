#include "HUD.h"
#include "Player.h"
#include <algorithm>
#include <string>
#include <cmath>

static constexpr float BAR_Y = 195.f;
static constexpr float BAR_H = 30.f;

static inline float px(float v) { return std::floor(v + 0.5f); }

static void sharpFont(sf::Font& font) {
    for (unsigned sz : {12u, 14u, 16u})
        const_cast<sf::Texture&>(font.getTexture(sz)).setSmooth(false);
}

HUD::HUD(sf::Font& font) : font(font) {
    roomText.setFont(font);
    roomText.setCharacterSize(14);
    roomText.setFillColor(sf::Color(200, 200, 200));
}

float HUD::drawCooldownBox(sf::RenderWindow& window, float bx, float by,
                           const std::string& label, float progress,
                           sf::Color fillReady, sf::Color fillWait)
{
    const float BW = 14.f, BH = 14.f;
    bx = px(bx - BW); 

    sf::RectangleShape back({BW, BH});
    back.setFillColor(sf::Color(30, 30, 38));
    back.setOutlineThickness(1.f);
    back.setOutlineColor(sf::Color(60, 60, 72));
    back.setPosition(bx, px(by));
    window.draw(back);

    float fillH = px(BH * std::max(0.f, std::min(1.f, progress)));
    if (fillH > 0.f) {
        sf::RectangleShape fill({BW, fillH});
        fill.setFillColor(progress >= 1.f ? fillReady : fillWait);
        fill.setPosition(bx, px(by + BH - fillH));
        window.draw(fill);
    }

    sf::Text lbl(label, font, 10);
    lbl.setFillColor(sf::Color(160, 160, 160));
    lbl.setPosition(px(bx - 1.f), px(by - 12.f));
    window.draw(lbl);

    return bx - 8.f;
}

void HUD::render(sf::RenderWindow& window, Player* p, int roomId) {
    sharpFont(font);

    // 1. Room w prawym górnym rogu
    roomText.setString("ROOM " + std::to_string(roomId + 1));
    roomText.setPosition(px(395.f - roomText.getLocalBounds().width), 0.f);
    window.draw(roomText);

    // 2. Tło paska
    sf::RectangleShape bar({400.f, BAR_H});
    bar.setFillColor(sf::Color(10, 10, 16));
    bar.setPosition(0.f, BAR_Y);
    window.draw(bar);

    if (!p) return;

    // --- RZĄD 1: HP (Przesunięte o 1px w górę) ---
    sf::Text hpLbl("HP", font, 14);
    hpLbl.setFillColor(sf::Color(200, 70, 70));
    hpLbl.setPosition(5.f, BAR_Y - 2.f); 
    window.draw(hpLbl);

    float heartX = 25.f;
    for (int i = 0; i < p->getMaxHp(); ++i) {
        sf::RectangleShape heart({8.f, 8.f});
        heart.setFillColor(i < p->getHp() ? sf::Color(210, 45, 45) : sf::Color(40, 20, 20));
        heart.setPosition(px(heartX + i * 11.f), BAR_Y + 4.f);
        window.draw(heart);
    }

    // --- RZĄD 2: LVL + XP + COOLDOWNS (Przesunięte o 1px w górę) ---
    sf::Text lvlText("LVL " + std::to_string(p->getLevel()), font, 14);
    lvlText.setFillColor(sf::Color(140, 100, 230));
    lvlText.setPosition(5.f, BAR_Y + 11.f);
    window.draw(lvlText);

    // Cooldowny
    float rightAnchor = 395.f;
    float boxY = BAR_Y + 10.f;

    float atkT = p->getAttackCooldownTimer(), atkM = p->getAttackCooldownMax();
    float atkProg = (atkM > 0.f) ? std::min(1.f, std::max(0.f, 1.f - atkT / atkM)) : 1.f;
    rightAnchor = drawCooldownBox(window, rightAnchor, boxY, "ATK", atkProg, sf::Color(230, 190, 25), sf::Color(100, 80, 10));

    float dshT = p->getDashCooldownTimer(), dshM = p->getDashCooldownMax();
    float dshProg = (dshM > 0.f) ? std::min(1.f, std::max(0.f, 1.f - dshT / dshM)) : 1.f;
    rightAnchor = drawCooldownBox(window, rightAnchor, boxY, "DSH", dshProg, sf::Color(35, 170, 220), sf::Color(15, 70, 100));

    // Pasek XP
    float xpStartX = lvlText.getPosition().x + lvlText.getLocalBounds().width + 10.f;
    float xpWidth = rightAnchor - xpStartX - 5.f;
    
    if (xpWidth > 20.f) {
        sf::RectangleShape xpBack({xpWidth, 5.f});
        xpBack.setFillColor(sf::Color(22, 18, 40));
        xpBack.setPosition(xpStartX, BAR_Y + 18.f);
        window.draw(xpBack);

        float xpPct = (p->getXPToNext() > 0) ? std::min(1.f, (float)p->getXP() / p->getXPToNext()) : 0.f;
        sf::RectangleShape xpFill({xpWidth * xpPct, 5.f});
        xpFill.setFillColor(sf::Color(90, 50, 200));
        xpFill.setPosition(xpStartX, BAR_Y + 18.f);
        window.draw(xpFill);
    }
}

void HUD::renderSkillsHint(sf::RenderWindow& window, Player* p) {
    if (!p) return;
    sharpFont(font);
    sf::Text hint("[M] SKILLS", font, 14);
    hint.setFillColor(p->getSkillPoints() > 0 ? sf::Color(255, 215, 50) : sf::Color(100, 80, 160));
    hint.setPosition(px(4.f), px(0.f));
    window.draw(hint);
}