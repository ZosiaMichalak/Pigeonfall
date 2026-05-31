#include "SaveSystem.h"
#include <fstream>
#include <iostream>
#include <cstdio>

static constexpr uint32_t MAGIC   = 0xB2EAD0C5u;
static constexpr uint32_t VERSION = 4u;

std::string SaveSystem::slotPath(int slot) {
    return "save" + std::to_string(slot) + ".dat";
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
    write(&data.roomIndex,         sizeof(data.roomIndex));
    write(&data.coins,             sizeof(data.coins));
    write(&data.playTime,          sizeof(data.playTime));
    write(&data.roomCleared,       sizeof(data.roomCleared));

    uint32_t heldLen = static_cast<uint32_t>(data.heldItem.size());
    write(&heldLen, sizeof(heldLen));
    if (heldLen > 0) write(data.heldItem.data(), heldLen);

    write(&data.fullscreen,   sizeof(data.fullscreen));
    write(&data.musicVolume,  sizeof(data.musicVolume));
    write(&data.sfxVolume,    sizeof(data.sfxVolume));

    std::cout << "[SaveSystem] Saved to slot " << slot << " (" << path << ")\n";
    return true;
}

SaveData SaveSystem::load(int slot) {
    SaveData out;
    std::string path = slotPath(slot);
    std::ifstream f(path, std::ios::binary);
    if (!f) return out;

    auto read = [&](void* ptr, std::size_t n) {
        f.read(reinterpret_cast<char*>(ptr), n);
    };

    uint32_t magic = 0, version = 0;
    read(&magic,   sizeof(magic));
    read(&version, sizeof(version));

    if (magic != MAGIC || version != VERSION) {
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
    read(&out.roomIndex,        sizeof(out.roomIndex));
    read(&out.coins,            sizeof(out.coins));
    read(&out.playTime,         sizeof(out.playTime));
    read(&out.roomCleared,      sizeof(out.roomCleared));

    uint32_t heldLen = 0;
    read(&heldLen, sizeof(heldLen));
    if (heldLen > 0 && heldLen < 256) {
        out.heldItem.resize(heldLen);
        read(&out.heldItem[0], heldLen);
    }

    read(&out.fullscreen,  sizeof(out.fullscreen));
    read(&out.musicVolume, sizeof(out.musicVolume));
    read(&out.sfxVolume,   sizeof(out.sfxVolume));

    if (f.good()) {
        out.exists = true;
        std::cout << "[SaveSystem] Loaded from slot " << slot << " (" << path << ")\n";
    }
    return out;
}

bool SaveSystem::hasSlot(int slot) {
    std::ifstream f(slotPath(slot), std::ios::binary);
    return f.good();
}

void SaveSystem::deleteSlot(int slot) {
    std::remove(slotPath(slot).c_str());
    std::cout << "[SaveSystem] Slot " << slot << " deleted.\n";
}