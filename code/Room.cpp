#include "Room.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
//  Prop collision sizes in game coords (400x225 space)
//  Assets are 2x the game resolution, so we scale sprites by 0.5
//  vending.png: 60x70 px  →  30x35 game units
//  trash.png:   32x32 px  →  16x16 game units
// ─────────────────────────────────────────────────────────────────────────────
static sf::Vector2f propSize(PropType t) {
    switch (t) {
        case PropType::VENDING: return { 30.f, 35.f };
        case PropType::TRASH:   return { 16.f, 16.f };
    }
    return { 16.f, 16.f };
}

// ═════════════════════════════════════════════════════════════════════════════
//  ROOM TEMPLATE CATALOGUE
//
//  Play area:  x 0–400,  y 0–185  (bottom 30 px = HUD, y>185 is off-limits)
//  Player starts near x=20, y=100. Keep enemies away from that corner.
//
//  Props:
//    VENDING  – max 1 per room,  size 30x35
//    TRASH    – max 2 per room,  size 16x16
//
//  Backgrounds:  0=B1 (straight road)  1=B2 (T-down)  2=B3 (crossroad)
//                3=B4 (bend L→down)    4=B5 (bend R→down)
// ═════════════════════════════════════════════════════════════════════════════

std::vector<RoomTemplate> RoomTemplates::getAll() {
    return {

        // ── 1. Open field ─────────────────────────────────────────────────────
        {
            "open_field", 0,   // B1 straight road
            {
                { EnemyType::BULLET, {300.f,  80.f}, 0 },
                { EnemyType::BULLET, {350.f, 130.f}, 0 },
                { EnemyType::DASH,   {250.f,  60.f}, 0 },
            },
            {
                { PropType::TRASH,   { 60.f,  30.f} },
                { PropType::TRASH,   {320.f, 155.f} },
            }
        },

        // ── 2. T-junction ─────────────────────────────────────────────────────
        {
            "t_junction", 1,   // B2 T-down
            {
                { EnemyType::BULLET, {320.f,  50.f}, 1 },
                { EnemyType::BULLET, {320.f, 150.f}, 1 },
                { EnemyType::DASH,   {370.f, 100.f}, 1 },
            },
            {
                { PropType::VENDING, { 50.f,  25.f} },
                { PropType::TRASH,   {340.f,  25.f} },
            }
        },

        // ── 3. Crossroads ─────────────────────────────────────────────────────
        {
            "crossroads", 2,   // B3 crossroad
            {
                { EnemyType::BULLET, {250.f,  30.f}, 1 },
                { EnemyType::BULLET, {250.f, 160.f}, 1 },
                { EnemyType::DASH,   {340.f,  95.f}, 1 },
                { EnemyType::DASH,   {340.f, 115.f}, 1 },
            },
            {
                { PropType::TRASH,   { 55.f,  25.f} },
                { PropType::TRASH,   { 55.f, 150.f} },
            }
        },

        // ── 4. Left bend ─────────────────────────────────────────────────────
        {
            "bend_left", 3,    // B4 bend L→down
            {
                { EnemyType::BULLET, {370.f,  40.f}, 2 },
                { EnemyType::BULLET, {370.f, 160.f}, 2 },
                { EnemyType::DASH,   {200.f, 100.f}, 1 },
            },
            {
                { PropType::VENDING, {330.f,  25.f} },
                { PropType::TRASH,   {150.f,  30.f} },
            }
        },

        // ── 5. Right bend ─────────────────────────────────────────────────────
        {
            "bend_right", 4,   // B5 bend R→down
            {
                { EnemyType::BULLET, {200.f,  30.f}, 2 },
                { EnemyType::BULLET, {200.f, 160.f}, 2 },
                { EnemyType::DASH,   {300.f,  70.f}, 1 },
                { EnemyType::DASH,   {300.f, 130.f}, 1 },
            },
            {
                { PropType::TRASH,   {340.f, 150.f} },
            }
        },

        // ── 6. Sniper alley ───────────────────────────────────────────────────
        {
            "sniper_alley", 0, // B1 straight road
            {
                { EnemyType::BULLET, {370.f,  40.f}, 2 },
                { EnemyType::BULLET, {370.f, 160.f}, 2 },
                { EnemyType::DASH,   {260.f, 100.f}, 1 },
            },
            {
                { PropType::VENDING, { 55.f, 130.f} },
                { PropType::TRASH,   {200.f,  25.f} },
                { PropType::TRASH,   {200.f, 155.f} },
            }
        },

        // ── 7. Boss antechamber ───────────────────────────────────────────────
        {
            "boss_antechamber", 2, // B3 crossroad
            {
                { EnemyType::BULLET, {280.f,  50.f}, 3 },
                { EnemyType::BULLET, {280.f, 150.f}, 3 },
                { EnemyType::DASH,   {360.f,  80.f}, 3 },
                { EnemyType::DASH,   {360.f, 120.f}, 3 },
                { EnemyType::BULLET, {200.f, 100.f}, 3 },
            },
            {
                { PropType::TRASH,   { 55.f,  25.f} },
                { PropType::TRASH,   { 55.f, 155.f} },
            }
        },

    };
}

