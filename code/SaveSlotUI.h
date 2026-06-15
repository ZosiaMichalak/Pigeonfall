#ifndef SAVE_SLOT_UI_H
#define SAVE_SLOT_UI_H

#include <SFML/Graphics.hpp>
#include "DifficultySettings.h"

// What the caller should do after handleEvent returns
enum class SlotUIResult {
    NONE,
    SELECTED,   // user confirmed difficulty
    CANCELLED   // user pressed Q / Esc
};

class DifficultySelectUI {
public:
    explicit DifficultySelectUI(sf::Font& font);

    // Open the overlay
    void open();
    void close();
    bool isOpen() const { return open_; }

    SlotUIResult handleEvent(const sf::Event& event);
    void render(sf::RenderWindow& window);

    Difficulty getChosenDifficulty() const { return chosenDifficulty_; }

private:
    sf::Font& font_;
    bool      open_         = false;

    // Difficulty picker state
    int        difficultySel_     = 1;  // 0=Easy 1=Normal 2=Hard
    Difficulty chosenDifficulty_  = Difficulty::NORMAL;

    void drawPanel(sf::RenderWindow& w,
                   float x, float y, float pw, float ph,
                   sf::Color fill, sf::Color outline);
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col, float x, float y);
};

#endif
