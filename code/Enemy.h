#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"
#include <vector>
#include <memory>

class Enemy : public GameObject {
protected:
    // Fallback shape (used when no texture loaded)
    sf::RectangleShape shape;
    sf::RectangleShape hpBarBack;
    sf::RectangleShape hpBarFront;

    // Sprite rendering
    sf::Sprite  sprite;
    bool        hasSprite  = false;
    bool        facingLeft = false;

    // Shared animation helpers
    int   animCol      = 0;
    int   animMaxCols  = 1;
    int   animSheetCols = 1;   // columns per row in the sheet
    int   frameW       = 51;
    int   frameH       = 32;
    float animTimer    = 0.f;
    float frameDur     = 0.18f;

    // Advances the frame and sets the texture rect on `sprite`.
    // Call this once per updateAI after position is final.
    void tickAnim(float dt);

    int   hp;
    int   maxHp;
    bool  isHit;
    float hitTimer;
    bool  isSpawning = false;  // true during spawn animation — immune to damage

    float moveSpeed;

public:
    Enemy(float x, float y);
    virtual ~Enemy() = default;

    void update(float dt, sf::RenderWindow& window) override {}

    virtual void updateAI(float dt, sf::Vector2f playerPos,
                          std::vector<std::unique_ptr<GameObject>>& spawnQueue) = 0;

    void draw(sf::RenderWindow& window) override;
    void takeDamage(int damage);

    bool          getIsHit()     const { return isHit; }
    bool          getIsSpawning()const { return isSpawning; }
    void          setSpawning(bool v)  { isSpawning = v; }
    int           getHp()     const { return hp; }
    int           getMaxHp()  const { return maxHp; }
    virtual bool  isBoss()    const { return false; }
    virtual bool  canTakeItemDamage() const { return true; }

    // Tight body hitbox used for collision with player
    sf::FloatRect getBounds() const {
        constexpr float W = 10.f, H = 10.f;
        return { position.x - W / 2.f, position.y - H / 2.f, W, H };
    }

    sf::FloatRect getShapeBounds() const { return shape.getGlobalBounds(); }
    void          nudgePosition(sf::Vector2f delta) { position += delta; shape.setPosition(position); }
};

#endif
