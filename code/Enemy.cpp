#include "Enemy.h"

// Constructor: Initializes default base stats, fallback shapes, sprite positions, and HP bars.
Enemy::Enemy(float x, float y) : GameObject(x, y) {
    maxHp    = 2;
    hp       = maxHp;
    isHit    = false;
    hitTimer = 0.f;
    moveSpeed = 40.f;

    // Configure fallback red square shape used if sprite texture fails to load
    shape.setSize(sf::Vector2f(9.f, 9.f));
    shape.setFillColor(sf::Color::Red);
    shape.setOrigin(4.5f, 4.5f);
    shape.setPosition(position);
    
    // Initial position setup for sprite
    sprite.setPosition(position);

    // Setup health bar background (dark grey bar positioned above the enemy)
    hpBarBack.setSize(sf::Vector2f(9.f, 1.5f));
    hpBarBack.setFillColor(sf::Color(50, 50, 50));
    hpBarBack.setOrigin(4.5f, 0.75f);
    hpBarBack.setPosition(position.x, position.y - 10.f);

    // Setup health bar foreground (red bar scaling with remaining HP)
    hpBarFront.setSize(sf::Vector2f(9.f, 1.5f));
    hpBarFront.setFillColor(sf::Color::Red);
    hpBarFront.setOrigin(4.5f, 0.75f);
    hpBarFront.setPosition(position.x, position.y - 10.f);
}

// Advances the sprite sheet animation frames and mirrors sprite based on direction.
void Enemy::tickAnim(float dt) {
    animTimer += dt;
    if (animTimer >= frameDur) {
        animTimer -= frameDur;
        animCol = (animCol + 1) % animMaxCols;
    }
    
    // Calculate sprite sheet coordinate offsets
    int col = animCol % animSheetCols;
    int row = animCol / animSheetCols;
    
    // Update texture viewport coordinates and screen position
    sprite.setTextureRect(sf::IntRect(col * frameW, row * frameH, frameW, frameH));
    sprite.setPosition(position);
    
    // Mirror horizontally when facing left (flip X scale)
    sprite.setScale(facingLeft ? 1.f : -1.f, 1.f);
}

// Inflicts damage on the enemy, applying temporary hit-stun/invulnerability.
void Enemy::takeDamage(int damage) {
    if (isSpawning) return;  // Immune to damage during initial spawn phase
    
    if (!isHit) {
        hp -= damage;
        isHit    = true;
        hitTimer = 0.2f; // Trigger 0.2s of hit recovery/invulnerability
        
        if (hp <= 0) {
            destroy(); // Mark for removal if health is depleted
        } else {
            // Recalculate health bar scale based on remaining HP percentage
            float pct = static_cast<float>(hp) / static_cast<float>(maxHp);
            hpBarFront.setSize(sf::Vector2f(9.f * pct, 1.5f));
        }
    }
}

// Renders the enemy sprite (or fallback shape) and health bars.
void Enemy::draw(sf::RenderWindow& window) {
    if (!isActive()) return;
    
    // Draw sprite if loaded, otherwise render fallback shape
    if (hasSprite) {
        window.draw(sprite);
    } else {
        window.draw(shape);
    }
    
    // Render health indicator bars
    window.draw(hpBarBack);
    window.draw(hpBarFront);
}