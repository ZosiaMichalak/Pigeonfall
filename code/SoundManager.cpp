/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "SoundManager.h"
#include <iostream>
#include <cstdlib>

// Constructor: Loads all SFX resources from the "SFX/" subdirectory on startup.
SoundManager::SoundManager() {
    auto load = [](sf::SoundBuffer& buf, const char* path) {
        if (!buf.loadFromFile(path))
            std::cerr << "[SoundManager] Failed to load: " << path << "\n";
    };

    load(bufCoin,    "SFX/coin.wav");
    load(bufDash,    "SFX/dash.wav");
    load(bufHit1,    "SFX/hit1.wav");
    load(bufHit2,    "SFX/hit2.wav");
    load(bufHit3,    "SFX/hit3.wav");
    load(bufShoot,   "SFX/shoot.wav");
    load(bufSteps,   "SFX/Steps.wav");
    load(bufSword,   "SFX/swordSwing.wav");
    load(bufVending, "SFX/vendingOpen.wav");
}

// Scans the sound array to find an idle (stopped) channel, falling back to a round-robin style takeover if full.
sf::Sound& SoundManager::getFreeSlot() {
    // Find a stopped slot first
    for (auto& s : slots)
        if (s.getStatus() == sf::Sound::Stopped) return s;
    
    // Otherwise round-robin takeover
    sf::Sound& s = slots[nextSlot];
    nextSlot = (nextSlot + 1) % SLOTS;
    return s;
}

// Configures and starts playback of a specific sound buffer.
void SoundManager::playBuffer(sf::SoundBuffer& buf) {
    if (buf.getSampleCount() == 0) return;
    sf::Sound& s = getFreeSlot();
    s.setBuffer(buf);
    s.setVolume(static_cast<float>(volume));
    s.play();
}

// Handles selecting, adjusting volume scaling, and launching the chosen sound effect.
void SoundManager::play(SFX sfx) {
    switch (sfx) {
        case SFX::COIN: {
            playBuffer(bufCoin);
            sf::Sound& s = getFreeSlot();
            s.setVolume(std::min(100.f, static_cast<float>(volume) * 0.6f));
            s.play();
            break;
        }         
        case SFX::DASH:        playBuffer(bufDash);   break;
        case SFX::HIT: {
            // Cycle through hit1 / hit2 / hit3 to randomize hit sounds
            hitCycle = (hitCycle + 1) % 3;
            if      (hitCycle == 0) playBuffer(bufHit1);
            else if (hitCycle == 1) playBuffer(bufHit2);
            else                    playBuffer(bufHit3);
            break;
        }
        case SFX::SHOOT: {
            if (bufShoot.getSampleCount() == 0) break;
            sf::Sound& s = getFreeSlot();
            s.setBuffer(bufShoot);
            s.setVolume(std::min(100.f, static_cast<float>(volume) * 1.6f));
            break;
        }
        case SFX::STEPS:       playBuffer(bufSteps);  break;
        case SFX::SWORD_SWING: {
            if (bufSword.getSampleCount() == 0) break;
            sf::Sound& s = getFreeSlot();
            s.setBuffer(bufSword);
            s.setVolume(static_cast<float>(volume) * 0.5f);
            s.play();
            break;
        }
        case SFX::VENDING_OPEN:playBuffer(bufVending);break;
        default: break;
    }
}

// Sets SFX volume dynamically for active sounds as well as future ones.
void SoundManager::setVolume(int vol) {
    volume = vol;
    // Update any currently playing sounds
    for (auto& s : slots)
        s.setVolume(static_cast<float>(volume));
}
