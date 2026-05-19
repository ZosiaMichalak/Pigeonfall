#include "Game.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include <cmath>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

sf::Vector2f getRectCenter(const sf::FloatRect& rect) {
    return sf::Vector2f(
        rect.left + rect.width / 2.f,
        rect.top + rect.height / 2.f
    );
}

Game::Game() {
    isFullscreen = false;
    wasF11Pressed = false;
    wasEPressed = false;
    enemiesRemainingToSpawn = 0;

    rng.seed(static_cast<unsigned>(time(nullptr)));

    initWindow();

    if (!font.loadFromFile("arial.ttf"))
        std::cerr << "[OSTRZEZENIE] Nie znaleziono arial.ttf!" << std::endl;

    roomText.setFont(font);
    roomText.setCharacterSize(14);
    roomText.setFillColor(sf::Color::White);
    roomText.setPosition(10.f, 10.f);

    interactText.setFont(font);
    interactText.setCharacterSize(18);
    interactText.setFillColor(sf::Color::White);
    interactText.setString("E");

    rooms.push_back(Room(0, 0, sf::Color(20, 25, 40)));
    currentRoomIndex = 0;

    doorShape.setSize(sf::Vector2f(12.f, 32.f));
    doorShape.setOrigin(6.f, 16.f);
    doorShape.setPosition(394.f, 112.f);
    doorShape.setFillColor(sf::Color(230, 180, 40));

    objects.push_back(std::make_unique<Player>(100.f, 112.f));
}

void Game::initWindow() {
    window.create(sf::VideoMode(800, 450),
                  "Bread_of_Crumbs",
                  sf::Style::Default);

    window.setFramerateLimit(60);
    applyWindowMode();
}

void Game::applyWindowMode() {
    if (isFullscreen) {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

        window.create(desktop,
                      "Bread_of_Crumbs",
                      sf::Style::Fullscreen);
    } else {
        window.create(sf::VideoMode(800, 450),
                      "Bread_of_Crumbs",
                      sf::Style::Default);
    }

    window.setFramerateLimit(60);
    applyLetterboxView();
}

void Game::applyLetterboxView() {
    sf::Vector2u wSize = window.getSize();

    float windowWidth = static_cast<float>(wSize.x);
    float windowHeight = static_cast<float>(wSize.y);

    float windowRatio = windowWidth / windowHeight;
    float viewRatio = VIEW_W / VIEW_H;

    sf::View view(sf::FloatRect(0.f, 0.f, VIEW_W, VIEW_H));
    sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);

    if (windowRatio > viewRatio) {
        float posX = (1.f - (viewRatio / windowRatio)) / 2.f;

        viewport = sf::FloatRect(
            posX,
            0.f,
            viewRatio / windowRatio,
            1.f
        );
    } else {
        float posY = (1.f - (windowRatio / viewRatio)) / 2.f;

        viewport = sf::FloatRect(
            0.f,
            posY,
            1.f,
            windowRatio / viewRatio
        );
    }

    view.setViewport(viewport);
    window.setView(view);
}

void Game::spawnEnemy() {
    std::uniform_real_distribution<float> xDis(50.f, 350.f);
    std::uniform_real_distribution<float> yDis(30.f, 190.f);

    float ex = xDis(rng);
    float ey = yDis(rng);

    if (auto* player = dynamic_cast<Player*>(objects.empty() ? nullptr : objects[0].get())) {
        sf::Vector2f pPos = getRectCenter(player->getBounds());

        while (std::abs(ex - pPos.x) < 40.f &&
               std::abs(ey - pPos.y) < 40.f)
        {
            ex = xDis(rng);
            ey = yDis(rng);
        }
    }

    objects.push_back(std::make_unique<Enemy>(ex, ey));
}

