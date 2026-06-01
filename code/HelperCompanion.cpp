#include "HelperCompanion.h"
#include "Coin.h"
#include "DashEnemy.h"
#include "PigeonKing.h"
#include "DifficultySettings.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

HelperCompanion::HelperCompanion(float x, float y)
    : GameObject(x, y)
{
    shape.setRadius(5.f);
    shape.setOrigin(5.f, 5.f);
    shape.setFillColor(sf::Color(100, 200, 255));
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(200, 240, 255));
    shape.setPosition(position);
    velocity = {0.f, 0.f};

    bool hitOk  = texHit.loadFromFile("assets/Duo_hit.png");
    bool walkOk = texWalk.loadFromFile("assets/Duo_walk.png");
    if (hitOk)  texHit.setSmooth(false);
    if (walkOk) texWalk.setSmooth(false);

    if (hitOk || walkOk) {
        hasSprite = true;
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);

        if (hitOk) {
            hitSheetCols = std::max(1, static_cast<int>(texHit.getSize().x / FRAME_W));
            hitFrames    = countSheetFrames(texHit);
            sprite.setTexture(texHit);
            setAnimFrame(texHit, 0, hitSheetCols);
        }
        if (walkOk) {
            walkSheetCols = std::max(1, static_cast<int>(texWalk.getSize().x / FRAME_W));
            walkFrames    = countSheetFrames(texWalk);
            if (!hitOk) {
                sprite.setTexture(texWalk);
                setAnimFrame(texWalk, 0, walkSheetCols);
            }
        }
        applyFacingScale();
        sprite.setPosition(position);
    }
}

void HelperCompanion::applyFacingScale() {
    sprite.setScale(facingLeft ? -SPRITE_SCALE : SPRITE_SCALE, SPRITE_SCALE);
}

int HelperCompanion::countSheetFrames(const sf::Texture& tex) const {
    if (tex.getSize().x == 0) return 1;
    int cols = std::max(1, static_cast<int>(tex.getSize().x / FRAME_W));
    int rows = std::max(1, static_cast<int>(tex.getSize().y / FRAME_H));
    return std::max(1, cols * rows - 1); // last sheet frame is empty
}

void HelperCompanion::setAnimFrame(sf::Texture& tex, int frame, int sheetCols) {
    int col = frame % sheetCols;
    int row = frame / sheetCols;
    sprite.setTexture(tex);
    sprite.setTextureRect(sf::IntRect(col * FRAME_W, row * FRAME_H, FRAME_W, FRAME_H));
}

void HelperCompanion::tickHitAnim(float dt) {
    if (!hasSprite || texHit.getSize().x == 0) return;
    animTimer += dt;
    if (animTimer >= HIT_FRAME_DUR) {
        animTimer -= HIT_FRAME_DUR;
        animFrame = (animFrame + 1) % hitFrames;
    }
    setAnimFrame(texHit, animFrame, hitSheetCols);
}

void HelperCompanion::tickWalkAnim(float dt) {
    if (!hasSprite || texWalk.getSize().x == 0) return;
    animTimer += dt;
    if (animTimer >= WALK_FRAME_DUR) {
        animTimer -= WALK_FRAME_DUR;
        animFrame = (animFrame + 1) % walkFrames;
    }
    setAnimFrame(texWalk, animFrame, walkSheetCols);
}

int HelperCompanion::damageFor(Enemy* e) const {
    if (!e) return 1;
    if (e->isBoss()) return BOSS_HIT_DMG;
    return std::max(1, (e->getMaxHp() + 1) / 2);
}

