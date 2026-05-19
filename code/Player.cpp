#include "Player.h"
#include <cmath>

Player::Player(float x, float y) : GameObject(x, y) {
    // Inicjalizacja flag wczytywania plików
    hasIdleTexture = textureIdle.loadFromFile("assets/player_idle.png");
    hasWalkTexture = textureWalk.loadFromFile("assets/player_walk.png");
    
    // Logowanie błędów do konsoli, jeśli ścieżka jest zła
    if (!hasIdleTexture) {
        std::cerr << "[BLAD SFML] Nie mozna znalezc: assets/player_idle.png !" << std::endl;
    }
    if (!hasWalkTexture) {
        std::cerr << "[BLAD SFML] Nie mozna znalezc: assets/player_walk.png !" << std::endl;
    }

    // Wymiary klatek gołębia (48x48 px)
    frameWidth = 32;
    frameHeight = 32;
    animationTimer = 0.f;
    frameDuration = 0.15f; 
    currentColumn = 0;
    maxColumns = 3; // Domyślnie dla idle

    // Zmienna skali w konstruktorze - zmień tutaj, aby zmienić początkowy rozmiar
    float baseScale = 2.0f;

    if (hasIdleTexture) {
        sprite.setTexture(textureIdle);
        currentFrame = sf::IntRect(0, 0, frameWidth, frameHeight);
        sprite.setTextureRect(currentFrame);
        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
        
        // Użycie zmiennej skali (gołąb patrzy domyślnie w lewo)
        sprite.setScale(baseScale, baseScale); 
    } else {
        // Ustawienie awaryjnego kształtu, jeśli nie ma tekstury
        fallbackShape.setSize(sf::Vector2f(40.f * baseScale, 40.f * baseScale)); 
        fallbackShape.setFillColor(sf::Color::Magenta); 
        fallbackShape.setOrigin((40.f * baseScale) / 2.f, (40.f * baseScale) / 2.f);
    }

    speed = 250.f;

    isDashing = false; 
    dashDuration = 0.2f; 
    dashCooldown = 1.0f; 
    dashCooldownTimer = 0.f;

    isAttacking = false; 
    attackDuration = 0.15f; 
    attackCooldown = 0.4f; 
    attackCooldownTimer = 0.f;

    swordHitbox.setSize(sf::Vector2f(100.f, 30.f)); // Miecz dopasowany do większego gołębia
    swordHitbox.setFillColor(sf::Color(255, 255, 255, 180));
    swordHitbox.setOrigin(0.f, 15.f); 
}

sf::FloatRect Player::getBounds() const {
    if (hasIdleTexture || hasWalkTexture) {
        return sprite.getGlobalBounds();
    }
    return fallbackShape.getGlobalBounds();
}

void Player::updateAttack(float dt, sf::RenderWindow& window) {
    if (attackCooldownTimer > 0.f) attackCooldownTimer -= dt;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !isAttacking && attackCooldownTimer <= 0.f) {
        isAttacking = true;
        attackTimer = attackDuration;
        attackCooldownTimer = attackCooldown;

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f playerCenter = position; 
        sf::Vector2f delta = mousePos - playerCenter;
        
        attackAngle = std::atan2(delta.y, delta.x) * 180.f / 3.14159f;
        swordHitbox.setRotation(attackAngle);
    }

    if (isAttacking) {
        attackTimer -= dt;
        if (attackTimer <= 0.f) isAttacking = false;
        swordHitbox.setPosition(position);
    }
}

void Player::update(float dt, sf::RenderWindow& window) {
    updateAttack(dt, window);
    
    // --- TUTAJ JEST TWOJA ZMIENNA SKALI ---
    // Zmień tę wartość (np. na 3.0f lub 4.0f), a gołąb zmieni rozmiar we wszystkich stanach!
    float skala = 4.0f;

    sf::Vector2f moveDir(0.f, 0.f);
    if (dashCooldownTimer > 0.f) dashCooldownTimer -= dt;

    sf::Texture const* currentTexture = sprite.getTexture();

    if (isDashing) {
        if (hasWalkTexture) {
            sprite.setTexture(textureWalk);
            maxColumns = 6;
        }
        position += dashDir * (speed * 4.f) * dt;
        
        // Pilnowanie skali i obrotu podczas dasha
        if (hasIdleTexture || hasWalkTexture) {
            if (dashDir.x < 0.f) {
                sprite.setScale(skala, skala);   // Dash w lewo
            } else if (dashDir.x > 0.f) {
                sprite.setScale(-skala, skala);  // Dash w prawo
            }
        }
        
        dashTimer -= dt;
        if (dashTimer <= 0.f) {
            isDashing = false;
        }
    } else {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1.f;
        
        // Klawisz A (w lewo) -> Skala dodatnia, bo gołąb naturalnie patrzy w lewo
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            moveDir.x -= 1.f;
            if (hasIdleTexture || hasWalkTexture) sprite.setScale(skala, skala); 
        }
        // Klawisz D (w prawo) -> Skala osi X ujemna – lustrzane odbicie w prawo
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            moveDir.x += 1.f;
            if (hasIdleTexture || hasWalkTexture) sprite.setScale(-skala, skala);  
        }

        if (moveDir.x != 0 || moveDir.y != 0) {
            if (hasWalkTexture) {
                sprite.setTexture(textureWalk);
                maxColumns = 6;
            }
            
            float length = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
            moveDir /= length;
            position += moveDir * speed * dt;
        } else {
            if (hasIdleTexture) {
                sprite.setTexture(textureIdle);
                maxColumns = 3;
                
                // Pilnowanie skali i obrotu, kiedy gołąb stoi w miejscu (idle)
                if (hasIdleTexture || hasWalkTexture) {
                    if (sprite.getScale().x < 0.f) {
                        sprite.setScale(-skala, skala); // Trzymaj obrót w prawo
                    } else {
                        sprite.setScale(skala, skala);  // Trzymaj obrót w lewo
                    }
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            startDash(moveDir);
        }
    }

    // Reset klatek przy zmianie stanu ruch/stanie
    if ((hasIdleTexture || hasWalkTexture) && currentTexture != sprite.getTexture()) {
        currentColumn = 0;
        animationTimer = 0.f;
    }

    // Odtwarzanie animacji gołębia
    if (hasIdleTexture || hasWalkTexture) {
        animationTimer += dt;
        if (animationTimer >= frameDuration) {
            animationTimer = 0.f;
            currentColumn = (currentColumn + 1) % maxColumns;
        }

        // Wycinanie klatek z siatki 2x3
        currentFrame.left = (currentColumn % 2) * frameWidth;
        currentFrame.top = (currentColumn / 2) * frameHeight;
        sprite.setTextureRect(currentFrame);
        sprite.setPosition(position);
    } else {
        fallbackShape.setPosition(position);
    }
}

void Player::startDash(sf::Vector2f moveDir) {
    if (!isDashing && dashCooldownTimer <= 0.f) {
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
        dashDir = (moveDir.x == 0 && moveDir.y == 0) ? sf::Vector2f(1.f, 0.f) : moveDir;
    }
}

void Player::draw(sf::RenderWindow& window) {
    if (hasIdleTexture || hasWalkTexture) {
        window.draw(sprite);
    } else {
        window.draw(fallbackShape);
    }
    
    if (isAttacking) window.draw(swordHitbox);
}