void Game::nextRoom() {
    currentRoomIndex++;

    if (currentRoomIndex >= static_cast<int>(rooms.size())) {
        int nextId = currentRoomIndex;

        std::uniform_int_distribution<int> enemyCountDis(3, 6);
        int eCount = enemyCountDis(rng);

        std::uniform_int_distribution<int> colorDis(15, 45);

        sf::Color rColor(
            colorDis(rng),
            colorDis(rng),
            colorDis(rng)
        );

        rooms.push_back(Room(nextId, eCount, rColor));
    }

    Player* player = nullptr;

    for (auto& obj : objects) {
        if (auto* p = dynamic_cast<Player*>(obj.get())) {
            player = p;
            break;
        }
    }

    std::vector<std::unique_ptr<GameObject>> newObjects;

    if (player) {
        player->setPosition(sf::Vector2f(20.f, 112.f));

        newObjects.push_back(std::move(
            const_cast<std::unique_ptr<GameObject>&>(
                *std::find_if(
                    objects.begin(),
                    objects.end(),
                    [](auto const& o) {
                        return dynamic_cast<Player*>(o.get()) != nullptr;
                    }
                )
            )
        ));
    } else {
        newObjects.push_back(std::make_unique<Player>(20.f, 112.f));
    }

    objects = std::move(newObjects);

    enemiesRemainingToSpawn =
        rooms[currentRoomIndex].getEnemyCount();
}

void Game::resetRun() {
    rooms.clear();

    rooms.push_back(
        Room(0, 0, sf::Color(20, 25, 40))
    );

    currentRoomIndex = 0;
    enemiesRemainingToSpawn = 0;

    objects.clear();

    objects.push_back(
        std::make_unique<Player>(100.f, 112.f)
    );
}

void Game::run() {
    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::Resized)
                applyLetterboxView();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F11) {
                    if (!wasF11Pressed) {
                        isFullscreen = !isFullscreen;
                        applyWindowMode();
                        wasF11Pressed = true;
                    }
                }
            }

            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::F11)
                    wasF11Pressed = false;
            }
        }

        float dt = clock.restart().asSeconds();

        if (dt > 0.1f)
            dt = 0.1f;

        update(dt);
        render();
    }
}

