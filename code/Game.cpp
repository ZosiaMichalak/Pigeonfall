#include "Game.h"
#include "SaveSystem.h"
#include "Bullet.h"
#include "Car.h"
#include "Flowers.h"
#include "Bench.h"
#include "Room.h"
#include "Vending.h"
#include "Trash.h"
#include "Hydrant.h"
#include <algorithm>
#include <ctime>
#include <cmath>
#include <iostream>

static constexpr float VIEW_W   = 400.f;
static constexpr float VIEW_H   = 225.f;
static constexpr float UI_BAR_Y = 195.f;

// Distance within which [E] prompt appears and vending can be opened
static constexpr float VENDING_INTERACT_DIST = 30.f;

// ── Utility ───────────────────────────────────────────────────────────────────
static sf::Vector2f getRectCenter(const sf::FloatRect& r) {
    return { r.left + r.width / 2.f, r.top + r.height / 2.f };
}

// ── Constructor ───────────────────────────────────────────────────────────────
Game::Game()
    : hud(font), skillTree(font), vendingUI(font),
      totalCoins(0), nearVending(false),
      isPaused(false), pauseSel(0)
{
    isFullscreen  = false;
    wasF11Pressed = false;
    wasEPressed   = false;
    wasFPressed   = false;
    wasMPressed   = false;
    enemiesRemainingToSpawn = 0;
    currentRoomIndex = 0;

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    rng.seed(static_cast<unsigned>(std::time(nullptr)));

    initWindow();

    if (!font.loadFromFile("m5x7.ttf"))
        std::cerr << "[Warning] m5x7.ttf not found!\n";

    refreshFontTextures();

    // ── Main menu (shown before gameplay starts) ──────────────────────────────
    appState = AppState::MENU;
    mainMenu = std::make_unique<MainMenu>(font, SaveSystem::hasSave(), isFullscreen);

    interactText.setFont(font);
    interactText.setCharacterSize(18);
    interactText.setFillColor(sf::Color::White);
    interactText.setString("[E]");

    doorShape.setSize(sf::Vector2f(12.f, 32.f));
    doorShape.setOrigin(6.f, 16.f);
    doorShape.setPosition(394.f, 100.f);
    doorShape.setFillColor(sf::Color(230, 180, 40));

    auto templates = RoomTemplates::getAll();

    Bench::loadTexture();
    Vending::loadTexture();
    Trash::loadTexture();
    Coin::loadTexture();
    Hydrant::loadTexture();
    Flowers::loadTexture();
    Car::loadTextures();

    rooms.push_back(std::make_unique<Room>(0, templates[0]));
    rooms[0]->loadAssets();
    objects.push_back(std::make_unique<Player>(100.f, 100.f));
}

// ── Window management ─────────────────────────────────────────────────────────
void Game::initWindow() {
    window.create(sf::VideoMode(800, 450), "Pigeonfall", sf::Style::Default);
    window.setFramerateLimit(60);
    applyWindowMode();
}

void Game::applyWindowMode() {
    if (isFullscreen) {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        window.create(desktop, "Pigeonfall", sf::Style::Fullscreen);
    } else {
        window.create(sf::VideoMode(800, 450), "Pigeonfall", sf::Style::Default);
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
        float p = (1.f - vr / wr) / 2.f;
        vp = { p, 0.f, vr / wr, 1.f };
    } else {
        float p = (1.f - wr / vr) / 2.f;
        vp = { 0.f, p, 1.f, wr / vr };
    }
    view.setViewport(vp);
    window.setView(view);
}

void Game::refreshFontTextures() {
    for (unsigned sz : {12u, 14u, 16u, 18u})
        const_cast<sf::Texture&>(font.getTexture(sz)).setSmooth(false);
}

