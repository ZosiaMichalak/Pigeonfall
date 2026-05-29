#include "AnnoyingDog.h"
#include <cmath>

AnnoyingDog::AnnoyingDog(float x, float y)
    : GameObject(x, y), exploding(false), explodeTimer(0.f)
{
    shape.setRadius(6.f);
    shape.setOrigin(6.f, 6.f);
    shape.setFillColor(sf::Color(240, 230, 210));
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(180, 160, 120));
    shape.setPosition(position);
    velocity = {0.f, 0.f};

    explosionShape.setRadius(BLAST_RADIUS);
    explosionShape.setOrigin(BLAST_RADIUS, BLAST_RADIUS);
    explosionShape.setFillColor(sf::Color(255, 160, 30, 160));
    explosionShape.setOutlineThickness(2.f);
    explosionShape.setOutlineColor(sf::Color(255, 80, 0, 200));
}

void AnnoyingDog::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;

    if (exploding) {
        explodeTimer -= dt;
        // Fade out explosion
        float alpha = std::max(0.f, explodeTimer / 0.3f);
        explosionShape.setFillColor(sf::Color(255, 160, 30, static_cast<sf::Uint8>(160 * alpha)));
        explosionShape.setOutlineColor(sf::Color(255, 80, 0, static_cast<sf::Uint8>(200 * alpha)));
        if (explodeTimer <= 0.f) destroy();
        return;
    }

    position += velocity * dt;
    shape.setPosition(position);
}

void AnnoyingDog::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    if (exploding) {
        window.draw(explosionShape);
    } else {
        window.draw(shape);
    }
}

bool AnnoyingDog::tryExplode(std::vector<std::unique_ptr<GameObject>>& objects,
                               std::vector<std::unique_ptr<GameObject>>& /*spawnQueue*/) {
    if (!isActive() || exploding) return false;

    // Find nearest living enemy
    Enemy* target = nullptr;
    float  minDist = 9999.f;
    for (auto& obj : objects) {
        auto* e = dynamic_cast<Enemy*>(obj.get());
        if (!e || !e->isActive()) continue;
        sf::FloatRect eb = e->getBounds();
        sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
        float dx = ec.x - position.x;
        float dy = ec.y - position.y;
        float d  = std::sqrt(dx * dx + dy * dy);
        if (d < minDist) { minDist = d; target = e; }
    }

    if (!target) { destroy(); return false; }

    // Steer towards target
    sf::FloatRect eb = target->getBounds();
    sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
    sf::Vector2f  dir = ec - position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f) velocity = (dir / len) * SPEED;

    // Explode on contact
    if (len < HIT_DIST + 6.f) {
        exploding    = true;
        explodeTimer = 0.3f;
        explosionShape.setPosition(position);

        // Direct target: one-hit kill
        target->takeDamage(9999);

        // Area damage: halve HP of all other enemies in blast radius
        for (auto& obj : objects) {
            auto* e = dynamic_cast<Enemy*>(obj.get());
            if (!e || !e->isActive() || e == target) continue;
            sf::FloatRect eeb = e->getBounds();
            sf::Vector2f  eec = {eeb.left + eeb.width / 2.f, eeb.top + eeb.height / 2.f};
            float dx = eec.x - position.x;
            float dy = eec.y - position.y;
            if (std::sqrt(dx * dx + dy * dy) < BLAST_RADIUS) {
                e->takeDamage(e->getHp() / 2 + (e->getHp() % 2)); // ceil half
            }
        }
        return true;
    }
    return false;
}
