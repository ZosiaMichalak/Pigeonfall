#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>

#include "GameObject.h"
#include "Room.h"
#include "Enemy.h"
#include "BulletEnemy.h"
#include "DashEnemy.h"
#include "Player.h"
#include "HUD.h"
#include "SkillTreeUI.h"

class Game {
private:
    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;

    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<std::unique_ptr<Room>> rooms;
    int currentRoomIndex;
    int enemiesRemainingToSpawn;

    std::mt19937 rng;

    sf::RectangleShape doorShape;
    sf::Text           interactText;

    bool wasEPressed;

    bool isFullscreen;
    bool wasF11Pressed;

    // Sub-systems
    HUD         hud;
    SkillTreeUI skillTree;

    bool wasMPressed;

    void initWindow();
    void applyWindowMode();
    void applyLetterboxView();
    void refreshFontTextures();

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
