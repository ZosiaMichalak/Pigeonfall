#include "HelperCompanion.h"
#include "Coin.h"
#include "DashEnemy.h"
#include "PigeonKing.h"
#include "DifficultySettings.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

#include "HelperCompanion.h"
#include "Coin.h"
#include "DashEnemy.h"
#include "PigeonKing.h"
#include "DifficultySettings.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

// Constructor: Initializes the companion's visual shapes, loads texture sheets, and configures animations.
HelperCompanion::HelperCompanion(float x, float y)
    : GameObject(x, y)
{
    // Configure default fallback circular shape
    shape.setRadius(5.f);
    shape.setOrigin(5.f, 5.f);
    shape.setFillColor(sf::Color(100, 200, 255));
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(200, 240, 255));
    shape.setPosition(position);
    velocity = {0.f, 0.f};

    // Load sheets for attacking and walking animations
    bool hitOk  = texHit.loadFromFile("assets/Duo_hit.png");
    bool walkOk = texWalk.loadFromFile("assets/Duo_walk.png");
    if (hitOk)  texHit.setSmooth(false);
    if (walkOk) texWalk.setSmooth(false);

    if (hitOk || walkOk) {
        hasSprite = true;
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);

        // Map sheets, calculate columns and frame counts
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

// Mirrors the sprite scale horizontally based on facing direction.
void HelperCompanion::applyFacingScale() {
    sprite.setScale(facingLeft ? -SPRITE_SCALE : SPRITE_SCALE, SPRITE_SCALE);
}

// Counts the number of non-empty layout columns/rows in a texture sheet.
int HelperCompanion::countSheetFrames(const sf::Texture& tex) const {
    if (tex.getSize().x == 0) return 1;
    int cols = std::max(1, static_cast<int>(tex.getSize().x / FRAME_W));
    int rows = std::max(1, static_cast<int>(tex.getSize().y / FRAME_H));
    return std::max(1, cols * rows - 1); // Subtract 1 as the final grid slot is reserved/empty
}

// Assigns the correct sub-rectangle of the texture to the sprite viewport.
void HelperCompanion::setAnimFrame(sf::Texture& tex, int frame, int sheetCols) {
    int col = frame % sheetCols;
    int row = frame / sheetCols;
    sprite.setTexture(tex);
    sprite.setTextureRect(sf::IntRect(col * FRAME_W, row * FRAME_H, FRAME_W, FRAME_H));
}

// Increments timer and cycles frames for the attacking hit animation sheet.
void HelperCompanion::tickHitAnim(float dt) {
    if (!hasSprite || texHit.getSize().x == 0) return;
    animTimer += dt;
    if (animTimer >= HIT_FRAME_DUR) {
        animTimer -= HIT_FRAME_DUR;
        animFrame = (animFrame + 1) % hitFrames;
    }
    setAnimFrame(texHit, animFrame, hitSheetCols);
}

// Increments timer and cycles frames for the walking animation sheet.
void HelperCompanion::tickWalkAnim(float dt) {
    if (!hasSprite || texWalk.getSize().x == 0) return;
    animTimer += dt;
    if (animTimer >= WALK_FRAME_DUR) {
        animTimer -= WALK_FRAME_DUR;
        animFrame = (animFrame + 1) % walkFrames;
    }
    setAnimFrame(texWalk, animFrame, walkSheetCols);
}

// Returns dynamic damage output depending on whether target is a boss.
int HelperCompanion::damageFor(Enemy* e) const {
    if (!e) return 1;
    if (e->isBoss()) return BOSS_HIT_DMG;
    return std::max(1, (e->getMaxHp() + 1) / 2); // Deals 50% max HP damage to regular enemies
}

