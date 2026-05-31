#include "VendingUI.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

static inline float px(float v) { return std::round(v); }

VendingUI::VendingUI(sf::Font& font)
    : font(font), open(false), selectedSlot(0)
{
    slotIndices = {0, 1, 2};
}

void VendingUI::rollItems(bool totemAlreadyBought) {
    // Build a pool of catalogue indices, optionally excluding Totem (index 4)
    std::array<int, CATALOGUE_SIZE> pool = {0, 1, 2, 3, 4};
    int poolSize = CATALOGUE_SIZE;

    if (totemAlreadyBought) {
        // Swap Totem to the end and shrink the pool
        pool[4] = 4; // already there, just shrink
        poolSize = CATALOGUE_SIZE - 1; // exclude last slot (Totem = index 4)
    }

    // Fisher-Yates on first SHOP_SLOTS entries within poolSize
    for (int i = 0; i < SHOP_SLOTS; ++i) {
        int j = i + std::rand() % (poolSize - i);
        std::swap(pool[i], pool[j]);
        slotIndices[i] = pool[i];
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

    // Can't buy if you already hold something
    if (!currentItem.empty()) return "";

    if (totalCoins < item.cost) return "";

    totalCoins -= item.cost;
    close();
    return item.name;
}

void VendingUI::render(sf::RenderWindow& window, int totalCoins, const std::string& heldItem) {
    if (!open) return;

    // Dim overlay
    sf::RectangleShape overlay({VIEW_W, VIEW_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    // Panel
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

    // Title
    sf::Text title("VENDING MACHINE", font, 16);
    title.setFillColor(sf::Color(80, 200, 80));
    title.setPosition(
        px(panelX + pw / 2.f - title.getLocalBounds().width / 2.f),
        px(panelY + 6.f));
    window.draw(title);

    // Coin count (top right)
    sf::Text coinLabel(std::to_string(totalCoins) + "c", font, 16);
    coinLabel.setFillColor(sf::Color(255, 210, 30));
    coinLabel.setPosition(
        px(panelX + pw - coinLabel.getLocalBounds().width - 6.f),
        px(panelY + 6.f));
    window.draw(coinLabel);

    // Item cards
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

        // Card background
        sf::RectangleShape card({CARD_W, CARD_H});
        card.setFillColor(sel ? sf::Color(22, 35, 22) : sf::Color(14, 20, 14));
        card.setOutlineThickness(1.f);
        card.setOutlineColor(sel ? item.color : sf::Color(35, 60, 35));
        card.setPosition(px(CARD_X), cardY);
        window.draw(card);

        // Colour strip on the left
        sf::RectangleShape strip({4.f, CARD_H - 2.f});
        strip.setFillColor(item.color);
        strip.setPosition(px(CARD_X + 1.f), cardY + 1.f);
        window.draw(strip);

        // Selection arrow
        if (sel) {
            sf::Text arr(">", font, 16);
            arr.setFillColor(sf::Color(255, 215, 50));
            arr.setPosition(px(CARD_X + 7.f), px(cardY + 4.f));
            window.draw(arr);
        }

        // Item name
        sf::Color nameCol = blocked   ? sf::Color(80, 80, 80)
                          : canAfford ? sf::Color(220, 220, 220)
                                      : sf::Color(130, 70, 70);
        sf::Text nameT(item.name, font, 16);
        nameT.setFillColor(nameCol);
        nameT.setPosition(px(CARD_X + 18.f), px(cardY + 4.f));
        window.draw(nameT);

        // Cost
        sf::Text costT(std::to_string(item.cost) + "c", font, 16);
        costT.setFillColor(canAfford && !blocked ? sf::Color(255, 210, 30) : sf::Color(100, 80, 30));
        costT.setPosition(
            px(CARD_X + CARD_W - costT.getLocalBounds().width - 6.f),
            px(cardY + 4.f));
        window.draw(costT);

        // Status line
        sf::Text status("", font, 16);
        status.setScale(0.75f, 0.75f);
        if (sel) {
            if (blocked)
                status.setString("Drop your item first!");
            else if (!canAfford)
                status.setString("Not enough coins");
            else
                status.setString("E to buy");
            status.setFillColor(blocked || !canAfford
                ? sf::Color(160, 60, 60) : sf::Color(120, 200, 120));
            status.setPosition(px(CARD_X + 18.f), px(cardY + CARD_H - 14.f));
            window.draw(status);
        }
    }

    // Footer hint
    sf::Text hint("W/S navigate  E=buy  Q=close", font, 16);
    hint.setScale(0.75f, 0.75f);
    hint.setFillColor(sf::Color(60, 100, 60));
    hint.setPosition(
        px(panelX + pw / 2.f - hint.getLocalBounds().width * 0.75f / 2.f),
        px(panelY + ph - 14.f));
    window.draw(hint);
}
