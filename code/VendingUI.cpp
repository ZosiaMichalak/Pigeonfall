/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "VendingUI.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

// Rounds coordinates to align layout precisely to pixels.
static inline float px(float v) { return std::round(v); }

// Constructor: Initializes cursor selection index and sets default item slots array.
VendingUI::VendingUI(sf::Font& font)
    : font(font), open(false), selectedSlot(0)
{
    slotIndices = {0, 1, 2};
}

// Selects 3 random unique items from the pool to populate the vending slots.
void VendingUI::rollItems(bool totemAlreadyBought) {
    std::vector<int> pool;
    pool.reserve(CATALOGUE_SIZE);
    
    // Build selection pool, excluding Totem if it has already been purchased
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

    // Shuffle and pick 3 unique indices from pool
    for (int i = 0; i < SHOP_SLOTS; ++i) {
        int j = i + std::rand() % (static_cast<int>(pool.size()) - i);
        std::swap(pool[static_cast<size_t>(i)], pool[static_cast<size_t>(j)]);
        slotIndices[static_cast<size_t>(i)] = pool[static_cast<size_t>(i)];
    }
    selectedSlot = 0;
}

// Opens the vending shop.
void VendingUI::openShop() {
    open = true;
}

// Closes the vending shop.
void VendingUI::close() {
    open = false;
}

// Increments/decrements selected slot index, wrapping around using modulo.
void VendingUI::moveSelection(int delta) {
    selectedSlot = (selectedSlot + SHOP_SLOTS + delta) % SHOP_SLOTS;
}

// Verifies if the player can afford the item and if they have space. Deducts coins if bought.
std::string VendingUI::tryBuy(int& totalCoins, const std::string& currentItem) {
    const VendingItem& item = VENDING_CATALOGUE[slotIndices[selectedSlot]];

    // Hands-full check: player can only carry one item at a time
    if (!currentItem.empty()) return "";

    // Affordability check
    if (totalCoins < item.cost) return "";

    // Complete transaction
    totalCoins -= item.cost;
    close();
    return item.name;
}

// Renders the background dim, panel box container, header titles, active slot cards, prices, and error highlights.
void VendingUI::render(sf::RenderWindow& window, int totalCoins, const std::string& heldItem) {
    if (!open) return;

    // Dark screen dim overlay
    sf::RectangleShape overlay({VIEW_W, VIEW_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    const float pw = 270.f;
    const float ph = 165.f;
    const float panelX = px((VIEW_W - pw) / 2.f);
    const float panelY = px((VIEW_H - ph) / 2.f);

    // Main green outline shop window panel
    sf::RectangleShape panel({pw, ph});
    panel.setFillColor(sf::Color(10, 14, 24));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(60, 120, 60));
    panel.setPosition(panelX, panelY);
    window.draw(panel);

    // Main header title
    sf::Text title("VENDING MACHINE", font, 16);
    title.setFillColor(sf::Color(80, 200, 80));
    title.setPosition(
        px(panelX + pw / 2.f - title.getLocalBounds().width / 2.f),
        px(panelY + 6.f));
    window.draw(title);

    // Active coin balance label
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

    // Draw the 3 item slot cards
    for (int i = 0; i < SHOP_SLOTS; ++i) {
        const VendingItem& item = VENDING_CATALOGUE[slotIndices[i]];
        float cardY = px(FIRST_Y + i * (CARD_H + GAP));
        bool  sel   = (i == selectedSlot);
        bool  canAfford = (totalCoins >= item.cost);
        bool  blocked   = !heldItem.empty();

        // Card box shape
        sf::RectangleShape card({CARD_W, CARD_H});
        card.setFillColor(sel ? sf::Color(22, 35, 22) : sf::Color(14, 20, 14));
        card.setOutlineThickness(1.f);
        // Highlight outline with item's card accent color if hovered
        card.setOutlineColor(sel ? item.color : sf::Color(35, 60, 35));
        card.setPosition(px(CARD_X), cardY);
        window.draw(card);

        // Item name label (greyed out if unaffordable or hands full)
        sf::Text nameText(item.name, font, 16);
        nameText.setFillColor(canAfford && !blocked ? sf::Color::White : sf::Color(100, 100, 100));
        nameText.setPosition(px(CARD_X + 6.f), px(cardY + 4.f));
        window.draw(nameText);

        // Price label text
        sf::Text costText(std::to_string(item.cost) + "c", font, 16);
        costText.setFillColor(canAfford ? sf::Color(255, 210, 30) : sf::Color(120, 80, 30));
        costText.setPosition(px(CARD_X + CARD_W - costText.getLocalBounds().width - 6.f),
                            px(cardY + 4.f));
        window.draw(costText);

        // Display hands-full warning if already holding an item
        if (blocked) {
            sf::Text blockHint("Hands full", font, 16);
            blockHint.setScale(0.55f, 0.55f);
            blockHint.setFillColor(sf::Color(180, 80, 80));
            blockHint.setPosition(px(CARD_X + 6.f), px(cardY + 20.f));
            window.draw(blockHint);
        }
    }

    // Shop controls footer hint
    sf::Text hint("W/S  E=buy  Q=close", font, 16);
    hint.setScale(0.65f, 0.65f);
    hint.setFillColor(sf::Color(80, 120, 80));
    hint.setPosition(px(panelX + 6.f), px(panelY + ph - 16.f));
    window.draw(hint);
}
