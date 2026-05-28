#ifndef ROOM_H
#define ROOM_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include "Prop.h"

enum class EnemyType { BULLET, DASH };

struct EnemySpawn {
    EnemyType type;
    sf::Vector2f position;
    int tier = 0;
};

struct PropDef {
    enum class Type { VENDING, TRASH } type;
    sf::Vector2f position;
};

struct RoomTemplate {
    std::string name;
    int background;
    std::vector<EnemySpawn> enemies;
    std::vector<PropDef> props;
};

namespace RoomTemplates {
    std::vector<RoomTemplate> getAll();
    RoomTemplate getRandom();
}

class Room {
public:
    Room(int id, int enemyCount, sf::Color fallbackColor);
    explicit Room(int id, const RoomTemplate& tmpl);

    int getId() const { return id; }
    int getEnemyCount() const { return enemyCount; }
    bool getIsCleared() const { return isCleared; }
    
    void setCleared(bool c) { isCleared = c; }
    const std::vector<EnemySpawn>& getEnemySpawns() const { return enemySpawns; }

    std::vector<sf::FloatRect> getPropColliders() const;
    void loadAssets(); // Wczytuje tylko tło
    void draw(sf::RenderWindow& window) const;

private:
    int id;
    int enemyCount;
    bool isCleared;
    sf::Color floorColor;
    int bgIndex;
    bool hasBackground;

    sf::Texture bgTexture;
    sf::Sprite bgSprite;

    std::vector<EnemySpawn> enemySpawns;
    std::vector<std::unique_ptr<Prop>> props; // Polimorficzne propsy
};

#endif