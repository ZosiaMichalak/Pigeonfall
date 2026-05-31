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

// Returns true if ANY of the 3 save slots has data
static bool anySlotExists() {
    for (int i = 0; i < SAVE_SLOT_COUNT; ++i)
        if (SaveSystem::hasSlot(i)) return true;
    return false;
}

// ── Constructor ───────────────────────────────────────────────────────────────
Game::Game()
    : hud(font), skillTree(font), vendingUI(font), slotUI(font),
      totalCoins(50), nearVending(false),
      isPaused(false), pauseSel(0),
      fadeTimer(0.f), gameOverSel(0), gameOverTimer(0.f)
{
    isFullscreen  = true;
    wasF11Pressed = false;
    wasEPressed   = false;
    wasFPressed   = false;
    wasMPressed   = false;
    enemiesRemainingToSpawn = 0;
    currentRoomIndex = 0;
    activeSlot = 0;
    pendingMenuAction = MenuAction::NONE;

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    rng.seed(static_cast<unsigned>(std::time(nullptr)));

    initWindow();

    if (!font.loadFromFile("m5x7.ttf"))
        std::cerr << "[Warning] m5x7.ttf not found!\n";

    refreshFontTextures();

    // ── Pre-load shared assets ────────────────────────────────────────────────
    Bench::loadTexture();
    Vending::loadTexture();
    Trash::loadTexture();
    Coin::loadTexture();
    Hydrant::loadTexture();
    Flowers::loadTexture();
    Car::loadTextures();
    loadSpawnTextures();

    // ── Main menu ─────────────────────────────────────────────────────────────
    appState = AppState::MENU;
    mainMenu = std::make_unique<MainMenu>(font, anySlotExists(), isFullscreen, musicVolume);

    // ── Music ─────────────────────────────────────────────────────────────────
    if (music.openFromFile("music/Track12.wav")) {
        music.setLoop(true);
        music.setVolume(static_cast<float>(musicVolume));
        music.play();
    } else {
        std::cerr << "[Warning] Track12.wav not found — no music.\n";
    }

    interactText.setFont(font);
    interactText.setCharacterSize(18);
    interactText.setFillColor(sf::Color::White);
    interactText.setString("[E]");

    // Door — sprite is 50x80, two 50x40 frames stacked vertically (locked | open)
    doorShape.setSize(sf::Vector2f(100.f, 80.f));
    doorShape.setOrigin(50.f, 40.f);
    doorShape.setFillColor(sf::Color::Transparent);

    if (doorTexture.loadFromFile("assets/Tram.png")) {
        doorTexture.setSmooth(false);
        doorSprite.setTexture(doorTexture);
        doorSprite.setTextureRect(sf::IntRect(0, 0, 50, 40));
        doorSprite.setOrigin(25.f, 20.f);
        doorSprite.setScale(1.7f, 1.7f);
    }

    // Create the starter room and spawn the player at its playerStart position
    auto starterTmpl = RoomTemplates::getAll()[0];
    rooms.push_back(std::make_unique<Room>(0, starterTmpl));
    rooms[0]->loadAssets();

    // Starter room has no enemies and is immediately cleared
    rooms[0]->setCleared(true);
    enemiesRemainingToSpawn = 0;

    objects.push_back(std::make_unique<Player>(
        starterTmpl.playerStart.x, starterTmpl.playerStart.y));
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

void Game::applyMusicVolume() {
    music.setVolume(static_cast<float>(musicVolume));
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

// ── Floating damage numbers ───────────────────────────────────────────────────
void Game::spawnDamageNumber(sf::Vector2f pos, int damage, bool isBig) {
    DamageNumber dn;
    dn.text.setFont(font);
    dn.text.setString(std::to_string(damage));
    dn.text.setCharacterSize(16);

    if (isBig) {
        dn.text.setFillColor(sf::Color(255, 220, 0));
        dn.text.setScale(1.1f, 1.1f);
        dn.maxLifetime = 0.75f;
    } else {
        dn.text.setFillColor(sf::Color(255, 255, 255));
        dn.text.setScale(0.6f, 0.6f);
        dn.maxLifetime = 0.5f;
    }

    dn.text.setPosition(pos.x - dn.text.getLocalBounds().width * dn.text.getScale().x / 2.f,
                        pos.y - 8.f);
    std::uniform_real_distribution<float> drift(-15.f, 15.f);
    dn.velocity  = { drift(rng), -40.f };
    dn.lifetime  = dn.maxLifetime;
    damageNumbers.push_back(std::move(dn));
}

void Game::updateDamageNumbers(float dt) {
    for (auto& dn : damageNumbers) {
        dn.lifetime -= dt;
        dn.text.move(dn.velocity * dt);
        float alpha = std::min(1.f, dn.lifetime / (dn.maxLifetime * 0.4f));
        sf::Color c = dn.text.getFillColor();
        c.a = static_cast<sf::Uint8>(255 * std::max(0.f, alpha));
        dn.text.setFillColor(c);
    }
    damageNumbers.erase(
        std::remove_if(damageNumbers.begin(), damageNumbers.end(),
            [](const DamageNumber& d){ return d.lifetime <= 0.f; }),
        damageNumbers.end());
}

void Game::drawDamageNumbers() {
    for (auto& dn : damageNumbers)
        window.draw(dn.text);
}

void Game::spawnEnemy() {
    std::uniform_real_distribution<float> xDis(50.f, 350.f);
    std::uniform_real_distribution<float> yDis(30.f, 160.f);
    float ex = xDis(rng), ey = yDis(rng);

    if (!objects.empty()) {
        if (auto* p = dynamic_cast<Player*>(objects[0].get())) {
            sf::Vector2f pp = getRectCenter(p->getBounds());
            int tries = 0;
            while (std::abs(ex - pp.x) < 40.f && std::abs(ey - pp.y) < 40.f && tries < 20) {
                ex = xDis(rng); ey = yDis(rng); ++tries;
            }
        }
    }

    int tier = std::min(4, currentRoomIndex / 3);

    int dashChance = 0;
    if      (currentRoomIndex >= 10) dashChance = 55;
    else if (currentRoomIndex >=  6) dashChance = 45;
    else if (currentRoomIndex >=  3) dashChance = 35;
    else if (currentRoomIndex >=  1) dashChance = 20;

    std::uniform_int_distribution<int> typeDis(0, 99);
    bool isDash = (typeDis(rng) < dashChance);

    pendingSpawns.push_back({ {ex, ey}, 0.f, isDash, tier });
}

// ── Spawn effects ─────────────────────────────────────────────────────────────
void Game::updateSpawnEffects(float dt) {
    for (auto it = pendingSpawns.begin(); it != pendingSpawns.end(); ) {
        it->timer += dt;
        if (it->timer >= it->duration()) {
            if (it->pendingObject) {
                if (auto* e = dynamic_cast<Enemy*>(it->pendingObject.get()))
                    e->setSpawning(false);
                objects.push_back(std::move(it->pendingObject));
            } else {
                if (it->isDash)
                    objects.push_back(std::make_unique<DashEnemy>(it->pos.x, it->pos.y, it->tier));
                else
                    objects.push_back(std::make_unique<BulletEnemy>(it->pos.x, it->pos.y, it->tier));
            }
            it = pendingSpawns.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::loadSpawnTextures() {
    if (spawnTexLoaded) return;
    spawnTexBullet.loadFromFile("assets/bulletEnemy_spawn.png");
    spawnTexBullet.setSmooth(false);
    spawnTexDash.loadFromFile("assets/dashEnemy_spawn.png");
    spawnTexDash.setSmooth(false);
    spawnTexLoaded = true;
}

void Game::drawSpawnEffects() {
    static constexpr int   FW     = 51;
    static constexpr int   FH     = 32;
    static constexpr int   COLS   = 2;
    static constexpr int   FRAMES = 8;

    for (const auto& sp : pendingSpawns) {
        sf::Texture& tex = sp.isDash ? spawnTexDash : spawnTexBullet;
        if (tex.getSize().x == 0) continue;

        float t     = sp.timer / sp.duration();
        int   frame = static_cast<int>(t * FRAMES);
        if (frame >= FRAMES) frame = FRAMES - 1;

        int col = frame % COLS;
        int row = frame / COLS;

        sf::Sprite s(tex);
        s.setTextureRect(sf::IntRect(col * FW, row * FH, FW, FH));
        s.setOrigin(FW / 2.f, FH / 2.f);
        s.setPosition(sp.pos);
        window.draw(s);
    }
}

// ── Next room ─────────────────────────────────────────────────────────────────
void Game::nextRoom() {
    currentRoomIndex++;

    if (currentRoomIndex >= static_cast<int>(rooms.size())) {
        RoomTemplate tmpl = RoomTemplates::getRandom();
        rooms.push_back(std::make_unique<Room>(currentRoomIndex, tmpl));
        rooms.back()->loadAssets();
    }

    sf::Vector2f startPos = rooms[currentRoomIndex]->getPlayerStart();

    std::vector<std::unique_ptr<GameObject>> newObjects;
    auto it = std::find_if(objects.begin(), objects.end(),
                           [](const auto& o){ return dynamic_cast<Player*>(o.get()) != nullptr; });
    if (it != objects.end()) {
        static_cast<Player*>(it->get())->setPosition(startPos);
        newObjects.push_back(std::move(*it));
    } else {
        newObjects.push_back(std::make_unique<Player>(startPos.x, startPos.y));
    }
    objects = std::move(newObjects);

    int totalEnemies = std::min(2 + (currentRoomIndex - 1), 12);
    enemiesRemainingToSpawn = totalEnemies;

    isBossRoom = (currentRoomIndex == 4);
    if (isBossRoom) {
        enemiesRemainingToSpawn = 0;

        RoomTemplate bossTemplate;
        bossTemplate.name         = "Boss Arena";
        bossTemplate.background   = 0;
        bossTemplate.props        = {};
        bossTemplate.doorPosition = {400.f, 100.f};
        bossTemplate.doorRotation = 90.f;
        bossTemplate.playerStart  = {50.f, 110.f};
        rooms[currentRoomIndex]   = std::make_unique<Room>(currentRoomIndex, bossTemplate);
        rooms[currentRoomIndex]->loadAssets();

        if (auto* p = dynamic_cast<Player*>(objects[0].get()))
            p->setPosition(bossTemplate.playerStart);

        objects.push_back(std::make_unique<PigeonKing>(200.f, 60.f));
    }

    vendingUI.rollItems();
    vendingUI.close();
    nearVending = false;
    pendingSpawns.clear();
}

// ── Reset run ─────────────────────────────────────────────────────────────────
void Game::resetRun() {
    Player::resetRunStats();

    rooms.clear();
    auto starterTmpl = RoomTemplates::getAll()[0];
    rooms.push_back(std::make_unique<Room>(0, starterTmpl));
    rooms[0]->loadAssets();
    rooms[0]->setCleared(true);

    currentRoomIndex        = 0;
    enemiesRemainingToSpawn = 0;
    totalCoins              = 50;
    heldItem                = "";
    skillTree.close();
    vendingUI.close();
    nearVending  = false;

    wasEPressed   = false;
    wasFPressed   = false;
    wasMPressed   = false;

    objects.clear();
    objects.push_back(std::make_unique<Player>(
        starterTmpl.playerStart.x, starterTmpl.playerStart.y));

    vendingUI.rollItems();
}

// ── Save / Load (slot-aware) ──────────────────────────────────────────────────
void Game::saveGame(int slot) {
    Player* playerPtr = objects.empty() ? nullptr : dynamic_cast<Player*>(objects[0].get());
    if (!playerPtr) return;

    SaveData sd;
    sd.exists           = true;
    sd.level            = playerPtr->getLevel();
    sd.xp               = playerPtr->getXP();
    sd.xpToNext         = playerPtr->getXPToNext();
    sd.skillPoints      = playerPtr->getSkillPoints();
    sd.secondChanceUsed = playerPtr->isSecondChanceUsed();
    sd.totemCharges     = playerPtr->getTotemCharges();

    for (int i = 0; i < SKILL_COUNT; ++i)
        sd.upgrades[i] = playerPtr->getUpgradeLevel(i);

    sd.roomIndex   = currentRoomIndex;
    sd.coins       = totalCoins;
    sd.heldItem    = heldItem;
    sd.fullscreen  = isFullscreen;
    sd.musicVolume = musicVolume;

    SaveSystem::save(sd, slot);
}

void Game::loadGame(int slot) {
    SaveData sd = SaveSystem::load(slot);
    if (!sd.exists) return;

    resetRun();
    if (!objects.empty()) {
        if (auto* p = dynamic_cast<Player*>(objects[0].get()))
            p->applyLoadedSave(sd);
    }
    isFullscreen = sd.fullscreen;
    musicVolume  = sd.musicVolume;
    applyWindowMode();
    applyMusicVolume();
    heldItem   = sd.heldItem;
    totalCoins = sd.coins;

    currentRoomIndex = 0;
    for (int i = 0; i < sd.roomIndex; ++i) nextRoom();
}

// ── Pause menu ────────────────────────────────────────────────────────────────
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

    sf::Text hint("W/S  E=select  Q=close", font, 16);
    hint.setScale(0.65f, 0.65f);
    hint.setFillColor(sf::Color(90, 80, 125));
    hint.setPosition(VIEW_W / 2.f - hint.getGlobalBounds().width / 2.f, VIEW_H - 20.f);
    window.draw(hint);
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)  window.close();
            if (event.type == sf::Event::Resized) applyLetterboxView();

            // ── Game-over state ───────────────────────────────────────────────
            if (appState == AppState::GAME_OVER) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::W ||
                        event.key.code == sf::Keyboard::Up   ||
                        event.key.code == sf::Keyboard::S    ||
                        event.key.code == sf::Keyboard::Down)
                        gameOverSel = 1 - gameOverSel;

                    if (event.key.code == sf::Keyboard::E ||
                        event.key.code == sf::Keyboard::Return ||
                        event.key.code == sf::Keyboard::Space) {
                        if (gameOverSel == 0) {
                            resetRun();
                            appState = AppState::PLAYING;
                        } else {
                            resetRun();
                            mainMenu = std::make_unique<MainMenu>(font, anySlotExists(), isFullscreen, musicVolume);
                            appState = AppState::MENU;
                        }
                        gameOverMenu.reset();
                    }
                }
                continue;
            }

            // ── Dying state — eat all input ───────────────────────────────────
            if (appState == AppState::DYING) continue;

            // ── Slot select state ─────────────────────────────────────────────
            if (appState == AppState::SLOT_SELECT) {
                // F11 still works here
                if (event.type == sf::Event::KeyPressed &&
                    event.key.code == sf::Keyboard::F11 && !wasF11Pressed) {
                    wasF11Pressed = true;
                    isFullscreen = !isFullscreen;
                    applyWindowMode();
                }
                if (event.type == sf::Event::KeyReleased &&
                    event.key.code == sf::Keyboard::F11)
                    wasF11Pressed = false;

                SlotUIResult result = slotUI.handleEvent(event);
                if (result == SlotUIResult::SELECTED) {
                    activeSlot = slotUI.getSelectedSlot();
                    if (pendingMenuAction == MenuAction::LOAD_GAME) {
                        loadGame(activeSlot);
                    } else {
                        // NEW_GAME: wipe any existing save in that slot and start fresh
                        SaveSystem::deleteSlot(activeSlot);
                        resetRun();
                    }
                    appState = AppState::PLAYING;
                } else if (result == SlotUIResult::CANCELLED) {
                    appState = AppState::MENU;
                }
                continue;
            }

            // ── Menu state ────────────────────────────────────────────────────
            if (appState == AppState::MENU) {
                if (event.type == sf::Event::KeyPressed &&
                    event.key.code == sf::Keyboard::F11 && !wasF11Pressed) {
                    wasF11Pressed = true;
                    isFullscreen = !isFullscreen;
                    applyWindowMode();
                    mainMenu = std::make_unique<MainMenu>(font, anySlotExists(), isFullscreen, musicVolume);
                }
                if (event.type == sf::Event::KeyReleased &&
                    event.key.code == sf::Keyboard::F11)
                    wasF11Pressed = false;

                MenuAction action = mainMenu->handleEvent(event);
                if (action == MenuAction::NEW_GAME) {
                    pendingMenuAction = MenuAction::NEW_GAME;
                    slotUI.open(SlotUIMode::NEW_GAME);
                    appState = AppState::SLOT_SELECT;
                }
                else if (action == MenuAction::LOAD_GAME) {
                    pendingMenuAction = MenuAction::LOAD_GAME;
                    slotUI.open(SlotUIMode::LOAD);
                    appState = AppState::SLOT_SELECT;
                }
                else if (action == MenuAction::QUIT) {
                    window.close();
                }

                if (mainMenu->fullscreen() != isFullscreen) {
                    isFullscreen = mainMenu->fullscreen();
                    applyWindowMode();
                    mainMenu = std::make_unique<MainMenu>(font, anySlotExists(), isFullscreen, musicVolume);
                }
                if (mainMenu->musicVolume() != musicVolume) {
                    musicVolume = mainMenu->musicVolume();
                    applyMusicVolume();
                }
                continue;
            }

            // ── Gameplay state input ──────────────────────────────────────────
            if (appState == AppState::PLAYING) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    isPaused = !isPaused;
                    pauseSel = 0;
                    continue;
                }

                if (isPaused) {
                    if (event.type == sf::Event::KeyPressed) {
                        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
                            pauseSel = (pauseSel - 1 + 3) % 3;
                        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
                            pauseSel = (pauseSel + 1) % 3;
                        if (event.key.code == sf::Keyboard::E) {
                            if (pauseSel == 0) {
                                // Show slot UI in SAVE mode
                                pendingMenuAction = MenuAction::NEW_GAME; // reuse as "save intent"
                                slotUI.open(SlotUIMode::SAVE);
                                appState = AppState::SLOT_SELECT;
                                isPaused = false;
                            }
                            else if (pauseSel == 1) { isFullscreen = !isFullscreen; applyWindowMode(); }
                            else if (pauseSel == 2) {
                                mainMenu = std::make_unique<MainMenu>(font, anySlotExists(), isFullscreen, musicVolume);
                                appState = AppState::MENU;
                                isPaused = false;
                            }
                        }
                        if (event.key.code == sf::Keyboard::Q) { isPaused = false; }
                    }
                    continue;
                }

                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::F11 && !wasF11Pressed) {
                        isFullscreen = !isFullscreen;
                        applyWindowMode();
                        wasF11Pressed = true;
                    }

                    // Vending UI input
                    if (vendingUI.isOpen()) {
                        if (event.key.code == sf::Keyboard::Up   || event.key.code == sf::Keyboard::W)
                            vendingUI.moveSelection(-1);
                        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
                            vendingUI.moveSelection(+1);
                        if (event.key.code == sf::Keyboard::E) {
                            std::string bought = vendingUI.tryBuy(totalCoins, heldItem);
                            if (!bought.empty()) {
                                Player* p0 = objects.empty() ? nullptr : dynamic_cast<Player*>(objects[0].get());
                                if (bought == "Totem" && p0 && p0->hasTotemThisRun()) {
                                    totalCoins += 100;
                                } else {
                                    heldItem = bought;
                                    if (p0) p0->setHeldItem(heldItem);
                                }
                            }
                        }
                        if (event.key.code == sf::Keyboard::Q)
                            vendingUI.close();
                        continue;
                    }

                    // Skill tree input
                    if (skillTree.isOpen()) {
                        if (event.key.code == sf::Keyboard::Up   || event.key.code == sf::Keyboard::W)
                            skillTree.moveSelection(-1);
                        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
                            skillTree.moveSelection(+1);
                        if (event.key.code == sf::Keyboard::E) {
                            Player* p = objects.empty() ? nullptr : dynamic_cast<Player*>(objects[0].get());
                            skillTree.buySelected(p);
                        }
                        if (event.key.code == sf::Keyboard::Q || event.key.code == sf::Keyboard::Tab) {
                            skillTree.close();
                            wasMPressed = true;
                        }
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
        } else if (appState == AppState::SLOT_SELECT) {
            // Draw the menu background then overlay the slot picker
            window.clear(sf::Color::Black);
            mainMenu->render(window);
            slotUI.render(window);
            window.display();
        } else if (appState == AppState::DYING) {
            fadeTimer += dt;
            render();
            if (fadeTimer >= FADE_DURATION) {
                appState      = AppState::GAME_OVER;
                gameOverTimer = 0.f;
                gameOverSel   = 0;
                gameOverMenu  = std::make_unique<MainMenu>(font, false, isFullscreen);
            }
        } else if (appState == AppState::GAME_OVER) {
            gameOverTimer += dt;
            drawGameOver();
        } else {
            // Handle SLOT_SELECT result when coming from pause menu save
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
            appState  = AppState::DYING;
            fadeTimer = 0.f;
            return;
        }
    }

    if (vendingUI.isOpen() || skillTree.isOpen()) return;

    bool mNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Tab);
    if (mNow && !wasMPressed && !skillTree.isOpen()) skillTree.toggle();
    wasMPressed = mNow;

    // ── Use held item (F key) ─────────────────────────────────────────────────
    bool fNow = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
    if (fNow && !wasFPressed && !heldItem.empty() && playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        sf::Vector2f  pc = {pb.left + pb.width / 2.f, pb.top + pb.height / 2.f};

        if      (heldItem == "Monster Energy") { playerPtr->applyMonsterBuff(); heldItem = ""; playerPtr->setHeldItem(""); }
        else if (heldItem == "Pizza")          { playerPtr->healFull();          heldItem = ""; playerPtr->setHeldItem(""); }
        else if (heldItem == "Duo")            { objects.push_back(std::make_unique<HelperCompanion>(pc.x, pc.y)); heldItem = ""; playerPtr->setHeldItem(""); }
        else if (heldItem == "Annoying Dog")   { objects.push_back(std::make_unique<AnnoyingDog>(pc.x, pc.y)); heldItem = ""; playerPtr->setHeldItem(""); }
        else if (heldItem == "Totem")          { playerPtr->addTotemCharge();    heldItem = ""; playerPtr->setHeldItem(""); }
    }
    wasFPressed = fNow;

    std::vector<std::unique_ptr<GameObject>> spawnQueue;

    // ── Object updates ────────────────────────────────────────────────────────
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

    // ── Boss wave zone damage ─────────────────────────────────────────────────
    if (isBossRoom && playerPtr && playerPtr->isActive()) {
        for (auto& obj : objects) {
            if (auto* pk = dynamic_cast<PigeonKing*>(obj.get())) {
                sf::FloatRect pb = playerPtr->getBounds();
                for (auto& z : pk->getWaveZones()) {
                    if (!z.active || z.done) {
                        z.dmgTimer = 0.f;
                        continue;
                    }
                    if (z.rect.intersects(pb)) {
                        z.dmgTimer += dt;
                        if (z.dmgTimer >= 0.5f) {
                            playerPtr->takeDamage(1);
                            z.dmgTimer = 0.f;
                        }
                    } else {
                        z.dmgTimer = 0.f;
                    }
                }
                break;
            }
        }
    }

    // ── Keep player inside play area ──────────────────────────────────────────
    if (playerPtr && playerPtr->isActive()) {
        sf::FloatRect pb = playerPtr->getBounds();
        if (pb.top + pb.height > UI_BAR_Y)
            playerPtr->setPosition({getRectCenter(pb).x, UI_BAR_Y - pb.height / 2.f});
    }

    // ── Player vs prop collision ───────────────────────────────────────────────
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

    // ── Enemy vs prop collision ───────────────────────────────────────────────
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
            sf::Vector2f newPos = (std::abs(minH) < std::abs(minV))
                ? sf::Vector2f{centre.x + minH, centre.y}
                : sf::Vector2f{centre.x, centre.y + minV};
            enemy->nudgePosition(newPos - centre);
            eb = enemy->getBounds();
        }
    }

    // ── AnnoyingDog vs prop collision ─────────────────────────────────────────
    for (auto& obj2 : objects) {
        auto* dog = dynamic_cast<AnnoyingDog*>(obj2.get());
        if (!dog || !dog->isActive()) continue;
        sf::FloatRect db = dog->getBounds();
        for (const sf::FloatRect& col : propColliders) {
            if (!db.intersects(col)) continue;
            float overlapL = (db.left + db.width)  - col.left;
            float overlapR = (col.left + col.width) - db.left;
            float overlapT = (db.top  + db.height)  - col.top;
            float overlapB = (col.top + col.height)  - db.top;
            float minH = (overlapL < overlapR) ? -overlapL :  overlapR;
            float minV = (overlapT < overlapB) ? -overlapT :  overlapB;
            sf::Vector2f centre2 = getRectCenter(db);
            sf::Vector2f newPos2 = (std::abs(minH) < std::abs(minV))
                ? sf::Vector2f{centre2.x + minH, centre2.y}
                : sf::Vector2f{centre2.x, centre2.y + minV};
            dog->nudgePosition(newPos2 - centre2);
            db = dog->getBounds();
        }
    }

    // In boss room: route enemy spawns through pendingSpawns (0.3s anim, capped at 7)
    for (auto& o : spawnQueue) {
        Enemy* asEnemy = dynamic_cast<Enemy*>(o.get());
        bool   isPK    = dynamic_cast<PigeonKing*>(o.get()) != nullptr;
        if (isBossRoom && asEnemy && !isPK) {
            int cur = 0;
            for (auto& ex : objects) if (dynamic_cast<Enemy*>(ex.get()) && !dynamic_cast<PigeonKing*>(ex.get()) && ex->isActive()) cur++;
            cur += static_cast<int>(pendingSpawns.size());
            if (cur >= 7) continue;
            asEnemy->setSpawning(true);
            sf::FloatRect eb = asEnemy->getBounds();
            sf::Vector2f  ep = { eb.left + eb.width/2.f, eb.top + eb.height/2.f };
            SpawnEffect se;
            se.pos           = ep;
            se.timer         = 0.f;
            se.isDash        = dynamic_cast<DashEnemy*>(o.get()) != nullptr;
            se.tier          = 0;
            se.isBossSpawn   = true;
            se.pendingObject = std::move(o);
            pendingSpawns.push_back(std::move(se));
        } else {
            objects.push_back(std::move(o));
        }
    }
    spawnQueue.clear();

    // ── Collision detection ───────────────────────────────────────────────────
    const int collisionCount = static_cast<int>(objects.size());
    for (int idx = 0; idx < collisionCount; ++idx) {
        auto& objA = objects[idx];
        if (!objA->isActive()) continue;

        // Player sword vs bullets (deflect) and enemies (hit)
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
                    if (auto* pk = dynamic_cast<PigeonKing*>(enemy))
                        if (!pk->isVulnerable()) continue;
                    bool wasAlive    = enemy->isActive();
                    bool alreadyHit  = playerPtr->hasHitThisSwing();
                    int  dmg         = playerPtr->isMonsterOneHit()
                        ? 9999 : playerPtr->registerHit();
                    enemy->takeDamage(dmg);
                    if (!alreadyHit) {
                        sf::Vector2f epos = getRectCenter(enemy->getBounds());
                        bool bigHit = (!playerPtr->isMonsterOneHit() && playerPtr->getComboCount() == 3);
                        spawnDamageNumber(epos, std::min(dmg, 999), bigHit);
                    }
                    if (wasAlive && !enemy->isActive()) {
                        if (auto* pk = dynamic_cast<PigeonKing*>(enemy))
                            pk->getWaveZones().clear();

                        int xpReward = dynamic_cast<PigeonKing*>(enemy) ? 20
                                     : dynamic_cast<DashEnemy*>(enemy)  ?  3 : 2;
                        playerPtr->addXP(xpReward);

                        sf::Vector2f deathPos = getRectCenter(enemy->getBounds());
                        int coinDrop = dynamic_cast<PigeonKing*>(enemy) ? 10
                                     : dynamic_cast<DashEnemy*>(enemy)  ?  2 : 1;
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
                            spawnQueue.push_back(std::make_unique<Coin>(
                                findOpenCoinPos(raw).x, findOpenCoinPos(raw).y));
                        }
                    }
                }
            }
        }

        // Bullet vs props / player / enemies
        if (auto* bullet = dynamic_cast<Bullet*>(objA.get())) {
            for (const sf::FloatRect& col : propColliders) {
                if (bullet->getBounds().intersects(col)) { bullet->destroy(); break; }
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
                            if (auto* pk = dynamic_cast<PigeonKing*>(e))
                                if (!pk->isVulnerable()) continue;
                            bool wasAlive = e->isActive();
                            int  dmg = (playerPtr && playerPtr->isMonsterOneHit()) ? 9999 : 1;
                            e->takeDamage(dmg);
                            if (wasAlive && !e->isActive()) {
                                int xpR = dynamic_cast<PigeonKing*>(e) ? 20
                                        : dynamic_cast<DashEnemy*>(e)  ?  3 : 2;
                                playerPtr->addXP(xpR);
                                sf::Vector2f deathPos = getRectCenter(e->getBounds());
                                int coinDrop = dynamic_cast<PigeonKing*>(e) ? 10
                                             : dynamic_cast<DashEnemy*>(e)  ?  2 : 1;
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

        // Player vs enemy contact
        if (auto* enemy = dynamic_cast<Enemy*>(objA.get())) {
            if (playerPtr && playerPtr->isActive()) {
                sf::FloatRect playerBounds = playerPtr->getBounds();
                sf::FloatRect enemyBounds  = enemy->getBounds();

                if (playerBounds.intersects(enemyBounds)) {
                    bool skipPush = false;
                    if (auto* de = dynamic_cast<DashEnemy*>(enemy)) {
                        if (de->isDashingNow()) {
                            if (!playerPtr->isDashingNow())
                                playerPtr->takeDamage(1);
                            skipPush = true;
                        }
                    }
                    if (dynamic_cast<PigeonKing*>(enemy))
                        skipPush = true;

                    if (!skipPush) {
                        float overlapLeft   = (playerBounds.left + playerBounds.width)  - enemyBounds.left;
                        float overlapRight  = (enemyBounds.left  + enemyBounds.width)   - playerBounds.left;
                        float overlapTop    = (playerBounds.top  + playerBounds.height) - enemyBounds.top;
                        float overlapBottom = (enemyBounds.top   + enemyBounds.height)  - playerBounds.top;

                        float minX = std::min(overlapLeft, overlapRight);
                        float minY = std::min(overlapTop,  overlapBottom);

                        sf::Vector2f pushEnemy(0.f, 0.f), pushPlayer(0.f, 0.f);
                        if (minX < minY) {
                            float half = minX * 0.5f;
                            if (overlapLeft < overlapRight) { pushPlayer.x = -half; pushEnemy.x =  half; }
                            else                            { pushPlayer.x =  half; pushEnemy.x = -half; }
                        } else {
                            float half = minY * 0.5f;
                            if (overlapTop < overlapBottom) { pushPlayer.y = -half; pushEnemy.y =  half; }
                            else                            { pushPlayer.y =  half; pushEnemy.y = -half; }
                        }

                        playerPtr->setPosition(sf::Vector2f(
                            playerBounds.left + playerBounds.width  / 2.f + pushPlayer.x,
                            playerBounds.top  + playerBounds.height / 2.f + pushPlayer.y));
                        enemy->nudgePosition(pushEnemy);
                    }
                }
            }
        }

        // Coin pickup
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

    // ── Wave spawning ─────────────────────────────────────────────────────────
    int alive = 0;
    for (auto& o : objects) if (dynamic_cast<Enemy*>(o.get()) && o->isActive()) alive++;
    int totalActive = alive + static_cast<int>(pendingSpawns.size());

    if (!isBossRoom && totalActive == 0 && enemiesRemainingToSpawn > 0) {
        int minW = 1;
        int maxW = std::min(2 + currentRoomIndex / 3, 5);
        std::uniform_int_distribution<int> wsd(minW, maxW);
        int wc = std::min(wsd(rng), enemiesRemainingToSpawn);
        for (int i = 0; i < wc; ++i) { spawnEnemy(); enemiesRemainingToSpawn--; }
        totalActive += wc;
    }

    bool bossGone = true;
    if (isBossRoom) { for (auto& o : objects) if (dynamic_cast<PigeonKing*>(o.get()) && o->isActive()) { bossGone = false; break; } }
    bool cleared = isBossRoom
        ? (bossGone && alive == 0 && pendingSpawns.empty())
        : (totalActive == 0 && enemiesRemainingToSpawn == 0);
    rooms[currentRoomIndex]->setCleared(cleared);

    updateSpawnEffects(dt);
    updateDamageNumbers(dt);

    // ── Vending proximity ─────────────────────────────────────────────────────
    nearVending = false;
    if (playerPtr && playerPtr->isActive()) {
        sf::Vector2f pc = getRectCenter(playerPtr->getBounds());
        sf::FloatRect vb = getClosestVendingBounds(pc);
        if (vb.width > 0.f) {
            sf::Vector2f vc = getRectCenter(vb);
            float dx = pc.x - vc.x, dy = pc.y - vc.y;
            if (std::sqrt(dx*dx + dy*dy) < VENDING_INTERACT_DIST + vb.width / 2.f) {
                nearVending = true;

                bool eNow = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
                if (eNow && !wasEPressed && !skillTree.isOpen())
                    vendingUI.openShop();
                wasEPressed = eNow;
            }
        }
    }

    // ── Door interaction ──────────────────────────────────────────────────────
    if (!nearVending && rooms[currentRoomIndex]->getIsCleared() && playerPtr) {
        doorShape.setPosition(rooms[currentRoomIndex]->getDoorPosition());
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

    {
        sf::Vector2f dp = rooms[currentRoomIndex]->getDoorPosition();
        bool cleared = rooms[currentRoomIndex]->getIsCleared();

        doorShape.setPosition(dp);
        doorShape.setRotation(0.f);

        int frameY = cleared ? 40 : 0;
        doorSprite.setTextureRect(sf::IntRect(0, frameY, 50, 40));
        doorSprite.setPosition(dp);
        float tmplRot  = rooms[currentRoomIndex]->getDoorRotation();
        float spriteRot = 0.f;
        if (tmplRot == 0.f) {
            spriteRot = (dp.x > 200.f) ? 90.f : 270.f;
        }
        doorSprite.setRotation(spriteRot);
        window.draw(doorSprite);
    }

    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i)
        if (objects[i]->isActive()) objects[i]->draw(window);

    drawSpawnEffects();
    drawDamageNumbers();

    Player* playerPtr = objects.empty()
        ? nullptr : dynamic_cast<Player*>(objects[0].get());

    hud.renderSkillsHint(window, playerPtr);
    hud.render(window, playerPtr, rooms[currentRoomIndex]->getId(), totalCoins);

    // Door [E] prompt
    if (playerPtr && rooms[currentRoomIndex]->getIsCleared() && !nearVending) {
        sf::Vector2f pp = getRectCenter(playerPtr->getBounds());
        sf::Vector2f dp = rooms[currentRoomIndex]->getDoorPosition();
        float dx = pp.x - dp.x, dy = pp.y - dp.y;
        if (std::sqrt(dx*dx + dy*dy) < 70.f) {
            interactText.setPosition(dp.x - 10.f, dp.y - 24.f);
            window.draw(interactText);
        }
    }

    // Vending [E] prompt
    if (nearVending && !vendingUI.isOpen() && playerPtr) {
        sf::Vector2f pc = getRectCenter(playerPtr->getBounds());
        sf::FloatRect vb = getClosestVendingBounds(pc);
        if (vb.width > 0.f) {
            sf::Vector2f vc = getRectCenter(vb);
            interactText.setPosition(vc.x - 10.f, vb.top - 16.f);
            window.draw(interactText);
        }
    }

    // Held item label
    if (!heldItem.empty()) {
        sf::Text heldLabel("[F] " + heldItem, font, 16);
        heldLabel.setScale(0.75f, 0.75f);
        heldLabel.setFillColor(sf::Color(180, 230, 180));
        float hw = heldLabel.getGlobalBounds().width;
        float hx = 328.f - hw - 4.f;
        if (hx < 160.f) {
            heldLabel.setScale(0.6f, 0.6f);
            hw = heldLabel.getGlobalBounds().width;
            hx = 328.f - hw - 4.f;
        }
        heldLabel.setPosition(std::max(2.f, hx), 195.f + 2.f);
        window.draw(heldLabel);
    }

    // Monster buff label
    if (playerPtr && playerPtr->hasMonsterBuff()) {
        sf::Text buffLabel("MONSTER!", font, 16);
        buffLabel.setScale(0.75f, 0.75f);
        buffLabel.setFillColor(sf::Color(50, 220, 80));
        float bw = buffLabel.getGlobalBounds().width;
        buffLabel.setPosition(328.f - bw - 4.f, 195.f + 14.f);
        window.draw(buffLabel);
    }

    if (skillTree.isOpen())  skillTree.render(window, playerPtr);
    if (vendingUI.isOpen())  vendingUI.render(window, totalCoins, heldItem);
    if (isPaused)            drawPauseMenu();

    // ── Death fade overlay ────────────────────────────────────────────────────
    if (appState == AppState::DYING) {
        float t = std::min(1.f, fadeTimer / FADE_DURATION);
        sf::Uint8 alpha = static_cast<sf::Uint8>(t * t * 255.f);
        sf::RectangleShape fadeRect({VIEW_W, VIEW_H});
        fadeRect.setFillColor(sf::Color(0, 0, 0, alpha));
        window.draw(fadeRect);
    }

    window.display();
}

// ── Game Over screen ──────────────────────────────────────────────────────────
void Game::drawGameOver() {
    if (!gameOverMenu) return;

    window.clear(sf::Color::Black);
    gameOverMenu->render(window);

    static constexpr float VIEW_W = 400.f;
    static constexpr float VIEW_H = 225.f;
    static constexpr float PI     = 3.14159265f;

    auto rpx  = [](float v){ return std::floor(v + 0.5f); };
    auto lerp = [](float a, float b, float t){ return a + (b - a) * t; };

    float selGlow = std::sin(gameOverTimer * 3.5f) * 0.5f + 0.5f;

    const float PW = 160.f;
    const float PH = 88.f;
    const float PX = rpx((VIEW_W - PW) * 0.5f);
    const float PY = rpx(VIEW_H * 0.5f - PH * 0.5f + 14.f);

    {
        float g = 3.f;
        sf::RectangleShape glow({PW + g*2.f, PH + g*2.f});
        glow.setFillColor(sf::Color(0,0,0,0));
        glow.setOutlineThickness(g);
        glow.setOutlineColor(sf::Color(
            static_cast<sf::Uint8>(lerp(50.f, 90.f, selGlow)),
            static_cast<sf::Uint8>(lerp(20.f, 40.f, selGlow)),
            static_cast<sf::Uint8>(lerp(90.f, 150.f, selGlow)),
            static_cast<sf::Uint8>(selGlow * 80.f)));
        glow.setPosition(rpx(PX - g), rpx(PY - g));
        window.draw(glow);
    }

    sf::RectangleShape panel({PW, PH});
    panel.setFillColor(sf::Color(9, 9, 20));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(
        static_cast<sf::Uint8>(lerp(50.f, 90.f, selGlow)),
        static_cast<sf::Uint8>(lerp(32.f, 55.f, selGlow)),
        static_cast<sf::Uint8>(lerp(90.f, 150.f, selGlow))));
    panel.setPosition(rpx(PX), rpx(PY));
    window.draw(panel);

    sf::RectangleShape topLine({PW - 4.f, 1.f});
    topLine.setFillColor(sf::Color(90, 60, 140, 40));
    topLine.setPosition(rpx(PX + 2.f), rpx(PY + 1.f));
    window.draw(topLine);

    for (int cx = 0; cx < 2; ++cx) for (int cy = 0; cy < 2; ++cy) {
        float cr = 2.2f;
        sf::CircleShape cd(cr); cd.setOrigin(cr, cr);
        cd.setPosition(rpx(PX + cx * PW), rpx(PY + cy * PH));
        cd.setFillColor(sf::Color(
            static_cast<sf::Uint8>(lerp(80.f, 140.f, selGlow)),
            static_cast<sf::Uint8>(lerp(55.f, 100.f, selGlow)),
            static_cast<sf::Uint8>(lerp(140.f, 220.f, selGlow)), 200));
        window.draw(cd);
    }

    float tp = std::sin(gameOverTimer * 1.4f) * 0.5f + 0.5f;
    sf::Text goTitle("GAME OVER", font, 16);
    goTitle.setFillColor(sf::Color(
        static_cast<sf::Uint8>(lerp(200.f, 255.f, tp)),
        static_cast<sf::Uint8>(lerp(50.f,  90.f,  tp)),
        static_cast<sf::Uint8>(lerp(100.f, 160.f, tp))));
    float gtw = goTitle.getLocalBounds().width;
    goTitle.setPosition(rpx(PX + PW * 0.5f - gtw * 0.5f), rpx(PY + 8.f));
    window.draw(goTitle);

    float lineW = 100.f;
    sf::RectangleShape titleLine({lineW, 1.f});
    titleLine.setFillColor(sf::Color(120, 40, 80, 120));
    titleLine.setPosition(rpx(PX + PW * 0.5f - lineW * 0.5f), rpx(PY + 26.f));
    window.draw(titleLine);
    for (int side = 0; side < 2; ++side) {
        float dr = 1.8f;
        sf::CircleShape d(dr); d.setOrigin(dr, dr);
        d.setPosition(rpx(PX + PW*0.5f - lineW*0.5f + side*lineW), rpx(PY + 26.f));
        d.setFillColor(sf::Color(180, 60, 120, 180));
        window.draw(d);
    }

    const float ITEM_H = 20.f;
    const char* labels[] = { "Reborn", "Main Menu" };
    for (int i = 0; i < 2; ++i) {
        float iy  = PY + 34.f + i * ITEM_H;
        bool  sel = (gameOverSel == i);
        float sg  = sel ? selGlow : 0.f;

        if (sel) {
            sf::RectangleShape hl({PW - 6.f, ITEM_H - 2.f});
            hl.setFillColor(sf::Color(
                static_cast<sf::Uint8>(lerp(28.f, 50.f, sg)),
                static_cast<sf::Uint8>(lerp(18.f, 30.f, sg)),
                static_cast<sf::Uint8>(lerp(55.f, 80.f, sg))));
            hl.setOutlineThickness(1.f);
            hl.setOutlineColor(sf::Color(
                static_cast<sf::Uint8>(lerp(80.f, 140.f, sg)),
                static_cast<sf::Uint8>(lerp(50.f, 90.f,  sg)),
                static_cast<sf::Uint8>(lerp(150.f, 210.f, sg))));
            hl.setPosition(rpx(PX + 3.f), rpx(iy));
            window.draw(hl);

            float dotR = 2.5f;
            sf::CircleShape dot(dotR); dot.setOrigin(dotR, dotR);
            dot.setPosition(rpx(PX + 3.f + dotR), rpx(iy + ITEM_H * 0.5f - 1.f));
            dot.setFillColor(sf::Color(255,
                static_cast<sf::Uint8>(lerp(190.f, 220.f, sg)), 50,
                static_cast<sf::Uint8>(lerp(180.f, 255.f, sg))));
            window.draw(dot);

            sf::Vector2f a{rpx(PX + 3.f + dotR*2.f), rpx(iy + ITEM_H*0.5f - 1.f)};
            sf::Vector2f b{rpx(PX + 16.f),            rpx(iy + ITEM_H*0.5f - 1.f)};
            sf::Vector2f d2 = b - a;
            float len = std::sqrt(d2.x*d2.x + d2.y*d2.y);
            if (len > 0.f) {
                sf::RectangleShape line({len, 0.8f});
                line.setFillColor(sf::Color(255, 210, 50,
                    static_cast<sf::Uint8>(lerp(100.f, 200.f, sg))));
                line.setOrigin(0.f, 0.4f);
                line.setPosition(a);
                line.setRotation(std::atan2(d2.y, d2.x) * 180.f / PI);
                window.draw(line);
            }
        }

        sf::Text itemText(labels[i], font, 16);
        itemText.setFillColor(sel
            ? sf::Color(
                static_cast<sf::Uint8>(lerp(200.f, 240.f, sg)),
                static_cast<sf::Uint8>(lerp(185.f, 225.f, sg)),
                255)
            : sf::Color(130, 120, 155));
        itemText.setPosition(rpx(PX + 20.f), rpx(iy + 1.f));
        window.draw(itemText);
    }

    sf::Text hint("W/S  E=select", font, 16);
    hint.setScale(0.65f, 0.65f);
    hint.setFillColor(sf::Color(45, 35, 70));
    float hw = hint.getGlobalBounds().width;
    hint.setPosition(rpx((VIEW_W - hw) * 0.5f), rpx(VIEW_H - 14.f));
    window.draw(hint);

    window.display();
}