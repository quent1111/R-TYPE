#include "Game.hpp"

#include "inputKey.hpp"

#include <SFML/Graphics.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

Game::Game(sf::RenderWindow& window,
           ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
           ThreadSafeQueue<NetworkToGame::Message>& net_to_game)
    : window_(window),
      game_to_network_queue_(game_to_net),
      network_to_game_queue_(net_to_game),
      is_running_(true) {
    std::cout << "[Game] Initializing game logic..." << std::endl;

    setup_ui();

    try {
        texture_manager_.load("assets/bg.png");
        texture_manager_.load("assets/r-typesheet1.png");
        texture_manager_.load("assets/r-typesheet26.png");
    } catch (const std::exception& e) {
        std::cerr << "[Game] Failed to load textures: " << e.what() << std::endl;
    }

    sf::Texture* bg_tex = texture_manager_.get("assets/bg.png");
    if (bg_tex) {
        bg_tex->setRepeated(true);
        bg_sprite1_.setTexture(*bg_tex);
        bg_sprite2_.setTexture(*bg_tex);

        bg_sprite1_.setTextureRect(sf::IntRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
        bg_sprite2_.setTextureRect(sf::IntRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

        bg_sprite1_.setPosition(0, 0);
        bg_sprite2_.setPosition(WINDOW_WIDTH, 0);
    }
}

Game::~Game() {
    std::cout << "[Game] Closing of the client..." << std::endl;
}

void Game::setup_ui() {
    if (!font_.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[Game] Warning: Could not load font assets/fonts/arial.ttf" << std::endl;
    }
    info_text_.setFont(font_);
    info_text_.setCharacterSize(20);
    info_text_.setFillColor(sf::Color::White);
    info_text_.setPosition(10, 10);
}

void Game::handle_input() {
    if (!has_focus_) {
        return;
    }

    uint8_t input_mask = 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
        input_mask |= KEY_Z;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        input_mask |= KEY_Q;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        input_mask |= KEY_S;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        input_mask |= KEY_D;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        input_mask |= KEY_SPACE;
    }

    if (input_mask != 0) {
        game_to_network_queue_.push(
            GameToNetwork::Message(GameToNetwork::MessageType::SendInput, input_mask));
    }
}

void Game::update() {
    float dt = 1.0F / 60.0F;
    bg_scroll_offset_ += bg_scroll_speed_ * dt;
    if (bg_scroll_offset_ > WINDOW_WIDTH) {
        bg_scroll_offset_ -= WINDOW_WIDTH;
    }

    bg_sprite1_.setPosition(-bg_scroll_offset_, 0);
    bg_sprite2_.setPosition(WINDOW_WIDTH - bg_scroll_offset_, 0);

    process_network_messages();
}

void Game::init_entity_sprite(Entity& entity) {

    if (entity.type == 0x01) {
        if (texture_manager_.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet1.png"));

            entity.frames = {
                {99, 0, 33, 17},
                {132, 0, 33, 17},
                {165, 0, 33, 17}
            };
            entity.frame_duration = 0.15F;
            entity.loop = true;

            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x02) {
        if (texture_manager_.has("assets/r-typesheet26.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet26.png"));

            entity.frames = {
                {0, 0, 65, 50},
                {65, 0, 65, 50},
                {130, 0, 65, 50}
            };
            entity.frame_duration = 0.15F;
            entity.loop = true;

            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(1.5F, 1.5F);
        }
    } else if (entity.type == 0x03) {
        if (texture_manager_.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet1.png"));

            entity.frames = {
                {231, 102, 16, 17},
                {247, 102, 16, 17}
            };
            entity.frame_duration = 0.08F;
            entity.loop = true;

            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x05) {
        if (texture_manager_.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet1.png"));

            entity.frames = {
                {330, 289, 28, 34},
                {362, 289, 28, 34},
                {394, 289, 28, 34},
                {426, 289, 28, 34},
                {0, 0, 1, 1}
            };
            entity.frame_duration = 0.08F;
            entity.loop = false;

            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    }

    sf::FloatRect bounds = entity.sprite.getLocalBounds();
    entity.sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
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
                        if (it->second.type != incoming.type) {
                            incoming.prev_x = incoming.x;
                            incoming.prev_y = incoming.y;
                            incoming.prev_time = now;
                            init_entity_sprite(incoming);
                        } else {
                            incoming.prev_x = it->second.x;
                            incoming.prev_y = it->second.y;
                            incoming.prev_time = it->second.curr_time;

                            incoming.sprite = it->second.sprite;
                            incoming.frames = it->second.frames;
                            incoming.current_frame_index = it->second.current_frame_index;
                            incoming.frame_duration = it->second.frame_duration;
                            incoming.time_accumulator = it->second.time_accumulator;
                            incoming.loop = it->second.loop;
                        }
                    } else {
                        incoming.prev_x = incoming.x;
                        incoming.prev_y = incoming.y;
                        incoming.prev_time = now;
                        init_entity_sprite(incoming);
                    }
                    incoming.curr_time = now;

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
    window_.clear(sf::Color(10, 10, 30));

    window_.draw(bg_sprite1_);
    window_.draw(bg_sprite2_);

    const auto interp_delay = std::chrono::milliseconds(100);
    const auto render_time = std::chrono::steady_clock::now() - interp_delay;
    float dt = 1.0F / 60.0F;

    for (auto& pair : entities_) {
        Entity& e = pair.second;

        e.update_animation(dt);

        float draw_x = e.x;
        float draw_y = e.y;

        auto prev_t = e.prev_time;
        auto curr_t = e.curr_time;

        if (curr_t > prev_t) {
            const float total_ms =
                std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(curr_t -
                                                                                     prev_t)
                    .count();
            const float elapsed_ms =
                std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(render_time -
                                                                                     prev_t)
                    .count();
            float alpha = (total_ms > 0.0f) ? (elapsed_ms / total_ms) : 1.0f;

            if (alpha < 0.0F) {
                alpha = 0.0F;
            }
            if (alpha > 1.0F) {
                alpha = 1.0F;
            }

            draw_x = e.prev_x + (e.x - e.prev_x) * alpha;
            draw_y = e.prev_y + (e.y - e.prev_y) * alpha;
        }

        e.sprite.setPosition(draw_x, draw_y);
        window_.draw(e.sprite);
    }

    std::string info = "Entities: " + std::to_string(entities_.size()) + "\n";
    info += "Controls: Z/Q/S/D + Space\n";
    info_text_.setString(info);
    window_.draw(info_text_);
}

void Game::run() {
}