RoomTemplate RoomTemplates::getRandom() {
    auto all = getAll();
    if (all.empty()) return {};
    return all[static_cast<std::size_t>(std::rand()) % all.size()];
}

RoomTemplate RoomTemplates::getByName(const std::string& name) {
    auto all = getAll();
    auto it  = std::find_if(all.begin(), all.end(),
        [&](const RoomTemplate& t){ return t.name == name; });
    return (it != all.end()) ? *it : all[0];
}

// ═════════════════════════════════════════════════════════════════════════════
//  Room runtime
// ═════════════════════════════════════════════════════════════════════════════

Room::Room(int id, int enemyCount, sf::Color fallbackColor)
    : id(id), enemyCount(enemyCount), isCleared(false),
      floorColor(fallbackColor), bgIndex(0), hasBackground(false),
      vendingLoaded(false), trashLoaded(false)
{}

Room::Room(int id, const RoomTemplate& tmpl)
    : id(id),
      enemyCount(static_cast<int>(tmpl.enemies.size())),
      isCleared(false),
      floorColor(sf::Color(20, 25, 40)),
      bgIndex(tmpl.background),
      hasBackground(false),
      enemySpawns(tmpl.enemies),
      props(tmpl.props),
      vendingLoaded(false),
      trashLoaded(false)
{}

void Room::loadAssets() {
    // Background  (B1.png … B5.png, 800x450, drawn at scale 0.5 → 400x225)
    std::string bgPath = "assets/B" + std::to_string(bgIndex + 1) + ".png";
    hasBackground = bgTexture.loadFromFile(bgPath);
    if (hasBackground) {
        bgSprite.setTexture(bgTexture);
        bgSprite.setScale(0.5f, 0.5f);
        bgSprite.setPosition(0.f, 0.f);
    } else {
        std::cerr << "[Room] Could not load " << bgPath << "\n";
    }

    // Props
    vendingLoaded = vendingTexture.loadFromFile("assets/vending.png");
    trashLoaded   = trashTexture.loadFromFile("assets/trash.png");
}

std::vector<sf::FloatRect> Room::getPropColliders() const {
    std::vector<sf::FloatRect> out;
    out.reserve(props.size());
    for (const auto& p : props) {
        sf::Vector2f sz = propSize(p.type);
        out.emplace_back(p.position.x, p.position.y, sz.x, sz.y);
    }
    return out;
}

void Room::draw(sf::RenderWindow& window) const {
    // ── Background ────────────────────────────────────────────────────────────
    if (hasBackground) {
        window.draw(bgSprite);
    } else {
        sf::RectangleShape bg({400.f, 225.f});
        bg.setFillColor(floorColor);
        window.draw(bg);
    }

    // ── Props ─────────────────────────────────────────────────────────────────
    for (const auto& p : props) {
        sf::Vector2f sz = propSize(p.type);

        bool loaded = (p.type == PropType::VENDING) ? vendingLoaded : trashLoaded;
        if (loaded) {
            sf::Sprite  spr;
            const sf::Texture& tex = (p.type == PropType::VENDING)
                                     ? vendingTexture : trashTexture;
            spr.setTexture(tex);
            // Scale so the sprite matches the game-coord size
            sf::Vector2u ts = tex.getSize();
            spr.setScale(sz.x / ts.x, sz.y / ts.y);
            spr.setPosition(p.position);
            window.draw(spr);
        } else {
            // Fallback coloured rect
            sf::RectangleShape rect(sz);
            rect.setPosition(p.position);
            rect.setFillColor(p.type == PropType::VENDING
                              ? sf::Color(180, 30, 30)
                              : sf::Color(80, 80, 100));
            window.draw(rect);
        }

        // Debug hitbox (comment out for release)
        // sf::RectangleShape dbg(sz);
        // dbg.setPosition(p.position);
        // dbg.setFillColor(sf::Color::Transparent);
        // dbg.setOutlineThickness(0.5f);
        // dbg.setOutlineColor(sf::Color(255,0,0,180));
        // window.draw(dbg);
    }
}
