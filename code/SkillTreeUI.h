/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef SKILL_TREE_UI_H
#define SKILL_TREE_UI_H

#include <SFML/Graphics.hpp>

class Player;

// UI Overlay class that displays the skill upgrades tree panel, handles upgrade logic, and monitors player SP levels.
class SkillTreeUI {
public:
    // Constructor: links active font to render text boxes
    explicit SkillTreeUI(sf::Font& font);

    // Checks if the skill tree is open
    bool isOpen() const { return open; }

    // Toggle open state
    void toggle() { open = !open; }
    
    // Explicit close state
    void close()  { open = false; }

    // Shifts cursor index down (+1) or up (-1)
    void moveSelection(int delta);

    // Attempts to buy upgrade level for selected skill
    void buySelected(Player* player);

    // Renders skill tree, player SP, skill level bar meters, and status labels
    void render(sf::RenderWindow& window, Player* player);

    // Gets the current cursor selection index
    int getSelectedSkill() const { return selectedSkill; }

private:
    sf::Font& font;
    bool open;           // Visibility flag
    int  selectedSkill;  // Cursor index
};

#endif
