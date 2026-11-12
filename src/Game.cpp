// #include <iostream>
#include <optional>
// #include <string_view>
#include <cmath>
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

namespace ZMath {
    sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
        return sf::Vector2f(
            std::lerp(a.x, b.x, t),
            std::lerp(a.y, b.y, t)
        );
    }
};

namespace {
    sf::RectangleShape makeWall(sf::Vector2f position, sf::Vector2f size) {
        sf::RectangleShape wall(size);

        wall.setPosition(position);
        wall.setFillColor(sf::Color::White);

        return wall;
    }

    Target makeTarget(sf::Vector2f position, sf::Vector2f size, sf::Color color) {
        sf::RectangleShape targetShape(size);

        targetShape.setPosition(position);
        targetShape.setFillColor(color);

        Target target { targetShape , color };

        return target;
    }

    sf::Vector2f getDirection() {
        sf::Vector2f direction = {0.0f, 0.0f};
        
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
    
        return direction;
    }

//     constexpr std::string_view getEnemyStateName(EnemyState state) {
//         switch (state) {
//             case EnemyState::Idle: return "Idle";
//             case EnemyState::Chase: return "Chase";
//             case EnemyState::WindUp: return "WindUp";
//             case EnemyState::Strike: return "Strike";
//             case EnemyState::Recover: return "Recover";
//             case EnemyState::Dead: return "Dead";
//         }
// 
//         return "The current state is not include in the switch, please add it";
//     }
}

Game::Game()
    : m_fixedDt(sf::seconds(1.0f / 60.0f))
    , m_window(sf::VideoMode({800, 600}), "My Window")
    , m_player({40.0f, 40.0f})
    , m_sword({90.0f, 24.0f})
    , m_playerKnockback({0.0f, 0.0f})
    , m_attackPhase(AttackPhase::None) 
    , m_enemy(makeTarget({420.0f, 500.0f}, {40.0f, 40.0f}, sf::Color::Magenta))
    , m_enemyState(EnemyState::Idle)
    , m_enemyKnockback({0.0f, 0.0f})
    , m_enemyStateTimer(0.0f)
    , m_facing({1.0f, 0.0f})
    , m_speed(250.0f)
    , m_attackTimer(0.0f)
    , m_wantAttack(false)
{
    m_window.setFramerateLimit(144);
    
    m_player.setFillColor(sf::Color::Green);
    m_player.setPosition({350.0f, 250.0f});

    m_sword.setFillColor(sf::Color::Yellow);
    m_sword.setOrigin({0.0f, 12.0f});

    m_enemy.hp = 3;

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
}

