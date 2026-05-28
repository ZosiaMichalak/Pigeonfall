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
      bgIndex(0), hasBackground(false), doorPosition({400.f, 100.f}), doorRotation(0.f) {}

// Konstruktor na podstawie szablonu
Room::Room(int id, const RoomTemplate& tmpl)
    : id(id), enemyCount(static_cast<int>(tmpl.enemies.size())), isCleared(false),
      floorColor(sf::Color(20, 25, 40)), bgIndex(tmpl.background), hasBackground(false),
      enemySpawns(tmpl.enemies), doorPosition(tmpl.doorPosition), doorRotation(tmpl.doorRotation)
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
    "Starter Room", 3,
    {},
    {{PropDef::Type::HYDRANT, {297.f, 65.f}}, {PropDef::Type::FLOWERS, {113.f, 57.f}}, {PropDef::Type::BENCH, {81.f, 55.f}}, {PropDef::Type::CAR1, {171.f, 108.f}}, {PropDef::Type::CAR3, {319.f, 0.f}}},
    {400.f, 120.f},0.f
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
