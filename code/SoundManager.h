/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <SFML/Audio.hpp>
#include <array>

// List of all sound effects (SFX) available in the game.
enum class SFX {
    COIN,
    DASH,
    HIT,          // Picks hit1/hit2/hit3 randomly
    SHOOT,
    STEPS,        // Caller is responsible for calling on a timer
    SWORD_SWING,
    VENDING_OPEN,
    COUNT
};

// Manager class that preloads audio buffers and plays sound effects using a pool of reusable sf::Sound channels.
class SoundManager {
public:
    // Constructor: loads all SFX audio files from disk into memory buffers
    SoundManager();

    // Plays the requested Sound Effect (SFX) using an available audio slot
    void play(SFX sfx);

    // Sets the global SFX volume level (clamped between 0 and 100)
    void setVolume(int vol);

    // Retrieves current sound effects volume level
    int  getVolume() const { return volume; }

private:
    // Preloaded sound buffers stored in RAM
    sf::SoundBuffer bufCoin;
    sf::SoundBuffer bufDash;
    sf::SoundBuffer bufHit1, bufHit2, bufHit3;
    sf::SoundBuffer bufShoot;
    sf::SoundBuffer bufSteps;
    sf::SoundBuffer bufSword;
    sf::SoundBuffer bufVending;

    // Concurrent sound slots: SFML requires sf::Sound objects to remain in memory while playing
    static constexpr int SLOTS = 16;
    std::array<sf::Sound, SLOTS> slots;
    int  nextSlot = 0; // Index for round-robin slot replacement when all are busy

    int  volume   = 70;   // Default SFX volume (quieter than background music)
    int  hitCycle = 0;    // Rotates through hit1/hit2/hit3 to avoid sound repetition

    // Finds or steals a sound slot that is currently not playing
    sf::Sound& getFreeSlot();

    // Helper function that configures a slot with the given buffer and plays it
    void playBuffer(sf::SoundBuffer& buf);
};

#endif
