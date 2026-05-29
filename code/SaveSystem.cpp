#include "SaveSystem.h"
#include <fstream>
#include <cstring>
#include <iostream>

static constexpr uint32_t MAGIC   = 0xB2EA'D0C5u; // "BREAD" marker
static constexpr uint32_t VERSION = 2u;

bool SaveSystem::save(const SaveData& data) {
    std::ofstream f(SAVE_PATH, std::ios::binary | std::ios::trunc);
    if (!f) {
        std::cerr << "[SaveSystem] Cannot open " << SAVE_PATH << " for writing.\n";
        return false;
    }

    auto write = [&](const void* ptr, std::size_t n) { f.write(reinterpret_cast<const char*>(ptr), n); };

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

    uint32_t heldLen = static_cast<uint32_t>(data.heldItem.size());
    write(&heldLen,                sizeof(heldLen));
    if (heldLen > 0) write(data.heldItem.data(), heldLen);

    write(&data.fullscreen,        sizeof(data.fullscreen));
    write(&data.musicVolume,       sizeof(data.musicVolume));

    std::cout << "[SaveSystem] Saved to " << SAVE_PATH << "\n";
    return true;
}

SaveData SaveSystem::load() {
    SaveData out;
    std::ifstream f(SAVE_PATH, std::ios::binary);
    if (!f) return out;

    auto read = [&](void* ptr, std::size_t n) { f.read(reinterpret_cast<char*>(ptr), n); };

    uint32_t magic = 0, version = 0;
    read(&magic,   sizeof(magic));
    read(&version, sizeof(version));

    if (magic != MAGIC || version != VERSION) {
        std::cerr << "[SaveSystem] Save file invalid or version mismatch.\n";
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

    uint32_t heldLen = 0;
    read(&heldLen, sizeof(heldLen));
    if (heldLen > 0 && heldLen < 256) {
        out.heldItem.resize(heldLen);
        // Zmiana z out.heldItem.data() na &out.heldItem[0] rozwiązuje błąd const_cast
        read(&out.heldItem[0], heldLen);
    }

    read(&out.fullscreen,   sizeof(out.fullscreen));
    read(&out.musicVolume,  sizeof(out.musicVolume));

    if (f.good()) {
        out.exists = true;
        std::cout << "[SaveSystem] Loaded from " << SAVE_PATH << "\n";
    }
    return out;
}

bool SaveSystem::hasSave() {
    std::ifstream f(SAVE_PATH, std::ios::binary);
    return f.good();
}

void SaveSystem::deleteSave() {
    std::remove(SAVE_PATH);
    std::cout << "[SaveSystem] Save deleted.\n";
}