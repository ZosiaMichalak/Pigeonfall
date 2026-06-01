#include "SaveSystem.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static constexpr uint32_t MAGIC   = 0xB2EAD0C5u;
static constexpr uint32_t VERSION = 8u;

static void ensureSaveDir() {
#if defined(_WIN32)
    _mkdir("saves");
#else
    mkdir("saves", 0755);
#endif
}

static std::string legacySlotPath(int slot) {
    return "save" + std::to_string(slot) + ".dat";
}

std::string SaveSystem::slotPath(int slot) {
    ensureSaveDir();
    return "saves/save" + std::to_string(slot) + ".dat";
}

static std::ifstream openSlotForRead(int slot) {
    std::string primary = SaveSystem::slotPath(slot);
    std::ifstream f(primary, std::ios::binary);
    if (f) return f;
    return std::ifstream(legacySlotPath(slot), std::ios::binary);
}

bool SaveSystem::save(const SaveData& data, int slot) {
    std::string path = slotPath(slot);
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) {
        std::cerr << "[SaveSystem] Cannot open " << path << " for writing.\n";
        return false;
    }

    auto write = [&](const void* ptr, std::size_t n) {
        f.write(reinterpret_cast<const char*>(ptr), n);
    };

    write(&MAGIC,   sizeof(MAGIC));
    write(&VERSION, sizeof(VERSION));

    write(&data.level,             sizeof(data.level));
    write(&data.xp,                sizeof(data.xp));
    write(&data.xpToNext,          sizeof(data.xpToNext));
    write(&data.skillPoints,       sizeof(data.skillPoints));
    write(data.upgrades.data(),    sizeof(int) * SAVE_SKILL_COUNT);
    write(&data.secondChanceUsed,  sizeof(data.secondChanceUsed));
    write(&data.totemCharges,      sizeof(data.totemCharges));
    write(&data.totemBoughtThisRun, sizeof(data.totemBoughtThisRun));
    write(&data.roomIndex,         sizeof(data.roomIndex));
    write(&data.coins,             sizeof(data.coins));
    write(&data.playTime,          sizeof(data.playTime));
    write(&data.roomCleared,       sizeof(data.roomCleared));
    write(&data.enemiesLeft,       sizeof(data.enemiesLeft));

    uint32_t heldLen = static_cast<uint32_t>(data.heldItem.size());
    write(&heldLen, sizeof(heldLen));
    if (heldLen > 0) write(data.heldItem.data(), heldLen);

    write(&data.fullscreen,   sizeof(data.fullscreen));
    write(&data.musicVolume,  sizeof(data.musicVolume));
    write(&data.sfxVolume,    sizeof(data.sfxVolume));

    int diffInt = static_cast<int>(data.difficulty);
    write(&diffInt, sizeof(diffInt));

    int32_t layoutCount = static_cast<int32_t>(data.roomLayouts.size());
    write(&layoutCount, sizeof(layoutCount));
    if (layoutCount > 0)
        write(data.roomLayouts.data(), sizeof(int) * static_cast<size_t>(layoutCount));

    f.flush();
    std::cout << "[SaveSystem] Saved slot " << slot << " room=" << data.roomIndex
              << " (" << path << ")\n";
    return static_cast<bool>(f);
}

SaveData SaveSystem::load(int slot) {
    SaveData out;
    std::ifstream f = openSlotForRead(slot);
    if (!f) return out;

    auto read = [&](void* ptr, std::size_t n) {
        f.read(reinterpret_cast<char*>(ptr), n);
    };

    uint32_t magic = 0, version = 0;
    read(&magic,   sizeof(magic));
    read(&version, sizeof(version));

    if (magic != MAGIC || (version != VERSION && version != 7u && version != 6u && version != 5u)) {
        std::cerr << "[SaveSystem] Slot " << slot << " invalid or version mismatch.\n";
        return out;
    }

    read(&out.level,            sizeof(out.level));
    read(&out.xp,               sizeof(out.xp));
    read(&out.xpToNext,         sizeof(out.xpToNext));
    read(&out.skillPoints,      sizeof(out.skillPoints));
    read(out.upgrades.data(),   sizeof(int) * SAVE_SKILL_COUNT);
    read(&out.secondChanceUsed, sizeof(out.secondChanceUsed));
    read(&out.totemCharges,     sizeof(out.totemCharges));
    if (version >= 8u)
        read(&out.totemBoughtThisRun, sizeof(out.totemBoughtThisRun));
    else
        out.totemBoughtThisRun = (out.totemCharges > 0);
    read(&out.roomIndex,        sizeof(out.roomIndex));
    read(&out.coins,            sizeof(out.coins));
    read(&out.playTime,         sizeof(out.playTime));
    read(&out.roomCleared,      sizeof(out.roomCleared));
    if (version >= 6u)
        read(&out.enemiesLeft,  sizeof(out.enemiesLeft));
    else
        out.enemiesLeft = 0;

    uint32_t heldLen = 0;
    read(&heldLen, sizeof(heldLen));
    if (heldLen > 0 && heldLen < 256) {
        out.heldItem.resize(heldLen);
        read(&out.heldItem[0], heldLen);
    }

    read(&out.fullscreen,  sizeof(out.fullscreen));
    read(&out.musicVolume, sizeof(out.musicVolume));
    read(&out.sfxVolume,   sizeof(out.sfxVolume));

    int diffInt = 1;
    read(&diffInt, sizeof(diffInt));
    out.difficulty = static_cast<Difficulty>(diffInt);

    if (version >= 7u) {
        int32_t layoutCount = 0;
        read(&layoutCount, sizeof(layoutCount));
        if (layoutCount > 0 && layoutCount < 256) {
            out.roomLayouts.resize(static_cast<size_t>(layoutCount));
            read(out.roomLayouts.data(), sizeof(int) * static_cast<size_t>(layoutCount));
        }
    }

    // good() is false when eof is set after the last read — use fail() instead
    if (!f.fail()) {
        out.exists = true;
        std::cout << "[SaveSystem] Loaded slot " << slot << " room=" << out.roomIndex << "\n";
    }
    return out;
}

bool SaveSystem::hasSlot(int slot) {
    std::ifstream f = openSlotForRead(slot);
    return static_cast<bool>(f);
}

void SaveSystem::deleteSlot(int slot) {
    std::remove(slotPath(slot).c_str());
    std::remove(legacySlotPath(slot).c_str());
    std::cout << "[SaveSystem] Slot " << slot << " deleted.\n";
}