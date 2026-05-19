#include "Game.h"
#include "Player.h"
#include "Enemy.h"
#include <algorithm>
#include <random>

Game::Game() {
    initWindow();
    objects.push_back(std::make_unique<Player>(400.f, 300.f)); //Player position
    for (int i = 0; i < 5; ++i) spawnEnemy(); //number of enemies
}

void Game::initWindow() {
    window.create(sf::VideoMode(1600,900), "Crumbs of War"); //window size and title
    window.setFramerateLimit(60); //framerate
}

void Game::spawnEnemy() {
    std::random_device rd; //non-deterministic random number generator (seed)
    std::mt19937 gen(rd()); //Meresenne Twister
    //Enemies are not spawning in the same place
    std::uniform_real_distribution<float> disX(50.f, 1600.f); 
    std::uniform_real_distribution<float> disY(50.f, 900.f);

    objects.push_back(std::make_unique<Enemy>(disX(gen), disY(gen))); //enemy spawn
}

//Events handling - delta time, window opening and closing, update and render
void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        float dt = clock.restart().asSeconds();
        update(dt);
        render();
    }
}

void Game::update(float dt) {
    //Updating all objects
    for (auto& obj : objects) {
        obj->update(dt, window);
    }
    //Dynamic Cast
    Player* playerPtr = nullptr;
    for (auto& obj : objects) {
        if (Player* temp = dynamic_cast<Player*>(obj.get())) {
            playerPtr = temp;
            break;
        }
    }

    // Check if the player object exists in memory
    if (playerPtr) {
        
        // Loop through all game objects
        for (auto& obj : objects) {
            
            // Check if the current object is an Enemy
            Enemy* enemyPtr = dynamic_cast<Enemy*>(obj.get());
            
            // If it is an enemy and NOT currently in hit-stun (not flashing white)
            if (enemyPtr && !enemyPtr->getIsHit()) {
                
                // --- SWORD ATTACK ---
                // If the player clicked the left mouse button (is attacking)
                if (playerPtr->isAttackingNow()) {
                    // If the sword hitbox touches the enemy -> inflict damage
                    if (playerPtr->getSwordBounds().intersects(enemyPtr->getBounds())) {
                        enemyPtr->takeDamage(1);
                    }
                }
                
                // --- DASH ATTACK ---
                // If the player pressed space and is currently dashing
                if (playerPtr->isDashingNow()) {
                    // If the player's body collides with the enemy -> inflict damage
                    if (playerPtr->getBounds().intersects(enemyPtr->getBounds())) {
                        enemyPtr->takeDamage(1);
                    }
                }
            }
        } 
    }
}


/**
 * RENDERING GRAPHICS (Called every frame)
 */
void Game::render() {
    // Clear the screen with a dark gray color (removes traces from the previous frame)
    window.clear(sf::Color(30, 30, 30));
    
    // Loop to draw every object (player, enemies) on the hidden screen
    for (auto& obj : objects) {
        obj->draw(window);
    }
    
    // Display the hidden screen – shows the finished image to the player
    window.display();
}