// ── Vending proximity helper ──────────────────────────────────────────────────
sf::FloatRect Game::getClosestVendingBounds(sf::Vector2f playerCenter) const {
    sf::FloatRect best{};
    float bestDist = 1e9f;

    for (const sf::FloatRect& col : rooms[currentRoomIndex]->getPropColliders()) {
        if (col.height < 45.f) continue;

        sf::Vector2f centre = getRectCenter(col);
        float dx = centre.x - playerCenter.x;
        float dy = centre.y - playerCenter.y;
        float d  = std::sqrt(dx*dx + dy*dy);
        if (d < bestDist) { bestDist = d; best = col; }
    }
    return best; 
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
    enemiesRemainingToSpawn = std::min(3 + currentRoomIndex / 2, 10);

    vendingUI.close();
    nearVending = false;
}

void Game::resetRun() {
    Player::resetRunStats();
    rooms.clear();
    rooms.push_back(std::make_unique<Room>(0, 0, sf::Color(20, 25, 40)));
    rooms[0]->loadAssets();
    currentRoomIndex        = 0;
    enemiesRemainingToSpawn = 0;
    totalCoins              = 0;
    heldItem                = "";
    skillTree.close();
    vendingUI.close();
    nearVending = false;
    objects.clear();
    objects.push_back(std::make_unique<Player>(100.f, 100.f));
}

void Game::saveGame() {
    Player* playerPtr = objects.empty() ? nullptr : dynamic_cast<Player*>(objects[0].get());
    if (!playerPtr) return;

    SaveData sd;
    sd.exists = true;

    sd.level            = playerPtr->getLevel();
    sd.xp               = playerPtr->getXP();
    sd.xpToNext         = playerPtr->getXPToNext();
    sd.skillPoints      = playerPtr->getSkillPoints();
    sd.secondChanceUsed = playerPtr->isSecondChanceUsed();
    sd.totemCharges     = playerPtr->getTotemCharges();

    for (int i = 0; i < SKILL_COUNT; ++i) {
        sd.upgrades[i] = playerPtr->getUpgradeLevel(i);
    }

    sd.roomIndex   = currentRoomIndex;
    sd.coins       = totalCoins;
    sd.heldItem    = heldItem;
    sd.fullscreen  = isFullscreen;
    sd.musicVolume = 100;

    SaveSystem::save(sd);
}

