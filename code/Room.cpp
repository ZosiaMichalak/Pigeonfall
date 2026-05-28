#include "Room.h"
#include "Vending.h"
#include "Trash.h"
#include <iostream>
#include <ctime>
#include <random>

// Konstruktor ręczny (ustawia domyślną pozycję drzwi)
Room::Room(int id, int enemyCount, sf::Color fallbackColor)
    : id(id), enemyCount(enemyCount), isCleared(false), floorColor(fallbackColor), 
      bgIndex(0), hasBackground(false), doorPosition({400.f, 100.f}) {}

// Konstruktor na podstawie szablonu
Room::Room(int id, const RoomTemplate& tmpl)
    : id(id), enemyCount(static_cast<int>(tmpl.enemies.size())), isCleared(false),
      floorColor(sf::Color(20, 25, 40)), bgIndex(tmpl.background), hasBackground(false),
      enemySpawns(tmpl.enemies), doorPosition(tmpl.doorPosition) 
{
    // Tworzenie obiektów prop na podstawie definicji
    for (const auto& p : tmpl.props) {
        if (p.type == PropDef::Type::VENDING)
            props.push_back(std::make_unique<Vending>(p.position));
        else
            props.push_back(std::make_unique<Trash>(p.position));
    }
}

void Room::loadAssets() {
    Vending::loadTexture();
    Trash::loadTexture();

    std::string bgPath = "assets/B" + std::to_string(bgIndex + 1) + ".png";
    hasBackground = bgTexture.loadFromFile(bgPath);
    if (hasBackground) {
        bgSprite.setTexture(bgTexture);
        bgSprite.setScale(0.5f, 0.5f);
    }
}

std::vector<sf::FloatRect> Room::getPropColliders() const {
    std::vector<sf::FloatRect> out;
    for (const auto& p : props) {
        out.push_back(p->getBounds());
    }
    return out;
}

// ── RoomTemplates ─────────────────────────────────────────────────────────────
namespace RoomTemplates {

std::vector<RoomTemplate> getAll() {
    return {
        // Nazwa, tło, wrogowie, propsy, pozycja drzwi
        {
            "Corridor", 0,
            {{EnemyType::BULLET, {200.f, 100.f}, 0}, {EnemyType::BULLET, {300.f, 80.f}, 0}},
            {{PropDef::Type::TRASH, {50.f, 150.f}}, {PropDef::Type::VENDING, {320.f, 140.f}}},
            {394.f, 100.f}
        },
        {
            "Ambush", 1,
            {{EnemyType::DASH, {150.f, 80.f}, 1}, {EnemyType::BULLET, {250.f, 120.f}, 0}, {EnemyType::DASH, {330.f, 60.f}, 1}},
            {{PropDef::Type::TRASH, {80.f, 160.f}}, {PropDef::Type::TRASH, {200.f, 155.f}}},
            {394.f, 100.f}
        },
        {
            "Showdown", 2,
            {{EnemyType::BULLET, {180.f, 90.f}, 1}, {EnemyType::BULLET, {280.f, 110.f}, 1}, {EnemyType::DASH, {230.f, 60.f}, 2}},
            {{PropDef::Type::VENDING, {60.f, 140.f}}, {PropDef::Type::VENDING, {300.f, 145.f}}},
            {394.f, 100.f}
        },
        {
            "Gauntlet", 0,
            {{EnemyType::BULLET, {120.f, 70.f}, 0}, {EnemyType::BULLET, {200.f, 130.f}, 0}, {EnemyType::BULLET, {300.f, 90.f}, 0}, {EnemyType::DASH, {260.f, 60.f}, 1}},
            {{PropDef::Type::TRASH, {100.f, 155.f}}},
            {394.f, 100.f}
        },
        {
            "Crossfire", 1,
            {{EnemyType::BULLET, {100.f, 100.f}, 1}, {EnemyType::DASH, {320.f, 100.f}, 1}},
            {{PropDef::Type::TRASH, {180.f, 150.f}}, {PropDef::Type::VENDING, {240.f, 145.f}}},
            {394.f, 100.f}
        },
    };
}

RoomTemplate getRandom() {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    auto all = getAll();
    std::uniform_int_distribution<int> dis(0, static_cast<int>(all.size()) - 1);
    return all[dis(rng)];
}

} // namespace RoomTemplates

void Room::draw(sf::RenderWindow& window) const {
    if (hasBackground) {
        window.draw(bgSprite);
    } else {
        sf::RectangleShape bg({400.f, 225.f});
        bg.setFillColor(floorColor);
        window.draw(bg);
    }

    for (const auto& p : props) {
        p->draw(window);
    }
}