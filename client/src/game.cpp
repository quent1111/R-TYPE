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
        texture_manager_.load("assets/r-typesheet1.3.png");
        texture_manager_.load("assets/r-typesheet1.4.png");
        texture_manager_.load("assets/r-typesheet1.5.png");
        texture_manager_.load("assets/r-typesheet26.png");
        texture_manager_.load("assets/r-typesheet24.png");
        texture_manager_.load("assets/ennemi-projectile.png");
        texture_manager_.load("assets/shield.png");
        texture_manager_.load("assets/r-typesheet30.gif");
        texture_manager_.load("assets/r-typesheet30a.gif");
        std::cout << "[Game] Boss textures loaded: r-typesheet30.gif and r-typesheet30a.gif"
                  << std::endl;
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

    auto& audio = AudioManager::getInstance();
    audio.loadSounds();
    audio.playMusic("assets/sounds/game-loop.ogg", true);
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

    score_text_.setFont(font_);
    score_text_.setCharacterSize(36);
    score_text_.setFillColor(sf::Color(255, 215, 0));
    score_text_.setStyle(sf::Text::Bold);
    score_text_.setString("SCORE: 0");
    score_text_.setPosition(WINDOW_WIDTH - 300, 20);

    timer_text_.setFont(font_);
    timer_text_.setCharacterSize(28);
    timer_text_.setFillColor(sf::Color(50, 255, 50));
    timer_text_.setStyle(sf::Text::Bold);
    timer_text_.setString("00:00.000");
    sf::FloatRect timer_bounds = timer_text_.getLocalBounds();
    timer_text_.setPosition((WINDOW_WIDTH - timer_bounds.width) / 2.0f, 15);

    combo_text_.setFont(font_);
    combo_text_.setCharacterSize(24);
    combo_text_.setFillColor(sf::Color::White);
    combo_text_.setStyle(sf::Text::Bold);
    combo_text_.setString("1x");
    combo_text_.setPosition(WINDOW_WIDTH - 250, 75);

    combo_bar_bg_.setSize(sf::Vector2f(150, 20));
    combo_bar_bg_.setFillColor(sf::Color(40, 40, 40, 200));
    combo_bar_bg_.setPosition(WINDOW_WIDTH - 210, 80);
    combo_bar_bg_.setOutlineColor(sf::Color(100, 100, 100));
    combo_bar_bg_.setOutlineThickness(2);

    combo_bar_fill_.setSize(sf::Vector2f(0, 16));
    combo_bar_fill_.setFillColor(sf::Color(255, 150, 0));
    combo_bar_fill_.setPosition(WINDOW_WIDTH - 206, 82);

    combo_timer_bar_.setSize(sf::Vector2f(150, 4));
    combo_timer_bar_.setFillColor(sf::Color(255, 80, 80));
    combo_timer_bar_.setPosition(WINDOW_WIDTH - 210, 103);

    EffectsManager::getInstance().setScorePosition(sf::Vector2f(WINDOW_WIDTH - 200, 40));

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
    level_text_.setPosition(10, 10);
    level_text_.setStyle(sf::Text::Bold);
    progress_text_.setFont(font_);
    progress_text_.setCharacterSize(20);
    progress_text_.setFillColor(sf::Color::White);
    progress_text_.setPosition(10, 45);
    progress_bar_bg_.setSize(sf::Vector2f(300.0f, 25.0f));
    progress_bar_bg_.setPosition(10.0f, 75.0f);
    progress_bar_bg_.setFillColor(sf::Color(50, 50, 50, 200));
    progress_bar_bg_.setOutlineColor(sf::Color::White);
    progress_bar_bg_.setOutlineThickness(2.0f);
    progress_bar_fill_.setSize(sf::Vector2f(0.0f, 21.0f));
    progress_bar_fill_.setPosition(12.0f, 77.0f);
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

    shield_frames_ = {
        {0, 0, 27, 27}, {27, 0, 34, 34}, {61, 0, 42, 42}, {103, 0, 51, 51}, {154, 0, 55, 55}};

    shield_visual_.setTextureRect(shield_frames_[0]);
    shield_visual_.setScale(2.0f, 2.0f);

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

    game_over_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    game_over_overlay_.setFillColor(sf::Color(0, 0, 0, 200));

    texture_manager_.load("assets/gameover2.png");
    if (texture_manager_.has("assets/gameover2.png")) {
        game_over_sprite_.setTexture(*texture_manager_.get("assets/gameover2.png"));
        auto bounds = game_over_sprite_.getLocalBounds();
        game_over_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        game_over_sprite_.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    }
}

