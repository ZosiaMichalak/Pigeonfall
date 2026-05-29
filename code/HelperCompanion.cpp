#include "HelperCompanion.h"
#include <cmath>
#include <algorithm>

static constexpr float SPEED    = 120.f;
static constexpr float HIT_DIST = 8.f;

HelperCompanion::HelperCompanion(float x, float y)
    : GameObject(x, y), bobTimer(0.f), lifetime(20.f)
{
    shape.setRadius(5.f);
    shape.setOrigin(5.f, 5.f);
    shape.setFillColor(sf::Color(100, 200, 255));
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(200, 240, 255));
    shape.setPosition(position);
    velocity = {0.f, 0.f};
}

void HelperCompanion::update(float dt, sf::RenderWindow& /*window*/) {
    if (!isActive()) return;
    lifetime -= dt;
    if (lifetime <= 0.f) { destroy(); return; }

    bobTimer += dt;
    // Bob up and down visually (doesn't affect logical position)
    float visualY = position.y + std::sin(bobTimer * 4.f) * 3.f;

    position += velocity * dt;
    shape.setPosition(position.x, visualY);
}

void HelperCompanion::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    window.draw(shape);
}

bool HelperCompanion::tryHitEnemy(std::vector<std::unique_ptr<GameObject>>& objects) {
    if (!isActive()) return false;

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

    if (!target) {
        // No enemies left – linger briefly then despawn
        return false;
    }

    // Steer towards target
    sf::FloatRect eb = target->getBounds();
    sf::Vector2f  ec = {eb.left + eb.width / 2.f, eb.top + eb.height / 2.f};
    sf::Vector2f  dir = ec - position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f) velocity = (dir / len) * SPEED;

    // Hit check
    if (len < HIT_DIST + 6.f) {
        target->takeDamage(9999); // one-hit
        destroy();
        return true;
    }
    return false;
}
