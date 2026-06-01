#ifndef SAVE_SLOT_UI_H
#define SAVE_SLOT_UI_H

#include <SFML/Graphics.hpp>
#include <array>
#include "SaveSystem.h"

// What the caller should do after handleEvent returns
enum class SlotUIResult {
    NONE,
    SELECTED,   // user confirmed a slot  → read getSelectedSlot()
    CANCELLED   // user pressed Q / Esc
};

// Mode passed when opening the UI
enum class SlotUIMode {
    SAVE,
    LOAD,
    NEW_GAME    // same as SAVE but label says "New Game" and slot content doesn't matter
};

class SaveSlotUI {
public:
    explicit SaveSlotUI(sf::Font& font);

    // Call before opening to refresh slot metadata from disk
    void refresh();

    // Open the overlay in a given mode; call refresh() first if needed
    void open(SlotUIMode mode);
    void close();
    bool isOpen() const { return open_; }

    SlotUIResult handleEvent(const sf::Event& event);
    void render(sf::RenderWindow& window);

    int        getSelectedSlot()     const { return selectedSlot_; }
    SlotUIMode getMode()             const { return mode_; }
    Difficulty getChosenDifficulty() const { return chosenDifficulty_; }

private:
    sf::Font& font_;
    bool      open_         = false;
    int       selectedSlot_ = 0;
    SlotUIMode mode_        = SlotUIMode::SAVE;

    struct SlotInfo {
        bool       exists     = false;
        int        level      = 0;
        int        roomIndex  = 0;
        int        coins      = 0;
        float      playTime   = 0.f;
        Difficulty difficulty = Difficulty::NORMAL;
    };
    std::array<SlotInfo, SAVE_SLOT_COUNT> slots_;

    // Delete-confirmation state
    bool confirmingDelete_ = false;   // true = showing "Delete? Y/N"
    int  confirmSel_       = 1;       // 0 = Yes, 1 = No

    // Difficulty picker state (NEW_GAME mode only)
    bool       pickingDifficulty_ = false;
    int        difficultySel_     = 1;  // 0=Easy 1=Normal 2=Hard
    Difficulty chosenDifficulty_  = Difficulty::NORMAL;

    float glowTimer_ = 0.f;

    void drawPanel(sf::RenderWindow& w,
                   float x, float y, float pw, float ph,
                   sf::Color fill, sf::Color outline);
    void drawSlotCard(sf::RenderWindow& w,
                      int idx, float cardX, float cardY,
                      float cw, float ch, bool selected);
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col, float x, float y);
};

#endif
