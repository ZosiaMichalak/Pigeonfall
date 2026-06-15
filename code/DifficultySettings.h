/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef DIFFICULTY_SETTINGS_H
#define DIFFICULTY_SETTINGS_H

// Represents the game's difficulty options: EASY, NORMAL, or HARD.
enum class Difficulty : int {
    EASY   = 0,
    NORMAL = 1,
    HARD   = 2
};

// Returns a user-friendly string representation of the chosen Difficulty level.
inline const char* difficultyName(Difficulty d) {
    switch (d) {
    case Difficulty::EASY:   return "EASY";
    case Difficulty::NORMAL: return "NORMAL";
    case Difficulty::HARD:   return "HARD";
    default:                 return "NORMAL";
    }
}

// Struct containing all scale factors and game parameters modified by selected difficulty.
struct DifficultySettings {

    // Multipliers for enemies, boss, player stats, and economy scaling
    float enemyHpMult      = 1.f;
    float enemySpeedMult   = 1.f;
    float bulletSpeedMult  = 1.f;
    float shootCooldownMult = 1.f;
    float dashWindupMult   = 1.f;
    float dashSpeedMult    = 1.f;
    float dashCdMult       = 1.f;
    float bossHpMult       = 1.f;
    float bossDiveSpeedMult = 1.f;
    float bossFeatherSpeedMult = 1.f;
    float bossVulnerableMult = 1.f;
    float bossWarnMult     = 1.f;
    float enemyCountMult   = 1.f;
    float bulletSpreadMult = 1.f;
    float playerHpMult     = 1.f;
    float invincibilityMult = 1.f;
    float xpGainMult       = 1.f;
    float coinDropMult     = 1.f;

    // Factory method that generates difficulty settings based on Difficulty mode.
    static DifficultySettings get(Difficulty d) {
        DifficultySettings s;
        switch (d) {
        case Difficulty::EASY:
            s.enemyHpMult          = 0.80f; // Enemies have lower HP
            s.enemySpeedMult       = 0.88f; // Enemies walk slower
            s.bulletSpeedMult      = 0.88f; // Enemy projectiles travel slower
            s.shootCooldownMult    = 1.20f; // Enemies shoot less frequently
            s.dashWindupMult       = 1.20f; // Dash windups last longer (easier to dodge)
            s.dashSpeedMult        = 0.90f; // Enemies dash slower
            s.dashCdMult           = 1.15f; // Dash cooldown is longer
            s.bossHpMult           = 0.85f; // Boss is easier to defeat
            s.bossDiveSpeedMult    = 0.88f; 
            s.bossFeatherSpeedMult = 0.88f;
            s.bossVulnerableMult   = 1.20f; // Boss remains vulnerable longer
            s.bossWarnMult         = 1.25f; // Boss warnings are longer
            s.enemyCountMult       = 0.80f; // Fewer enemies spawn
            s.bulletSpreadMult     = 1.15f; 
            s.playerHpMult         = 1.40f; // Player gets more health
            s.invincibilityMult    = 1.25f; // Longer post-hit invincibility
            s.xpGainMult           = 1.10f; // Faster leveling
            s.coinDropMult         = 1.15f; // More coins dropped
            break;

        case Difficulty::NORMAL:
            s.enemyHpMult          = 1.00f; // Standard enemy health
            s.enemySpeedMult       = 0.95f;
            s.bulletSpeedMult      = 0.95f;
            s.shootCooldownMult    = 1.05f;
            s.dashWindupMult       = 1.05f;
            s.dashSpeedMult        = 1.05f;
            s.dashCdMult           = 0.95f;
            s.bossHpMult           = 1.05f;
            s.bossDiveSpeedMult    = 1.00f;
            s.bossFeatherSpeedMult = 0.98f;
            s.bossVulnerableMult   = 1.05f;
            s.bossWarnMult         = 1.00f;
            s.enemyCountMult       = 0.95f;
            s.bulletSpreadMult     = 1.05f;
            s.playerHpMult         = 1.20f;
            s.invincibilityMult    = 1.10f;
            s.xpGainMult           = 1.10f;
            s.coinDropMult         = 1.05f;
            break;

        case Difficulty::HARD:
            s.enemyHpMult          = 1.30f; // High enemy HP
            s.enemySpeedMult       = 1.10f; // Faster enemies
            s.bulletSpeedMult      = 1.10f; // Faster bullets
            s.shootCooldownMult    = 0.80f; // High shooting rate
            s.dashWindupMult       = 0.80f; // Faster dash warnings
            s.dashSpeedMult        = 1.15f;
            s.dashCdMult           = 0.75f;
            s.bossHpMult           = 1.35f; // Extra tanky boss
            s.bossDiveSpeedMult    = 1.12f;
            s.bossFeatherSpeedMult = 1.10f;
            s.bossVulnerableMult   = 0.85f;
            s.bossWarnMult         = 0.75f;
            s.enemyCountMult       = 1.15f; // More enemies spawn
            s.bulletSpreadMult     = 0.85f;
            s.playerHpMult         = 1.05f; // Player has less health
            s.invincibilityMult    = 0.95f; // Shorter invincibility window
            s.xpGainMult           = 1.15f;
            s.coinDropMult         = 1.00f;
            break;
        }
        return s;
    }
};

// Global active difficulty settings state.
namespace ActiveDifficulty {
    inline Difficulty         current  = Difficulty::NORMAL;
    inline DifficultySettings settings = DifficultySettings::get(Difficulty::NORMAL);

    // Updates the global difficulty state and recalculates active game modifiers.
    inline void set(Difficulty d) {
        current  = d;
        settings = DifficultySettings::get(d);
    }
}

#endif // DIFFICULTY_SETTINGS_H
