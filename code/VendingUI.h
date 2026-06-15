/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef VENDING_UI_H
#define VENDING_UI_H

#include <SFML/Graphics.hpp>
#include <string>
#include <array>

class Player;

// Represents a purchasable item record within the shop catalogue.
struct VendingItem {
    std::string name;   // Name tag of item (e.g. "Pizza")
    int         cost;   // Cost of item in gold coins
    sf::Color   color;  // Card highlight accent color
};

// Available items list setup
static const VendingItem VENDING_CATALOGUE[] = {
    { "Monster Energy", 50, sf::Color(  0, 200,  60) },
    { "Pizza",          30, sf::Color(220, 100,  20) },
    { "Duo",            60, sf::Color(200, 160,  20) },
    { "Annoying Dog",   30, sf::Color(200, 200, 200) },
    { "Totem",         100, sf::Color(120,  60, 200) },
};

static constexpr int CATALOGUE_SIZE = 5;
static constexpr int SHOP_SLOTS     = 3; // Number of items concurrently shown in shop window
static constexpr int TOTEM_CATALOGUE_INDEX = 4;

// UI Overlay class that displays shop inventory cards, checks purchasing funds, and handles selection scrolling.
class VendingUI {
public:
    // Constructor: attaches font resource
    explicit VendingUI(sf::Font& font);

    // Checks if the shop overlay is open
    bool isOpen() const { return open; }
    
    // Rolls 3 random items to display in this room's vending machine (excluding Totem if already purchased)
    void rollItems(bool totemAlreadyBought = false);
    
    // Opens shop window UI
    void openShop();
    
    // Closes shop window UI
    void close();

    // Shifts cursor index down (+1) or up (-1)
    void moveSelection(int delta);
    
    // Attempts to buy selected item, checking coins and player inventory space. Deducts coins if successful.
    std::string tryBuy(int& totalCoins, const std::string& currentItem);

    // Renders shop overlay, player coins, item card slot boxes, cost values, hands-full warning, and key help hint
    void render(sf::RenderWindow& window, int totalCoins, const std::string& heldItem);

    // Gets currently selected shop slot index
    int getSelectedSlot() const { return selectedSlot; }
    
    // Gets active item struct in slot i
    const VendingItem& getSlotItem(int i) const { return VENDING_CATALOGUE[slotIndices[i]]; }

private:
    sf::Font& font;
    bool open;           // Visibility flag
    int  selectedSlot;   // Cursor slot index (0..2)

    std::array<int, SHOP_SLOTS> slotIndices; // Mapped indices of the rolled items
};

#endif
