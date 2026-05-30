#include "SkillTreeUI.h"
#include "Player.h"
#include <cmath>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

static inline float px(float v) { return std::round(v); }

static void sharpFont(sf::Font& font) {
    const_cast<sf::Texture&>(font.getTexture(16)).setSmooth(false);
}

SkillTreeUI::SkillTreeUI(sf::Font& font)
    : font(font), open(false), selectedSkill(0)
{}

void SkillTreeUI::moveSelection(int delta) {
    selectedSkill = (selectedSkill + SKILL_COUNT + delta) % SKILL_COUNT;
}

void SkillTreeUI::buySelected(Player* player) {
    if (player) player->buySkill(selectedSkill);
}

void SkillTreeUI::render(sf::RenderWindow& window, Player* p) {
    if (!p) return;
    sharpFont(font);

    sf::RectangleShape overlay({VIEW_W, VIEW_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    const float ROW_H  = 22.f; // Powiększone rzędy by zmieścić font 16
    const float HEADER = 30.f;
    const float FOOTER = 20.f;
    const float pw     = 250.f;
    const float ph     = HEADER + SKILL_COUNT * ROW_H + FOOTER + 4.f;
    const float panelX = px((VIEW_W - pw) / 2.f);
    const float panelY = px((VIEW_H - ph) / 2.f);

    sf::RectangleShape panel({pw, ph});
    panel.setFillColor(sf::Color(12, 12, 20));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(75, 55, 130));
    panel.setPosition(panelX, panelY);
    window.draw(panel);

    sf::Text title("SKILL TREE", font, 16);
    title.setFillColor(sf::Color(170, 130, 255));
    title.setPosition(
        px(panelX + pw / 2.f - title.getLocalBounds().width / 2.f),
        px(panelY + 6.f));
    window.draw(title);

    sf::Text spLabel("SP: " + std::to_string(p->getSkillPoints()), font, 16);
    spLabel.setFillColor(sf::Color(255, 215, 50));
    spLabel.setPosition(
        px(panelX + pw - spLabel.getLocalBounds().width - 6.f),
        px(panelY + 6.f));
    window.draw(spLabel);

    const float startY = panelY + HEADER;

    for (int i = 0; i < SKILL_COUNT; ++i) {
        float ry     = px(startY + i * ROW_H);
        bool  sel    = (i == selectedSkill);
        bool  maxed  = (p->getUpgradeLevel(i) >= SKILL_DEFS[i].maxLevel);
        bool  canBuy = p->canBuySkill(i);

        if (sel) {
            sf::RectangleShape rowBg({pw - 6.f, ROW_H - 2.f});
            rowBg.setFillColor(sf::Color(38, 28, 65));
            rowBg.setOutlineThickness(1.f);
            rowBg.setOutlineColor(sf::Color(110, 70, 190));
            rowBg.setPosition(px(panelX + 3.f), ry);
            window.draw(rowBg);
        }

        if (sel) {
            sf::Text arr(">", font, 16);
            arr.setFillColor(sf::Color(255, 215, 50));
            arr.setPosition(px(panelX + 6.f), px(ry));
            window.draw(arr);
        }

        sf::Text nameT(SKILL_DEFS[i].name, font, 16);
        nameT.setFillColor(maxed  ? sf::Color(90, 190, 90)
                         : canBuy ? sf::Color(215, 215, 215)
                                  : sf::Color(110, 110, 110));
        nameT.setPosition(px(panelX + 18.f), px(ry));
        window.draw(nameT);

        // ── Obliczanie układu prawej strony (Od prawej do lewej) ──────────────
        float rightEdge = panelX + pw - 6.f;

        sf::Text statusT("", font, 16);
        if (maxed) {
            statusT.setString("MAX");
            statusT.setFillColor(sf::Color(70, 185, 70));
        } else if (sel) {
            statusT.setString(canBuy ? "BUY" : "NO SP");
            statusT.setFillColor(canBuy ? sf::Color(255, 215, 50) : sf::Color(170, 55, 55));
        }

        if (!statusT.getString().isEmpty()) {
            float tw = statusT.getLocalBounds().width;
            statusT.setPosition(px(rightEdge - tw), px(ry));
            rightEdge -= (tw + 10.f); 
        } else {
            rightEdge -= 4.f; 
        }

        int   maxLv   = SKILL_DEFS[i].maxLevel;
        int   curLv   = p->getUpgradeLevel(i);
        float pipSize = 8.f; // Lekko większe by pasowały do rozmiaru 16
        
        float pipTotalW = maxLv * (pipSize + 2.f);
        float pipStartX = rightEdge - pipTotalW;

        for (int j = 0; j < maxLv; ++j) {
            sf::RectangleShape pip({pipSize, pipSize});
            pip.setFillColor(j < curLv ? sf::Color(110, 70, 210) : sf::Color(30, 26, 50));
            pip.setOutlineThickness(0.5f);
            pip.setOutlineColor(sf::Color(70, 55, 120));
            pip.setPosition(px(pipStartX + j * (pipSize + 2.f)), px(ry + 6.f));
            window.draw(pip);
        }
    }

    sf::Text hint("W/S  E=buy  Tab/Q=close", font, 16);
    hint.setFillColor(sf::Color(90, 80, 125));
    hint.setPosition(
        px(panelX + pw / 2.f - hint.getLocalBounds().width / 2.f),
        px(panelY + ph - FOOTER + 2.f));
    window.draw(hint);
}