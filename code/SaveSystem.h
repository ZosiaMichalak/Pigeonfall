#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <string>
#include <array>

static constexpr int SAVE_SKILL_COUNT = 6;

struct SaveData {
    bool  exists        = false;

    // Player persistent stats
    int   level         = 1;
    int   xp            = 0;
    int   xpToNext      = 10;
    int   skillPoints   = 0;
    std::array<int, SAVE_SKILL_COUNT> upgrades = {};
    bool  secondChanceUsed = false;
    int   totemCharges  = 0;

    // Run state
    int   roomIndex     = 0;
    int   coins         = 0;
    std::string heldItem;

    // Options
    bool  fullscreen    = false;
    int   musicVolume   = 100;  // 0-100, reserved for future use
};

class SaveSystem {
public:
    static constexpr const char* SAVE_PATH = "save.dat";

    static bool    save(const SaveData& data);
    static SaveData load();
    static bool    hasSave();
    static void    deleteSave();
};

#endif
