#include <iostream>
#include <optional>
#include "Game.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/VideoMode.hpp"

namespace {
    
    sf::RectangleShape makeWall(sf::Vector2f position, sf::Vector2f size) {
        sf::RectangleShape wall(size);

        wall.setPosition(position);
        wall.setFillColor(sf::Color::White);

        return wall;
    }

    Target makeTarget(sf::Vector2f position, sf::Vector2f size) {
        sf::RectangleShape targetShape(size);

        targetShape.setPosition(position);
        targetShape.setFillColor(sf::Color::Red);

        Target target { targetShape };

        return target;
    }
}

Game::Game()
    : m_fixedDt(sf::seconds(1.0f / 60.0f))
    , m_window(sf::VideoMode({800, 600}), "My Window")
    , m_player({40.0f, 40.0f})
    , m_attackPhase(AttackPhase::None) 
    , m_facing({1.0f, 0.0f})
    , m_speed(250.0f)
    , m_attackTimer(0.0f)
    , m_wantAttack(false)
{
    m_player.setFillColor(sf::Color::Green);
    m_player.setPosition({350.0f, 250.0f});
    m_window.setFramerateLimit(144);

    sf::RectangleShape upWall =    makeWall({0.f, 0.f  }, {800.f, 16.f });
    sf::RectangleShape downWall =  makeWall({0.f, 584.f}, {800.f, 16.f });
    sf::RectangleShape rightWall = makeWall({784.f, 16.f }, {16.f, 568.f});
    sf::RectangleShape leftWall =  makeWall({0.f, 16.f }, {16.f, 568.f});

    m_walls.push_back(upWall);
    m_walls.push_back(downWall);
    m_walls.push_back(rightWall);
    m_walls.push_back(leftWall);
    m_walls.push_back(makeWall({200.f,  200.f}, {64.f,  64.f }));
    m_walls.push_back(makeWall({450.f,  350.f}, {96.f,  48.f }));

    m_targets.push_back(makeTarget({80.0f, 80.0f}, {40.0f, 40.0f}));
    m_targets.push_back(makeTarget({400.0f, 280.0f}, {40.0f, 40.0f}));
    m_targets.push_back(makeTarget({180.0f, 380.0f}, {40.0f, 40.0f}));
    m_targets.push_back(makeTarget({300.0f, 500.0f}, {40.0f, 40.0f}));
}

void Game::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                m_wantAttack = true;
            }
            else if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                m_window.close();
            }
        }
    }
}

void Game::update(sf::Time dt) {
    sf::Vector2f direction = { 0.0f, 0.0f };

    if (m_attackTimer > 0.0f) {
        m_attackTimer -= dt.asSeconds();
    }
    else if (m_wantAttack && m_attackTimer <= 0.0) {
        m_attackPhase = AttackPhase::None;
        std::cout << "Attack phase: None\n";
    }

    if (m_wantAttack && m_attackPhase == AttackPhase::None) {
        m_attackPhase = AttackPhase::Swing;
        m_attackTimer = 2.0f;
        std::cout << "Player has attacked\n";
    }

    m_wantAttack = false;

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)
    ) {
        direction += { 1.0f, 0.0f };
    }

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)
    ) {
        direction += { -1.0f, 0.0f };
    }

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)
    ) {
        direction += { 0.0f, -1.0f };
    }

    if (
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)
    ) {
        direction += { 0.0f, 1.0f };
    }

    if (direction.lengthSquared() <= 0.0f) {
        return;
    }

    m_facing = direction.normalized();

    std::cout << "facing: x: " << m_facing.x << ", y: " << m_facing.y << "\n";

    sf::Vector2f velocity = direction.normalized() * (m_speed * dt.asSeconds());

    for (const auto& wall : m_walls) {
        sf::FloatRect wallRect = wall.getGlobalBounds();

        sf::FloatRect playerRect = m_player.getGlobalBounds();

        playerRect.position.x += velocity.x;

        std::optional<sf::FloatRect> rect = playerRect.findIntersection(wallRect);

        if (rect.has_value()) {
            const float invertDirection = velocity.x > 0 ? -1.0f : 1.0f;

            velocity.x += rect->size.x * invertDirection;
        }
    }

    for (const auto& wall : m_walls) {
        sf::FloatRect wallRect = wall.getGlobalBounds();

        sf::FloatRect playerRect = m_player.getGlobalBounds();

        playerRect.position.y += velocity.y;

        std::optional<sf::FloatRect> rect = playerRect.findIntersection(wallRect);

        if (rect.has_value()) {
            const float invertDirection = velocity.y > 0 ? -1.0f : 1.0f;

            velocity.y += rect->size.y * invertDirection;
        }
    }

    m_player.move(velocity);
}

void Game::render() {
    m_window.clear(sf::Color::Black);

    for (const auto& wall : m_walls) {
        m_window.draw(wall);
    }
    
    for (const auto& target : m_targets) {
        m_window.draw(target.shape);
    }

    m_window.draw(m_player);

    m_window.display();
}

int Game::run() {
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;

    constexpr float maxDt = 0.25f;

    while (m_window.isOpen()) {
        float timeElapsed = clock.restart().asSeconds();

        if (timeElapsed > maxDt) {
            timeElapsed = maxDt;
        }

        accumulator += sf::seconds(timeElapsed);

        processEvents();

        while (accumulator >= m_fixedDt) {
            update(m_fixedDt);
            accumulator -= m_fixedDt;
        }

        render();
    }

    return 0;
}