void Game::handle_event(const sf::Event& event) {
    if (!has_focus_) {
        return;
    }

    if (show_powerup_selection_ && event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pixel_pos(event.mouseButton.x, event.mouseButton.y);
            sf::Vector2f mouse_pos = window_.mapPixelToCoords(pixel_pos);

            if (powerup_option1_bg_.getGlobalBounds().contains(mouse_pos)) {
                game_to_network_queue_.push(GameToNetwork::Message::powerup_choice(1));
                show_powerup_selection_ = false;
                powerup_type_ = 1;
                std::cout << "[Game] Powerup 1 selected via click" << std::endl;
            } else if (powerup_option2_bg_.getGlobalBounds().contains(mouse_pos)) {
                game_to_network_queue_.push(GameToNetwork::Message::powerup_choice(2));
                show_powerup_selection_ = false;
                powerup_type_ = 2;
                std::cout << "[Game] Powerup 2 selected via click" << std::endl;
            }
        }
    }
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
        if (!was_shooting_) {
            AudioManager::getInstance().playSound(AudioManager::SoundType::Laser);
            was_shooting_ = true;
        }
    } else {
        was_shooting_ = false;
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

    EffectsManager::getInstance().update(dt);

    if (timer_running_ && !show_game_over_) {
        game_time_ += dt;
        int total_ms = static_cast<int>(game_time_ * 1000.0f);
        int minutes = total_ms / 60000;
        int seconds = (total_ms % 60000) / 1000;
        int milliseconds = total_ms % 1000;
        char timer_str[16];
        std::snprintf(timer_str, sizeof(timer_str), "%02d:%02d.%03d", minutes, seconds,
                      milliseconds);
        timer_text_.setString(timer_str);
        sf::FloatRect bounds = timer_text_.getLocalBounds();
        timer_text_.setPosition((WINDOW_WIDTH - bounds.width) / 2.0f, 15);
    }

    if (displayed_score_ < current_score_) {
        uint32_t diff = current_score_ - displayed_score_;
        uint32_t increment = std::max(1u, diff / 10);
        displayed_score_ = std::min(displayed_score_ + increment, current_score_);
        score_text_.setString("SCORE: " + std::to_string(displayed_score_));
    }

    process_network_messages();

    for (auto& [player_id, powerup_info] : player_powerups_) {
        uint8_t type = powerup_info.first;
        float time = powerup_info.second;
        if (type == 2) {
            if (player_shield_anim_timer_.find(player_id) == player_shield_anim_timer_.end()) {
                player_shield_anim_timer_[player_id] = 0.0f;
                player_shield_frame_[player_id] = 0;
            }
            player_shield_anim_timer_[player_id] += dt;
            if (time > 1.0f) {
                if (player_shield_anim_timer_[player_id] < 0.3f) {
                    int target_frame =
                        static_cast<int>((player_shield_anim_timer_[player_id] / 0.3f) * 4.0f);
                    player_shield_frame_[player_id] = std::min(target_frame, 4);
                } else {
                    player_shield_frame_[player_id] = 4;
                }
            } else if (time > 0.0f && time <= 1.0f) {
                int target_frame = 4 - static_cast<int>((1.0f - time) * 4.0f);
                player_shield_frame_[player_id] = std::max(0, target_frame);
            } else {
                player_shield_anim_timer_.erase(player_id);
                player_shield_frame_.erase(player_id);
            }
        }
    }

    if (show_game_over_) {
        game_over_timer_ += dt;
        if (game_over_timer_ >= game_over_duration_) {
            is_running_ = false;
        }
        return;
    }

    if (show_level_intro_) {
        level_intro_timer_ += dt;
        if (level_intro_timer_ >= level_intro_duration_) {
            show_level_intro_ = false;
            if (!timer_running_) {
                timer_running_ = true;
            }
        }
    }
}

