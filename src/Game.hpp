#pragma once
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/System/Time.hpp"
#include <vector>

class Game {
public:
    Game();
    
    int run();

private:
    sf::Time m_fixedDt;
    
    sf::RenderWindow m_window;
    sf::RectangleShape m_player;

    std::vector<sf::RectangleShape> m_walls;

    float m_speed;

    void processEvents();
    void update(sf::Time dt);
    void render();
};