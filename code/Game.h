//main loop - window, time, objects

#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "GameObject.h"

class Game {
private:
    sf::RenderWindow window; //window
    sf::Clock clock; //delta time
    std::vector<std::unique_ptr<GameObject>> objects; //objects (player, enemy etc.)

    void initWindow(); //window inicialization
    void spawnEnemy(); 

public:
    Game(); //constructor
    void run(); //game runner
    void update(float dt); //logic of the game
    void render(); //visuals
};

#endif