void Game::update(float dt) {
    Player* playerPtr = nullptr;

    if (!objects.empty())
        playerPtr = dynamic_cast<Player*>(objects[0].get());

    if (playerPtr && playerPtr->getHp() <= 0) {
        resetRun();
        return;
    }

    std::vector<std::unique_ptr<GameObject>> spawnQueue;

    for (auto& obj : objects) {
        if (obj->isActive()) {
            obj->update(dt, window);

            if (auto* enemy = dynamic_cast<Enemy*>(obj.get())) {
                if (playerPtr) {
                    enemy->updateAI(
                        dt,
                        getRectCenter(playerPtr->getBounds()),
                        spawnQueue
                    );
                }
            }
        }
    }

    for (auto& newObj : spawnQueue)
        objects.push_back(std::move(newObj));

    for (auto& objA : objects) {
        if (!objA->isActive())
            continue;

        if (playerPtr && playerPtr->isAttackingNow()) {
            if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
                if (bullet->isFromEnemy() &&
                    playerPtr->getSwordBounds().intersects(
                        bullet->getBounds()))
                {
                    Enemy* targetEnemy = nullptr;
                    float minDistance = 999999.f;

                    for (auto& objB : objects) {
                        if (auto* enemy =
                            dynamic_cast<Enemy*>(objB.get()))
                        {
                            if (enemy->isActive()) {
                                sf::Vector2f diff =
                                    getRectCenter(enemy->getBounds()) -
                                    getRectCenter(bullet->getBounds());

                                float dist =
                                    std::sqrt(diff.x * diff.x +
                                              diff.y * diff.y);

                                if (dist < minDistance) {
                                    minDistance = dist;
                                    targetEnemy = enemy;
                                }
                            }
                        }
                    }

                    sf::Vector2f reflectionDir;

                    if (targetEnemy) {
                        reflectionDir =
                            getRectCenter(targetEnemy->getBounds()) -
                            getRectCenter(bullet->getBounds());
                    } else {
                        reflectionDir = sf::Vector2f(-1.f, -1.f);
                    }

                    bullet->deflect(reflectionDir, 180.f);
                }
            }
        }

        if (playerPtr && playerPtr->isAttackingNow()) {
            if (auto* enemy = dynamic_cast<Enemy*>(objA.get())) {
                if (playerPtr->getSwordBounds().intersects(
                    enemy->getBounds()))
                {
                    enemy->takeDamage(1);
                }
            }
        }

        if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
            if (bullet->isFromEnemy()) {
                if (playerPtr &&
                    playerPtr->getBounds().intersects(
                        bullet->getBounds()))
                {
                    if (!playerPtr->isDashingNow()) {
                        playerPtr->takeDamage(1);
                        bullet->destroy();
                    }
                }
            } else {
                for (auto& objB : objects) {
                    if (auto* enemy =
                        dynamic_cast<Enemy*>(objB.get()))
                    {
                        if (enemy->isActive() &&
                            enemy->getBounds().intersects(
                                bullet->getBounds()))
                        {
                            enemy->takeDamage(1);
                            bullet->destroy();
                            break;
                        }
                    }
                }
            }
        }
    }

    objects.erase(
        std::remove_if(
            objects.begin(),
            objects.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                return !obj->isActive();
            }),
        objects.end()
    );

    int aliveEnemies = 0;

    for (auto& obj : objects) {
        if (dynamic_cast<Enemy*>(obj.get()))
            aliveEnemies++;
    }

    if (aliveEnemies == 0 &&
        enemiesRemainingToSpawn > 0)
    {
        std::uniform_int_distribution<int> waveSizeDis(1, 3);

        int nextWaveLimit = waveSizeDis(rng);

        int nextWaveCount =
            std::min(nextWaveLimit,
                     enemiesRemainingToSpawn);

        for (int i = 0; i < nextWaveCount; ++i) {
            spawnEnemy();
            enemiesRemainingToSpawn--;
        }

        aliveEnemies = nextWaveCount;
    }

    if (aliveEnemies == 0 &&
        enemiesRemainingToSpawn == 0)
    {
        rooms[currentRoomIndex].setCleared(true);

        doorShape.setFillColor(
            sf::Color(230, 180, 40)
        );
    } else {
        rooms[currentRoomIndex].setCleared(false);

        doorShape.setFillColor(
            sf::Color(70, 70, 70)
        );
    }

    if (rooms[currentRoomIndex].getIsCleared() &&
        playerPtr)
    {
        if (playerPtr->getBounds().intersects(
            doorShape.getGlobalBounds()))
        {
            bool eNow =
                sf::Keyboard::isKeyPressed(
                    sf::Keyboard::E);

            if (eNow && !wasEPressed)
                nextRoom();

            wasEPressed = eNow;
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);

    sf::RectangleShape bg(
        sf::Vector2f(400.f, 225.f)
    );

    bg.setFillColor(
        rooms[currentRoomIndex].getFloorColor()
    );

    window.draw(bg);
    window.draw(doorShape);

    for (int i = static_cast<int>(objects.size()) - 1;
         i >= 0;
         --i)
    {
        if (objects[i]->isActive())
            objects[i]->draw(window);
    }

    roomText.setString(
        "ROOM " +
        std::to_string(
            rooms[currentRoomIndex].getId() + 1
        )
    );

    window.draw(roomText);

    Player* playerPtr = nullptr;

    if (!objects.empty())
        playerPtr = dynamic_cast<Player*>(objects[0].get());

    if (playerPtr &&
        rooms[currentRoomIndex].getIsCleared())
    {
        sf::Vector2f playerPos =
            getRectCenter(playerPtr->getBounds());

        sf::Vector2f doorPos =
            doorShape.getPosition();

        float dx = playerPos.x - doorPos.x;
        float dy = playerPos.y - doorPos.y;

        float distance =
            std::sqrt(dx * dx + dy * dy);

        if (distance < 45.f) {
            interactText.setPosition(
                doorPos.x - 5.f,
                doorPos.y - 40.f
            );

            window.draw(interactText);
        }
    }

    window.display();
}