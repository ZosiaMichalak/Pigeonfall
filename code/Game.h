#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include "GameObject.h"
#include "Room.h"

class Game {
private:
    sf::RenderWindow window;
    sf::Clock clock;
    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<Room> rooms;
    int currentRoomIndex;
    int enemiesRemainingToSpawn; 

    std::mt19937 rng;

    sf::RectangleShape doorShape;
    bool wasEPressed;

    bool isFullscreen;
    bool wasF11Pressed;
    void applyWindowMode();
    void applyLetterboxView();

    sf::Font font;
    sf::Text roomText;
    sf::Text interactText;

    void initWindow();
    void spawnEnemy();
    void nextRoom();
    void resetRun();

public:
    Game();
    void run();
    void update(float dt);
    void render();
};

#endif