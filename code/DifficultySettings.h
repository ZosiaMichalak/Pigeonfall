#ifndef DIFFICULTY_SETTINGS_H
#define DIFFICULTY_SETTINGS_H

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


struct DifficultySettings {

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

    static DifficultySettings get(Difficulty d) {
        DifficultySettings s;
        switch (d) {
        case Difficulty::EASY:
            s.enemyHpMult          = 0.80f;
            s.enemySpeedMult       = 0.88f;
            s.bulletSpeedMult      = 0.88f;
            s.shootCooldownMult    = 1.20f;
            s.dashWindupMult       = 1.20f;
            s.dashSpeedMult        = 0.90f;
            s.dashCdMult           = 1.15f;
            s.bossHpMult           = 0.85f;
            s.bossDiveSpeedMult    = 0.88f;
            s.bossFeatherSpeedMult = 0.88f;
            s.bossVulnerableMult   = 1.20f;
            s.bossWarnMult         = 1.25f;
            s.enemyCountMult       = 0.80f;
            s.bulletSpreadMult     = 1.15f;
            s.playerHpMult         = 1.40f;
            s.invincibilityMult    = 1.25f;
            s.xpGainMult           = 1.10f;
            s.coinDropMult         = 1.15f;
            break;

        case Difficulty::NORMAL:
            s.enemyHpMult          = 1.00f;
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
            s.enemyHpMult          = 1.30f;
            s.enemySpeedMult       = 1.10f;
            s.bulletSpeedMult      = 1.10f;
            s.shootCooldownMult    = 0.80f;
            s.dashWindupMult       = 0.80f;
            s.dashSpeedMult        = 1.15f;
            s.dashCdMult           = 0.75f;
            s.bossHpMult           = 1.35f;
            s.bossDiveSpeedMult    = 1.12f;
            s.bossFeatherSpeedMult = 1.10f;
            s.bossVulnerableMult   = 0.85f;
            s.bossWarnMult         = 0.75f;
            s.enemyCountMult       = 1.15f;
            s.bulletSpreadMult     = 0.85f;
            s.playerHpMult         = 1.05f;
            s.invincibilityMult    = 0.95f;
            s.xpGainMult           = 1.15f;
            s.coinDropMult         = 1.00f;
            break;
        }
        return s;
    }
};

namespace ActiveDifficulty {
    inline Difficulty         current  = Difficulty::NORMAL;
    inline DifficultySettings settings = DifficultySettings::get(Difficulty::NORMAL);

    inline void set(Difficulty d) {
        current  = d;
        settings = DifficultySettings::get(d);
    }
}

#endif // DIFFICULTY_SETTINGS_H
