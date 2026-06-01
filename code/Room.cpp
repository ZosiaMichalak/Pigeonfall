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

// ── Constructors ──────────────────────────────────────────────────────────────

Room::Room(int id, int enemyCount, sf::Color fallbackColor)
    : id(id), layoutIndex(0), enemyCount(enemyCount), isCleared(false), floorColor(fallbackColor),
      bgIndex(0), hasBackground(false),
      doorPosition({400.f, 100.f}), doorRotation(0.f),
      playerStart({100.f, 100.f}) {}

Room::Room(int id, const RoomTemplate& tmpl, int layoutIndex)
    : id(id), layoutIndex(layoutIndex), enemyCount(0), isCleared(false),
      floorColor(sf::Color(20, 25, 40)), bgIndex(tmpl.background), hasBackground(false),
      doorPosition(tmpl.doorPosition), doorRotation(tmpl.doorRotation),
      playerStart(tmpl.playerStart)
{
    for (const auto& p : tmpl.props) {
        switch (p.type) {
            case PropDef::Type::VENDING:  props.push_back(std::make_unique<Vending>(p.position));            break;
            case PropDef::Type::TRASH:    props.push_back(std::make_unique<Trash>(p.position));              break;
            case PropDef::Type::BENCH:    props.push_back(std::make_unique<Bench>(p.position));              break;
            case PropDef::Type::FLOWERS:  props.push_back(std::make_unique<Flowers>(p.position));            break;
            case PropDef::Type::HYDRANT:  props.push_back(std::make_unique<Hydrant>(p.position));            break;
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
// Enemy spawns have been removed from templates entirely.
// Enemy count and types are decided at runtime in Game::nextRoom().
// playerStart defines where the player appears when entering this room.

namespace RoomTemplates {

std::vector<RoomTemplate> getAll() {
    return {

        {
            "StarterRoom", 3,
            {
            {PropDef::Type::VENDING, {175.5f, 35.9f}},
            {PropDef::Type::TRASH, {149.0f, 53.9f}},
            {PropDef::Type::BENCH, {219.5f, 54.7f}},
            {PropDef::Type::CAR2, {14.0f, 108.3f}},
            {PropDef::Type::FLOWERS, {231.4f, 151.3f}},
            {PropDef::Type::FLOWERS, {271.9f, 151.6f}},
            {PropDef::Type::HYDRANT, {305.4f, 159.7f}},
        },
            {378.3f, 120.8f}, 90.0f,
            {195.1f, 122.6f}
        },

        {
            "1", 0,
            {
            {PropDef::Type::CAR1, {80.0f, 8.0f}},
            {PropDef::Type::CAR2, {0.0f, 8.0f}},
            {PropDef::Type::FLOWERS, {224.0f, 160.0f}},
            {PropDef::Type::FLOWERS, {280.0f, 160.0f}},
            {PropDef::Type::FLOWERS, {336.0f, 160.0f}},
            {PropDef::Type::VENDING, {0.0f, 136.0f}},
            {PropDef::Type::HYDRANT, {376.0f, 0.0f}},
        },
            {384.0f, 88.0f}, 90.0f,
            {192.0f, 96.0f}
        },

                // ── 2 ──
        {
            "2", 1,
            {
            {PropDef::Type::BENCH, {256.0f, 40.0f}},
            {PropDef::Type::BENCH, {344.0f, 40.0f}},
            {PropDef::Type::BENCH, {344.0f, 112.0f}},
            {PropDef::Type::BENCH, {256.0f, 112.0f}},
            {PropDef::Type::TRASH, {304.0f, 112.0f}},
            {PropDef::Type::TRASH, {304.0f, 40.0f}},
            {PropDef::Type::VENDING, {9.5f, 3.6f}},
        },
            {190.2f, 180.9f}, 270.0f,
            {188.2f, 90.5f}
        }


    };      
};


const RoomTemplate& getByIndex(int index) {
    static const std::vector<RoomTemplate> all = getAll();
    if (index < 0 || index >= static_cast<int>(all.size()))
        index = 0;
    return all[static_cast<size_t>(index)];
}

int getRandomIndex() {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    auto all = getAll();
    if (all.size() <= 1) return 0;
    std::uniform_int_distribution<int> dis(1, static_cast<int>(all.size()) - 1);
    return dis(rng);
}

RoomTemplate getRandom() {
    return getByIndex(getRandomIndex());
}

RoomTemplate getBossArena() {
    RoomTemplate boss;
    boss.name         = "Boss Arena";
    boss.background   = 0;
    boss.props        = {};
    boss.doorPosition = {400.f, 100.f};
    boss.doorRotation = 90.f;
    boss.playerStart  = {50.f, 110.f};
    return boss;
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