// Spawns reward coins upon target destruction, scaled by active difficulty modifiers.
void HelperCompanion::spawnCoinsForKill(Enemy* e, sf::Vector2f at,
                                      std::vector<std::unique_ptr<GameObject>>& spawnQueue) {
    if (!e) return;
    // Determine reward amount based on enemy classification
    int baseDrop = dynamic_cast<PigeonKing*>(e) ? 10
                 : dynamic_cast<DashEnemy*>(e)  ?  2 : 1;
    int coinDrop = std::max(0, static_cast<int>(std::round(
        baseDrop * ActiveDifficulty::settings.coinDropMult)));

    // Instantiate and push coin collectibles into the queue with slight positional offsets
    for (int ci = 0; ci < coinDrop; ++ci) {
        float ox = (static_cast<float>(std::rand()) / RAND_MAX) * 16.f - 8.f;
        float oy = (static_cast<float>(std::rand()) / RAND_MAX) * 16.f - 8.f;
        spawnQueue.push_back(std::make_unique<Coin>(at.x + ox, at.y + oy));
    }
}

// Searches the active object registry to find the closest valid enemy target.
Enemy* HelperCompanion::findTarget(std::vector<std::unique_ptr<GameObject>>& objects,
                                   bool skipBoss) const {
    Enemy* target   = nullptr;
    Enemy* fallback = nullptr;
    float  minDist  = 9999.f;
    float  fallbackDist = 9999.f;

    for (auto& obj : objects) {
        auto* e = dynamic_cast<Enemy*>(obj.get());
        if (!e || !e->isActive()) continue;
        if (skipBoss && e->isBoss()) continue; // Skip boss focus if recently hit/focused

        sf::FloatRect eb = e->getBounds();
        sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
        float dx = ec.x - position.x;
        float dy = ec.y - position.y;
        float d  = std::sqrt(dx * dx + dy * dy);

        // Handle item-damage immunity fallbacks
        if (!e->canTakeItemDamage()) {
            if (d < fallbackDist) { fallbackDist = d; fallback = e; }
            continue;
        }
        if (d < minDist) { minDist = d; target = e; }
    }

    if (!target) target = fallback;
    return target;
}

// Main logic update: tracks remaining summon duration, processes movements, cycles animations.
void HelperCompanion::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;

    // EXIT State: Summon has expired. Despawn the companion after brief walk cycle.
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

    // HUNTING State: Update position, attack animation ticks, and manage summon cooldown.
    huntTimer -= dt;
    position += velocity * dt;
    tickHitAnim(dt);

    if (hasSprite) {
        applyFacingScale();
        sprite.setPosition(position);
    } else {
        shape.setPosition(position);
    }

    // Shift to exit state when summoning time finishes
    if (huntTimer <= 0.f) {
        state     = DuoState::EXIT;
        exitTimer = EXIT_DURATION;
        velocity  = {0.f, 0.f};
        animFrame = 0;
        animTimer = 0.f;
    }
}

// Draws either the animated companion sprite or fallback shape.
void HelperCompanion::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (hasSprite)
        window.draw(sprite);
    else
        window.draw(shape);
}

// Returns the global bounds rect bounding visual components.
sf::FloatRect HelperCompanion::getBounds() const {
    if (hasSprite)
        return sprite.getGlobalBounds();
    return shape.getGlobalBounds();
}

// Tracks the selected target, moves towards it, and inflicts damage when colliding.
void HelperCompanion::tryHitEnemy(float dt,
                                  std::vector<std::unique_ptr<GameObject>>& objects,
                                  std::vector<std::unique_ptr<GameObject>>& spawnQueue) {
    if (!isActive() || state != DuoState::HUNTING) return;

    // Determine target (skip boss focus if boss lock-on is currently in a cooldown)
    bool skipBoss = (bossTarget && bossFocusTimer <= 0.f);
    Enemy* target = findTarget(objects, skipBoss);

    if (!target) {
        velocity = {0.f, 0.f};
        return;
    }

    // Lock on focus logic for boss targets (so the companion doesn't instantly melt bosses)
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

    // Vector mechanics to guide the companion towards target coordinates
    sf::FloatRect eb = target->getBounds();
    sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
    sf::Vector2f  dir = ec - position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (len > 0.f) {
        facingLeft = dir.x < 0.f;
        velocity   = (dir / len) * SPEED;
    }

    // Hit registration logic: check for contact overlaps, deal damage, and spawn reward coins
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

