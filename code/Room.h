/*
g++ -std=c++17 -DSFML_STATIC `
  code\*.cpp `
  -I SFML-2.5.1\include `
  -L SFML-2.5.1\lib `
  -o Gra.exe `
  -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s `
  -lopengl32 -lwinmm -lgdi32 -lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg
*/
#ifndef ROOM_H
#define ROOM_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include "Prop.h"

// Categories of enemies that can spawn in the room.
enum class EnemyType { BULLET, DASH };

// Spawn info representing a single enemy type, position, and tier difficulty level.
struct EnemySpawn {
    EnemyType    type;
    sf::Vector2f position;
    int          tier = 0;
};

// Layout blueprint data for placing props (vending machines, benches, etc.) in a room.
struct PropDef {
    enum class Type {
        VENDING,
        TRASH,
        BENCH,
        FLOWERS,
        HYDRANT,
        CAR1,
        CAR2,
        CAR3
    } type;
    sf::Vector2f position;
};

// Represents a predesigned room layout preset containing props and door placement.
struct RoomTemplate {
    std::string             name;
    int                     background;    // Background texture ID index
    std::vector<PropDef>    props;         // Props present in this room layout
    sf::Vector2f            doorPosition;  // Coordinates of the room's door
    float                   doorRotation;  // Rotation angle of the door sprite
    sf::Vector2f            playerStart;   // Starting position of the player when entering this room
};

// Utility functions for loading and querying predefined room templates.
namespace RoomTemplates {
    constexpr int BOSS_LAYOUT = -1; // Special index denoting the boss room

    // Returns a list of all defined room layout templates
    std::vector<RoomTemplate> getAll();
    
    // Gets a template by its index
    const RoomTemplate& getByIndex(int index);
    
    // Gets a random room template index (excluding the starter room)
    int                 getRandomIndex();
    
    // Returns a random room layout template
    RoomTemplate        getRandom();
    
    // Generates and returns a dedicated layout for the Pigeon King boss battle
    RoomTemplate        getBossArena();
}

// Manages the state, props, background, and clearing state of the current active room.
class Room {
public:
    // Constructors for fallback room generation or template-based instantiation
    Room(int id, int enemyCount, sf::Color fallbackColor);
    Room(int id, const RoomTemplate& tmpl, int layoutIndex = 0);

    // Getters for room properties and state variables
    int  getId()            const { return id; }
    int  getLayoutIndex()   const { return layoutIndex; }
    int  getEnemyCount() const { return enemyCount; }
    bool getIsCleared()  const { return isCleared; }

    sf::Vector2f getDoorPosition()  const { return doorPosition; }
    float        getDoorRotation()  const { return doorRotation; }
    sf::Vector2f getPlayerStart()   const { return playerStart; }

    // Sets room cleared status (true = all enemies defeated)
    void setCleared(bool c) { isCleared = c; }

    // Generates a list of bounding boxes for all props to handle physics collisions
    std::vector<sf::FloatRect> getPropColliders() const;
    
    // Loads assets specific to this room (textures, backgrounds)
    void loadAssets();
    
    // Renders the background and all active props in the room
    void draw(sf::RenderWindow& window) const;

private:
    int          id;             // Numeric identifier of the room (e.g. sequence number)
    int          layoutIndex;    // Reference index to the template layout used
    int          enemyCount;     // Current count of alive enemies in the room
    bool         isCleared;      // Status of room completion
    sf::Color    floorColor;     // Floor background color (fallback)
    int          bgIndex;        // Texture file identifier index
    bool         hasBackground;  // Checked if background texture loaded successfully
    float        doorRotation;   // Cached door sprite rotation

    sf::Vector2f doorPosition;   // Door coordinates
    sf::Vector2f playerStart;    // Spawn point coordinates

    sf::Texture  bgTexture;      // Renders the background image
    sf::Sprite   bgSprite;       // Background sprite object

    std::vector<std::unique_ptr<Prop>> props; // List of physical props in this room
};

#endif
