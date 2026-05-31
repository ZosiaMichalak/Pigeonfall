#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <string>
#include <array>

static constexpr int SAVE_SKILL_COUNT = 6;
static constexpr int SAVE_SLOT_COUNT  = 3;

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
    float playTime      = 0.f;   // total play time in seconds
    bool  roomCleared   = false; // whether current room was already cleared
    std::string heldItem;

    // Options
    bool  fullscreen    = false;
    int   musicVolume   = 100;
    int   sfxVolume     = 70;
};

class SaveSystem {
public:
    // Returns e.g. "save0.dat", "save1.dat", "save2.dat"
    static std::string slotPath(int slot);

    static bool     save(const SaveData& data, int slot);
    static SaveData load(int slot);
    static bool     hasSlot(int slot);
    static void     deleteSlot(int slot);

    // Legacy single-file helpers (slot 0)
    static bool     save(const SaveData& data)  { return save(data, 0); }
    static SaveData load()                       { return load(0); }
    static bool     hasSave()                   { return hasSlot(0); }
    static void     deleteSave()                { deleteSlot(0); }
};

#endif