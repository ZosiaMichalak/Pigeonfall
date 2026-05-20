#include "Game.h"
#include "Player.h"
#include "Bullet.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include <cmath>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;
static constexpr float UI_BAR_Y = 195.f; 

sf::Vector2f getRectCenter(const sf::FloatRect& rect) {
    return sf::Vector2f(rect.left + rect.width / 2.f,
                        rect.top  + rect.height / 2.f);
}

void Game::refreshFontTextures() {
    for (unsigned sz : {14u, 18u}) {
        const_cast<sf::Texture&>(font.getTexture(sz)).setSmooth(false);
    }
}

Game::Game() {
    isFullscreen  = false;
    wasF11Pressed = false;
    wasEPressed   = false;
    enemiesRemainingToSpawn = 0;

    rng.seed(static_cast<unsigned>(time(nullptr)));

    initWindow();

    if (!font.loadFromFile("m5x7.ttf"))
        std::cerr << "[OSTRZEZENIE] Nie znaleziono m5x7.ttf!" << std::endl;

    roomText.setFont(font);
    roomText.setCharacterSize(14);
    roomText.setFillColor(sf::Color::White);

    roomText.setPosition(15.f, UI_BAR_Y + 7.f);

    interactText.setFont(font);
    interactText.setCharacterSize(18);
    interactText.setFillColor(sf::Color::White);
    interactText.setString("E");

    refreshFontTextures();

    rooms.push_back(Room(0, 0, sf::Color(20, 25, 40)));
    currentRoomIndex = 0;

    doorShape.setSize(sf::Vector2f(12.f, 32.f));
    doorShape.setOrigin(6.f, 16.f);
    doorShape.setPosition(394.f, 100.f); 
    doorShape.setFillColor(sf::Color(230, 180, 40));

    objects.push_back(std::make_unique<Player>(100.f, 100.f));
}

void Game::initWindow() {
    window.create(sf::VideoMode(800, 450), "Bread_of_Crumbs", sf::Style::Default);
    window.setFramerateLimit(60);
    applyWindowMode();
}

void Game::applyWindowMode() {
    if (isFullscreen) {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window.create(desktop, "Bread_of_Crumbs", sf::Style::Fullscreen);
    } else {
        window.create(sf::VideoMode(800, 450), "Bread_of_Crumbs", sf::Style::Default);
    }
    window.setFramerateLimit(60);
    applyLetterboxView();
    if (font.getInfo().family != "")
        refreshFontTextures();
}

void Game::applyLetterboxView() {
    sf::Vector2u wSize = window.getSize();
    float windowWidth  = static_cast<float>(wSize.x);
    float windowHeight = static_cast<float>(wSize.y);
    float windowRatio  = windowWidth / windowHeight;
    float viewRatio    = VIEW_W / VIEW_H;

    sf::View view(sf::FloatRect(0.f, 0.f, VIEW_W, VIEW_H));
    sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);

    if (windowRatio > viewRatio) {
        float posX = (1.f - (viewRatio / windowRatio)) / 2.f;
        viewport = sf::FloatRect(posX, 0.f, viewRatio / windowRatio, 1.f);
    } else {
        float posY = (1.f - (windowRatio / viewRatio)) / 2.f;
        viewport = sf::FloatRect(0.f, posY, 1.f, windowRatio / viewRatio);
    }

    view.setViewport(viewport);
    window.setView(view);
}

void Game::spawnEnemy() {
    std::uniform_real_distribution<float> xDis(50.f, 350.f);
    std::uniform_real_distribution<float> yDis(30.f, 160.f);
    float ex = xDis(rng);
    float ey = yDis(rng);

    if (auto* player = dynamic_cast<Player*>(objects.empty() ? nullptr : objects[0].get())) {
        sf::Vector2f pPos = getRectCenter(player->getBounds());
        while (std::abs(ex - pPos.x) < 40.f && std::abs(ey - pPos.y) < 40.f) {
            ex = xDis(rng);
            ey = yDis(rng);
        }
    }

    int tier = std::min(4, currentRoomIndex / 3);

    int dashChance = 0;
    if      (currentRoomIndex >= 10) dashChance = 55;
    else if (currentRoomIndex >=  6) dashChance = 45;
    else if (currentRoomIndex >=  3) dashChance = 35;
    else if (currentRoomIndex >=  1) dashChance = 20;

    std::uniform_int_distribution<int> typeDis(0, 99);
    if (typeDis(rng) < dashChance)
        objects.push_back(std::make_unique<DashEnemy>(ex, ey, tier));
    else
        objects.push_back(std::make_unique<BulletEnemy>(ex, ey, tier));
}