void Game::drawPauseMenu() {
    sf::RectangleShape overlay(sf::Vector2f(VIEW_W, VIEW_H));
    overlay.setFillColor(sf::Color(0, 0, 0, 180)); 
    window.draw(overlay);

    sf::Text pauseTitle("PAUSE", font, 24);
    pauseTitle.setFillColor(sf::Color::White);
    pauseTitle.setPosition(VIEW_W / 2.f - pauseTitle.getGlobalBounds().width / 2.f, 30.f);
    window.draw(pauseTitle);

    std::vector<std::string> pauseItems = { "SAVE GAME", "OPTIONS", "QUIT" };
    
    for (int i = 0; i < 3; ++i) {
        sf::Text itemText(pauseItems[i], font, 16);
        
        if (i == pauseSel) {
            itemText.setFillColor(sf::Color(255, 210, 50)); 
            
            sf::Text marker(">", font, 16);
            marker.setFillColor(sf::Color(255, 210, 50));
            marker.setPosition(VIEW_W / 2.f - 60.f, 85.f + i * 25.f);
            window.draw(marker);
        } else {
            itemText.setFillColor(sf::Color(140, 130, 160));
        }
        
        itemText.setPosition(VIEW_W / 2.f - 40.f, 85.f + i * 25.f);
        window.draw(itemText);
    }
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)  window.close();
            if (event.type == sf::Event::Resized) applyLetterboxView();

            // ── Menu state: feed all events to MainMenu ───────────────────────
            if (appState == AppState::MENU) {
                MenuAction action = mainMenu->handleEvent(event);
                if (action == MenuAction::NEW_GAME) {
                    resetRun();
                    appState = AppState::PLAYING;
                }
                else if (action == MenuAction::LOAD_GAME) {
                    SaveData sd = SaveSystem::load();
                    if (sd.exists) {
                        resetRun();
                        if (!objects.empty()) {
                            if (auto* p = dynamic_cast<Player*>(objects[0].get())) {
                                p->applyLoadedSave(sd);
                            }
                        }
                        isFullscreen = sd.fullscreen;
                        applyWindowMode();
                        heldItem    = sd.heldItem;
                        totalCoins  = sd.coins;
                        currentRoomIndex = 0;
                        for (int i = 0; i < sd.roomIndex; ++i) nextRoom();
                    }
                    appState = AppState::PLAYING;
                }
                else if (action == MenuAction::QUIT) {
                    window.close();
                }

                if (mainMenu->fullscreen() != isFullscreen) {
                    isFullscreen = mainMenu->fullscreen();
                    applyWindowMode();
                    mainMenu = std::make_unique<MainMenu>(font, SaveSystem::hasSave(), isFullscreen);
                }
                continue; 
            }

            // ── Gameplay state input ──────────────────────────────────────────
            if (appState == AppState::PLAYING) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
                    isPaused = !isPaused;
                    pauseSel = 0;
                    continue; 
                }

                if (isPaused) {
                    if (event.type == sf::Event::KeyPressed) {
                        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                            pauseSel = (pauseSel - 1 + 3) % 3;
                        }
                        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                            pauseSel = (pauseSel + 1) % 3;
                        }
                        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                            if (pauseSel == 0) {
                                saveGame();
                                isPaused = false;
                            } 
                            else if (pauseSel == 1) {
                                isFullscreen = !isFullscreen;
                                applyWindowMode();
                            } 
                            else if (pauseSel == 2) {
                                mainMenu = std::make_unique<MainMenu>(font, SaveSystem::hasSave(), isFullscreen);
                                appState = AppState::MENU;
                                isPaused = false;
                            }
                        }
                    }
                    continue; 
                }

                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::F11 && !wasF11Pressed) {
                        isFullscreen = !isFullscreen;
                        applyWindowMode();
                        wasF11Pressed = true;
                    }

                    if (vendingUI.isOpen()) {
                        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
                            vendingUI.moveSelection(-1);

                        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
                            vendingUI.moveSelection(+1);

                        if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                            std::string bought = vendingUI.tryBuy(totalCoins, heldItem);
                            if (!bought.empty()) heldItem = bought;
                        }

                        if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::E)
                            vendingUI.close();

                        continue;
                    }

                    if (skillTree.isOpen()) {
                        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
                            skillTree.moveSelection(-1);

                        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
                            skillTree.moveSelection(+1);

                        if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::E) {
                            Player* p = objects.empty() ? nullptr : dynamic_cast<Player*>(objects[0].get());
                            skillTree.buySelected(p);
                        }

                        if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::M)
                            skillTree.close();
                    }
                }
                if (event.type == sf::Event::KeyReleased) {
                    if (event.key.code == sf::Keyboard::F11) wasF11Pressed = false;
                }
            }
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        if (appState == AppState::MENU) {
            window.clear(sf::Color::Black);
            mainMenu->render(window);
            window.display();
        } else {
            update(dt);
            render();
        }
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void Game::update(float dt) {
    if (isPaused) return;

    Player* playerPtr = objects.empty()
        ? nullptr : dynamic_cast<Player*>(objects[0].get());

    if (playerPtr && playerPtr->isDeadNow()) {
        if (!playerPtr->consumeSecondChance()) {
            resetRun();
            return;
        }
    }

    if (vendingUI.isOpen() || skillTree.isOpen()) return;

    bool mNow = sf::Keyboard::isKeyPressed(sf::Keyboard::M);
    if (mNow && !wasMPressed) skillTree.toggle();
    wasMPressed = mNow;

    // ── Use held item (F key) ─────────────────────────────────────────────────
    bool fNow = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
    if (fNow && !wasFPressed && !heldItem.empty() && playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        sf::Vector2f  pc = {pb.left + pb.width / 2.f, pb.top + pb.height / 2.f};

        if (heldItem == "Monster Energy") {
            playerPtr->applyMonsterBuff();
            heldItem = "";
        }
        else if (heldItem == "Pizza") {
            playerPtr->healFull();
            heldItem = "";
        }
        else if (heldItem == "Duo") {
            objects.push_back(std::make_unique<HelperCompanion>(pc.x, pc.y));
            heldItem = "";
        }
        else if (heldItem == "Annoying Dog") {
            objects.push_back(std::make_unique<AnnoyingDog>(pc.x, pc.y));
            heldItem = "";
        }
        else if (heldItem == "Totem") {
            playerPtr->addTotemCharge();
            heldItem = "";
        }
    }
    wasFPressed = fNow;

    std::vector<std::unique_ptr<GameObject>> spawnQueue;

    for (auto& obj : objects) {
        if (!obj->isActive()) continue;
        obj->update(dt, window);
        if (auto* enemy = dynamic_cast<Enemy*>(obj.get()))
            if (playerPtr)
                enemy->updateAI(dt, getRectCenter(playerPtr->getBounds()), spawnQueue);
        if (auto* helper = dynamic_cast<HelperCompanion*>(obj.get()))
            helper->tryHitEnemy(objects);
        if (auto* dog = dynamic_cast<AnnoyingDog*>(obj.get()))
            dog->tryExplode(objects, spawnQueue);
    }

    if (playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        if (pb.top + pb.height > UI_BAR_Y)
            playerPtr->setPosition({getRectCenter(pb).x, UI_BAR_Y - pb.height / 2.f});
    }

    if (playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        for (const sf::FloatRect& col : rooms[currentRoomIndex]->getPropColliders()) {
            if (!pb.intersects(col)) continue;
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
            pb = playerPtr->getBounds();
        }
    }

    const auto& propColliders = rooms[currentRoomIndex]->getPropColliders();
    for (auto& obj : objects) {
        auto* enemy = dynamic_cast<Enemy*>(obj.get());
        if (!enemy || !enemy->isActive()) continue;
        sf::FloatRect eb = enemy->getBounds();
        for (const sf::FloatRect& col : propColliders) {
            if (!eb.intersects(col)) continue;
            float overlapL = (eb.left + eb.width)  - col.left;
            float overlapR = (col.left + col.width) - eb.left;
            float overlapT = (eb.top  + eb.height)  - col.top;
            float overlapB = (col.top + col.height)  - eb.top;
            float minH = (overlapL < overlapR) ? -overlapL :  overlapR;
            float minV = (overlapT < overlapB) ? -overlapT :  overlapB;
            sf::Vector2f centre = getRectCenter(eb);
            sf::Vector2f newPos;
            if (std::abs(minH) < std::abs(minV))
                newPos = {centre.x + minH, centre.y};
            else
                newPos = {centre.x, centre.y + minV};
            enemy->nudgePosition(newPos - centre);
            eb = enemy->getBounds();
        }
    }

    for (auto& o : spawnQueue) objects.push_back(std::move(o));
    spawnQueue.clear();

    // ── Collisions ────────────────────────────────────────────────────────────
    const int collisionCount = static_cast<int>(objects.size());
    for (int idx = 0; idx < collisionCount; ++idx) {
        auto& objA = objects[idx];
        if (!objA->isActive()) continue;

        if (playerPtr && playerPtr->isAttackingNow()) {
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

            if (auto* enemy = dynamic_cast<Enemy*>(objA.get())) {
                if (playerPtr->getSwordBounds().intersects(enemy->getBounds())) {
                    bool wasAlive = enemy->isActive();
                    int dmg = playerPtr->isMonsterOneHit() ? 9999 : playerPtr->getComboHitDamage();
                    enemy->takeDamage(dmg);
                    if (wasAlive && !enemy->isActive()) {
                        int xpReward = dynamic_cast<DashEnemy*>(enemy) ? 3 : 2;
                        playerPtr->addXP(xpReward);

                        sf::Vector2f deathPos = getRectCenter(enemy->getBounds());
                        int coinDrop = dynamic_cast<DashEnemy*>(enemy) ? 2 : 1;
                        static std::uniform_real_distribution<float> scatter(-8.f, 8.f);
                        auto findOpenCoinPos = [&](sf::Vector2f base) -> sf::Vector2f {
                            static const sf::Vector2f offsets[] = {
                                {0,0},{12,0},{-12,0},{0,12},{0,-12},
                                {12,12},{-12,12},{12,-12},{-12,-12},{20,0},{-20,0}
                            };
                            sf::FloatRect test{base.x-5.f, base.y-5.f, 10.f, 10.f};
                            for (auto& off : offsets) {
                                test.left = base.x + off.x - 5.f;
                                test.top  = base.y + off.y - 5.f;
                                bool blocked = false;
                                for (const sf::FloatRect& col : propColliders)
                                    if (test.intersects(col)) { blocked = true; break; }
                                if (!blocked) return {base.x + off.x, base.y + off.y};
                            }
                            return base;
                        };
                        for (int ci = 0; ci < coinDrop; ++ci) {
                            sf::Vector2f raw(deathPos.x + scatter(rng), deathPos.y + scatter(rng));
                            sf::Vector2f safe = findOpenCoinPos(raw);
                            spawnQueue.push_back(std::make_unique<Coin>(safe.x, safe.y));
                        }
                    }
                }
            }
        }

        if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
            for (const sf::FloatRect& col : propColliders) {
                if (bullet->getBounds().intersects(col)) {
                    bullet->destroy();
                    break;
                }
            }
            if (!bullet->isActive()) continue;

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
                            int dmg = (playerPtr && playerPtr->isMonsterOneHit()) ? 9999 : 1;
                            e->takeDamage(dmg);
                            if (wasAlive && !e->isActive()) {
                                playerPtr->addXP(dynamic_cast<DashEnemy*>(e) ? 3 : 2);

                                sf::Vector2f deathPos = getRectCenter(e->getBounds());
                                int coinDrop = dynamic_cast<DashEnemy*>(e) ? 2 : 1;
                                static std::uniform_real_distribution<float> scatter2(-8.f, 8.f);
                                auto findOpenCoinPos2 = [&](sf::Vector2f base) -> sf::Vector2f {
                                    static const sf::Vector2f offsets[] = {
                                        {0,0},{12,0},{-12,0},{0,12},{0,-12},
                                        {12,12},{-12,12},{12,-12},{-12,-12},{20,0},{-20,0}
                                    };
                                    sf::FloatRect test{base.x-5.f, base.y-5.f, 10.f, 10.f};
                                    for (auto& off : offsets) {
                                        test.left = base.x + off.x - 5.f;
                                        test.top  = base.y + off.y - 5.f;
                                        bool blocked = false;
                                        for (const sf::FloatRect& col : propColliders)
                                            if (test.intersects(col)) { blocked = true; break; }
                                        if (!blocked) return {base.x + off.x, base.y + off.y};
                                    }
                                    return base;
                                };
                                for (int ci = 0; ci < coinDrop; ++ci) {
                                    sf::Vector2f raw(deathPos.x + scatter2(rng), deathPos.y + scatter2(rng));
                                    sf::Vector2f safe = findOpenCoinPos2(raw);
                                    spawnQueue.push_back(std::make_unique<Coin>(safe.x, safe.y));
                                }
                            }
                            bullet->destroy();
                            break;
                        }
                    }
                }
            }
        }

       // ── Player & Enemy Contact (Damage & Push Apart) ──────────────────────
        if (auto* enemy = dynamic_cast<Enemy*>(objA.get())) {
            if (playerPtr && playerPtr->isActive()) {
                sf::FloatRect playerBounds = playerPtr->getBounds();
                sf::FloatRect enemyBounds  = enemy->getBounds();

                if (playerBounds.intersects(enemyBounds)) {
                    bool skipPush = false;
                    if (auto* de = dynamic_cast<DashEnemy*>(enemy)) {
                        if (de->isDashingNow()) {
                            if (!playerPtr->isDashingNow()) {
                                playerPtr->takeDamage(1);
                            }
                            skipPush = true;
                        }
                    }

                    if (!skipPush) {
                        float overlapLeft   = (playerBounds.left + playerBounds.width)  - enemyBounds.left;
                        float overlapRight  = (enemyBounds.left  + enemyBounds.width)   - playerBounds.left;
                        float overlapTop    = (playerBounds.top  + playerBounds.height) - enemyBounds.top;
                        float overlapBottom = (enemyBounds.top   + enemyBounds.height)  - playerBounds.top;

                        float minX = std::min(overlapLeft, overlapRight);
                        float minY = std::min(overlapTop,  overlapBottom);

                        sf::Vector2f pushEnemy(0.f, 0.f);
                        sf::Vector2f pushPlayer(0.f, 0.f);

                        if (minX < minY) {
                            float half = minX * 0.5f;
                            if (overlapLeft < overlapRight) {
                                pushPlayer.x = -half;
                                pushEnemy.x  =  half;
                            } else {
                                pushPlayer.x =  half;
                                pushEnemy.x  = -half;
                            }
                        } else {
                            float half = minY * 0.5f;
                            if (overlapTop < overlapBottom) {
                                pushPlayer.y = -half;
                                pushEnemy.y  =  half;
                            } else {
                                pushPlayer.y =  half;
                                pushEnemy.y  = -half;
                            }
                        }

                        playerPtr->setPosition(sf::Vector2f(
                            playerBounds.left + playerBounds.width  / 2.f + pushPlayer.x,
                            playerBounds.top  + playerBounds.height / 2.f + pushPlayer.y
                        ));

                        enemy->nudgePosition(pushEnemy);

                        if (!playerPtr->isDashingNow() && 
                            !dynamic_cast<DashEnemy*>(enemy) && 
                            !dynamic_cast<BulletEnemy*>(enemy)) 
                        {
                            playerPtr->takeDamage(1);
                        }
                    }
                }
            }
        }
        // ──────────────────────────────────────────────────────────────────────

        if (auto* coin = dynamic_cast<Coin*>(objA.get())) {
            if (playerPtr && playerPtr->getBounds().intersects(coin->getBounds())) {
                coin->destroy();
                ++totalCoins;
            }
        }
    }

    for (auto& o : spawnQueue) objects.push_back(std::move(o));
    spawnQueue.clear();

    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [](const std::unique_ptr<GameObject>& o){ return !o->isActive(); }),
        objects.end());

    int alive = 0;
    for (auto& o : objects) if (dynamic_cast<Enemy*>(o.get())) alive++;

    if (alive == 0 && enemiesRemainingToSpawn > 0) {
        int minW = 1;
        int maxW = std::min(2 + currentRoomIndex / 3, 5);
        std::uniform_int_distribution<int> wsd(minW, maxW);
        int wc = std::min(wsd(rng), enemiesRemainingToSpawn);
        for (int i = 0; i < wc; ++i) { spawnEnemy(); enemiesRemainingToSpawn--; }
        alive = wc;
    }

    bool cleared = (alive == 0 && enemiesRemainingToSpawn == 0);
    rooms[currentRoomIndex]->setCleared(cleared);
    doorShape.setFillColor(cleared ? sf::Color(230, 180, 40) : sf::Color(70, 70, 70));

    // ── Vending proximity ─────────────────────────────────────────────────────
    nearVending = false;
    sf::Vector2f vendingPromptPos{};
    if (playerPtr && playerPtr->isActive()) {
        sf::Vector2f pc = getRectCenter(playerPtr->getBounds());
        sf::FloatRect vb = getClosestVendingBounds(pc);
        if (vb.width > 0.f) {
            sf::Vector2f vc = getRectCenter(vb);
            float dx = pc.x - vc.x, dy = pc.y - vc.y;
            if (std::sqrt(dx*dx + dy*dy) < VENDING_INTERACT_DIST + vb.width / 2.f) {
                nearVending = true;
                vendingPromptPos = {vc.x - 10.f, vb.top - 16.f};

                bool eNow = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
                if (eNow && !wasEPressed && !skillTree.isOpen()) {
                    vendingUI.openShop();
                }
                wasEPressed = eNow;
            }
        }
    }

    // ── Door interaction (only when not near vending) ─────────────────────────
    if (!nearVending && cleared && playerPtr) {
        if (playerPtr->getBounds().intersects(doorShape.getGlobalBounds())) {
            bool eNow = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
            if (eNow && !wasEPressed) nextRoom();
            wasEPressed = eNow;
        }
    }

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        wasEPressed = false;
}

