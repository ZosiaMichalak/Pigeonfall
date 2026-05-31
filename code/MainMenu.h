#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

enum class MenuAction {
    NONE,
    NEW_GAME,
    LOAD_GAME,
    OPTIONS,
    QUIT
};

enum class MenuScreen {
    MAIN,
    OPTIONS
};

// ── Particle for the ambient background drift ─────────────────────────────────
struct MenuParticle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float        life;      // 0..1
    float        maxLife;
    float        radius;
    sf::Color    col;
};

// ── A decorative skill-tree node drawn in the background ─────────────────────
struct BgNode {
    sf::Vector2f pos;
    float        radius;
    float        pulse;     // 0..2π timer
    float        pulseSpeed;
    sf::Color    col;
};

class MainMenu {
public:
    MainMenu(sf::Font& font, bool hasSave, bool startFullscreen, int startMusicVolume = 100);

    // Feed every polled event here.  Returns the chosen action (or NONE).
    MenuAction handleEvent(const sf::Event& event);

    // Draw the current screen (call every frame; advances internal timers).
    void render(sf::RenderWindow& window);

    // Expose options so Game can read them.
    bool fullscreen()   const { return optFullscreen; }
    int  musicVolume()  const { return optMusicVolume; }

private:
    sf::Font& font;
    MenuScreen screen;

    // ── Main screen ───────────────────────────────────────────────────────────
    bool hasSave;
    int  mainSel;
    std::vector<std::string> mainItems;

    // ── Options screen ────────────────────────────────────────────────────────
    bool optFullscreen;
    int  optMusicVolume; // 0-100 in steps of 10
    int  optSel;
    bool optChanged;

    // ── Timers ────────────────────────────────────────────────────────────────
    float titleTimer;
    float globalTimer;  // ever-increasing, drives animations

    // ── Ambient particles ─────────────────────────────────────────────────────
    std::vector<MenuParticle> particles;
    float particleSpawnTimer;

    // ── Background skill-tree nodes ───────────────────────────────────────────
    std::vector<BgNode> bgNodes;

    // ── Selection animation ───────────────────────────────────────────────────
    float selGlow;      // 0..1 oscillation for the highlighted row

    // Helpers
    void buildMainItems();
    void spawnParticle();
    void initBgNodes();

    void drawBgNodes(sf::RenderWindow& w);
    void drawParticles(sf::RenderWindow& w);
    void drawNodeConnector(sf::RenderWindow& w,
                           sf::Vector2f a, sf::Vector2f b,
                           sf::Color col, float thickness = 0.8f);
    void drawPanel(sf::RenderWindow& w,
                   float px, float py, float pw, float ph,
                   sf::Color fill, sf::Color outline, float glowStrength = 0.f);
    void drawMenuItem(sf::RenderWindow& w,
                      const std::string& label, int idx, bool selected,
                      float panelX, float itemY, float panelW,
                      bool dimmed = false);
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col, float x, float y);
};

#endif