void Game::nextRoom() {
    currentRoomIndex++;

    if (currentRoomIndex >= static_cast<int>(rooms.size())) {
        int nextId = currentRoomIndex;
        int depthBonus = currentRoomIndex / 3;
        int minCount   = std::min(3 + depthBonus,  8);
        int maxCount   = std::min(6 + depthBonus, 12);
        std::uniform_int_distribution<int> enemyCountDis(minCount, maxCount);
        int eCount = enemyCountDis(rng);
        std::uniform_int_distribution<int> colorDis(15, 45);
        sf::Color rColor(colorDis(rng), colorDis(rng), colorDis(rng));
        rooms.push_back(Room(nextId, eCount, rColor));
    }

    Player* player = nullptr;
    for (auto& obj : objects) {
        if (auto* p = dynamic_cast<Player*>(obj.get())) { player = p; break; }
    }

    std::vector<std::unique_ptr<GameObject>> newObjects;
    if (player) {
        player->setPosition(sf::Vector2f(20.f, 100.f));
        newObjects.push_back(std::move(
            const_cast<std::unique_ptr<GameObject>&>(
                *std::find_if(objects.begin(), objects.end(),
                    [](auto const& o) { return dynamic_cast<Player*>(o.get()) != nullptr; }))));
    } else {
        newObjects.push_back(std::make_unique<Player>(20.f, 100.f));
    }

    objects = std::move(newObjects);
    enemiesRemainingToSpawn = rooms[currentRoomIndex].getEnemyCount();
}

void Game::resetRun() {
    rooms.clear();
    rooms.push_back(Room(0, 0, sf::Color(20, 25, 40)));
    currentRoomIndex        = 0;
    enemiesRemainingToSpawn = 0;
    objects.clear();
    objects.push_back(std::make_unique<Player>(100.f, 100.f));
}

void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) applyLetterboxView();

            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::F11 && !wasF11Pressed)
            {
                isFullscreen = !isFullscreen;
                applyWindowMode();
                wasF11Pressed = true;
            }
            if (event.type == sf::Event::KeyReleased &&
                event.key.code == sf::Keyboard::F11)
                wasF11Pressed = false;
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        update(dt);
        render();
    }
}