// ── Render ────────────────────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color::Black);

    rooms[currentRoomIndex]->draw(window);

    doorShape.setPosition(rooms[currentRoomIndex]->getDoorPosition());
    doorShape.setRotation(rooms[currentRoomIndex]->getDoorRotation());
    window.draw(doorShape);

    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i)
        if (objects[i]->isActive()) objects[i]->draw(window);

    Player* playerPtr = objects.empty()
        ? nullptr : dynamic_cast<Player*>(objects[0].get());

    hud.renderSkillsHint(window, playerPtr);
    hud.render(window, playerPtr, rooms[currentRoomIndex]->getId(), totalCoins);

    if (playerPtr && rooms[currentRoomIndex]->getIsCleared() && !nearVending) {
        sf::Vector2f pp = getRectCenter(playerPtr->getBounds());
        sf::Vector2f dp = doorShape.getPosition();
        float dx = pp.x - dp.x, dy = pp.y - dp.y;
        if (std::sqrt(dx*dx + dy*dy) < 45.f) {
            interactText.setPosition(dp.x - 15.f, dp.y - 25.f);
            window.draw(interactText);
        }
    }

    if (nearVending && !vendingUI.isOpen() && playerPtr) {
        sf::Vector2f pc = getRectCenter(playerPtr->getBounds());
        sf::FloatRect vb = getClosestVendingBounds(pc);
        if (vb.width > 0.f) {
            sf::Vector2f vc = getRectCenter(vb);
            interactText.setPosition(vc.x - 10.f, vb.top - 16.f);
            window.draw(interactText);
        }
    }

    if (!heldItem.empty()) {
        sf::Text heldLabel("[F] " + heldItem, font, 16);
        heldLabel.setScale(0.75f, 0.75f);
        heldLabel.setFillColor(sf::Color(180, 230, 180));
        float hw = heldLabel.getGlobalBounds().width;
        float hx = 328.f - hw - 4.f;
        if (hx < 160.f) { heldLabel.setScale(0.6f, 0.6f); hw = heldLabel.getGlobalBounds().width; hx = 328.f - hw - 4.f; }
        heldLabel.setPosition(std::max(2.f, hx), 195.f + 2.f);
        window.draw(heldLabel);
    }

    if (playerPtr && playerPtr->hasMonsterBuff()) {
        sf::Text buffLabel("MONSTER!", font, 16);
        buffLabel.setScale(0.75f, 0.75f);
        buffLabel.setFillColor(sf::Color(50, 220, 80));
        float bw = buffLabel.getGlobalBounds().width;
        buffLabel.setPosition(328.f - bw - 4.f, 195.f + 14.f);
        window.draw(buffLabel);
    }

    if (skillTree.isOpen())
        skillTree.render(window, playerPtr);

    if (vendingUI.isOpen())
        vendingUI.render(window, totalCoins, heldItem);

    if (isPaused) {
        drawPauseMenu();
    }

    window.display();
}