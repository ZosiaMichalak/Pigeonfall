/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Actions returned by the main menu interaction handler.
enum class MenuAction {
    NONE,
    NEW_GAME,
    LOAD_GAME,
    OPTIONS,
    QUIT
};

// Represents the screen categories shown within the menu layout.
enum class MenuScreen {
    MAIN,
    OPTIONS
};

// Particle drifting in the background decoration of the main menu.
struct MenuParticle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float        life;      // Current lifetime countdown (0..maxLife)
    float        maxLife;   // Initial particle lifetime
    float        radius;    // Particle render size
    sf::Color    col;       // Accent color of the particle
};

// Background decorative nodes connected by lines to represent a dummy skill tree.
struct BgNode {
    sf::Vector2f pos;
    float        radius;
    float        pulse;     // Phase of the pulse oscillation (0..2π)
    float        pulseSpeed;
    sf::Color    col;
};

// MainMenu class that renders the starting screen, manages input, and lets players configure audio or fullscreen settings.
class MainMenu {
public:
    // Constructor: loads options configurations, seats active sizes, and starts background particle systems
    MainMenu(sf::Font& font, bool hasSave, bool startFullscreen, int startMusicVolume = 100, int startSfxVolume = 70);

    // Processes key inputs and handles row selections, returning appropriate MenuActions
    MenuAction handleEvent(const sf::Event& event);

    // Render loop helper that advances background particle drift timers and draws the menu panels
    void render(sf::RenderWindow& window);

    // Getters for settings variables
    bool fullscreen()   const { return optFullscreen; }
    int  musicVolume()  const { return optMusicVolume; }
    int  sfxVolume()    const { return optSfxVolume; }

private:
    sf::Font& font;
    MenuScreen screen;

    // Main screen state
    bool hasSave;
    int  mainSel;
    std::vector<std::string> mainItems;

    // Options screen settings
    bool optFullscreen;
    int  optMusicVolume; // 0-100 (10% increments)
    int  optSfxVolume;   // 0-100 (10% increments)
    int  optSel;
    bool optChanged;

    // Clock timers
    float titleTimer;
    float globalTimer;  // Tracks global elapsed time for continuous wave updates

    // Background particle array
    std::vector<MenuParticle> particles;
    float particleSpawnTimer;

    // Background skill-tree nodes list
    std::vector<BgNode> bgNodes;

    float selGlow;      // Oscillating glow factor (0..1) for selected menu row

    // Setup helpers
    void buildMainItems();
    void spawnParticle();
    void initBgNodes();

    // Renders different background visual elements
    void drawBgNodes(sf::RenderWindow& w);
    void drawParticles(sf::RenderWindow& w);
    
    // Helper that draws a clean line between two coordinates
    void drawNodeConnector(sf::RenderWindow& w,
                           sf::Vector2f a, sf::Vector2f b,
                           sf::Color col, float thickness = 0.8f);
    
    // Helper that renders the central translucent panel box
    void drawPanel(sf::RenderWindow& w,
                   float px, float py, float pw, float ph,
                   sf::Color fill, sf::Color outline, float glowStrength = 0.f);
    
    // Helper that renders individual menu text rows
    void drawMenuItem(sf::RenderWindow& w,
                      const std::string& label, int idx, bool selected,
                      float panelX, float itemY, float panelW,
                      bool dimmed = false);
    
    // Text generator utility
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col, float x, float y);
};

#endif
