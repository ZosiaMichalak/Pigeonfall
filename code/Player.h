#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <iostream>

class Player : public GameObject {
private:
    sf::Sprite sprite;              
    sf::Texture textureIdle;        // Tekstura dla stania w miejscu
    sf::Texture textureWalk;        // Tekstura dla chodzenia/biegu
    sf::RectangleShape swordHitbox; 
    sf::RectangleShape fallbackShape; // Awaryjny kształt, jeśli tekstury nie zadziałają
    float speed;                    

    // --- TUTAJ BYŁ BRAK ---
    // Flagi informujące, czy tekstury załadowały się poprawnie
    bool hasIdleTexture;
    bool hasWalkTexture;

    // Parametry animacji gołębia (wymiary klatki: 48x48 px)
    sf::IntRect currentFrame;       
    float animationTimer;           
    float frameDuration;            
    int frameWidth;                 
    int frameHeight;                
    int currentColumn;              
    int maxColumns;                 

    // Mechanika Dasha
    bool isDashing; 
    float dashTimer, dashDuration, dashCooldownTimer, dashCooldown;
    sf::Vector2f dashDir;           

    // Mechanika Ataku
    bool isAttacking; 
    float attackTimer, attackDuration, attackCooldownTimer, attackCooldown, attackAngle;

    void updateAttack(float dt, sf::RenderWindow& window);

public:
    Player(float x, float y);
    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;
    
    sf::FloatRect getSwordBounds() const { return swordHitbox.getGlobalBounds(); }
    bool isAttackingNow() const { return isAttacking; }
    void startDash(sf::Vector2f moveDir);
    bool isDashingNow() const { return isDashing; }
    sf::FloatRect getBounds() const;
};

#endif