void Game::init_entity_sprite(Entity& entity) {
    if (entity.type == 0x01) {
        std::string sprite_sheet;
        if (entity.id == 2) {
            sprite_sheet = "assets/r-typesheet1.3.png";
        } else if (entity.id == 3) {
            sprite_sheet = "assets/r-typesheet1.4.png";
        } else if (entity.id == 4) {
            sprite_sheet = "assets/r-typesheet1.5.png";
        } else {
            sprite_sheet = "assets/r-typesheet1.png";
        }

        if (!texture_manager_.has(sprite_sheet)) {
            sprite_sheet = "assets/r-typesheet1.png";
        }

        if (texture_manager_.has(sprite_sheet)) {
            entity.sprite.setTexture(*texture_manager_.get(sprite_sheet));

            entity.frames = {{100, 0, 33, 17},
                             {133, 0, 33, 17},
                             {166, 0, 33, 17},
                             {199, 0, 33, 17},
                             {232, 0, 33, 17}};
            entity.current_frame_index = 2;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[2]);
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
    } else if (entity.type == 0x06) {
        if (texture_manager_.has("assets/r-typesheet24.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet24.png"));
            entity.frames = {{0, 0, 65, 66},
                             {65, 0, 65, 66},
                             {130, 0, 65, 66},
                             {195, 0, 65, 66},
                             {260, 0, 66, 66}};
            entity.frame_duration = 0.12F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            sf::FloatRect temp_bounds = entity.sprite.getLocalBounds();
            entity.sprite.setOrigin(temp_bounds.width / 2.0f, temp_bounds.height / 2.0f);
            entity.sprite.setScale(-1.5F, 1.5F);
            return;
        }
    } else if (entity.type == 0x03) {
        bool is_enemy2_projectile = (entity.vx < 0 && std::abs(entity.vy) > 10.0f);

        if (is_enemy2_projectile && texture_manager_.has("assets/ennemi-projectile.png")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/ennemi-projectile.png"));
            entity.frames = {{0, 0, 18, 19}, {18, 0, 18, 19}};
            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        } else {
            std::string sprite_sheet =
                (entity.vx < 0) ? "assets/r-typesheet1.3.png" : "assets/r-typesheet1.png";

            if (!texture_manager_.has(sprite_sheet)) {
                sprite_sheet = "assets/r-typesheet1.png";
            }

            if (texture_manager_.has(sprite_sheet)) {
                entity.sprite.setTexture(*texture_manager_.get(sprite_sheet));
                entity.frames = {{231, 102, 16, 17}, {247, 102, 16, 17}};
                entity.frame_duration = 0.08F;
                entity.loop = true;
                entity.sprite.setTextureRect(entity.frames[0]);
                entity.sprite.setScale(2.0F, 2.0F);
            }
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
            entity.sprite.setScale(4.0F, 4.0F);
        }
    } else if (entity.type == 0x08) {
        // Boss entity - main boss sprite (r-typesheet30.gif)
        if (texture_manager_.has("assets/r-typesheet30.gif")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet30.gif"));
            entity.frames = {{0, 0, 185, 204},    {0, 215, 185, 204}, {0, 428, 185, 204},
                             {0, 642, 185, 204},  {0, 859, 185, 204}, {0, 1071, 185, 204},
                             {0, 1283, 185, 204}, {0, 1496, 185, 204}};

            entity.frame_duration = 1.15F;
            entity.loop = false;
            entity.ping_pong = true;
            entity.forward = true;
            entity.pause_at_end = 0.01f;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(3.5F, 3.5F);
        }
    } else if (entity.type == 0x07) {
        // Boss projectile (r-typesheet30a.gif)
        if (texture_manager_.has("assets/r-typesheet30a.gif")) {
            entity.sprite.setTexture(*texture_manager_.get("assets/r-typesheet30a.gif"));

            entity.frames = {{0, 0, 33, 33}, {33, 0, 33, 33}, {66, 0, 33, 33}};

            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(3.0F, 3.0F);
        }
    }

    sf::FloatRect bounds = entity.sprite.getLocalBounds();
    entity.sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
}

