#ifndef ROOM_H
#define ROOM_H

#include <SFML/Graphics.hpp>

class Room {
private:
    int id;
    int enemyCount;
    bool isCleared;
    sf::Color floorColor;

public:
    Room(int id, int enemyCount, sf::Color color) {
        this->id = id;
        this->enemyCount = enemyCount;
        this->isCleared = false;
        this->floorColor = color;
    }
    
    int getId() const { return id; }
    int getEnemyCount() const { return enemyCount; }
    void setEnemyCount(int count) { enemyCount = count; } 
    bool getIsCleared() const { return isCleared; }
    void setCleared(bool cleared) { isCleared = cleared; }
    sf::Color getFloorColor() const { return floorColor; }
};

#endif