#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <string>
#include <array>
#include <vector>

static constexpr int SAVE_SKILL_COUNT = 6;
static constexpr int SAVE_SLOT_COUNT  = 3;

enum class Difficulty : int {
    EASY   = 0,
    NORMAL = 1,
    HARD   = 2
};

inline const char* difficultyName(Difficulty d) {
    switch (d) {
    case Difficulty::EASY:   return "EASY";
    case Difficulty::NORMAL: return "NORMAL";
    case Difficulty::HARD:   return "HARD";
    default:                 return "NORMAL";
    }
}

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
    bool  totemBoughtThisRun = false;

    // Run state
    int   roomIndex     = 0;
    int   coins         = 0;
    float playTime      = 0.f;   // total play time in seconds
    bool  roomCleared   = false; // whether current room was already cleared
    int   enemiesLeft   = 0;     // enemies still waiting to spawn in this room
    std::vector<int> roomLayouts; // template index per room (RoomTemplates::BOSS_LAYOUT = boss)
    std::string heldItem;

    // Options
    bool  fullscreen    = false;
    int   musicVolume   = 100;
    int   sfxVolume     = 70;

    // Difficulty — locked at new-game creation
    Difficulty difficulty = Difficulty::NORMAL;
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