void Game::moveWithCollisions(sf::RectangleShape& body, sf::Vector2f velocity) {
    for (const auto& wall : m_walls) {
        sf::FloatRect wallRect = wall.getGlobalBounds();

        sf::FloatRect bodyRect = body.getGlobalBounds();

        bodyRect.position.x += velocity.x;

        std::optional<sf::FloatRect> rect = bodyRect.findIntersection(wallRect);

        if (rect.has_value()) {
            const float invertDirection = velocity.x > 0 ? -1.0f : 1.0f;

            velocity.x += rect->size.x * invertDirection;
        }
    }

    for (const auto& wall : m_walls) {
        sf::FloatRect wallRect = wall.getGlobalBounds();

        sf::FloatRect bodyRect = body.getGlobalBounds();

        bodyRect.position.y += velocity.y;

        std::optional<sf::FloatRect> rect = bodyRect.findIntersection(wallRect);

        if (rect.has_value()) {
            const float invertDirection = velocity.y > 0 ? -1.0f : 1.0f;

            velocity.y += rect->size.y * invertDirection;
        }
    }
   
    body.move(velocity);
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

void Game::updatePlayer(sf::Time dt) {
    sf::Vector2f velocity;
    
    if (m_playerKnockback.lengthSquared() > 20.0f) {
        moveWithCollisions(m_player, m_playerKnockback * dt.asSeconds());
        m_attackPhase = AttackPhase::None;
        m_wantAttack = false;
        m_playerKnockback *= 0.90f;
    }
    // we don't need to set back m_playerKnockback to ZERO
    
    if (m_attackTimer > 0.0f) {
        m_attackTimer -= dt.asSeconds();
        if (m_attackTimer <= 0.0f) {
            m_attackPhase = AttackPhase::None;
        }
    }

    if (m_wantAttack && m_attackPhase == AttackPhase::None) {
        constexpr float kRange = 90.0f + 20.0f;
        constexpr float kRangeSquared = kRange * kRange;
        const float kHalfAngleCos = 0.707f;
        
        m_attackPhase = AttackPhase::Swing;
        m_attackTimer = 0.5f;

        const sf::Vector2f playerCenter = m_player.getGlobalBounds().getCenter();
        m_sword.setPosition(playerCenter);
        m_sword.setRotation(m_facing.angle());

        const sf::Vector2f enemyCenter = m_enemy.shape.getGlobalBounds().getCenter();

        const sf::Vector2f toEnemy = enemyCenter - playerCenter;
        
        float distanceSquaredToTarget = toEnemy.lengthSquared();

        if (distanceSquaredToTarget > 0.0f) {
            const float facingDot = m_facing.dot(toEnemy.normalized());

            if (
                distanceSquaredToTarget < kRangeSquared &&
                facingDot > kHalfAngleCos
            ) {
                m_enemy.hp -= 1;
                if (m_enemy.hp <= 0) {
                    setEnemyState(EnemyState::Dead);
                }
                else {
                    const float kForce = 500.0f;
                    m_enemyKnockback = toEnemy.normalized() * kForce;
                    setEnemyState(EnemyState::Idle);
                }
            }
        }
    }

    m_wantAttack = false;

    if (m_attackPhase == AttackPhase::Swing) {
        return;
    }
    
    const sf::Vector2f direction = getDirection();

    if (direction.lengthSquared() <= 0.0f) {
        return;
    }

    m_facing = direction.normalized();

    velocity = direction.normalized() * (m_speed * dt.asSeconds());

    moveWithCollisions(m_player, velocity);
}

void Game::setEnemyState(EnemyState newState) {
    // std::string_view oldStateName = getEnemyStateName(m_enemyState);
    // std::string_view newStateName = getEnemyStateName(newState);
    // 
    // std::cout << "state " <<  oldStateName << " to " <<  newStateName << "\n";
    m_enemyState = newState;
    m_enemyStateTimer = 0.0f;
}

void Game::updateEnemy(sf::Time dt) {
    constexpr float speed = 150.0f;
    constexpr float kStrikeRange = 60.0f;
    constexpr float kStrikeRangeSquared = kStrikeRange * kStrikeRange;
    constexpr float kDetectionRange = 200.0f;
    constexpr float kDetectionRangeSquared = kDetectionRange * kDetectionRange;
    
    sf::Vector2f playerCenter = m_player.getGlobalBounds().getCenter();
    const sf::Vector2f enemyCenter = m_enemy.shape.getGlobalBounds().getCenter();
    
    sf::Vector2f toPlayer = playerCenter - enemyCenter;

    const float distanceToPlayerSquared =  toPlayer.lengthSquared();

    if (distanceToPlayerSquared <= 0.0f) {
        return;
    }

    if (m_enemyKnockback.lengthSquared() > 20.0f) {
        m_enemy.shape.setFillColor(m_enemy.baseColor);
        moveWithCollisions(m_enemy.shape, m_enemyKnockback * dt.asSeconds());
        m_enemyKnockback *= 0.90f;
        return;
    }

    m_enemyKnockback = {0.0f, 0.0f};

    switch (m_enemyState) {
        case EnemyState::Idle:
            if (distanceToPlayerSquared < kDetectionRangeSquared) {
                setEnemyState(EnemyState::Chase);
            }
            break;
        case EnemyState::Chase: {
            if (distanceToPlayerSquared <= kStrikeRangeSquared) {
                setEnemyState(EnemyState::WindUp);
                break;
            }
            
            if (distanceToPlayerSquared > kDetectionRangeSquared) {
                setEnemyState(EnemyState::Idle);
            }

            sf::Vector2f velocity = toPlayer.normalized() * (speed * dt.asSeconds());
            moveWithCollisions(m_enemy.shape, velocity);
        }
            break;
        case EnemyState::WindUp: {
            constexpr float WindUpChargeStrikeTime = 0.3f;

            m_enemy.shape.setFillColor(sf::Color::Cyan);
            
            m_enemyStateTimer += dt.asSeconds();

            if (m_enemyStateTimer >= WindUpChargeStrikeTime) {
                setEnemyState(EnemyState::Strike);
            }
        }
            break;
        case EnemyState::Strike: {
            constexpr float kForce = 500.0f;
            
            m_enemy.shape.setFillColor(sf::Color::Magenta);

            if (distanceToPlayerSquared > kStrikeRangeSquared) {
                setEnemyState(EnemyState::Chase);
                break;
            }

            sf::Vector2f knockback = toPlayer.normalized() * kForce;
            m_playerKnockback = knockback;
            setEnemyState(EnemyState::Recover);
        }
            break;
        case EnemyState::Recover: {
            constexpr float recoverTime = 1.0f;

            m_enemyStateTimer += dt.asSeconds();

            if (m_enemyStateTimer >= recoverTime) {
                setEnemyState(EnemyState::Chase);
            }
        }
            break;
        case EnemyState::Dead: m_enemy.shape.setFillColor(sf::Color(20, 20, 20, 255)); break;
        default:
            break;
    }
}

void Game::update(sf::Time dt) {
    updatePlayer(dt);
    updateEnemy(dt);
}

void Game::render() {
    m_window.clear(sf::Color::Black);
    
    if (m_attackPhase == AttackPhase::Swing) {
        m_window.draw(m_sword);
    }

    for (const auto& wall : m_walls) {
        m_window.draw(wall);
    }

    m_window.draw(m_enemy.shape);
    
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
