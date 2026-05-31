#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <SFML/Audio.hpp>
#include <array>

enum class SFX {
    COIN,
    DASH,
    HIT,          // picks hit1/hit2/hit3 randomly
    SHOOT,
    STEPS,        // caller is responsible for calling on a timer
    SWORD_SWING,
    VENDING_OPEN,
    COUNT
};

class SoundManager {
public:
    SoundManager();

    void play(SFX sfx);
    void setVolume(int vol);   // 0-100
    int  getVolume() const { return volume; }

private:
    // Three hit buffers + one slot per other SFX
    sf::SoundBuffer bufCoin;
    sf::SoundBuffer bufDash;
    sf::SoundBuffer bufHit1, bufHit2, bufHit3;
    sf::SoundBuffer bufShoot;
    sf::SoundBuffer bufSteps;
    sf::SoundBuffer bufSword;
    sf::SoundBuffer bufVending;

    // Concurrent sound slots — SFML needs the sf::Sound alive while playing
    static constexpr int SLOTS = 16;
    std::array<sf::Sound, SLOTS> slots;
    int  nextSlot = 0;

    int  volume   = 70;   // default: quieter than music
    int  hitCycle = 0;    // round-robins hit1/hit2/hit3

    sf::Sound& getFreeSlot();
    void playBuffer(sf::SoundBuffer& buf);
};

#endif