void HelperCompanion::spawnCoinsForKill(Enemy* e, sf::Vector2f at,
                                      std::vector<std::unique_ptr<GameObject>>& spawnQueue) {
    if (!e) return;
    int baseDrop = dynamic_cast<PigeonKing*>(e) ? 10
                 : dynamic_cast<DashEnemy*>(e)  ?  2 : 1;
    int coinDrop = std::max(0, static_cast<int>(std::round(
        baseDrop * ActiveDifficulty::settings.coinDropMult)));

    for (int ci = 0; ci < coinDrop; ++ci) {
        float ox = (static_cast<float>(std::rand()) / RAND_MAX) * 16.f - 8.f;
        float oy = (static_cast<float>(std::rand()) / RAND_MAX) * 16.f - 8.f;
        spawnQueue.push_back(std::make_unique<Coin>(at.x + ox, at.y + oy));
    }
}

Enemy* HelperCompanion::findTarget(std::vector<std::unique_ptr<GameObject>>& objects,
                                   bool skipBoss) const {
    Enemy* target   = nullptr;
    Enemy* fallback = nullptr;
    float  minDist  = 9999.f;
    float  fallbackDist = 9999.f;

    for (auto& obj : objects) {
        auto* e = dynamic_cast<Enemy*>(obj.get());
        if (!e || !e->isActive()) continue;
        if (skipBoss && e->isBoss()) continue;

        sf::FloatRect eb = e->getBounds();
        sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
        float dx = ec.x - position.x;
        float dy = ec.y - position.y;
        float d  = std::sqrt(dx * dx + dy * dy);

        if (!e->canTakeItemDamage()) {
            if (d < fallbackDist) { fallbackDist = d; fallback = e; }
            continue;
        }
        if (d < minDist) { minDist = d; target = e; }
    }

    if (!target) target = fallback;
    return target;
}

void HelperCompanion::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;

    if (state == DuoState::EXIT) {
        exitTimer -= dt;
        tickWalkAnim(dt);
        if (hasSprite) {
            applyFacingScale();
            sprite.setPosition(position);
        } else {
            shape.setPosition(position);
        }
        if (exitTimer <= 0.f) destroy();
        return;
    }

    huntTimer -= dt;
    position += velocity * dt;
    tickHitAnim(dt);

    if (hasSprite) {
        applyFacingScale();
        sprite.setPosition(position);
    } else {
        shape.setPosition(position);
    }

    if (huntTimer <= 0.f) {
        state     = DuoState::EXIT;
        exitTimer = EXIT_DURATION;
        velocity  = {0.f, 0.f};
        animFrame = 0;
        animTimer = 0.f;
    }
}

void HelperCompanion::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (hasSprite)
        window.draw(sprite);
    else
        window.draw(shape);
}

sf::FloatRect HelperCompanion::getBounds() const {
    if (hasSprite)
        return sprite.getGlobalBounds();
    return shape.getGlobalBounds();
}

void HelperCompanion::tryHitEnemy(float dt,
                                  std::vector<std::unique_ptr<GameObject>>& objects,
                                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) {
    if (!isActive() || state != DuoState::HUNTING) return;

    bool skipBoss = (bossTarget && bossFocusTimer <= 0.f);
    Enemy* target = findTarget(objects, skipBoss);

    if (!target) {
        velocity = {0.f, 0.f};
        return;
    }

    if (target->isBoss()) {
        if (bossTarget != target) {
            bossTarget     = target;
            bossFocusTimer = BOSS_FOCUS_DUR;
        }
        bossFocusTimer -= dt;
        if (bossFocusTimer <= 0.f) {
            velocity = {0.f, 0.f};
            return;
        }
    } else {
        bossTarget     = nullptr;
        bossFocusTimer = 0.f;
    }

    sf::FloatRect eb = target->getBounds();
    sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
    sf::Vector2f  dir = ec - position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (len > 0.f) {
        facingLeft = dir.x < 0.f;
        velocity   = (dir / len) * SPEED;
    }

    if (len < HIT_DIST + target->getBounds().width * 0.25f) {
        if (!target->canTakeItemDamage()) return;
        bool wasAlive = target->isActive();
        target->takeDamage(damageFor(target));
        if (wasAlive && !target->isActive())
            spawnCoinsForKill(target, ec, spawnQueue);
        animFrame = 0;
        animTimer = 0.f;
    }
}
