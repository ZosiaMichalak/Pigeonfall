#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    sf::Texture texture;
    if (texture.loadFromFile("assets/attack_smudge.png")) {
        std::cout << "SUCCESS: attack_smudge.png loaded successfully! Size: " 
                  << texture.getSize().x << "x" << texture.getSize().y << std::endl;
    } else {
        std::cout << "FAILURE: Failed to load attack_smudge.png" << std::endl;
    }
    return 0;
}
