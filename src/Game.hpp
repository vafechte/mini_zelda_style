#pragma once
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vector2.hpp"
#include <vector>

struct Target {
    sf::RectangleShape shape;
    sf::Color baseColor;
    bool hit = false;
};

enum class AttackPhase {
    Swing,
    None
};

class Game {
public:
    Game();
    
    int run();

private:
    sf::Time m_fixedDt;
    
    sf::RenderWindow m_window;
    sf::RectangleShape m_player;
    sf::RectangleShape m_sword;

    AttackPhase m_attackPhase;

    std::vector<sf::RectangleShape> m_walls;
    std::vector<Target> m_targets;

    sf::Vector2f m_facing;

    float m_speed;
    float m_attackTimer;

    bool m_wantAttack;

    void processEvents();
    void update(sf::Time dt);
    void render();
};