void Game::update_ship_tilt(Entity& entity, float) {
    if (entity.type != 0x01 || entity.frames.size() != 5) {
        return;
    }

    int target_frame = 2;
    const float velocity_threshold = 50.0f;
    float target_rotation = 0.0f;

    if (entity.vy < -velocity_threshold) {
        if (entity.vy < -200.0f) {
            target_frame = 4;
            target_rotation = -15.0f;
        } else {
            target_frame = 3;
            target_rotation = -8.0f;
        }
    } else if (entity.vy > velocity_threshold) {
        if (entity.vy > 200.0f) {
            target_frame = 0;
            target_rotation = 15.0f;
        } else {
            target_frame = 1;
            target_rotation = 8.0f;
        }
    } else if (std::abs(entity.vy) < velocity_threshold * 0.5f) {
        if (entity.vx > velocity_threshold) {
            target_rotation = 15.0f;
        } else if (entity.vx < -velocity_threshold) {
            target_rotation = -15.0f;
        }
    }

    int current = static_cast<int>(entity.current_frame_index);
    if (current != target_frame) {
        if (current < target_frame) {
            current = std::min(current + 1, target_frame);
        } else {
            current = std::max(current - 1, target_frame);
        }
        entity.current_frame_index = static_cast<size_t>(current);
        entity.sprite.setTextureRect(entity.frames[entity.current_frame_index]);
    }

    float current_rotation = entity.sprite.getRotation();
    if (current_rotation > 180.0f) {
        current_rotation -= 360.0f;
    }

    float rotation_diff = target_rotation - current_rotation;
    float rotation_speed = 120.0f;
    float max_change = rotation_speed * (1.0f / 60.0f);

    if (std::abs(rotation_diff) > max_change) {
        if (rotation_diff > 0) {
            entity.sprite.setRotation(current_rotation + max_change);
        } else {
            entity.sprite.setRotation(current_rotation - max_change);
        }
    } else {
        entity.sprite.setRotation(target_rotation);
    }
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
                        if (id == my_network_id_ && incoming.type == 0x01) {
                            if (prev_player_health_ > 0 && incoming.health < prev_player_health_) {
                                AudioManager::getInstance().playSound(
                                    AudioManager::SoundType::PlayerHit);
                                EffectsManager::getInstance().triggerDamageFlash();
                                EffectsManager::getInstance().triggerScreenShake(10.0f, 0.15f);
                            }
                            prev_player_health_ = incoming.health;
                        }

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
                        if (id == my_network_id_ && incoming.type == 0x01) {
                            prev_player_health_ = incoming.health;
                        }
                        incoming.prev_x = incoming.x;
                        incoming.prev_y = incoming.y;
                        incoming.prev_time = now;
                        init_entity_sprite(incoming);
                    }
                    incoming.curr_time = now;
                    next[id] = std::move(incoming);
                }

                for (const auto& [id, entity] : entities_) {
                    if ((entity.type == 0x02 || entity.type == 0x06) &&
                        next.find(id) == next.end()) {
                        AudioManager::getInstance().playSound(AudioManager::SoundType::HitSound);

                        EffectsManager::getInstance().addComboKill();
                        int combo_mult = EffectsManager::getInstance().getComboMultiplier();

                        sf::Vector2f enemy_pos(entity.x, entity.y);
                        EffectsManager::getInstance().spawnExplosion(enemy_pos, 25);

                        float shake_intensity = 16.0f + static_cast<float>(combo_mult - 1) * 4.0f;
                        EffectsManager::getInstance().triggerScreenShake(shake_intensity, 0.25f);

                        sf::Vector2f score_pos(WINDOW_WIDTH - 200, 40);
                        EffectsManager::getInstance().spawnScoreParticles(enemy_pos, score_pos, 12);

                        current_score_ += static_cast<uint32_t>(100 * combo_mult);
                        EffectsManager::getInstance().triggerScoreBounce();
                        AudioManager::getInstance().playSound(AudioManager::SoundType::Coin);
                    }
                }

                entities_.swap(next);
            } break;

            case NetworkToGame::MessageType::ConnectionStatus:
                if (!msg.is_connected) {
                    std::cout << "[Game] Connexion lost" << std::endl;
                    is_running_ = false;
                }
                break;
            case NetworkToGame::MessageType::LevelProgress: {
                uint16_t new_kills = static_cast<uint16_t>(msg.kills);
                if (new_kills > prev_enemies_killed_) {
                    AudioManager::getInstance().playSound(AudioManager::SoundType::Explosion);
                }
                prev_enemies_killed_ = new_kills;
                current_level_ = static_cast<uint8_t>(msg.level);
                enemies_killed_ = new_kills;
                enemies_needed_ = static_cast<uint16_t>(msg.enemies_needed);
            } break;
            case NetworkToGame::MessageType::LevelStart:
                current_level_ = static_cast<uint8_t>(msg.level);
                enemies_killed_ = 0;
                enemies_needed_ = static_cast<uint16_t>(
                    msg.level);  // Match server logic: level N needs N enemies
                show_level_intro_ = true;
                level_intro_timer_ = 0.0f;
                prev_enemies_killed_ = 0;
                break;
            case NetworkToGame::MessageType::LevelComplete:
                break;
            case NetworkToGame::MessageType::PowerUpSelection:
                show_powerup_selection_ = true;
                break;
            case NetworkToGame::MessageType::PowerUpStatus:
                player_powerups_[msg.powerup_player_id] = {msg.powerup_type,
                                                           msg.powerup_time_remaining};
                if (msg.powerup_player_id == my_network_id_) {
                    if (msg.powerup_type != 0) {
                        powerup_type_ = msg.powerup_type;
                    }
                    powerup_time_remaining_ = msg.powerup_time_remaining;
                }
                break;
            case NetworkToGame::MessageType::GameOver:
                show_game_over_ = true;
                game_over_timer_ = 0.0f;
                break;
        }
    }
}

