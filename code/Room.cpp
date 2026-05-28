#include "Room.h"
#include "Vending.h"
#include "Trash.h"
#include "Bench.h"
#include "Car.h"
#include "Flowers.h"
#include "Hydrant.h"
#include <iostream>
#include <ctime>
#include <random>

// Konstruktor ręczny
Room::Room(int id, int enemyCount, sf::Color fallbackColor)
    : id(id), enemyCount(enemyCount), isCleared(false), floorColor(fallbackColor),
      bgIndex(0), hasBackground(false), doorPosition({400.f, 100.f}) {}

// Konstruktor na podstawie szablonu
Room::Room(int id, const RoomTemplate& tmpl)
    : id(id), enemyCount(static_cast<int>(tmpl.enemies.size())), isCleared(false),
      floorColor(sf::Color(20, 25, 40)), bgIndex(tmpl.background), hasBackground(false),
      enemySpawns(tmpl.enemies), doorPosition(tmpl.doorPosition)
{
    for (const auto& p : tmpl.props) {
        switch (p.type) {
            case PropDef::Type::VENDING:  props.push_back(std::make_unique<Vending>(p.position));          break;
            case PropDef::Type::TRASH:    props.push_back(std::make_unique<Trash>(p.position));            break;
            case PropDef::Type::BENCH:    props.push_back(std::make_unique<Bench>(p.position));            break;
            case PropDef::Type::FLOWERS:  props.push_back(std::make_unique<Flowers>(p.position));          break;
            case PropDef::Type::HYDRANT:  props.push_back(std::make_unique<Hydrant>(p.position));          break;
            case PropDef::Type::CAR1:     props.push_back(std::make_unique<Car>(p.position, CarType::CAR1)); break;
            case PropDef::Type::CAR2:     props.push_back(std::make_unique<Car>(p.position, CarType::CAR2)); break;
            case PropDef::Type::CAR3:     props.push_back(std::make_unique<Car>(p.position, CarType::CAR3)); break;
        }
    }
}

void Room::loadAssets() {
    Vending::loadTexture();
    Trash::loadTexture();
    Bench::loadTexture();
    Flowers::loadTexture();
    Hydrant::loadTexture();
    Car::loadTextures();

    std::string bgPath = "assets/B" + std::to_string(bgIndex + 1) + ".png";
    hasBackground = bgTexture.loadFromFile(bgPath);
    if (hasBackground) {
        bgSprite.setTexture(bgTexture);
        bgSprite.setScale(0.5f, 0.5f);
    }
}

std::vector<sf::FloatRect> Room::getPropColliders() const {
    std::vector<sf::FloatRect> out;
    for (const auto& p : props)
        out.push_back(p->getBounds());
    return out;
}

// ── RoomTemplates ─────────────────────────────────────────────────────────────
namespace RoomTemplates {

std::vector<RoomTemplate> getAll() {
    return {
        {
            "Corridor", 0,
            {{EnemyType::BULLET, {200.f, 100.f}, 0}, {EnemyType::BULLET, {300.f, 80.f}, 0}},
            {{PropDef::Type::BENCH,   {50.f,  150.f}},
             {PropDef::Type::HYDRANT, {300.f, 140.f}}},
            {394.f, 100.f}
        },
        {
            "Ambush", 1,
            {{EnemyType::DASH, {150.f, 80.f}, 1}, {EnemyType::BULLET, {250.f, 120.f}, 0}, {EnemyType::DASH, {330.f, 60.f}, 1}},
            {{PropDef::Type::TRASH,   {80.f,  160.f}},
             {PropDef::Type::FLOWERS, {200.f, 155.f}},
             {PropDef::Type::CAR1,    {240.f, 130.f}}},
            {394.f, 100.f}
        },
        {
            "Showdown", 2,
            {{EnemyType::BULLET, {180.f, 90.f}, 1}, {EnemyType::BULLET, {280.f, 110.f}, 1}, {EnemyType::DASH, {230.f, 60.f}, 2}},
            {{PropDef::Type::VENDING, {0.f,   0.f}},
             {PropDef::Type::CAR2,    {200.f, 130.f}},
             {PropDef::Type::BENCH,   {320.f, 150.f}}},
            {394.f, 100.f}
        },
        {
            "Gauntlet", 0,
            {{EnemyType::BULLET, {120.f, 70.f}, 0}, {EnemyType::BULLET, {200.f, 130.f}, 0}, {EnemyType::BULLET, {300.f, 90.f}, 0}, {EnemyType::DASH, {260.f, 60.f}, 1}},
            {{PropDef::Type::CAR3,    {150.f, 130.f}},
             {PropDef::Type::HYDRANT, {80.f,  145.f}}},
            {394.f, 100.f}
        },
        {
            "Crossfire", 1,
            {{EnemyType::BULLET, {100.f, 100.f}, 1}, {EnemyType::DASH, {320.f, 100.f}, 1}},
            {{PropDef::Type::FLOWERS, {180.f, 150.f}},
             {PropDef::Type::BENCH,   {260.f, 155.f}},
             {PropDef::Type::CAR1,    {100.f, 130.f}}},
            {394.f, 100.f}
        },
        {
            "StartRoom", 0,
            {},
            {{PropDef::Type::TRASH, {200.f, 110.f}}},
            {394.f, 100.f}
        }
    };
}

RoomTemplate getRandom() {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    auto all = getAll();
    // Exclude the last entry (StartRoom) from random selection
    std::uniform_int_distribution<int> dis(0, static_cast<int>(all.size()) - 2);
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
    for (const auto& p : props)
        p->draw(window);
}
