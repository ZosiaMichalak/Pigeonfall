#ifndef ROOM_H
#define ROOM_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ── Enemy spawn descriptor ────────────────────────────────────────────────────
enum class EnemyType { BULLET, DASH };

struct EnemySpawn {
    EnemyType    type;
    sf::Vector2f position;
    int          tier = 0;
};

// ── Prop types ────────────────────────────────────────────────────────────────
enum class PropType { VENDING, TRASH };

// ── Prop descriptor ───────────────────────────────────────────────────────────
// Collision hitbox is derived from PropType automatically.
struct PropDef {
    PropType     type;
    sf::Vector2f position;   // top-left corner in game coords (400x225 space)
};

// ── Background index (maps to assets/B1.png … B5.png) ────────────────────────
//   0 = B1, 1 = B2, 2 = B3, 3 = B4, 4 = B5
using BackgroundIndex = int;

// ── Room template (the "recipe") ──────────────────────────────────────────────
struct RoomTemplate {
    std::string              name;
    BackgroundIndex          background;  // 0-4
    std::vector<EnemySpawn>  enemies;
    std::vector<PropDef>     props;       // vending (max 1) + trash (max 2)
};

// ── Catalogue ─────────────────────────────────────────────────────────────────
namespace RoomTemplates {
    std::vector<RoomTemplate> getAll();
    RoomTemplate getRandom();
    RoomTemplate getByName(const std::string& name);
}

// ── Runtime room instance ─────────────────────────────────────────────────────
class Room {
public:
    // Legacy constructor (no template, random enemy count)
    Room(int id, int enemyCount, sf::Color fallbackColor);

    // Template constructor
    explicit Room(int id, const RoomTemplate& tmpl);

    int  getId()         const { return id; }
    int  getEnemyCount() const { return enemyCount; }
    bool getIsCleared()  const { return isCleared; }

    // Still exposed for the HUD floor draw fallback
    sf::Color getFloorColor() const { return floorColor; }

    void setEnemyCount(int c) { enemyCount = c; }
    void setCleared(bool c)   { isCleared  = c; }

    const std::vector<EnemySpawn>& getEnemySpawns() const { return enemySpawns; }

    // Returns AABB collision rects for all props (used by Game for player push-out)
    std::vector<sf::FloatRect> getPropColliders() const;

    // Load textures (call once after construction; safe to call multiple times)
    void loadAssets();

    // Draw background + props
    void draw(sf::RenderWindow& window) const;

private:
    int           id;
    int           enemyCount;
    bool          isCleared;
    sf::Color     floorColor;   // fallback when no bg texture
    BackgroundIndex bgIndex;
    bool          hasBackground;

    std::vector<EnemySpawn> enemySpawns;
    std::vector<PropDef>    props;

    // Textures (loaded by loadAssets)
    sf::Texture bgTexture;
    sf::Sprite  bgSprite;

    sf::Texture vendingTexture;
    sf::Texture trashTexture;
    bool        vendingLoaded;
    bool        trashLoaded;
};

#endif
