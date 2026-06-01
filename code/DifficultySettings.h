#ifndef DIFFICULTY_SETTINGS_H
#define DIFFICULTY_SETTINGS_H

#include "SaveSystem.h"   // for Difficulty enum

// Deklaracja wyprzedzająca (Forward declaration)
struct DifficultySettings;

// ─────────────────────────────────────────────────────────────────────────────
// DifficultySettings
//
// Single source of truth for every number that changes between Easy / Normal /
// Hard.  Pass a Difficulty value once and read the fields you need.
//
// Naming convention: values > 1 = "more than normal", < 1 = "less than normal"
// ─────────────────────────────────────────────────────────────────────────────
struct DifficultySettings {

    // ── Enemy HP multiplier ───────────────────────────────────────────────────
    float enemyHpMult      = 1.f;

    // ── Enemy move speed multiplier ───────────────────────────────────────────
    float enemySpeedMult   = 1.f;

    // ── Enemy bullet speed multiplier ─────────────────────────────────────────
    float bulletSpeedMult  = 1.f;

    // ── Enemy shoot cooldown multiplier (< 1 = shoots faster) ────────────────
    float shootCooldownMult = 1.f;

    // ── DashEnemy: wind-up duration multiplier (< 1 = shorter = harder) ──────
    float dashWindupMult   = 1.f;

    // ── DashEnemy: dash speed multiplier ─────────────────────────────────────
    float dashSpeedMult    = 1.f;

    // ── DashEnemy: dash cooldown multiplier (< 1 = dashes more often) ────────
    float dashCdMult       = 1.f;

    // ── Boss HP multiplier ────────────────────────────────────────────────────
    float bossHpMult       = 1.f;

    // ── Boss dive speed multiplier ────────────────────────────────────────────
    float bossDiveSpeedMult = 1.f;

    // ── Boss feather bullet speed multiplier ──────────────────────────────────
    float bossFeatherSpeedMult = 1.f;

    // ── Boss vulnerable window multiplier (< 1 = less time to hit boss) ──────
    float bossVulnerableMult = 1.f;

    // ── Boss wave warning duration multiplier (< 1 = less warning) ───────────
    float bossWarnMult     = 1.f;

    // ── Enemies per room multiplier ───────────────────────────────────────────
    float enemyCountMult   = 1.f;

    // ── Enemy bullet spread (lower = more accurate, Hard only) ───────────────
    float bulletSpreadMult = 1.f;

    // ── Player max HP multiplier ──────────────────────────────────────────────
    float playerHpMult     = 1.f;

    // ── Player invincibility window after taking damage multiplier ────────────
    float invincibilityMult = 1.f;

    // ── Player XP gain multiplier ─────────────────────────────────────────────
    float xpGainMult       = 1.f;

    // ── Coin drop multiplier ──────────────────────────────────────────────────
    float coinDropMult     = 1.f;

    // ── Factory ───────────────────────────────────────────────────────────────
    static DifficultySettings get(Difficulty d) {
        DifficultySettings s;
        switch (d) {
        case Difficulty::EASY:
            s.enemyHpMult          = 0.65f;
            s.enemySpeedMult       = 0.75f;
            s.bulletSpeedMult      = 0.70f;
            s.shootCooldownMult    = 1.50f;
            s.dashWindupMult       = 1.50f;
            s.dashSpeedMult        = 0.75f;
            s.dashCdMult           = 1.40f;
            s.bossHpMult           = 0.60f;
            s.bossDiveSpeedMult    = 0.75f;
            s.bossFeatherSpeedMult = 0.70f;
            s.bossVulnerableMult   = 1.60f;
            s.bossWarnMult         = 1.50f;
            s.enemyCountMult       = 0.75f;
            s.bulletSpreadMult     = 1.60f;
            s.playerHpMult         = 1.50f;
            s.invincibilityMult    = 1.40f;
            s.xpGainMult           = 0.80f;
            s.coinDropMult         = 1.20f;
            break;

        case Difficulty::NORMAL:
            break;

        case Difficulty::HARD:
            s.enemyHpMult          = 1.50f;
            s.enemySpeedMult       = 1.25f;
            s.bulletSpeedMult      = 1.30f;
            s.shootCooldownMult    = 0.65f;
            s.dashWindupMult       = 0.65f;
            s.dashSpeedMult        = 1.35f;
            s.dashCdMult           = 0.65f;
            s.bossHpMult           = 1.60f;
            s.bossDiveSpeedMult    = 1.35f;
            s.bossFeatherSpeedMult = 1.35f;
            s.bossVulnerableMult   = 0.65f;
            s.bossWarnMult         = 0.60f;
            s.enemyCountMult       = 1.40f;
            s.bulletSpreadMult     = 0.55f;
            s.playerHpMult         = 0.80f;
            s.invincibilityMult    = 0.70f;
            s.xpGainMult           = 1.30f;
            s.coinDropMult         = 0.85f;
            break;
        }
        return s;
    }
};

// ── Global active difficulty ──────────────────────────────────────────────────
namespace ActiveDifficulty {
    inline Difficulty            current  = Difficulty::NORMAL;
    // Przemieszczone poniżej definicji struct, by kompilator znał pełny rozmiar i strukturę typu
    inline DifficultySettings    settings = DifficultySettings::get(Difficulty::NORMAL);

    inline void set(Difficulty d) {
        current  = d;
        settings = DifficultySettings::get(d);
    }
}

#endif // DIFFICULTY_SETTINGS_H