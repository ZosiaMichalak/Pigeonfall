#ifndef SKILL_TREE_UI_H
#define SKILL_TREE_UI_H

#include <SFML/Graphics.hpp>

class Player;

class SkillTreeUI {
public:
    explicit SkillTreeUI(sf::Font& font);

    // Returns true if the overlay is currently open
    bool isOpen() const { return open; }

    // Toggle open/close
    void toggle() { open = !open; }
    void close()  { open = false; }

    // Move selection cursor (delta = +1 or -1)
    void moveSelection(int delta);

    // Attempt to purchase the currently selected skill
    void buySelected(Player* player);

    // Render the overlay (call only when isOpen())
    void render(sf::RenderWindow& window, Player* player);

    int getSelectedSkill() const { return selectedSkill; }

private:
    sf::Font& font;
    bool open;
    int  selectedSkill;
};

#endif
