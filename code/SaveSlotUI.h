/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef SAVE_SLOT_UI_H
#define SAVE_SLOT_UI_H

#include <SFML/Graphics.hpp>
#include "DifficultySettings.h"

// Action results returned to the game loop when selecting a difficulty slot.
enum class SlotUIResult {
    NONE,
    SELECTED,   // User confirmed difficulty selection
    CANCELLED   // User pressed Q / Escape
};

// UI Overlay class that presents the difficulty options menu (Easy, Normal, Hard) when starting a new game.
class DifficultySelectUI {
public:
    // Constructor: attaches the font resource to the text renderer elements
    explicit DifficultySelectUI(sf::Font& font);

    // Opens the overlay and resets the selection index to default
    void open();
    
    // Closes the overlay
    void close();
    
    // Checks if the overlay is currently open
    bool isOpen() const { return open_; }

    // Listens to arrow keys for difficulty changes and return buttons to confirm choices
    SlotUIResult handleEvent(const sf::Event& event);
    
    // Draws the pop-up window panel, difficulty buttons, and key help hint
    void render(sf::RenderWindow& window);

    // Gets the chosen difficulty level
    Difficulty getChosenDifficulty() const { return chosenDifficulty_; }

private:
    sf::Font& font_;
    bool      open_         = false; // Visibility flag

    // Active state indicators
    int        difficultySel_     = 1;  // Selection index: 0=Easy, 1=Normal, 2=Hard
    Difficulty chosenDifficulty_  = Difficulty::NORMAL;

    // Helper that draws the panel container shape
    void drawPanel(sf::RenderWindow& w,
                   float x, float y, float pw, float ph,
                   sf::Color fill, sf::Color outline);
    
    // Helper that generates configured text boxes
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col, float x, float y);
};

#endif
