#include "Game.h"
#include "Bullet.h"
#include "Room.h"
#include <algorithm>
#include <ctime>
#include <cmath>
#include <iostream>

static constexpr float VIEW_W   = 400.f;
static constexpr float VIEW_H   = 225.f;
static constexpr float UI_BAR_Y = 195.f;

// ── Utility ───────────────────────────────────────────────────────────────────
static sf::Vector2f getRectCenter(const sf::FloatRect& r) {
    return { r.left + r.width / 2.f, r.top + r.height / 2.f };
}

// ── Constructor ───────────────────────────────────────────────────────────────
Game::Game()
    : hud(font), skillTree(font)
{
    isFullscreen  = false;
    wasF11Pressed = false;
    wasEPressed   = false;
    wasMPressed   = false;
    enemiesRemainingToSpawn = 0;
    currentRoomIndex = 0;

    rng.seed(static_cast<unsigned>(time(nullptr)));

    initWindow();

    if (!font.loadFromFile("m5x7.ttf"))
        std::cerr << "[Warning] m5x7.ttf not found!\n";

    refreshFontTextures();

    interactText.setFont(font);
    interactText.setCharacterSize(18);
    interactText.setFillColor(sf::Color::White);
    interactText.setString("E");

    doorShape.setSize(sf::Vector2f(12.f, 32.f));
    doorShape.setOrigin(6.f, 16.f);
    doorShape.setPosition(394.f, 100.f);
    doorShape.setFillColor(sf::Color(230, 180, 40));

    rooms.push_back(std::make_unique<Room>(0, 0, sf::Color(20, 25, 40)));
    rooms[0]->loadAssets();
    objects.push_back(std::make_unique<Player>(100.f, 100.f));
}

// ── Window management ─────────────────────────────────────────────────────────
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
    if (!font.getInfo().family.empty()) refreshFontTextures();
}

void Game::applyLetterboxView() {
    sf::Vector2u wSize = window.getSize();
    float wr = static_cast<float>(wSize.x) / static_cast<float>(wSize.y);
    float vr = VIEW_W / VIEW_H;
    sf::View view(sf::FloatRect(0.f, 0.f, VIEW_W, VIEW_H));
    sf::FloatRect vp(0.f, 0.f, 1.f, 1.f);
    if (wr > vr) {
        float px = (1.f - vr / wr) / 2.f;
        vp = { px, 0.f, vr / wr, 1.f };
    } else {
        float py = (1.f - wr / vr) / 2.f;
        vp = { 0.f, py, 1.f, wr / vr };
    }
    view.setViewport(vp);
    window.setView(view);
}

void Game::refreshFontTextures() {
    for (unsigned sz : {12u, 14u, 16u, 18u})
        const_cast<sf::Texture&>(font.getTexture(sz)).setSmooth(false);
}

// ── Game management ───────────────────────────────────────────────────────────
void Game::spawnEnemy() {
    std::uniform_real_distribution<float> xDis(50.f, 350.f);
    std::uniform_real_distribution<float> yDis(30.f, 160.f);
    float ex = xDis(rng), ey = yDis(rng);

    if (!objects.empty()) {
        if (auto* p = dynamic_cast<Player*>(objects[0].get())) {
            sf::Vector2f pp = getRectCenter(p->getBounds());
            while (std::abs(ex - pp.x) < 40.f && std::abs(ey - pp.y) < 40.f) {
                ex = xDis(rng); ey = yDis(rng);
            }
        }
    }

    int tier       = std::min(4, currentRoomIndex / 3);
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
        RoomTemplate tmpl = RoomTemplates::getRandom();
        rooms.push_back(std::make_unique<Room>(currentRoomIndex, tmpl));
        rooms.back()->loadAssets();
    }

    std::vector<std::unique_ptr<GameObject>> newObjects;
    auto it = std::find_if(objects.begin(), objects.end(),
                           [](const auto& o){ return dynamic_cast<Player*>(o.get()); });
    if (it != objects.end()) {
        static_cast<Player*>(it->get())->setPosition({20.f, 100.f});
        newObjects.push_back(std::move(*it));
    } else {
        newObjects.push_back(std::make_unique<Player>(20.f, 100.f));
    }
    objects = std::move(newObjects);
    enemiesRemainingToSpawn = rooms[currentRoomIndex]->getEnemyCount();
}

