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
},

{
    "1", 0,
    {{EnemyType::BULLET, {35.f, 170.f}, 1}, {EnemyType::BULLET, {370.f, 173.f}, 1}, {EnemyType::BULLET, {115.f, 17.f}, 1}, {EnemyType::BULLET, {327.f, 22.f}, 1}, {EnemyType::DASH, {367.f, 102.f}, 1}, {EnemyType::DASH, {36.f, 92.f}, 1}},
    {{PropDef::Type::FLOWERS, {224.f, 24.f}}, {PropDef::Type::FLOWERS, {264.f, 24.f}}, {PropDef::Type::HYDRANT, {136.f, 107.f}}, {PropDef::Type::CAR1, {218.f, 104.f}}, {PropDef::Type::CAR2, {219.f, 135.f}}, {PropDef::Type::BENCH, {35.f, 22.f}}, {PropDef::Type::TRASH, {66.f, 22.f}}, {PropDef::Type::VENDING, {357.f, 4.f}}},
    {191.f, 2.f}, 90.f
},

{
    "2", 1,
    {{EnemyType::BULLET, {254.f, 54.f}, 1}, {EnemyType::BULLET, {280.f, 121.f}, 1}, {EnemyType::BULLET, {259.f, 169.f}, 1}, {EnemyType::DASH, {111.f, 46.f}, 1}, {EnemyType::DASH, {162.f, 119.f}, 1}, {EnemyType::DASH, {125.f, 169.f}, 1}},
    {{PropDef::Type::BENCH, {186.f, 24.f}}, {PropDef::Type::CAR1, {329.f, 1.f}}, {PropDef::Type::HYDRANT, {163.f, 33.f}}, {PropDef::Type::CAR3, {329.f, 179.f}}, {PropDef::Type::CAR2, {327.f, 81.f}}},
    {191.f, 191.f}, 90.f
}





    };
}

RoomTemplate getRandom() {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    auto all = getAll();
    
    // Zabezpieczenie na wypadek, gdyby w tabeli był tylko 1 pokój
    if (all.size() <= 1) {
        return all[0];
    }
    
    // Pomijamy indeks 0 ("Starter Room") i losujemy od indeksu 1 do samego końca
    std::uniform_int_distribution<int> dis(1, static_cast<int>(all.size()) - 1);
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
