#include "Game.hpp"

#include "inputKey.hpp"

#include <SFML/Graphics.hpp>
#include <cmath>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

Game::Game(sf::RenderWindow& window, ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
           ThreadSafeQueue<NetworkToGame::Message>& net_to_game)
    : window_(window),
      game_to_network_queue_(game_to_net),
      network_to_game_queue_(net_to_game),
      is_running_(true) {
    std::cout << "[Game] Initializing game logic..." << std::endl;

    setup_ui();
    current_level_ = 1;
    enemies_needed_ = 20;
    enemies_killed_ = 0;
    show_level_intro_ = true;
    level_intro_timer_ = 0.0f;

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
    level_intro_title_.setFont(font_);
    level_intro_title_.setCharacterSize(80);
    level_intro_title_.setFillColor(sf::Color::Yellow);
    level_intro_title_.setStyle(sf::Text::Bold);
    level_intro_subtitle_.setFont(font_);
    level_intro_subtitle_.setCharacterSize(40);
    level_intro_subtitle_.setFillColor(sf::Color::White);
    level_intro_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    level_intro_overlay_.setFillColor(sf::Color(0, 0, 0, 150));
    level_text_.setFont(font_);
    level_text_.setCharacterSize(24);
    level_text_.setFillColor(sf::Color::Yellow);
    level_text_.setPosition(10, 50);
    level_text_.setStyle(sf::Text::Bold);
    progress_text_.setFont(font_);
    progress_text_.setCharacterSize(20);
    progress_text_.setFillColor(sf::Color::White);
    progress_text_.setPosition(10, 85);
    progress_bar_bg_.setSize(sf::Vector2f(300.0f, 25.0f));
    progress_bar_bg_.setPosition(10.0f, 115.0f);
    progress_bar_bg_.setFillColor(sf::Color(50, 50, 50, 200));
    progress_bar_bg_.setOutlineColor(sf::Color::White);
    progress_bar_bg_.setOutlineThickness(2.0f);
    progress_bar_fill_.setSize(sf::Vector2f(0.0f, 21.0f));
    progress_bar_fill_.setPosition(12.0f, 117.0f);
    progress_bar_fill_.setFillColor(sf::Color(0, 200, 0));

    powerup_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    powerup_overlay_.setFillColor(sf::Color(0, 0, 0, 180));
    powerup_title_.setFont(font_);
    powerup_title_.setCharacterSize(60);
    powerup_title_.setFillColor(sf::Color::Yellow);
    powerup_title_.setStyle(sf::Text::Bold);
    powerup_title_.setString("CHOOSE YOUR POWER-UP");
    powerup_option1_bg_.setSize(sf::Vector2f(400.0f, 200.0f));
    powerup_option1_bg_.setPosition(400.0f, 400.0f);
    powerup_option1_bg_.setFillColor(sf::Color(50, 50, 150, 200));
    powerup_option1_bg_.setOutlineColor(sf::Color::White);
    powerup_option1_bg_.setOutlineThickness(3.0f);
    powerup_option1_text_.setFont(font_);
    powerup_option1_text_.setCharacterSize(30);
    powerup_option1_text_.setFillColor(sf::Color::White);
    powerup_option1_text_.setString(
        "1. POWER CANNON\n\n   5x Damage\n   Press X to use\n   10 seconds");
    powerup_option1_text_.setPosition(420.0f, 420.0f);
    powerup_option2_bg_.setSize(sf::Vector2f(400.0f, 200.0f));
    powerup_option2_bg_.setPosition(1120.0f, 400.0f);
    powerup_option2_bg_.setFillColor(sf::Color(150, 50, 50, 200));
    powerup_option2_bg_.setOutlineColor(sf::Color::White);
    powerup_option2_bg_.setOutlineThickness(3.0f);
    powerup_option2_text_.setFont(font_);
    powerup_option2_text_.setCharacterSize(30);
    powerup_option2_text_.setFillColor(sf::Color::White);
    powerup_option2_text_.setString(
        "2. SHIELD\n\n   Kill enemies on touch\n   Press X to use\n   10 seconds");
    powerup_option2_text_.setPosition(1140.0f, 420.0f);
    powerup_instruction_.setFont(font_);
    powerup_instruction_.setCharacterSize(25);
    powerup_instruction_.setFillColor(sf::Color::Cyan);
    powerup_instruction_.setString("Press 1 or 2 to choose");
    powerup_instruction_.setPosition(760.0f, 650.0f);
    powerup_active_text_.setFont(font_);
    powerup_active_text_.setCharacterSize(22);
    powerup_active_text_.setFillColor(sf::Color::Cyan);
    powerup_active_text_.setPosition(10, 150);
    powerup_active_text_.setStyle(sf::Text::Bold);

    shield_visual_.setRadius(100.0f);
    shield_visual_.setFillColor(sf::Color(100, 200, 255, 80));
    shield_visual_.setOutlineColor(sf::Color(0, 255, 255, 220));
    shield_visual_.setOutlineThickness(5.0f);
    shield_visual_.setOrigin(100.0f, 100.0f);

    powerup_hint_text_.setFont(font_);
    powerup_hint_text_.setCharacterSize(28);
    powerup_hint_text_.setFillColor(sf::Color(255, 255, 0, 255));
    powerup_hint_text_.setStyle(sf::Text::Bold);
    powerup_hint_text_.setOutlineColor(sf::Color::Black);
    powerup_hint_text_.setOutlineThickness(3.0f);
    powerup_hint_text_.setPosition(20, 950);

    powerup_hint_bg_.setSize(sf::Vector2f(600.0f, 50.0f));
    powerup_hint_bg_.setPosition(10, 940);
    powerup_hint_bg_.setFillColor(sf::Color(0, 0, 0, 180));
    powerup_hint_bg_.setOutlineColor(sf::Color::Yellow);
    powerup_hint_bg_.setOutlineThickness(2.0f);

    health_bar_bg_.setSize(sf::Vector2f(300.0f, 30.0f));
    health_bar_bg_.setPosition(10.0f, 160.0f);
    health_bar_bg_.setFillColor(sf::Color(50, 50, 50, 200));
    health_bar_bg_.setOutlineColor(sf::Color::White);
    health_bar_bg_.setOutlineThickness(2.0f);

    health_bar_fill_.setSize(sf::Vector2f(296.0f, 26.0f));
    health_bar_fill_.setPosition(12.0f, 162.0f);
    health_bar_fill_.setFillColor(sf::Color(0, 255, 0));

    health_text_.setFont(font_);
    health_text_.setCharacterSize(20);
    health_text_.setFillColor(sf::Color::White);
    health_text_.setPosition(20.0f, 165.0f);
    health_text_.setStyle(sf::Text::Bold);
    health_text_.setString("HP: 100 / 100");
}