void Game::render() {
    window_.clear(sf::Color(10, 10, 30));

    sf::View view = window_.getDefaultView();
    sf::Vector2f shake_offset = EffectsManager::getInstance().getScreenShakeOffset();
    view.move(shake_offset);
    window_.setView(view);

    window_.draw(bg_sprite1_);
    window_.draw(bg_sprite2_);
    const auto interp_delay = std::chrono::milliseconds(100);
    const auto render_time = std::chrono::steady_clock::now() - interp_delay;
    float dt = 1.0F / 60.0F;
    for (auto& pair : entities_) {
        Entity& e = pair.second;

        if (e.type == 0x01) {
            update_ship_tilt(e, dt);
        } else {
            e.update_animation(dt);
        }

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

    EffectsManager::getInstance().render(window_);

    window_.setView(window_.getDefaultView());

    window_.draw(timer_text_);

    float score_scale = EffectsManager::getInstance().getScoreScale();
    score_text_.setScale(score_scale, score_scale);
    sf::FloatRect bounds = score_text_.getLocalBounds();
    score_text_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    score_text_.setPosition(WINDOW_WIDTH - 150, 40);
    window_.draw(score_text_);
    score_text_.setOrigin(0, 0);
    score_text_.setScale(1.0f, 1.0f);

    for (const auto& [id, entity] : entities_) {
        if (entity.type == 0x01 && id == my_network_id_) {
            float health_percentage =
                static_cast<float>(entity.health) / static_cast<float>(entity.max_health);

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

            health_text_.setString("HP: " + std::to_string(entity.health) + " / " +
                                   std::to_string(entity.max_health));

            window_.draw(health_bar_bg_);
            window_.draw(health_bar_fill_);
            window_.draw(health_text_);
            break;
        }
    }

    render_level_hud();
    render_powerup_active();
    render_combo_bar();
    render_level_intro();
    render_powerup_selection();
    render_game_over();

    float flash_alpha = EffectsManager::getInstance().getDamageFlashAlpha();
    if (flash_alpha > 0.0f) {
        sf::RectangleShape flash_overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        flash_overlay.setFillColor(sf::Color(255, 0, 0, static_cast<sf::Uint8>(flash_alpha)));
        window_.draw(flash_overlay);
    }
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

void Game::render_combo_bar() {
    int combo_mult = EffectsManager::getInstance().getComboMultiplier();
    float combo_progress = EffectsManager::getInstance().getComboProgress();
    float combo_timer = EffectsManager::getInstance().getComboTimer();

    if (combo_mult == 1 && combo_progress == 0.0f) {
        return;
    }

    combo_text_.setString(std::to_string(combo_mult) + "x");

    sf::Color combo_color;
    switch (combo_mult) {
        case 1:
            combo_color = sf::Color(200, 200, 200);
            break;
        case 2:
            combo_color = sf::Color(255, 200, 0);
            break;
        case 3:
            combo_color = sf::Color(255, 150, 0);
            break;
        case 4:
            combo_color = sf::Color(255, 80, 0);
            break;
        case 5:
            combo_color = sf::Color(255, 50, 50);
            break;
        default:
            combo_color = sf::Color(255, 50, 50);
            break;
    }
    combo_text_.setFillColor(combo_color);
    combo_bar_fill_.setFillColor(combo_color);

    window_.draw(combo_bar_bg_);

    combo_bar_fill_.setSize(sf::Vector2f(142.0f * combo_progress, 16));
    window_.draw(combo_bar_fill_);

    combo_timer_bar_.setSize(sf::Vector2f(150.0f * combo_timer, 4));
    window_.draw(combo_timer_bar_);

    window_.draw(combo_text_);
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
    for (const auto& [player_id, powerup_info] : player_powerups_) {
        uint8_t type = powerup_info.first;
        float time = powerup_info.second;
        if (type == 2 && time > 0.0f) {
            auto it = entities_.find(player_id);
            if (it != entities_.end() && it->second.type == 0x01) {
                auto frame_it = player_shield_frame_.find(player_id);
                if (frame_it != player_shield_frame_.end()) {
                    int frame_index = frame_it->second;
                    if (frame_index >= 0 &&
                        static_cast<size_t>(frame_index) < shield_frames_.size()) {
                        if (texture_manager_.has("assets/shield.png")) {
                            shield_visual_.setTexture(*texture_manager_.get("assets/shield.png"));
                        }
                        shield_visual_.setTextureRect(
                            shield_frames_[static_cast<size_t>(frame_index)]);
                        sf::FloatRect bounds = shield_visual_.getLocalBounds();
                        shield_visual_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                        shield_visual_.setPosition(it->second.x, it->second.y);
                        window_.draw(shield_visual_);
                    }
                }
            }
        }
    }
}

void Game::render_game_over() {
    if (!show_game_over_) {
        return;
    }
    window_.draw(game_over_overlay_);
    window_.draw(game_over_sprite_);
}

void Game::run() {}
