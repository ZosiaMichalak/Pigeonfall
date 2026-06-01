#ifndef VENDING_UI_H
#define VENDING_UI_H

#include <SFML/Graphics.hpp>
#include <string>
#include <array>

class Player;

// ── Item catalogue ─────────────────────────────────────────────────────────────
struct VendingItem {
    std::string name;
    int         cost;
    sf::Color   color; // accent colour for the card
};

static const VendingItem VENDING_CATALOGUE[] = {
    { "Monster Energy", 50, sf::Color(  0, 200,  60) },
    { "Pizza",          30, sf::Color(220, 100,  20) },
    { "Duo",            60, sf::Color(200, 160,  20) },
    { "Annoying Dog",   30, sf::Color(200, 200, 200) },
    { "Totem",         100, sf::Color(120,  60, 200) },
};
static constexpr int CATALOGUE_SIZE = 5;
static constexpr int SHOP_SLOTS     = 3;
static constexpr int TOTEM_CATALOGUE_INDEX = 4;

// ── VendingUI ─────────────────────────────────────────────────────────────────
class VendingUI {
public:
    explicit VendingUI(sf::Font& font);

    bool isOpen() const { return open; }
    // Call once on room entry. Pass true if Totem was already bought this run
    // so it gets excluded from the pool entirely.
    void rollItems(bool totemAlreadyBought = false);
    void openShop();   // opens the UI (no re-roll)
    void close();

    // Navigation & purchase
    void moveSelection(int delta);
    // Returns item name bought (empty string if failed). Deducts coins from totalCoins.
    std::string tryBuy(int& totalCoins, const std::string& currentItem);

    void render(sf::RenderWindow& window, int totalCoins, const std::string& heldItem);

    int getSelectedSlot() const { return selectedSlot; }
    const VendingItem& getSlotItem(int i) const { return VENDING_CATALOGUE[slotIndices[i]]; }

private:
    sf::Font& font;
    bool open;
    int  selectedSlot;

    // Indices into VENDING_CATALOGUE for this room's 3 slots
    std::array<int, SHOP_SLOTS> slotIndices;
};

#endif