void Game::handle_input() {
    if (!has_focus_) {
        return;
    }
    if (show_powerup_selection_) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
            game_to_network_queue_.push(GameToNetwork::Message::powerup_choice(1));
            show_powerup_selection_ = false;
            powerup_type_ = 1;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
            game_to_network_queue_.push(GameToNetwork::Message::powerup_choice(2));
            show_powerup_selection_ = false;
            powerup_type_ = 2;
        }
        return;
    }
    static bool x_pressed_last_frame = false;
    bool x_pressed_now = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
    if (x_pressed_now && !x_pressed_last_frame) {
        if (powerup_type_ != 0 && powerup_time_remaining_ <= 0.0f) {
            game_to_network_queue_.push(GameToNetwork::Message::powerup_activate());
        }
    }
    x_pressed_last_frame = x_pressed_now;

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
    if (show_level_intro_) {
        level_intro_timer_ += dt;
        if (level_intro_timer_ >= level_intro_duration_) {
            show_level_intro_ = false;
        }
    }
}

void Game::init_entity_sprite(Entity& entity) {
    if (entity.type == 0x01) {
        if (texture_manager_.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet1.png"));

            entity.frames = {{99, 0, 33, 17}, {132, 0, 33, 17}, {165, 0, 33, 17}};
            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x02) {
        if (texture_manager_.has("assets/r-typesheet26.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet26.png"));
            entity.frames = {{0, 0, 65, 50}, {65, 0, 65, 50}, {130, 0, 65, 50}};
            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(1.5F, 1.5F);
        }
    } else if (entity.type == 0x03) {
        if (texture_manager_.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet1.png"));
            entity.frames = {{231, 102, 16, 17}, {247, 102, 16, 17}};
            entity.frame_duration = 0.08F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x05) {
        if (texture_manager_.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet1.png"));
            entity.frames = {{330, 289, 28, 34},
                             {362, 289, 28, 34},
                             {394, 289, 28, 34},
                             {426, 289, 28, 34},
                             {0, 0, 1, 1}};
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
                my_network_id_ = msg.my_network_id;
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
            case NetworkToGame::MessageType::LevelProgress:
                current_level_ = static_cast<uint8_t>(msg.level);
                enemies_killed_ = static_cast<uint16_t>(msg.kills);
                enemies_needed_ = static_cast<uint16_t>(msg.enemies_needed);
                break;
            case NetworkToGame::MessageType::LevelStart:
                current_level_ = static_cast<uint8_t>(msg.level);
                enemies_killed_ = 0;
                enemies_needed_ = static_cast<uint16_t>(20 + (msg.level - 1) * 10);
                show_level_intro_ = true;
                level_intro_timer_ = 0.0f;
                break;
            case NetworkToGame::MessageType::LevelComplete:
                break;
            case NetworkToGame::MessageType::PowerUpSelection:
                show_powerup_selection_ = true;
                break;
            case NetworkToGame::MessageType::PowerUpStatus:
                if (msg.powerup_type != 0) {
                    powerup_type_ = msg.powerup_type;
                }
                powerup_time_remaining_ = msg.powerup_time_remaining;
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

        if (e.type == 0x03) {
            if (e.vx != 0.0f || e.vy != 0.0f) {
                float angle_rad = std::atan2(e.vy, e.vx);
                float angle_deg = angle_rad * 180.0f / 3.14159265f;
                e.sprite.setRotation(angle_deg);
            }
        }

        window_.draw(e.sprite);
    }

    window_.setView(window_.getDefaultView());

    std::string info = "Entities: " + std::to_string(entities_.size()) + "\n";
    info += "Controls: Z/Q/S/D + Space\n";
    info_text_.setString(info);
    window_.draw(info_text_);

    for (const auto& [id, entity] : entities_) {
        if (entity.type == 0x01 && id == my_network_id_) {
            static int last_health = -1;
            if (entity.health != last_health) {
                std::cout << "[Health] My player (ID=" << my_network_id_ << ") HP: " << entity.health << "/" << entity.max_health << std::endl;
                last_health = entity.health;
            }
            float health_percentage = static_cast<float>(entity.health) / static_cast<float>(entity.max_health);

            sf::Color health_color;
            if (health_percentage > 0.6f) {
                health_color = sf::Color(0, 255, 0);
            } else if (health_percentage > 0.3f) {
                health_color = sf::Color(255, 165, 0);
            } else {
                health_color = sf::Color(255, 0, 0);
            }

            health_bar_fill_.setFillColor(health_color);
            health_bar_fill_.setSize(sf::Vector2f(296.0f * health_percentage, 26.0f));

            health_text_.setString("HP: " + std::to_string(entity.health) + " / " + std::to_string(entity.max_health));

            window_.draw(health_bar_bg_);
            window_.draw(health_bar_fill_);
            window_.draw(health_text_);
            break;
        }
    }

    render_level_hud();
    render_powerup_active();
    render_level_intro();
    render_powerup_selection();
}

void Game::render_level_intro() {
    if (!show_level_intro_) {
        return;
    }
    window_.draw(level_intro_overlay_);
    level_intro_title_.setString("LEVEL " + std::to_string(current_level_));
    sf::FloatRect title_bounds = level_intro_title_.getLocalBounds();
    level_intro_title_.setPosition(WINDOW_WIDTH / 2 - title_bounds.width / 2,
                                   WINDOW_HEIGHT / 2 - 80.0f);
    window_.draw(level_intro_title_);
    level_intro_subtitle_.setString("KILL " + std::to_string(enemies_needed_) + " ENEMIES");
    sf::FloatRect sub_bounds = level_intro_subtitle_.getLocalBounds();
    level_intro_subtitle_.setPosition(WINDOW_WIDTH / 2 - sub_bounds.width / 2,
                                      WINDOW_HEIGHT / 2 + 20.0f);
    window_.draw(level_intro_subtitle_);
}

void Game::render_level_hud() {
    if (show_level_intro_) {
        return;
    }
    level_text_.setString("Level " + std::to_string(current_level_));
    window_.draw(level_text_);
    progress_text_.setString("Enemies: " + std::to_string(enemies_killed_) + " / " +
                             std::to_string(enemies_needed_));
    window_.draw(progress_text_);
    window_.draw(progress_bar_bg_);
    float progress = 0.0f;
    if (enemies_needed_ > 0) {
        progress = static_cast<float>(enemies_killed_) / static_cast<float>(enemies_needed_);
    }
    progress_bar_fill_.setSize(sf::Vector2f(296.0f * progress, 21.0f));
    window_.draw(progress_bar_fill_);
}

void Game::render_powerup_selection() {
    if (!show_powerup_selection_) {
        return;
    }
    window_.draw(powerup_overlay_);
    sf::FloatRect title_bounds = powerup_title_.getLocalBounds();
    powerup_title_.setPosition(WINDOW_WIDTH / 2 - title_bounds.width / 2, 200.0f);
    window_.draw(powerup_title_);
    window_.draw(powerup_option1_bg_);
    window_.draw(powerup_option1_text_);
    window_.draw(powerup_option2_bg_);
    window_.draw(powerup_option2_text_);
    sf::FloatRect inst_bounds = powerup_instruction_.getLocalBounds();
    powerup_instruction_.setPosition(WINDOW_WIDTH / 2 - inst_bounds.width / 2, 650.0f);
    window_.draw(powerup_instruction_);
}

void Game::render_powerup_active() {
    if (powerup_type_ == 0) {
        return;
    }
    std::string powerup_name = (powerup_type_ == 1) ? "Power Cannon" : "Protection";
    if (powerup_time_remaining_ > 0.0f) {
        int seconds = static_cast<int>(powerup_time_remaining_);
        std::string hint_text = powerup_name + " ACTIF: " + std::to_string(seconds) + "s";
        powerup_hint_text_.setString(hint_text);
        powerup_hint_text_.setFillColor(sf::Color::Green);
        powerup_hint_bg_.setOutlineColor(sf::Color::Green);
    } else {
        std::string hint_text = powerup_name + " disponible - Appuyez sur X";
        powerup_hint_text_.setString(hint_text);
        powerup_hint_text_.setFillColor(sf::Color(255, 255, 0, 255));
        powerup_hint_bg_.setOutlineColor(sf::Color::Yellow);
    }
    window_.draw(powerup_hint_bg_);
    window_.draw(powerup_hint_text_);
    if (powerup_type_ == 2 && powerup_time_remaining_ > 0.0f) {
        for (const auto& [id, entity] : entities_) {
            if (entity.type == 0x01) {
                shield_visual_.setPosition(entity.x, entity.y);
                window_.draw(shield_visual_);
                break;
            }
        }
    }
}

void Game::run() {}
