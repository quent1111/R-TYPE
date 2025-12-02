#include "Game.hpp"

#include <SFML/Graphics.hpp>
#include "inputKey.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <chrono>

Game::Game(ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
           ThreadSafeQueue<NetworkToGame::Message>& net_to_game)
    : game_to_network_queue_(game_to_net)
    , network_to_game_queue_(net_to_game)
    , window_(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "R-TYPE Client")
    , is_running_(true)
{
    std::cout << "[Game] Starting of the R-TYPE client..." << std::endl;

    window_.setFramerateLimit(FRAMERATE);

    setup_ui();
}

Game::~Game() {
    std::cout << "[Game] Closing of the client..." << std::endl;
}

void Game::setup_ui() {
    info_text_.setCharacterSize(20);
    info_text_.setFillColor(sf::Color::White);
    info_text_.setPosition(10, 10);
}

void Game::process_events() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
            is_running_ = false;
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window_.close();
                is_running_ = false;
            }
        }
    }
}

void Game::handle_input() {
    uint8_t input_mask = 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        input_mask |= KEY_Z;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        input_mask |= KEY_Q;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        input_mask |= KEY_S;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        input_mask |= KEY_D;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        input_mask |= KEY_SPACE;

    if (input_mask != 0) {
        game_to_network_queue_.push(
            GameToNetwork::Message(GameToNetwork::MessageType::SendInput, input_mask)
        );
    }
}

void Game::update() {
    process_network_messages();
}

void Game::process_network_messages() {
    NetworkToGame::Message msg(NetworkToGame::MessageType::EntityUpdate);

    while (network_to_game_queue_.try_pop(msg)) {
        switch (msg.type) {
            case NetworkToGame::MessageType::EntityUpdate: {
                const auto now = std::chrono::steady_clock::now();
                std::map<uint32_t, Entity> next;

                for (const auto& p : msg.entities) {
                    const uint32_t id = p.first;
                    Entity incoming = p.second;

                    auto it = entities_.find(id);
                    if (it != entities_.end()) {
                        incoming.prev_x = it->second.x;
                        incoming.prev_y = it->second.y;
                        incoming.prev_time = it->second.curr_time;
                    } else {
                        incoming.prev_x = incoming.x;
                        incoming.prev_y = incoming.y;
                        incoming.prev_time = now;
                    }
                    incoming.curr_time = now;

                    incoming.shape.setSize(sf::Vector2f(30.f, 30.f));
                    switch (incoming.type) {
                        case 0x01: incoming.shape.setFillColor(sf::Color::Green); break;
                        case 0x02: incoming.shape.setFillColor(sf::Color::Red); break;
                        case 0x03: incoming.shape.setFillColor(sf::Color::Yellow); break;
                        default: incoming.shape.setFillColor(sf::Color::White); break;
                    }

                    next[id] = std::move(incoming);
                }

                entities_.swap(next);
            } break;

            case NetworkToGame::MessageType::ConnectionStatus:
                if (!msg.is_connected) {
                    std::cout << "[Game] Connexion lost" << std::endl;
                    is_running_ = false;
                }
                break;
        }
    }
}

void Game::render() {
    window_.clear(sf::Color::Black);

    const auto interp_delay = std::chrono::milliseconds(100);
    const auto render_time = std::chrono::steady_clock::now() - interp_delay;

    for (auto& pair : entities_) {
        Entity& e = pair.second;

        float draw_x = e.x;
        float draw_y = e.y;

        auto prev_t = e.prev_time;
        auto curr_t = e.curr_time;

        if (curr_t > prev_t) {
            const double total_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(curr_t - prev_t).count();
            const double elapsed_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(render_time - prev_t).count();
            double alpha = (total_ms > 0.0) ? (elapsed_ms / total_ms) : 1.0;
            if (alpha <= 0.0) {
                draw_x = e.prev_x;
                draw_y = e.prev_y;
            } else if (alpha >= 1.0) {
                const double after_ms = std::chrono::duration_cast<std::chrono::duration<double>>(render_time - curr_t).count();
                if (after_ms > 0.0) {
                    draw_x = e.x + static_cast<float>(e.vx * after_ms);
                    draw_y = e.y + static_cast<float>(e.vy * after_ms);
                } else {
                    draw_x = e.x;
                    draw_y = e.y;
                }
            } else {
                draw_x = static_cast<float>(e.prev_x + (e.x - e.prev_x) * alpha);
                draw_y = static_cast<float>(e.prev_y + (e.y - e.prev_y) * alpha);
            }
        } else {
            draw_x = e.x;
            draw_y = e.y;
        }

        e.shape.setPosition(draw_x, draw_y);
        window_.draw(e.shape);
    }

    std::string info = "Entités: " + std::to_string(entities_.size()) + "\n";
    info += "Controls: Z/Q/S/D for moving\n";
    info += "ESC for quit";
    info_text_.setString(info);
    window_.draw(info_text_);

    window_.display();
}

void Game::run() {
    while (window_.isOpen() && is_running_) {
        process_events();
        handle_input();
        update();
        render();
    }
}
