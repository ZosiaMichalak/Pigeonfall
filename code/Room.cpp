/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
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

// Constructor (Fallback): Initializes standard state without props and with custom background color.
Room::Room(int id, int enemyCount, sf::Color fallbackColor)
    : id(id), layoutIndex(0), enemyCount(enemyCount), isCleared(false), floorColor(fallbackColor),
      bgIndex(0), hasBackground(false),
      doorPosition({400.f, 100.f}), doorRotation(0.f),
      playerStart({100.f, 100.f}) {}

// Constructor (Template): Instantiates all physical props mapped in the RoomTemplate.
Room::Room(int id, const RoomTemplate& tmpl, int layoutIndex)
    : id(id), layoutIndex(layoutIndex), enemyCount(0), isCleared(false),
      floorColor(sf::Color(20, 25, 40)), bgIndex(tmpl.background), hasBackground(false),
      doorPosition(tmpl.doorPosition), doorRotation(tmpl.doorRotation),
      playerStart(tmpl.playerStart)
{
    // Build actual game entities out of the template's prop definitions list
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

// Preloads textures for static elements and updates background sprites.
void Room::loadAssets() {
    Vending::loadTexture();
    Trash::loadTexture();
    Bench::loadTexture();
    Flowers::loadTexture();
    Hydrant::loadTexture();
    Car::loadTextures();

    // Try loading room-specific background images from "assets/" folder
    std::string bgPath = "assets/B" + std::to_string(bgIndex + 1) + ".png";
    hasBackground = bgTexture.loadFromFile(bgPath);
    if (hasBackground) {
        bgSprite.setTexture(bgTexture);
        bgSprite.setScale(0.5f, 0.5f); // Scale down due to high resolution assets
    }
}

// Collates all bounding box rectangles for room props to calculate physics collisions.
std::vector<sf::FloatRect> Room::getPropColliders() const {
    std::vector<sf::FloatRect> out;
    for (const auto& p : props)
        out.push_back(p->getBounds());
    return out;
}

namespace RoomTemplates {

// Layout templates definition. Enemy count, types, and tiers are decided dynamically in game logic.
std::vector<RoomTemplate> getAll() {
    return {
        // Starter Room layout
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

        // Room Layout 1
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

        // Room Layout 2
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
        },
                // ── 3 ──
        {
            "3", 0,
            {
            {PropDef::Type::BENCH, {16.0f, 16.0f}},
            {PropDef::Type::BENCH, {96.0f, 16.0f}},
            {PropDef::Type::TRASH, {56.0f, 16.0f}},
            {PropDef::Type::VENDING, {224.0f, 8.0f}},
            {PropDef::Type::FLOWERS, {120.0f, 112.0f}},
            {PropDef::Type::FLOWERS, {56.0f, 112.0f}},
            {PropDef::Type::CAR1, {320.0f, 144.0f}},
        },
            {376.0f, 88.0f}, 90.0f,
            {192.0f, 96.0f}
        },
        // ── 4 ──
        {
            "4", 1,
            {
            {PropDef::Type::VENDING, {344.0f, 8.0f}},
            {PropDef::Type::CAR2, {320.0f, 136.0f}},
            {PropDef::Type::CAR3, {320.0f, 96.0f}},
            {PropDef::Type::HYDRANT, {200.0f, 32.0f}},
        },
            {192.0f, 176.0f}, 90.0f,
            {192.0f, 88.0f}
        },
        // ── 5 ──
        {
            "5", 2,
            {
            {PropDef::Type::CAR3, {0.0f, 136.0f}},
            {PropDef::Type::CAR2, {0.0f, 96.0f}},
            {PropDef::Type::CAR1, {0.0f, 56.0f}},
            {PropDef::Type::TRASH, {288.0f, 16.0f}},
            {PropDef::Type::BENCH, {256.0f, 16.0f}},
        },
            {368.0f, 88.0f}, 90.0f,
            {208.0f, 80.0f}
        },
        // ── 6 ──
        {
            "6", 3,
            {
            {PropDef::Type::BENCH, {80.0f, 24.0f}},
            {PropDef::Type::BENCH, {144.0f, 24.0f}},
            {PropDef::Type::BENCH, {208.0f, 24.0f}},
            {PropDef::Type::TRASH, {112.0f, 24.0f}},
            {PropDef::Type::TRASH, {176.0f, 24.0f}},
            {PropDef::Type::VENDING, {248.0f, 8.0f}},
            {PropDef::Type::CAR2, {312.0f, 144.0f}},
            {PropDef::Type::CAR2, {216.0f, 144.0f}},
        },
            {32.0f, 120.0f}, 90.0f,
            {264.0f, 120.0f}
        },
        // ── 7 ──
        {
            "7", 4,
            {
            {PropDef::Type::HYDRANT, {128.0f, 120.0f}},
            {PropDef::Type::HYDRANT, {232.0f, 120.0f}},
            {PropDef::Type::CAR3, {312.0f, 8.0f}},
            {PropDef::Type::CAR3, {224.0f, 8.0f}},
            {PropDef::Type::CAR2, {152.0f, 8.0f}},
            {PropDef::Type::CAR1, {72.0f, 8.0f}},
            {PropDef::Type::CAR2, {0.0f, 8.0f}},
            {PropDef::Type::VENDING, {8.0f, 112.0f}},
        },
            {192.0f, 176.0f}, 90.0f,
            {192.0f, 80.0f}
        },
        // ── 8 ──
        {
            "8", 4,
            {
            {PropDef::Type::VENDING, {224.0f, 112.0f}},
            {PropDef::Type::VENDING, {104.0f, 112.0f}},
            {PropDef::Type::BENCH, {72.0f, 128.0f}},
            {PropDef::Type::BENCH, {272.0f, 128.0f}},
            {PropDef::Type::CAR2, {0.0f, 8.0f}},
            {PropDef::Type::TRASH, {296.0f, 16.0f}},
        },
            {368.0f, 88.0f}, 90.0f,
            {48.0f, 88.0f}
        },
        // ── 9 ──
        {
            "9", 3,
            {
            {PropDef::Type::CAR2, {56.0f, 0.0f}},
            {PropDef::Type::CAR2, {128.0f, 0.0f}},
            {PropDef::Type::CAR3, {128.0f, 32.0f}},
            {PropDef::Type::CAR1, {56.0f, 32.0f}},
            {PropDef::Type::VENDING, {344.0f, 136.0f}},
            {PropDef::Type::FLOWERS, {200.0f, 16.0f}},
        },
            {32.0f, 120.0f}, 90.0f,
            {312.0f, 120.0f}
        },

    };      
}

// Safe getter that bounds-checks index, falling back to starter room index 0.
const RoomTemplate& getByIndex(int index) {
    static const std::vector<RoomTemplate> all = getAll();
    if (index < 0 || index >= static_cast<int>(all.size()))
        index = 0;
    return all[static_cast<size_t>(index)];
}

// Returns a random room layout index starting from index 1 (starter room is skipped).
int getRandomIndex() {
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    auto all = getAll();
    if (all.size() <= 1) return 0;
    std::uniform_int_distribution<int> dis(1, static_cast<int>(all.size()) - 1);
    return dis(rng);
}

// Selects and returns a random room template.
RoomTemplate getRandom() {
    return getByIndex(getRandomIndex());
}

// Configures and returns template settings for the boss battle room.
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

// Draws floor shape or background texture, followed by all props.
void Room::draw(sf::RenderWindow& window) const {
    if (hasBackground) {
        window.draw(bgSprite);
    } else {
        // Draw standard color shape if image background fails to load
        sf::RectangleShape bg({400.f, 225.f});
        bg.setFillColor(floorColor);
        window.draw(bg);
    }
    // Draw all props placed in this room
    for (const auto& p : props)
        p->draw(window);
}
