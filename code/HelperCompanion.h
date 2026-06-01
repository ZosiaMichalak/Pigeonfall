#ifndef HELPER_COMPANION_H
#define HELPER_COMPANION_H

#include "GameObject.h"
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

enum class DuoState { HUNTING, EXIT };

class HelperCompanion : public GameObject {
public:
    HelperCompanion(float x, float y);

    void update(float dt, sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;

    void tryHitEnemy(float dt,
                     std::vector<std::unique_ptr<GameObject>>& objects,
                     std::vector<std::unique_ptr<GameObject>>& spawnQueue);

    sf::FloatRect getBounds() const;

private:
    sf::CircleShape shape;
    sf::Vector2f    velocity;

    sf::Texture texHit;
    sf::Texture texWalk;
    sf::Sprite  sprite;
    bool        hasSprite = false;
    bool        facingLeft = false;

    DuoState state      = DuoState::HUNTING;
    float    huntTimer  = 10.f;
    float    exitTimer  = 2.f;

    Enemy*   bossTarget     = nullptr;
    float    bossFocusTimer = 0.f;

    int   animFrame     = 0;
    float animTimer     = 0.f;
    int   hitFrames     = 5;
    int   walkFrames    = 3;
    int   hitSheetCols  = 2;
    int   walkSheetCols = 2;

    static constexpr int   FRAME_W        = 16;
    static constexpr int   FRAME_H        = 16;
    static constexpr float SPRITE_SCALE   = 2.f;   // 16*2 = 32 px — player frame height
    static constexpr float HUNT_DURATION  = 10.f;
    static constexpr float EXIT_DURATION  = 2.f;
    static constexpr float HIT_FRAME_DUR  = 0.08f;
    static constexpr float WALK_FRAME_DUR = 0.10f;
    static constexpr float SPEED          = 175.f;
    static constexpr float HIT_DIST       = 12.f;
    static constexpr float BOSS_FOCUS_DUR = 2.f;
    static constexpr int   BOSS_HIT_DMG   = 30;

    void setAnimFrame(sf::Texture& tex, int frame, int sheetCols);
    void applyFacingScale();
    void tickHitAnim(float dt);
    void tickWalkAnim(float dt);
    int  countSheetFrames(const sf::Texture& tex) const;
    int  damageFor(Enemy* e) const;
    void spawnCoinsForKill(Enemy* e, sf::Vector2f at,
                           std::vector<std::unique_ptr<GameObject>>& spawnQueue);
    Enemy* findTarget(std::vector<std::unique_ptr<GameObject>>& objects, bool skipBoss) const;
};

#endif