void Game::update(float dt) {
    Player* playerPtr = nullptr;
    if (!objects.empty())
        playerPtr = dynamic_cast<Player*>(objects[0].get());

    if (playerPtr && playerPtr->getHp() <= 0) { resetRun(); return; }

    std::vector<std::unique_ptr<GameObject>> spawnQueue;

    for (auto& obj : objects) {
        if (!obj->isActive()) continue;
        obj->update(dt, window);

        if (auto* enemy = dynamic_cast<Enemy*>(obj.get())) {
            if (playerPtr)
                enemy->updateAI(dt, getRectCenter(playerPtr->getBounds()), spawnQueue);
        }
    }

    if (playerPtr && playerPtr->isActive()) {
        sf::FloatRect pBounds = playerPtr->getBounds();
        if (pBounds.top + pBounds.height > UI_BAR_Y) {
            sf::Vector2f center = getRectCenter(pBounds);
            playerPtr->setPosition(sf::Vector2f(center.x, UI_BAR_Y - pBounds.height / 2.f));
        }
    }

    for (auto& newObj : spawnQueue)
        objects.push_back(std::move(newObj));

    for (auto& objA : objects) {
        if (!objA->isActive()) continue;

        if (playerPtr && playerPtr->isAttackingNow()) {
            if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
                if (bullet->isFromEnemy() &&
                    playerPtr->getSwordBounds().intersects(bullet->getBounds()))
                {
                    Enemy* targetEnemy = nullptr;
                    float  minDist     = 999999.f;
                    for (auto& objB : objects) {
                        if (auto* e = dynamic_cast<Enemy*>(objB.get())) {
                            if (!e->isActive()) continue;
                            sf::Vector2f d = getRectCenter(e->getBounds()) -
                                             getRectCenter(bullet->getBounds());
                            float dist = std::sqrt(d.x*d.x + d.y*d.y);
                            if (dist < minDist) { minDist = dist; targetEnemy = e; }
                        }
                    }
                    sf::Vector2f reflDir = targetEnemy
                        ? getRectCenter(targetEnemy->getBounds()) - getRectCenter(bullet->getBounds())
                        : sf::Vector2f(-1.f, -1.f);
                    bullet->deflect(reflDir, 180.f);
                }
            }

            if (auto* enemy = dynamic_cast<Enemy*>(objA.get())) {
                if (playerPtr->getSwordBounds().intersects(enemy->getBounds()))
                    enemy->takeDamage(playerPtr->getComboHitDamage());
            }
        }

        if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
            if (bullet->isFromEnemy()) {
                if (playerPtr && playerPtr->getBounds().intersects(bullet->getBounds())) {
                    if (!playerPtr->isDashingNow()) {
                        playerPtr->takeDamage(1);
                        bullet->destroy();
                    }
                }
            } else {
                for (auto& objB : objects) {
                    if (auto* enemy = dynamic_cast<Enemy*>(objB.get())) {
                        if (enemy->isActive() &&
                            enemy->getBounds().intersects(bullet->getBounds()))
                        {
                            enemy->takeDamage(1);
                            bullet->destroy();
                            break;
                        }
                    }
                }
            }
        }

        if (auto* dasher = dynamic_cast<DashEnemy*>(objA.get())) {
            if (dasher->isDashingNow() && playerPtr) {
                if (dasher->getBounds().intersects(playerPtr->getBounds())) {
                    if (!playerPtr->isDashingNow()) {
                        playerPtr->takeDamage(1);
                    }
                }
            }
        }
    }

    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [](const std::unique_ptr<GameObject>& obj) { return !obj->isActive(); }),
        objects.end());

    int aliveEnemies = 0;
    for (auto& obj : objects)
        if (dynamic_cast<Enemy*>(obj.get())) aliveEnemies++;

    if (aliveEnemies == 0 && enemiesRemainingToSpawn > 0) {
        int maxWave = (currentRoomIndex >= 6) ? 4 : 3;
        std::uniform_int_distribution<int> waveSizeDis(1, maxWave);
        int nextWaveCount = std::min(waveSizeDis(rng), enemiesRemainingToSpawn);
        for (int i = 0; i < nextWaveCount; ++i) { spawnEnemy(); enemiesRemainingToSpawn--; }
        aliveEnemies = nextWaveCount;
    }

    if (aliveEnemies == 0 && enemiesRemainingToSpawn == 0) {
        rooms[currentRoomIndex].setCleared(true);
        doorShape.setFillColor(sf::Color(230, 180, 40));
    } else {
        rooms[currentRoomIndex].setCleared(false);
        doorShape.setFillColor(sf::Color(70, 70, 70));
    }

    if (rooms[currentRoomIndex].getIsCleared() && playerPtr) {
        if (playerPtr->getBounds().intersects(doorShape.getGlobalBounds())) {
            bool eNow = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
            if (eNow && !wasEPressed) nextRoom();
            wasEPressed = eNow;
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);


    sf::RectangleShape bg(sf::Vector2f(400.f, 225.f));
    bg.setFillColor(rooms[currentRoomIndex].getFloorColor());
    window.draw(bg);
    window.draw(doorShape);


    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i)
        if (objects[i]->isActive()) objects[i]->draw(window);


    sf::RectangleShape uiBarBg(sf::Vector2f(400.f, 30.f));
    uiBarBg.setFillColor(sf::Color(15, 15, 22)); 
    uiBarBg.setOutlineThickness(-1.f);
    uiBarBg.setOutlineColor(sf::Color(50, 50, 65));
    uiBarBg.setPosition(0.f, UI_BAR_Y);
    window.draw(uiBarBg);

    const_cast<sf::Texture&>(font.getTexture(14)).setSmooth(false);
    const_cast<sf::Texture&>(font.getTexture(18)).setSmooth(false);

    roomText.setString("ROOM " + std::to_string(rooms[currentRoomIndex].getId() + 1));
    roomText.setPosition(15.f, UI_BAR_Y + 7.f);
    window.draw(roomText);

    Player* playerPtr = nullptr;
    if (!objects.empty())
        playerPtr = dynamic_cast<Player*>(objects[0].get());

    if (playerPtr) {
        sf::Text hpLabel("HP:", font, 14);
        hpLabel.setFillColor(sf::Color(230, 230, 230));
        hpLabel.setPosition(290.f, UI_BAR_Y + 7.f); 
        window.draw(hpLabel);

        sf::RectangleShape hpBarBackUI(sf::Vector2f(60.f, 10.f)); 
        hpBarBackUI.setFillColor(sf::Color(60, 20, 20));
        hpBarBackUI.setPosition(315.f, UI_BAR_Y + 12.f); 
        window.draw(hpBarBackUI);

        float hpPct = std::max(0.f, static_cast<float>(playerPtr->getHp()) / 5.f);
        sf::RectangleShape hpBarFrontUI(sf::Vector2f(60.f * hpPct, 10.f));
        hpBarFrontUI.setFillColor(sf::Color(235, 40, 40));
        hpBarFrontUI.setPosition(315.f, UI_BAR_Y + 12.f);
        window.draw(hpBarFrontUI);
     
        // Atak (ATK)
        sf::Text atkLabel("ATK", font, 14);
        atkLabel.setFillColor(sf::Color(180, 180, 180));
        atkLabel.setPosition(75.f, UI_BAR_Y + 7.f);
        window.draw(atkLabel);

        sf::RectangleShape atkBoxBack(sf::Vector2f(10.f, 10.f));
        atkBoxBack.setFillColor(sf::Color(40, 40, 45));
        atkBoxBack.setOutlineThickness(1.f);
        atkBoxBack.setOutlineColor(sf::Color(70, 70, 80));
        atkBoxBack.setPosition(100.f, UI_BAR_Y + 11.f);
        window.draw(atkBoxBack);

        float atkTimer = playerPtr->getAttackCooldownTimer();
        float atkMax   = playerPtr->getAttackCooldownMax();
        float atkProgress = (atkMax > 0.f) ? (1.f - (atkTimer / atkMax)) : 1.f;
        atkProgress = std::min(1.f, std::max(0.f, atkProgress));

        sf::RectangleShape atkBoxFront(sf::Vector2f(10.f, 10.f * atkProgress));
        atkBoxFront.setPosition(100.f, UI_BAR_Y + 11.f + (10.f * (1.f - atkProgress)));
        atkBoxFront.setFillColor(atkProgress >= 1.f ? sf::Color(240, 200, 30) : sf::Color(120, 100, 20));
        window.draw(atkBoxFront);

        // Dash (DSH)
        sf::Text dashLabel("DSH", font, 14);
        dashLabel.setFillColor(sf::Color(180, 180, 180));
        dashLabel.setPosition(125.f, UI_BAR_Y + 7.f);
        window.draw(dashLabel);

        sf::RectangleShape dashBoxBack(sf::Vector2f(10.f, 10.f));
        dashBoxBack.setFillColor(sf::Color(40, 40, 45));
        dashBoxBack.setOutlineThickness(1.f);
        dashBoxBack.setOutlineColor(sf::Color(70, 70, 80));
        dashBoxBack.setPosition(152.f, UI_BAR_Y + 11.f);
        window.draw(dashBoxBack);

        float dashTimer = playerPtr->getDashCooldownTimer();
        float dashMax   = playerPtr->getDashCooldownMax();
        float dashProgress = (dashMax > 0.f) ? (1.f - (dashTimer / dashMax)) : 1.f;
        dashProgress = std::min(1.f, std::max(0.f, dashProgress));

        sf::RectangleShape dashBoxFront(sf::Vector2f(10.f, 10.f * dashProgress));
        dashBoxFront.setPosition(152.f, UI_BAR_Y + 11.f + (10.f * (1.f - dashProgress)));
        dashBoxFront.setFillColor(dashProgress >= 1.f ? sf::Color(40, 180, 230) : sf::Color(20, 90, 120));
        window.draw(dashBoxFront);

        // Przycisk Drzewka Rozwoju
        sf::Text skillsButton("[ M ] SKILLS", font, 14);
        skillsButton.setFillColor(sf::Color(160, 100, 240)); 
        skillsButton.setPosition(210.f, UI_BAR_Y + 7.f);
        window.draw(skillsButton);
    }

    if (playerPtr && rooms[currentRoomIndex].getIsCleared()) {
        sf::Vector2f playerPos = getRectCenter(playerPtr->getBounds());
        sf::Vector2f doorPos   = doorShape.getPosition();
        float dx = playerPos.x - doorPos.x;
        float dy = playerPos.y - doorPos.y;
        if (std::sqrt(dx*dx + dy*dy) < 45.f) {
            interactText.setPosition(doorPos.x - 15.f, doorPos.y - 25.f);
            window.draw(interactText);
        }
    }

    window.display();
}