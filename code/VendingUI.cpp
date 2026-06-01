#include "VendingUI.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

static inline float px(float v) { return std::round(v); }

VendingUI::VendingUI(sf::Font& font)
    : font(font), open(false), selectedSlot(0)
{
    slotIndices = {0, 1, 2};
}

void VendingUI::rollItems(bool totemAlreadyBought) {
    std::vector<int> pool;
    pool.reserve(CATALOGUE_SIZE);
    for (int i = 0; i < CATALOGUE_SIZE; ++i) {
        if (totemAlreadyBought && i == TOTEM_CATALOGUE_INDEX)
            continue;
        pool.push_back(i);
    }

    if (static_cast<int>(pool.size()) < SHOP_SLOTS) {
        slotIndices = {0, 1, 2};
        selectedSlot = 0;
        return;
    }

    for (int i = 0; i < SHOP_SLOTS; ++i) {
        int j = i + std::rand() % (static_cast<int>(pool.size()) - i);
        std::swap(pool[static_cast<size_t>(i)], pool[static_cast<size_t>(j)]);
        slotIndices[static_cast<size_t>(i)] = pool[static_cast<size_t>(i)];
    }
    selectedSlot = 0;
}

void VendingUI::openShop() {
    open = true;
}

void VendingUI::close() {
    open = false;
}

void VendingUI::moveSelection(int delta) {
    selectedSlot = (selectedSlot + SHOP_SLOTS + delta) % SHOP_SLOTS;
}

std::string VendingUI::tryBuy(int& totalCoins, const std::string& currentItem) {
    const VendingItem& item = VENDING_CATALOGUE[slotIndices[selectedSlot]];

    if (!currentItem.empty()) return "";

    if (totalCoins < item.cost) return "";

    totalCoins -= item.cost;
    close();
    return item.name;
}

void VendingUI::render(sf::RenderWindow& window, int totalCoins, const std::string& heldItem) {
    if (!open) return;

    sf::RectangleShape overlay({VIEW_W, VIEW_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    const float pw = 270.f;
    const float ph = 165.f;
    const float panelX = px((VIEW_W - pw) / 2.f);
    const float panelY = px((VIEW_H - ph) / 2.f);

    sf::RectangleShape panel({pw, ph});
    panel.setFillColor(sf::Color(10, 14, 24));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(60, 120, 60));
    panel.setPosition(panelX, panelY);
    window.draw(panel);

    sf::Text title("VENDING MACHINE", font, 16);
    title.setFillColor(sf::Color(80, 200, 80));
    title.setPosition(
        px(panelX + pw / 2.f - title.getLocalBounds().width / 2.f),
        px(panelY + 6.f));
    window.draw(title);

    sf::Text coinLabel(std::to_string(totalCoins) + "c", font, 16);
    coinLabel.setFillColor(sf::Color(255, 210, 30));
    coinLabel.setPosition(
        px(panelX + pw - coinLabel.getLocalBounds().width - 6.f),
        px(panelY + 6.f));
    window.draw(coinLabel);

    const float CARD_H  = 38.f;
    const float CARD_W  = pw - 10.f;
    const float CARD_X  = panelX + 5.f;
    const float FIRST_Y = panelY + 28.f;
    const float GAP     = 3.f;

    for (int i = 0; i < SHOP_SLOTS; ++i) {
        const VendingItem& item = VENDING_CATALOGUE[slotIndices[i]];
        float cardY = px(FIRST_Y + i * (CARD_H + GAP));
        bool  sel   = (i == selectedSlot);
        bool  canAfford = (totalCoins >= item.cost);
        bool  blocked   = !heldItem.empty();

        sf::RectangleShape card({CARD_W, CARD_H});
        card.setFillColor(sel ? sf::Color(22, 35, 22) : sf::Color(14, 20, 14));
        card.setOutlineThickness(1.f);
        card.setOutlineColor(sel ? item.color : sf::Color(35, 60, 35));
        card.setPosition(px(CARD_X), cardY);
        window.draw(card);

        sf::Text nameText(item.name, font, 16);
        nameText.setFillColor(canAfford && !blocked ? sf::Color::White : sf::Color(100, 100, 100));
        nameText.setPosition(px(CARD_X + 6.f), px(cardY + 4.f));
        window.draw(nameText);

        sf::Text costText(std::to_string(item.cost) + "c", font, 16);
        costText.setFillColor(canAfford ? sf::Color(255, 210, 30) : sf::Color(120, 80, 30));
        costText.setPosition(px(CARD_X + CARD_W - costText.getLocalBounds().width - 6.f),
                            px(cardY + 4.f));
        window.draw(costText);

        if (blocked) {
            sf::Text blockHint("Hands full", font, 16);
            blockHint.setScale(0.55f, 0.55f);
            blockHint.setFillColor(sf::Color(180, 80, 80));
            blockHint.setPosition(px(CARD_X + 6.f), px(cardY + 20.f));
            window.draw(blockHint);
        }
    }

    sf::Text hint("W/S  E=buy  Q=close", font, 16);
    hint.setScale(0.65f, 0.65f);
    hint.setFillColor(sf::Color(80, 120, 80));
    hint.setPosition(px(panelX + 6.f), px(panelY + ph - 16.f));
    window.draw(hint);
}