void Game::resetRun() {
    Player::resetRunStats();
    rooms.clear();
    rooms.push_back(std::make_unique<Room>(0, 0, sf::Color(20, 25, 40)));
    rooms[0]->loadAssets();
    currentRoomIndex        = 0;
    enemiesRemainingToSpawn = 0;
    skillTree.close();
    objects.clear();
    objects.push_back(std::make_unique<Player>(100.f, 100.f));
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)  window.close();
            if (event.type == sf::Event::Resized) applyLetterboxView();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F11 && !wasF11Pressed) {
                    isFullscreen = !isFullscreen;
                    applyWindowMode();
                    wasF11Pressed = true;
                }

                if (skillTree.isOpen()) {
                    if (event.key.code == sf::Keyboard::Up   ||
                        event.key.code == sf::Keyboard::W)
                        skillTree.moveSelection(-1);

                    if (event.key.code == sf::Keyboard::Down ||
                        event.key.code == sf::Keyboard::S)
                        skillTree.moveSelection(+1);

                    if (event.key.code == sf::Keyboard::Return ||
                        event.key.code == sf::Keyboard::Space  ||
                        event.key.code == sf::Keyboard::E)
                    {
                        Player* p = objects.empty()
                            ? nullptr : dynamic_cast<Player*>(objects[0].get());
                        skillTree.buySelected(p);
                    }
                }
            }
            if (event.type == sf::Event::KeyReleased)
                if (event.key.code == sf::Keyboard::F11) wasF11Pressed = false;
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        update(dt);
        render();
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void Game::update(float dt) {
    Player* playerPtr = objects.empty()
        ? nullptr : dynamic_cast<Player*>(objects[0].get());

    // Handle player death
    if (playerPtr && playerPtr->isDeadNow()) {
        if (!playerPtr->consumeSecondChance()) {
            resetRun();
            return;
        }
    }

    // Toggle skill tree (M key) — pauses game logic while open
    bool mNow = sf::Keyboard::isKeyPressed(sf::Keyboard::M);
    if (mNow && !wasMPressed) skillTree.toggle();
    wasMPressed = mNow;

    if (skillTree.isOpen()) return;

    std::vector<std::unique_ptr<GameObject>> spawnQueue;

    for (auto& obj : objects) {
        if (!obj->isActive()) continue;
        obj->update(dt, window);
        if (auto* enemy = dynamic_cast<Enemy*>(obj.get()))
            if (playerPtr)
                enemy->updateAI(dt, getRectCenter(playerPtr->getBounds()), spawnQueue);
    }

    // Keep player above the HUD bar
    if (playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        if (pb.top + pb.height > UI_BAR_Y)
            playerPtr->setPosition({getRectCenter(pb).x, UI_BAR_Y - pb.height / 2.f});
    }

    // Push player out of prop colliders
    if (playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        for (const sf::FloatRect& col : rooms[currentRoomIndex]->getPropColliders()) {
            if (!pb.intersects(col)) continue;

            // Find smallest overlap axis and push out
            float overlapL = (pb.left + pb.width)  - col.left;
            float overlapR = (col.left + col.width) - pb.left;
            float overlapT = (pb.top  + pb.height)  - col.top;
            float overlapB = (col.top + col.height)  - pb.top;

            float minH = (overlapL < overlapR) ? -overlapL :  overlapR;
            float minV = (overlapT < overlapB) ? -overlapT :  overlapB;

            sf::Vector2f centre = getRectCenter(pb);
            if (std::abs(minH) < std::abs(minV))
                playerPtr->setPosition({centre.x + minH, centre.y});
            else
                playerPtr->setPosition({centre.x, centre.y + minV});

            pb = playerPtr->getBounds(); // refresh after push
        }
    }

    for (auto& o : spawnQueue) objects.push_back(std::move(o));

    // ── Collisions ────────────────────────────────────────────────────────────
    for (auto& objA : objects) {
        if (!objA->isActive()) continue;

        if (playerPtr && playerPtr->isAttackingNow()) {
            // Deflect enemy bullets with sword
            if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
                if (bullet->isFromEnemy() &&
                    playerPtr->getSwordBounds().intersects(bullet->getBounds()))
                {
                    Enemy* target = nullptr;
                    float  minD   = 999999.f;
                    for (auto& ob : objects) {
                        if (auto* e = dynamic_cast<Enemy*>(ob.get())) {
                            if (!e->isActive()) continue;
                            sf::Vector2f d = getRectCenter(e->getBounds())
                                           - getRectCenter(bullet->getBounds());
                            float dist = std::sqrt(d.x*d.x + d.y*d.y);
                            if (dist < minD) { minD = dist; target = e; }
                        }
                    }
                    sf::Vector2f rd = target
                        ? getRectCenter(target->getBounds()) - getRectCenter(bullet->getBounds())
                        : sf::Vector2f(-1.f, -1.f);
                    bullet->deflect(rd, 180.f);
                }
            }

            // Sword hits enemy
            if (auto* enemy = dynamic_cast<Enemy*>(objA.get())) {
                if (playerPtr->getSwordBounds().intersects(enemy->getBounds())) {
                    bool wasAlive = enemy->isActive();
                    enemy->takeDamage(playerPtr->getComboHitDamage());
                    if (wasAlive && !enemy->isActive()) {
                        int xpReward = dynamic_cast<DashEnemy*>(enemy) ? 3 : 2;
                        playerPtr->addXP(xpReward);
                    }
                }
            }
        }

        // Bullet hits player or deflected bullet hits enemy
        if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
            if (bullet->isFromEnemy()) {
                if (playerPtr && playerPtr->getBounds().intersects(bullet->getBounds())) {
                    if (!playerPtr->isDashingNow()) {
                        playerPtr->takeDamage(1);
                        bullet->destroy();
                    }
                }
            } else {
                for (auto& ob : objects) {
                    if (auto* e = dynamic_cast<Enemy*>(ob.get())) {
                        if (e->isActive() && e->getBounds().intersects(bullet->getBounds())) {
                            bool wasAlive = e->isActive();
                            e->takeDamage(1);
                            if (wasAlive && !e->isActive())
                                playerPtr->addXP(dynamic_cast<DashEnemy*>(e) ? 3 : 2);
                            bullet->destroy();
                            break;
                        }
                    }
                }
            }
        }

        // DashEnemy contact damage
        if (auto* dasher = dynamic_cast<DashEnemy*>(objA.get())) {
            if (dasher->isDashingNow() && playerPtr)
                if (dasher->getBounds().intersects(playerPtr->getBounds()))
                    if (!playerPtr->isDashingNow())
                        playerPtr->takeDamage(1);
        }
    }

    // Remove inactive objects
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [](const std::unique_ptr<GameObject>& o){ return !o->isActive(); }),
        objects.end());

    // Count living enemies
    int alive = 0;
    for (auto& o : objects) if (dynamic_cast<Enemy*>(o.get())) alive++;

    if (alive == 0 && enemiesRemainingToSpawn > 0) {
        const auto& spawns = rooms[currentRoomIndex]->getEnemySpawns();
        if (!spawns.empty()) {
            for (const auto& s : spawns) {
                if (s.type == EnemyType::DASH)
                    objects.push_back(std::make_unique<DashEnemy>(s.position.x, s.position.y, s.tier));
                else
                    objects.push_back(std::make_unique<BulletEnemy>(s.position.x, s.position.y, s.tier));
            }
            enemiesRemainingToSpawn = 0;
            alive = static_cast<int>(spawns.size());
        } else {
            int maxW = (currentRoomIndex >= 6) ? 4 : 3;
            std::uniform_int_distribution<int> wsd(1, maxW);
            int wc = std::min(wsd(rng), enemiesRemainingToSpawn);
            for (int i = 0; i < wc; ++i) { spawnEnemy(); enemiesRemainingToSpawn--; }
            alive = wc;
        }
    }

    bool cleared = (alive == 0 && enemiesRemainingToSpawn == 0);
    rooms[currentRoomIndex]->setCleared(cleared);
    doorShape.setFillColor(cleared ? sf::Color(230, 180, 40) : sf::Color(70, 70, 70));

    // Door interaction
    if (cleared && playerPtr) {
        if (playerPtr->getBounds().intersects(doorShape.getGlobalBounds())) {
            bool eNow = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
            if (eNow && !wasEPressed) nextRoom();
            wasEPressed = eNow;
        }
    }
}

// ── Render ────────────────────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color::Black);

    rooms[currentRoomIndex]->draw(window);

    // Update the door position dynamically based on the current room
    doorShape.setPosition(rooms[currentRoomIndex]->getDoorPosition());
    window.draw(doorShape);

    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i)
        if (objects[i]->isActive()) objects[i]->draw(window);

    Player* playerPtr = objects.empty()
        ? nullptr : dynamic_cast<Player*>(objects[0].get());

    hud.renderSkillsHint(window, playerPtr);
    hud.render(window, playerPtr, rooms[currentRoomIndex]->getId());

    // "E" prompt near door when room is cleared
    if (playerPtr && rooms[currentRoomIndex]->getIsCleared()) {
        sf::Vector2f pp = getRectCenter(playerPtr->getBounds());
        sf::Vector2f dp = doorShape.getPosition();
        float dx = pp.x - dp.x, dy = pp.y - dp.y;
        if (std::sqrt(dx*dx + dy*dy) < 45.f) {
            interactText.setPosition(dp.x - 15.f, dp.y - 25.f);
            window.draw(interactText);
        }
    }

    if (skillTree.isOpen())
        skillTree.render(window, playerPtr);

    window.display();
}
