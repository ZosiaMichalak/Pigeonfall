/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#include "AnnoyingDog.h"
#include <cmath>

// Constructor: Configures fallback shape, explosion visual representation, loads textures, and sets up sprite components.
AnnoyingDog::AnnoyingDog(float x, float y)
    : GameObject(x, y)
{
    // Define the basic fallback circle shape (in case texture assets fail to load)
    shape.setRadius(6.f);
    shape.setOrigin(6.f, 6.f);
    shape.setFillColor(sf::Color(240, 230, 210));
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(180, 160, 120));
    shape.setPosition(position);
    velocity = {0.f, 0.f};

    // Setup the explosion visual overlay (fading orange circle showing blast radius)
    explosionShape.setRadius(BLAST_RADIUS);
    explosionShape.setOrigin(BLAST_RADIUS, BLAST_RADIUS);
    explosionShape.setFillColor(sf::Color(255, 160, 30, 160));
    explosionShape.setOutlineThickness(2.f);
    explosionShape.setOutlineColor(sf::Color(255, 80, 0, 200));

    // Attempt to load walking and explosion animation spritesheets
    bool walkOk = texWalk.loadFromFile("assets/Dog_walk.png");
    bool boomOk = texBoom.loadFromFile("assets/Dog_boom.png");

    if (walkOk || boomOk) {
        hasSprite = true;
        sprite.setOrigin(FRAME_W / 2.f, FRAME_H / 2.f);
        if (walkOk) {
            sprite.setTexture(texWalk);
            sprite.setTextureRect(sf::IntRect(0, 0, FRAME_W, FRAME_H));
        }
        sprite.setPosition(position);
    }
}

// Updates the dog's state machine (movement, animation progress, explosion fade/timers).
void AnnoyingDog::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;

    // State 1: EXPLODING - Dog has detoned; fade out the explosion visual and destroy the entity.
    if (state == DogState::EXPLODING) {
        windupTimer -= dt;
        float alpha = std::max(0.f, windupTimer / 0.3f);
        explosionShape.setFillColor(sf::Color(255, 160, 30, static_cast<sf::Uint8>(160 * alpha)));
        explosionShape.setOutlineColor(sf::Color(255, 80, 0, static_cast<sf::Uint8>(200 * alpha)));
        if (windupTimer <= 0.f) destroy();
        return;
    }

    // State 2: WINDUP - Preparing to detonate. Stop moving and cycle through the fuse animation frames.
    if (state == DogState::WINDUP) {
        windupTimer -= dt;
        
        animTimer += dt;
        float frameDur = WINDUP_DURATION / BOOM_FRAMES;
        if (animTimer >= frameDur) {
            animTimer -= frameDur;
            if (animFrame < BOOM_FRAMES - 1) animFrame++;
        }
        
        if (hasSprite && texBoom.getSize().x > 0) {
            int col = animFrame % 3;
            int row = animFrame / 3;
            sprite.setTexture(texBoom);
            sprite.setTextureRect(sf::IntRect(col * FRAME_W, row * FRAME_H, FRAME_W, FRAME_H));
            sprite.setScale(1.f, 1.f);
            sprite.setPosition(position);
        }
        return;
    }

    // State 3: WALK - Actively tracking and walking towards the nearest enemy target.
    position += velocity * dt;
    shape.setPosition(position);

    // Cycle through standard run/walk frames
    animTimer += dt;
    if (animTimer >= WALK_DUR) {
        animTimer -= WALK_DUR;
        animFrame = (animFrame + 1) % WALK_FRAMES;
    }
    
    if (hasSprite && texWalk.getSize().x > 0) {
        int col = animFrame % 2;
        int row = animFrame / 2;
        sprite.setTexture(texWalk);
        sprite.setTextureRect(sf::IntRect(col * FRAME_W, row * FRAME_H, FRAME_W, FRAME_H));
        sprite.setScale(facingLeft ? 1.f : -1.f, 1.f);
        sprite.setRotation(0.f);
        sprite.setPosition(position);
    }
}

// Renders the appropriate component (dog sprite, fallback circle, or active explosion wave).
void AnnoyingDog::draw(sf::RenderWindow& window) {
    if (!isActive()) return;

    if (state == DogState::EXPLODING) {
        window.draw(explosionShape);
        return;
    }

    if (hasSprite) {
        window.draw(sprite);
    } else {
        window.draw(shape);
    }
}

// State transition handler and targeting AI: tracks nearest enemy and triggers explosion calculations.
bool AnnoyingDog::tryExplode(std::vector<std::unique_ptr<GameObject>>& objects,
                               std::vector<std::unique_ptr<GameObject>>& /*spawnQueue*/) {
    if (!isActive()) return false;
    if (state == DogState::EXPLODING) return false;

    // Trigger the actual explosion when the windup timer reaches zero.
    if (state == DogState::WINDUP && windupTimer <= 0.f) {
        state       = DogState::EXPLODING;
        windupTimer = 0.3f; // Explosion visual lingers for 0.3 seconds
        explosionShape.setPosition(position);

        // Scan game objects to identify the nearest enemy to eliminate first
        Enemy* target = nullptr;
        float  minDist = 9999.f;
        for (auto& obj : objects) {
            auto* e = dynamic_cast<Enemy*>(obj.get());
            if (!e || !e->isActive()) continue;
            sf::FloatRect eb = e->getBounds();
            sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
            float dx = ec.x - position.x, dy = ec.y - position.y;
            float d  = std::sqrt(dx*dx + dy*dy);
            if (d < minDist) { minDist = d; target = e; }
        }
        
        // Inflict massive single-target damage to primary target, and half-HP damage to nearby splash targets
        if (target) {
            target->takeDamage(9999);
            for (auto& obj : objects) {
                auto* e = dynamic_cast<Enemy*>(obj.get());
                if (!e || !e->isActive() || e == target) continue;
                sf::FloatRect eeb = e->getBounds();
                sf::Vector2f  eec = {eeb.left + eeb.width / 2.f, eeb.top + eeb.height / 2.f};
                float dx = eec.x - position.x, dy = eec.y - position.y;
                if (std::sqrt(dx*dx + dy*dy) < BLAST_RADIUS)
                    e->takeDamage(e->getHp() / 2 + (e->getHp() % 2));
            }
        }
        return true;
    }

    // Standard behavior: find target and steer or begin winding up.
    if (state == DogState::WALK) {
        Enemy* target = nullptr;
        float  minDist = 9999.f;
        for (auto& obj : objects) {
            auto* e = dynamic_cast<Enemy*>(obj.get());
            if (!e || !e->isActive()) continue;
            sf::FloatRect eb = e->getBounds();
            sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
            float dx = ec.x - position.x, dy = ec.y - position.y;
            float d  = std::sqrt(dx*dx + dy*dy);
            if (d < minDist) { minDist = d; target = e; }
        }

        // If no active enemies are found, clean up the dog
        if (!target) { destroy(); return false; }

        // Determine direction to target
        sf::FloatRect eb = target->getBounds();
        sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
        sf::Vector2f  dir = ec - position;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);

        if (len < HIT_DIST + 6.f) {
            // Dog has reached target; transition to windup and start ticking down the fuse
            state       = DogState::WINDUP;
            windupTimer = WINDUP_DURATION;
            velocity    = {0.f, 0.f};
            animFrame   = 0;
            animTimer   = 0.f;
        } else if (len > 0.f) {
            // Keep running towards the target enemy
            facingLeft = dir.x < 0.f;
            velocity   = (dir / len) * SPEED;
        }
    }

    return false;
}

