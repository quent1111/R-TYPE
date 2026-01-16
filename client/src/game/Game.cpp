#include "game/Game.hpp"

#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "common/Settings.hpp"
#include "input/InputKey.hpp"
#include "level/CustomLevelLoader.hpp"

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

    my_activable_slots_.resize(2, {std::nullopt, 0});
    my_slot_timers_.resize(2, 0.0f);
    my_slot_cooldowns_.resize(2, 0.0f);
    my_slot_active_.resize(2, false);

    auto& texture_mgr = managers::TextureManager::instance();
    try {
        texture_mgr.load("assets/bg.png");
        texture_mgr.load("assets/r-typesheet1.png");
        texture_mgr.load("assets/r-typesheet1.3.png");
        texture_mgr.load("assets/r-typesheet1.4.png");
        texture_mgr.load("assets/r-typesheet1.5.png");
        texture_mgr.load("assets/r-typesheet26.png");
        texture_mgr.load("assets/r-typesheet24.png");
        texture_mgr.load("assets/r-typesheet14-1.gif");
        texture_mgr.load("assets/r-typesheet14-22.gif");
        texture_mgr.load("assets/r-typesheet9-1.gif");
        texture_mgr.load("assets/r-typesheet9-22.gif");
        texture_mgr.load("assets/r-typesheet7.gif");
        texture_mgr.load("assets/r-typesheet9-3.gif");
        texture_mgr.load("assets/ennemi-projectile.png");
        texture_mgr.load("assets/canonpowerup.png");
        texture_mgr.load("assets/shield.png");
        texture_mgr.load("assets/r-typesheet30.gif");
        texture_mgr.load("assets/r-typesheet30a.gif");
        texture_mgr.load("assets/explosion.gif");
        texture_mgr.load("assets/weirdbaby.gif");
        texture_mgr.load("assets/r-typesheet5.gif");
        texture_mgr.load("assets/missile.png");
        texture_mgr.load("assets/drone.png");
        texture_mgr.load("assets/serpent_head.png");
        texture_mgr.load("assets/serpent_body.png");
        texture_mgr.load("assets/serpent_scale.png");
        texture_mgr.load("assets/serpent_tail.png");
        texture_mgr.load("assets/serpent_nest.png");
        texture_mgr.load("assets/serpent_homing.png");
        texture_mgr.load("assets/r-typesheet38-22.gif");
        std::cout << "[Game] Boss textures loaded: r-typesheet30.gif and r-typesheet30a.gif"
                  << std::endl;
        std::cout << "[Game] Serpent boss textures loaded" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Game] Failed to load textures: " << e.what() << std::endl;
    }
    try {
        texture_mgr.load("assets/laserbeam.png");
    } catch (...) {
    }

    game_renderer_.init(window_);
    hud_renderer_.init(font_);
    overlay_renderer_.init(font_);
    
    // Créer la RenderTexture pour le post-processing shader
    if (!render_texture_.create(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        std::cerr << "[Game] Failed to create render texture for colorblind shader" << std::endl;
    }

    setup_input_handler();

    auto& audio = managers::AudioManager::instance();
    audio.load_sounds();
    audio.play_music("assets/sounds/game-loop.ogg", true);

    entities_.clear();
    has_server_position_ = false;
    boss_spawn_triggered_ = false;
    show_game_over_ = false;
    game_over_timer_ = 0.0f;

    request_game_state();
}

Game::~Game() {
    std::cout << "[Game] Closing of the client..." << std::endl;
}

void Game::request_game_state() {
    std::cout << "[Game] Requesting full game state from server..." << std::endl;

    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::RequestGameState;

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket, serializer.data());
    game_to_network_queue_.push(msg);
}

void Game::setup_ui() {
    if (!font_.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[Game] Warning: Could not load font assets/fonts/arial.ttf" << std::endl;
    }

    auto& texture_mgr = managers::TextureManager::instance();
    texture_mgr.load("assets/bonus.png");
    auto* bonus_texture = texture_mgr.get("assets/bonus.png");
    if (bonus_texture) {
        const int card_width = 459;
        const int card_height = 759;
        powerup_card1_sprite_.setTexture(*bonus_texture);
        powerup_card1_sprite_.setTextureRect(
            sf::IntRect(card_width * 2, 0, card_width, card_height));
        powerup_card1_sprite_.setScale(1.2f, 1.2f);
        powerup_card1_sprite_.setPosition(560.0f, 500.0f);
        powerup_card2_sprite_.setTexture(*bonus_texture);
        powerup_card2_sprite_.setTextureRect(
            sf::IntRect(card_width * 1, 0, card_width, card_height));
        powerup_card2_sprite_.setScale(1.2f, 1.2f);
        powerup_card2_sprite_.setPosition(960.0f, 500.0f);
        powerup_card3_sprite_.setTexture(*bonus_texture);
        powerup_card3_sprite_.setTextureRect(
            sf::IntRect(card_width * 0, 0, card_width, card_height));
        powerup_card3_sprite_.setScale(1.2f, 1.2f);
        powerup_card3_sprite_.setPosition(1360.0f, 500.0f);
    }

    managers::EffectsManager::instance().set_score_position(sf::Vector2f(WINDOW_WIDTH - 200, 40));
}

void Game::setup_input_handler() {
    input_handler_.set_input_callback([this](uint8_t input_mask) {
        last_input_mask_ = input_mask;
        game_to_network_queue_.push(
            GameToNetwork::Message(GameToNetwork::MessageType::SendInput, input_mask));
    });

    input_handler_.set_powerup_choice_callback([this](uint8_t choice) {
        if (!powerup_choice_pending_) {
            return;
        }
        game_to_network_queue_.push(GameToNetwork::Message::powerup_choice(choice));
        show_powerup_selection_ = false;
        powerup_choice_pending_ = false;
        powerup_cards_.clear();
        std::cout << "[Game] Power-up choice made: " << static_cast<int>(choice) << std::endl;
    });

    input_handler_.set_powerup_activate_callback([this](uint8_t type) {
        game_to_network_queue_.push(GameToNetwork::Message::powerup_activate(type));
    });

    input_handler_.set_shoot_sound_callback([]() {
        managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Laser);
    });
}

void Game::handle_event(const sf::Event& event) {
    if (!has_focus_) {
        return;
    }

    if (m_settings_panel && m_settings_panel->is_open()) {
        if (event.type == sf::Event::MouseMoved) {
            sf::Vector2i pixel_pos(event.mouseMove.x, event.mouseMove.y);
            sf::Vector2f mouse_pos = window_.mapPixelToCoords(pixel_pos);
            m_settings_panel->handle_mouse_move(mouse_pos);
            return;
        }
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i pixel_pos(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f click_pos = window_.mapPixelToCoords(pixel_pos);
                m_settings_panel->handle_mouse_click(click_pos);
                return;
            }
        }
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i pixel_pos(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f release_pos = window_.mapPixelToCoords(pixel_pos);
                m_settings_panel->handle_mouse_release(release_pos);
                return;
            }
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                m_settings_panel->close();
                return;
            }
            m_settings_panel->handle_key_press(event.key.code);
            return;
        }
    }

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        if (!m_settings_panel) {
            m_settings_panel = std::make_unique<rtype::ui::SettingsPanel>(window_.getSize());
        }
        int connected = 0;
        for (const auto& [id, e] : entities_) {
            if (e.type == 0x01)
                ++connected;
        }
        m_settings_panel->open(true, connected);
        m_settings_panel->set_quit_callback([this]() { this->request_return_to_menu(); });
        return;
    }

    input_handler_.set_focus(has_focus_);
    input_handler_.set_powerup_selection_active(show_powerup_selection_);
    input_handler_.set_powerup_card_bounds(powerup_card1_sprite_.getGlobalBounds(),
                                           powerup_card2_sprite_.getGlobalBounds(),
                                           powerup_card3_sprite_.getGlobalBounds());

    input_handler_.handle_event(event, window_);

    show_powerup_selection_ = input_handler_.is_powerup_selection_active();
    powerup_type_ = input_handler_.get_selected_powerup_type();
}

void Game::handle_input(float dt) {
    if (!has_focus_) {
        return;
    }

    input_handler_.set_focus(has_focus_);
    input_handler_.set_powerup_selection_active(show_powerup_selection_);
    input_handler_.set_powerup_state(powerup_type_, powerup_time_remaining_);

    input_handler_.handle_input(dt);

    show_powerup_selection_ = input_handler_.is_powerup_selection_active();
    powerup_type_ = input_handler_.get_selected_powerup_type();
}

void Game::update() {
    float dt = 1.0F / 60.0F;

    game_renderer_.update(dt);

    managers::EffectsManager::instance().update(dt);

    if (timer_running_ && !show_game_over_) {
        game_time_ += dt;
    }

    if (displayed_score_ < current_score_) {
        uint32_t diff = current_score_ - displayed_score_;
        uint32_t increment = std::max(1u, diff / 10);
        displayed_score_ = std::min(displayed_score_ + increment, current_score_);
    }

    process_network_messages();

    if (m_settings_panel) {
        m_settings_panel->update(dt);

        if (m_settings_panel->needs_window_recreate()) {
            sf::Vector2u new_size;
            bool fullscreen = false;
            m_settings_panel->get_new_window_settings(new_size, fullscreen);
            m_settings_panel->clear_window_recreate_flag();

            if (fullscreen) {
                window_.create(sf::VideoMode(new_size.x, new_size.y), "R-TYPE - Multiplayer",
                               sf::Style::Fullscreen);
            } else {
                window_.create(sf::VideoMode(new_size.x, new_size.y), "R-TYPE - Multiplayer");
            }
            window_.setVerticalSyncEnabled(false);
            window_.setFramerateLimit(60);

            m_settings_panel.reset();
        }
    }

    if (has_server_position_ && my_network_id_ != 0) {
        float speed = 300.0f;
        float vx = 0.0f;
        float vy = 0.0f;

        if (last_input_mask_ & 0x01)
            vy = -speed;
        if (last_input_mask_ & 0x02)
            vy = speed;
        if (last_input_mask_ & 0x04)
            vx = -speed;
        if (last_input_mask_ & 0x08)
            vx = speed;

        predicted_player_x_ += vx * dt;
        predicted_player_y_ += vy * dt;

        predicted_player_x_ = std::max(0.0f, std::min(1920.0f, predicted_player_x_));
        predicted_player_y_ = std::max(0.0f, std::min(1080.0f, predicted_player_y_));

        auto it = entities_.find(my_network_id_);
        if (it != entities_.end() && it->second.type == 0x01) {
            float correction_speed = 10.0f;
            float dx = it->second.x - predicted_player_x_;
            float dy = it->second.y - predicted_player_y_;

            if (std::abs(dx) > 50.0f || std::abs(dy) > 50.0f) {
                predicted_player_x_ = it->second.x;
                predicted_player_y_ = it->second.y;
            } else {
                predicted_player_x_ += dx * correction_speed * dt;
                predicted_player_y_ += dy * correction_speed * dt;
            }
        }
    }

    if (boss_spawn_triggered_) {
        boss_roar_timer_ += dt;
        if (boss_roar_timer_ >= boss_roar_delay_) {
            managers::AudioManager::instance().play_sound(
                managers::AudioManager::SoundType::BossRoar);
            std::cout << "[Game] BOSS ROAR! (triggered at " << boss_roar_timer_ << "s)"
                      << std::endl;
            boss_spawn_triggered_ = false;
        }
    }

    for (auto& [id, entity] : entities_) {
        if ((entity.type == 0x08 || entity.type == 0x11 || entity.type == 0x12 || 
             entity.type == 0x1C || entity.type == 0x1D || entity.type == 0x1E) && 
            entity.damage_flash_timer > 0.0f) {
            float prev_timer = prev_boss_damage_timer_[id];
            if (prev_timer <= 0.0f && entity.damage_flash_timer > 0.0f) {
                sf::Vector2f boss_pos(entity.x, entity.y);
                managers::EffectsManager::instance().spawn_explosion(boss_pos, 8);
                managers::EffectsManager::instance().trigger_screen_shake(8.0f, 0.15f);
            }
            prev_boss_damage_timer_[id] = entity.damage_flash_timer;
            entity.damage_flash_timer -= dt;
            if (entity.damage_flash_timer < 0.0f) {
                entity.damage_flash_timer = 0.0f;
            }
        } else {
            prev_boss_damage_timer_[id] = 0.0f;
        }
    }

    for (auto& [key, time] : player_powerups_) {
        uint32_t player_id = key.first;
        uint8_t type = key.second;
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

void Game::init_entity_sprite(Entity& entity, [[maybe_unused]] uint32_t entity_id) {
    auto& texture_mgr = managers::TextureManager::instance();

    if (entity.type == 0x01) {
        std::string sprite_sheet;

        switch (entity.player_index) {
            case 1:
                sprite_sheet = "assets/r-typesheet1.png";
                break;
            case 2:
                sprite_sheet = "assets/r-typesheet1.3.png";
                break;
            case 3:
                sprite_sheet = "assets/r-typesheet1.4.png";
                break;
            case 4:
                sprite_sheet = "assets/r-typesheet1.5.png";
                break;
            default:
                sprite_sheet = "assets/r-typesheet1.png";
                break;
        }

        if (!texture_mgr.has(sprite_sheet)) {
            sprite_sheet = "assets/r-typesheet1.png";
        }

        if (texture_mgr.has(sprite_sheet)) {
            entity.sprite.setTexture(*texture_mgr.get(sprite_sheet));

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
        if (texture_mgr.has("assets/r-typesheet26.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet26.png"));
            entity.frames = {{0, 0, 65, 50}, {65, 0, 65, 50}, {130, 0, 65, 50}};
            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(1.5F, 1.5F);
        }
    } else if (entity.type == 0x30) {
        if (custom_level_config_ && !entity.custom_entity_id.empty()) {
            auto it = custom_level_config_->enemy_definitions.find(entity.custom_entity_id);
            if (it != custom_level_config_->enemy_definitions.end()) {
                const auto& enemy_def = it->second;
                if (texture_mgr.has(enemy_def.sprite.texture_path)) {
                    entity.sprite.setTexture(*texture_mgr.get(enemy_def.sprite.texture_path));
                    entity.frames.clear();
                    int fw = enemy_def.sprite.frame_width;
                    int fh = enemy_def.sprite.frame_height;

                    if (enemy_def.id == "fairy2") {
                        for (int i = 0; i < 5; ++i) {
                            entity.frames.push_back({i * 72, 0, 72, fh});
                        }
                        auto tex_size = texture_mgr.get(enemy_def.sprite.texture_path)->getSize();
                        int last_frame_x = 5 * 72;
                        unsigned int last_frame_w = tex_size.x - static_cast<unsigned int>(last_frame_x);
                        entity.frames.push_back({last_frame_x, 0, static_cast<int>(last_frame_w), fh});
                    } else if (enemy_def.id == "fairy3") {
                        entity.frames.push_back({0, 0, 74, fh});
                        entity.frames.push_back({100, 0, 74, fh});
                        entity.frames.push_back({195, 0, 74, fh});
                    } else {
                        for (int i = 0; i < enemy_def.sprite.frame_count; ++i) {
                            entity.frames.push_back({i * fw, 0, fw, fh});
                        }
                    }

                    entity.frame_duration = enemy_def.sprite.frame_duration;
                    entity.loop = true;
                    entity.sprite.setTextureRect(entity.frames[0]);
                    float sx = enemy_def.sprite.mirror_x ? -enemy_def.sprite.scale_x
                                                         : enemy_def.sprite.scale_x;
                    float sy = enemy_def.sprite.mirror_y ? -enemy_def.sprite.scale_y
                                                         : enemy_def.sprite.scale_y;
                    entity.sprite.setScale(sx, sy);
                    entity.sprite.setRotation(
                        enemy_def.sprite.rotation);
                } else {
                    std::cerr << "[Game] Error: Texture not loaded for custom enemy ID: "
                              << entity.custom_entity_id << std::endl;
                }
            } else {
                std::cerr << "[Game] Error: Custom enemy definition not found for ID: "
                          << entity.custom_entity_id << std::endl;
            }
        }
    } else if (entity.type == 0x06) {
        if (texture_mgr.has("assets/r-typesheet24.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet24.png"));
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
    } else if (entity.type == 0x0E) {
        if (texture_mgr.has("assets/r-typesheet14-1.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet14-1.gif"));
            entity.frames = {{62, 0, 64, 52},
                             {3, 0, 58, 52},
                             {62, 0, 64, 52}};
            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            sf::FloatRect temp_bounds = entity.sprite.getLocalBounds();
            entity.sprite.setOrigin(temp_bounds.width / 2.0f, temp_bounds.height / 2.0f);
            entity.sprite.setScale(1.8F, 1.8F);
            return;
        }
    } else if (entity.type == 0x0F) {
        if (texture_mgr.has("assets/r-typesheet9-1.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet9-1.gif"));
            entity.frames = {{56, 0, 55, 59}};
            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
            return;
        }
    } else if (entity.type == 0x17) {
        entity.frames = {};
        entity.sprite.setColor(sf::Color::Transparent);
        return;
    } else if (entity.type == 0x03) {
        float projectile_speed = std::sqrt(entity.vx * entity.vx + entity.vy * entity.vy);
        bool is_explosive_grenade = (projectile_speed >= 180.0f && projectile_speed <= 320.0f && 
                                      std::abs(entity.vy) > 50.0f);
        bool is_compiler_boss_level = (current_level_ >= 15 && current_level_ % 15 == 0);
        bool is_compiler_boss_projectile = (is_compiler_boss_level && projectile_speed < 450.0f);
        bool is_player_projectile = (entity.vx > 0 && !is_compiler_boss_projectile);
        bool is_enemy2_projectile = ((entity.vx < 0 || is_compiler_boss_projectile) && std::abs(entity.vy) > 10.0f && !is_explosive_grenade);
        bool is_missile_drone_projectile = (projectile_speed >= 580.0f && projectile_speed <= 620.0f && is_player_projectile);
        bool is_fast_projectile = (projectile_speed > 650.0f && is_player_projectile);
        bool is_enemy3_projectile = ((entity.vx < 0 || is_compiler_boss_projectile) && std::abs(projectile_speed - 400.0f) < 20.0f);
        bool is_enemy4_projectile = ((entity.vx < 0 || is_compiler_boss_projectile) && std::abs(projectile_speed - 500.0f) < 30.0f && std::abs(entity.vy) < 5.0f);
        bool is_enemy5_projectile = ((entity.vx < 0 || is_compiler_boss_projectile) && std::abs(projectile_speed - 450.0f) < 30.0f && std::abs(entity.vy) < 5.0f);
        bool is_compiler_normal_projectile = (is_compiler_boss_projectile &&
                                              projectile_speed >= 280.0f && projectile_speed <= 420.0f && 
                                              !is_explosive_grenade);

        if (is_explosive_grenade && texture_mgr.has("assets/r-typesheet16.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet16.gif"));
            entity.frames = {
                {0, 0, 32, 32},
                {33, 0, 32, 32},
                {66, 0, 32, 32},
                {33, 0, 32, 32}
            };
            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.ping_pong = false;
            entity.forward = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(1.8F, 1.8F);
        } else if (is_enemy5_projectile && texture_mgr.has("assets/r-typesheet9-3.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet9-3.gif"));
            entity.frames = {
                {0, 0, 30, 12},
                {30, 0, 30, 12}
            };
            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.ping_pong = true;
            entity.forward = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(-3.0F, 3.0F);
        } else if (is_enemy4_projectile && texture_mgr.has("assets/r-typesheet9-22.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet9-22.gif"));
            entity.frames = {
                {0, 0, 65, 18},
                {65, 0, 65, 18}
            };
            entity.frame_duration = 0.2F;
            entity.loop = true;
            entity.ping_pong = false;
            entity.forward = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(-2.0F, 2.0F);
        } else if (is_enemy3_projectile && texture_mgr.has("assets/r-typesheet14-22.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet14-22.gif"));
            entity.frames = {
                {48, 0, 16, 14},
                {32, 0, 16, 14},
                {16, 0, 16, 14},
                {0, 0, 16, 14}
            };
            entity.frame_duration = 0.15F;
            entity.loop = false;
            entity.ping_pong = true;
            entity.forward = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        } else if (is_missile_drone_projectile && !is_fast_projectile && texture_mgr.has("assets/missile.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/missile.png"));
            entity.frames = {{0, 0, 18, 17}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        } else if (is_fast_projectile && texture_mgr.has("assets/canonpowerup.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/canonpowerup.png"));
            entity.frames = {{0, 0, 51, 21}, {52, 0, 51, 21}};
            entity.frame_duration = 0.08F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        } else if (is_enemy2_projectile && texture_mgr.has("assets/ennemi-projectile.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/ennemi-projectile.png"));
            entity.frames = {{0, 0, 18, 19}, {18, 0, 18, 19}};
            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        } else if (is_compiler_normal_projectile && texture_mgr.has("assets/ennemi-projectile.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/ennemi-projectile.png"));
            entity.frames = {{0, 0, 18, 19}, {18, 0, 18, 19}};
            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(3.0F, 3.0F);
        } else {
            bool is_enemy_proj = (entity.vx < 0 || is_compiler_boss_projectile);
            std::string sprite_sheet = is_enemy_proj ? "assets/r-typesheet1.3.png" : "assets/r-typesheet1.png";

            if (!texture_mgr.has(sprite_sheet)) {
                sprite_sheet = "assets/r-typesheet1.png";
            }

            if (texture_mgr.has(sprite_sheet)) {
                entity.sprite.setTexture(*texture_mgr.get(sprite_sheet));
                entity.frames = {{231, 102, 16, 17}, {247, 102, 16, 17}};
                entity.frame_duration = 0.08F;
                entity.loop = true;
                entity.sprite.setTextureRect(entity.frames[0]);
                entity.sprite.setScale(2.0F, 2.0F);
            }
        }
    } else if (entity.type == 0x32) {
        if (!entity.custom_entity_id.empty() && texture_mgr.has(entity.custom_entity_id)) {
            bool found_projectile = false;

            if (custom_level_config_) {
                for (const auto& pair : custom_level_config_->enemy_definitions) {
                    const auto& enemy_def = pair.second;
                    if (enemy_def.projectile &&
                        enemy_def.projectile->sprite.texture_path == entity.custom_entity_id) {
                        const auto& proj = *enemy_def.projectile;
                        entity.sprite.setTexture(*texture_mgr.get(proj.sprite.texture_path));
                        entity.frames.clear();
                        int fw = proj.sprite.frame_width;
                        int fh = proj.sprite.frame_height;
                        for (int i = 0; i < proj.sprite.frame_count; ++i) {
                            entity.frames.push_back({i * fw, 0, fw, fh});
                        }
                        entity.frame_duration = proj.sprite.frame_duration;
                        entity.loop = true;
                        entity.sprite.setTextureRect(entity.frames[0]);
                        entity.sprite.setScale(proj.sprite.scale_x, proj.sprite.scale_y);
                        entity.sprite.setRotation(
                            proj.sprite.rotation);
                        found_projectile = true;
                        break;
                    }
                }

                if (!found_projectile && custom_level_config_->boss_definition &&
                    custom_level_config_->boss_definition->projectile &&
                    custom_level_config_->boss_definition->projectile->sprite.texture_path ==
                        entity.custom_entity_id) {
                    const auto& proj = *custom_level_config_->boss_definition->projectile;
                    entity.sprite.setTexture(*texture_mgr.get(proj.sprite.texture_path));
                    entity.frames.clear();
                    int fw = proj.sprite.frame_width;
                    int fh = proj.sprite.frame_height;
                    for (int i = 0; i < proj.sprite.frame_count; ++i) {
                        entity.frames.push_back({i * fw, 0, fw, fh});
                    }
                    entity.frame_duration = proj.sprite.frame_duration;
                    entity.loop = true;
                    entity.sprite.setTextureRect(entity.frames[0]);
                    entity.sprite.setScale(proj.sprite.scale_x, proj.sprite.scale_y);
                    entity.sprite.setRotation(proj.sprite.rotation);
                    found_projectile = true;
                }
            }

            if (!found_projectile) {
                std::cerr << "[Game] Warning: Custom projectile definition not found for texture: "
                          << entity.custom_entity_id << std::endl;
            }
        }
    } else if (entity.type == 0x05) {
        if (texture_mgr.has("assets/explosion.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/explosion.gif"));
            entity.frames = {{0, 0, 34, 38},   {34, 0, 33, 38},  {67, 0, 35, 38},
                             {102, 0, 34, 38}, {136, 0, 35, 38}, {171, 0, 35, 38}};
            entity.frame_duration = 0.08F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x08) {
        if (texture_mgr.has("assets/r-typesheet30.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet30.gif"));
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
    } else if (entity.type == 0x31) {
        if (custom_level_config_ && custom_level_config_->boss_definition &&
            !entity.custom_entity_id.empty()) {
            const auto& boss = *custom_level_config_->boss_definition;
            if (texture_mgr.has(boss.sprite.texture_path)) {
                entity.sprite.setTexture(*texture_mgr.get(boss.sprite.texture_path));
                entity.frames.clear();
                int fw = boss.sprite.frame_width;
                int fh = boss.sprite.frame_height;
                for (int i = 0; i < boss.sprite.frame_count; ++i) {
                    entity.frames.push_back({i * fw, 0, fw, fh});
                }
                entity.frame_duration = boss.sprite.frame_duration;
                entity.loop = true;
                entity.sprite.setTextureRect(entity.frames[0]);
                float sx = boss.sprite.mirror_x ? -boss.sprite.scale_x : boss.sprite.scale_x;
                float sy = boss.sprite.mirror_y ? -boss.sprite.scale_y : boss.sprite.scale_y;
                entity.sprite.setScale(sx, sy);
                entity.sprite.setRotation(boss.sprite.rotation);
            } else {
                std::cerr << "[Game] Error: Texture not loaded for custom boss ID: "
                          << entity.custom_entity_id << std::endl;
            }
        }
    } else if (entity.type == 0x07) {
        if (texture_mgr.has("assets/r-typesheet30a.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet30a.gif"));

            entity.frames = {{0, 0, 33, 33}, {33, 0, 33, 33}, {66, 0, 33, 33}};

            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(3.0F, 3.0F);
        }
    } else if (entity.type == 0x1A) {
        if (texture_mgr.has("assets/r-typesheet7.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet7.gif"));

            entity.frames = {{66, 34, 33, 33}};

            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        }
    } else if (entity.type == 0x09) {
        if (texture_mgr.has("assets/weirdbaby.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/weirdbaby.gif"));

            entity.frames = {{0, 0, 34, 35}, {34, 0, 35, 35}};

            entity.frame_duration = 0.15F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        }
    } else if (entity.type == 0x0B) {
        entity.sprite.setTexture(sf::Texture());
        entity.frames = {{0, 0, 1, 1}};
        entity.loop = false;
    } else if (entity.type == 0x0C) {
        if (texture_mgr.has("assets/r-typesheet5.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet5.gif"));
            entity.frames = {{495, 0, 33, 32}, {462, 0, 33, 32}};
            entity.current_frame_index = 0;
            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
            entity.grayscale = false;
        }
    } else if (entity.type == 0x0D) {
        if (texture_mgr.has("assets/drone.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/drone.png"));
            entity.frames = {{0, 0, 32, 40}};
            entity.current_frame_index = 0;
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(1.0F, 1.0F);
            entity.grayscale = false;
        }
    } else if (entity.type == 0x0A) {
        std::cout << "[WARNING] Entity " << entity.id << " uses deprecated Ally type 0x0A" << std::endl;
    } else if (entity.type == 0x10) {
        if (texture_mgr.has("assets/serpent_nest.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/serpent_nest.png"));
            auto tex_size = texture_mgr.get("assets/serpent_nest.png")->getSize();
            entity.frames = {{0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(4.0F, 4.0F);
        }
    } else if (entity.type == 0x11) {
        if (texture_mgr.has("assets/serpent_head.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/serpent_head.png"));
            auto tex_size = texture_mgr.get("assets/serpent_head.png")->getSize();
            entity.frames = {{0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        }
    } else if (entity.type == 0x12) {
        if (texture_mgr.has("assets/serpent_body.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/serpent_body.png"));
            auto tex_size = texture_mgr.get("assets/serpent_body.png")->getSize();
            entity.frames = {{0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        }
    } else if (entity.type == 0x13) {
        if (texture_mgr.has("assets/serpent_scale.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/serpent_scale.png"));
            auto tex_size = texture_mgr.get("assets/serpent_scale.png")->getSize();
            entity.frames = {{0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        }
    } else if (entity.type == 0x14) {
        if (texture_mgr.has("assets/serpent_tail.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/serpent_tail.png"));
            auto tex_size = texture_mgr.get("assets/serpent_tail.png")->getSize();
            entity.frames = {{0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
        }
    } else if (entity.type == 0x15) {
        if (texture_mgr.has("assets/serpent_homing.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/serpent_homing.png"));
            auto tex_size = texture_mgr.get("assets/serpent_homing.png")->getSize();
            entity.frames = {{0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)}};
            entity.frame_duration = 0.1F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x16) {
        entity.frames = {};
        entity.sprite.setColor(sf::Color::Transparent);
    } else if (entity.type == 0x17 && entity.frames.empty()) {
        entity.frames = {};
        entity.sprite.setColor(sf::Color::Transparent);
    } else if (entity.type == 0x18) {
        entity.frames = {};
        entity.sprite.setColor(sf::Color::Transparent);
    } else if (entity.type == 0x19) {
        entity.frames = {};
        entity.sprite.setColor(sf::Color::Transparent);
    } else if (entity.type == 0x1B) {
        entity.frames = {};
        entity.sprite.setColor(sf::Color::Transparent);
    } else if (entity.type == 0x1C) {
        if (texture_mgr.has("assets/r-typesheet38-22.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet38-22.gif"));
            entity.frames = {
                {2, 31, 113, 69},
                {2, 130, 114, 67}
            };
            entity.frame_duration = 0.5F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x1D) {
        if (texture_mgr.has("assets/r-typesheet38-22.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet38-22.gif"));
            entity.frames = {
                {115, 0, 99, 100},
                {115, 100, 99, 100}
            };
            entity.frame_duration = 0.5F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x1E) {
        if (texture_mgr.has("assets/r-typesheet38-22.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet38-22.gif"));
            entity.frames = {
                {213, 17, 99, 83},
                {213, 117, 99, 83}
            };
            entity.frame_duration = 0.5F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        }
    } else if (entity.type == 0x1F) {
        texture_mgr.load("assets/r-typesheet44.gif");
        if (texture_mgr.has("assets/r-typesheet44.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet44.gif"));
            entity.frames = {
                {-1, 98, 67, 65},
                {1, 98, 65, 63},
                {66, 98, 65, 63},
                {131, 98, 65, 63},
                {196, 98, 65, 63},
                {261, 98, 65, 63}
            };
            entity.frame_duration = 0.04F;
            entity.loop = false;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.5F, 2.5F);
            std::cout << "[CLIENT] CompilerExplosion sprite loaded! 6 frames, Scale: 2.5" << std::endl;
        }
    } else if (entity.type == 0x30 || entity.type == 0x31 || entity.type == 0x32) {
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
                        if (id == my_network_id_ && incoming.type == 0x01) {
                            if (prev_player_health_ > 0 && incoming.health < prev_player_health_) {
                                managers::AudioManager::instance().play_sound(
                                    managers::AudioManager::SoundType::PlayerHit);
                                managers::EffectsManager::instance().trigger_damage_flash();
                                managers::EffectsManager::instance().trigger_screen_shake(10.0f,
                                                                                          0.15f);
                            }
                            prev_player_health_ = incoming.health;
                        }

                        if ((incoming.type == 0x08 || incoming.type == 0x11 || incoming.type == 0x12 ||
                             incoming.type == 0x1C || incoming.type == 0x1D || incoming.type == 0x1E) &&
                            (it->second.type == incoming.type)) {
                            if (incoming.health < it->second.health) {
                                incoming.damage_flash_timer = incoming.damage_flash_duration;
                            }
                        }

                        if (it->second.type != incoming.type) {
                            incoming.prev_x = incoming.x;
                            incoming.prev_y = incoming.y;
                            incoming.prev_time = now;
                            init_entity_sprite(incoming, id);
                        } else {
                            bool needs_sprite_reset = false;

                            if (incoming.type == 0x01 &&
                                it->second.player_index != incoming.player_index) {
                                needs_sprite_reset = true;
                            }

                            if (it->second.sprite.getTexture() == nullptr) {
                                needs_sprite_reset = true;
                            }

                            if (needs_sprite_reset) {
                                incoming.prev_x = incoming.x;
                                incoming.prev_y = incoming.y;
                                incoming.prev_time = now;
                                init_entity_sprite(incoming, id);
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
                                incoming.grayscale = it->second.grayscale;
                            }
                        }
                    } else {
                        if (id == my_network_id_ && incoming.type == 0x01) {
                            prev_player_health_ = incoming.health;
                            if (!has_server_position_) {
                                predicted_player_x_ = incoming.x;
                                predicted_player_y_ = incoming.y;
                                has_server_position_ = true;
                            }
                        }
                        if (incoming.type == 0x08 && !boss_spawn_triggered_) {
                            std::cout
                                << "[Game] BOSS DETECTED! Launching music and preparing roar..."
                                << std::endl;
                            managers::AudioManager::instance().play_music(
                                "assets/sounds/bossfight.mp3", true);
                            boss_spawn_triggered_ = true;
                            boss_roar_timer_ = 0.0f;
                        }
                        if (incoming.type == 0x1F) {
                            boss_explosion_count_++;
                            if (boss_explosion_count_ % 5 == 1) {
                                managers::AudioManager::instance().play_sound(
                                    managers::AudioManager::SoundType::BossExplosion);
                                std::cout << "[CLIENT] Boss explosion sound #" << (boss_explosion_count_ / 5 + 1) << std::endl;
                            }
                        }
                        incoming.prev_x = incoming.x;
                        incoming.prev_y = incoming.y;
                        incoming.prev_time = now;
                        init_entity_sprite(incoming, id);
                    }
                    incoming.curr_time = now;
                    next[id] = std::move(incoming);
                }

                for (const auto& [id, entity] : entities_) {
                    if ((entity.type == 0x02 || entity.type == 0x06) &&
                        next.find(id) == next.end()) {
                        managers::AudioManager::instance().play_sound(
                            managers::AudioManager::SoundType::HitSound);

                        managers::EffectsManager::instance().add_combo_kill();
                        int combo_mult =
                            managers::EffectsManager::instance().get_combo_multiplier();

                        sf::Vector2f enemy_pos(entity.x, entity.y);
                        managers::EffectsManager::instance().spawn_explosion(enemy_pos, 25);

                        float shake_intensity = 16.0f + static_cast<float>(combo_mult - 1) * 4.0f;
                        managers::EffectsManager::instance().trigger_screen_shake(shake_intensity,
                                                                                  0.25f);

                        sf::Vector2f score_pos(WINDOW_WIDTH - 200, 40);
                        managers::EffectsManager::instance().spawn_score_particles(enemy_pos,
                                                                                   score_pos, 12);

                        current_score_ += static_cast<uint32_t>(100 * combo_mult);
                        managers::EffectsManager::instance().trigger_score_bounce();
                        managers::AudioManager::instance().play_sound(
                            managers::AudioManager::SoundType::Coin);
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
                    managers::AudioManager::instance().play_sound(
                        managers::AudioManager::SoundType::Explosion);
                }
                prev_enemies_killed_ = new_kills;
                current_level_ = static_cast<uint8_t>(msg.level);
                enemies_killed_ = new_kills;
                enemies_needed_ = static_cast<uint16_t>(msg.enemies_needed);
            } break;
            case NetworkToGame::MessageType::LevelStart:
                current_level_ = static_cast<uint8_t>(msg.level);
                enemies_killed_ = 0;
                enemies_needed_ = static_cast<uint16_t>(msg.level);
                show_level_intro_ = true;
                level_intro_timer_ = 0.0f;
                prev_enemies_killed_ = 0;
                boss_spawn_triggered_ = false;
                boss_explosion_count_ = 0;
                if (!msg.custom_level_id.empty()) {
                    load_custom_level(msg.custom_level_id);
                    if (custom_level_config_) {
                        game_renderer_.set_custom_background(
                            custom_level_config_->environment.background_texture,
                            custom_level_config_->environment.background_static);
                    }
                } else {
                    custom_level_config_ = std::nullopt;
                    current_custom_level_id_.clear();
                    game_renderer_.set_background_level(current_level_);
                }
                break;
            case NetworkToGame::MessageType::LevelComplete:
                break;
            case NetworkToGame::MessageType::PowerUpSelection:
                show_powerup_selection_ = true;
                powerup_choice_pending_ = true;
                break;
            case NetworkToGame::MessageType::PowerUpCards:
                powerup_cards_ = msg.powerup_cards;
                show_powerup_selection_ = true;
                powerup_choice_pending_ = true;

                overlay_renderer_.update_powerup_cards(powerup_cards_);

                update_powerup_card_sprites();

                std::cout << "[Game] Received " << powerup_cards_.size() << " power-up cards"
                          << std::endl;
                break;
            case NetworkToGame::MessageType::PowerUpStatus:
                player_powerups_[{msg.powerup_player_id, msg.powerup_type}] =
                    msg.powerup_time_remaining;
                if (msg.powerup_player_id == my_network_id_) {
                    powerup_type_ = msg.powerup_type;
                    powerup_time_remaining_ = msg.powerup_time_remaining;
                }
                break;
            case NetworkToGame::MessageType::ActivableSlots:
                if (msg.activable_slots.size() >= 2) {
                    for (size_t i = 0; i < 2; ++i) {
                        if (i < msg.activable_slots.size()) {
                            const auto& slot = msg.activable_slots[i];
                            if (slot.has_powerup) {
                                my_activable_slots_[i] = {
                                    static_cast<powerup::PowerupId>(slot.powerup_id), slot.level};
                                my_slot_timers_[i] = slot.time_remaining;
                                my_slot_cooldowns_[i] = slot.cooldown_remaining;
                                my_slot_active_[i] = slot.is_active;
                            } else {
                                my_activable_slots_[i] = {std::nullopt, 0};
                                my_slot_timers_[i] = 0.0f;
                                my_slot_cooldowns_[i] = 0.0f;
                                my_slot_active_[i] = false;
                            }
                        }
                    }
                }
                break;
            case NetworkToGame::MessageType::BossSpawn:
                managers::AudioManager::instance().play_music("assets/sounds/bossfight.mp3", true);
                boss_spawn_triggered_ = true;
                boss_roar_timer_ = 0.0f;
                std::cout << "[Game] Boss spawn received! Starting music, roar in "
                          << boss_roar_delay_ << "s" << std::endl;
                break;
            case NetworkToGame::MessageType::GameOver:
                show_game_over_ = true;
                game_over_timer_ = 0.0f;
                break;
        }
    }
}

void Game::render() {
    ColorBlindMode mode = Settings::instance().colorblind_mode;
    use_render_texture_ = (mode != ColorBlindMode::Normal);

    if (use_render_texture_) {
        render_texture_.clear(sf::Color(0, 0, 0));
        sf::View temp_view = window_.getView();
        render_texture_.setView(temp_view);
    }

    window_.clear(sf::Color(0, 0, 0));

    float dt = 1.0F / 60.0F;

    game_renderer_.apply_screen_shake(window_);
    game_renderer_.render_background(window_);

    game_renderer_.render_entities(window_, entities_, my_network_id_, dt,
                                    predicted_player_x_, predicted_player_y_);

    game_renderer_.render_laser_particles(window_, entities_, dt);

    game_renderer_.render_effects(window_);

    game_renderer_.restore_view(window_);

    hud_renderer_.update_score(current_score_);
    hud_renderer_.update_timer(game_time_);
    hud_renderer_.update_level(current_level_, enemies_killed_, enemies_needed_);

    hud_renderer_.render_timer(window_);
    hud_renderer_.render_score(window_);
    hud_renderer_.render_health_bar(window_, entities_, my_network_id_, show_level_intro_);
    hud_renderer_.render_level_hud(window_, show_level_intro_, !current_custom_level_id_.empty());
    hud_renderer_.render_combo_bar(window_);
    hud_renderer_.render_boss_health_bar(window_, entities_);

    overlay_renderer_.render_powerup_active(window_, player_powerups_, entities_,
                                            player_shield_frame_, my_network_id_);
    overlay_renderer_.render_level_intro(window_, show_level_intro_, current_level_,
                                         enemies_needed_, !current_custom_level_id_.empty());
    overlay_renderer_.render_powerup_selection(window_, show_powerup_selection_);
    overlay_renderer_.render_activable_slots(window_, my_activable_slots_, my_slot_timers_,
                                             my_slot_cooldowns_, my_slot_active_);
    overlay_renderer_.render_game_over(window_, show_game_over_);

    game_renderer_.render_damage_flash(window_);
    game_renderer_.render_level_transition(window_);

    if (m_settings_panel && m_settings_panel->is_open()) {
        m_settings_panel->render(window_);
    }
    
    // Appliquer le shader daltonien si nécessaire
    if (use_render_texture_) {
        game_renderer_.apply_colorblind_shader(window_, render_texture_);
    }
    
    // Afficher l'indicateur de mode daltonien
    game_renderer_.render_colorblind_overlay(window_);
}

void Game::update_powerup_card_sprites() {
    auto& texture_mgr = managers::TextureManager::instance();
    const auto& registry = powerup::PowerupRegistry::instance();

    if (powerup_cards_.size() > 0) {
        auto powerup_def =
            registry.get_powerup(static_cast<powerup::PowerupId>(powerup_cards_[0].id));
        if (powerup_def) {
            const auto& def = *powerup_def;
            std::string asset_path = def.asset_path;

            texture_mgr.load(asset_path);
            if (texture_mgr.has(asset_path)) {
                powerup_card1_sprite_.setTexture(*texture_mgr.get(asset_path));
                auto tex_size = texture_mgr.get(asset_path)->getSize();
                powerup_card1_sprite_.setTextureRect(
                    sf::IntRect(0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)));

                auto bounds = powerup_card1_sprite_.getLocalBounds();
                powerup_card1_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                powerup_card1_sprite_.setPosition(560.0f, 500.0f);
                powerup_card1_sprite_.setScale(1.2f, 1.2f);
            }
        }
    }

    if (powerup_cards_.size() > 1) {
        auto powerup_def =
            registry.get_powerup(static_cast<powerup::PowerupId>(powerup_cards_[1].id));
        if (powerup_def) {
            const auto& def = *powerup_def;
            std::string asset_path = def.asset_path;

            texture_mgr.load(asset_path);
            if (texture_mgr.has(asset_path)) {
                powerup_card2_sprite_.setTexture(*texture_mgr.get(asset_path));
                auto tex_size = texture_mgr.get(asset_path)->getSize();
                powerup_card2_sprite_.setTextureRect(
                    sf::IntRect(0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)));

                auto bounds = powerup_card2_sprite_.getLocalBounds();
                powerup_card2_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                powerup_card2_sprite_.setPosition(960.0f, 500.0f);
                powerup_card2_sprite_.setScale(1.2f, 1.2f);
            }
        }
    }

    if (powerup_cards_.size() > 2) {
        auto powerup_def =
            registry.get_powerup(static_cast<powerup::PowerupId>(powerup_cards_[2].id));
        if (powerup_def) {
            const auto& def = *powerup_def;
            std::string asset_path = def.asset_path;

            texture_mgr.load(asset_path);
            if (texture_mgr.has(asset_path)) {
                powerup_card3_sprite_.setTexture(*texture_mgr.get(asset_path));
                auto tex_size = texture_mgr.get(asset_path)->getSize();
                powerup_card3_sprite_.setTextureRect(
                    sf::IntRect(0, 0, static_cast<int>(tex_size.x), static_cast<int>(tex_size.y)));

                auto bounds = powerup_card3_sprite_.getLocalBounds();
                powerup_card3_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                powerup_card3_sprite_.setPosition(1360.0f, 500.0f);
                powerup_card3_sprite_.setScale(1.2f, 1.2f);
            }
        }
    }
}

void Game::load_custom_level(const std::string& level_id) {
    if (level_id.empty()) {
        custom_level_config_ = std::nullopt;
        current_custom_level_id_.clear();
        return;
    }

    auto loaded = level::CustomLevelLoader::load(level_id);
    if (loaded) {
        custom_level_config_ = std::move(loaded);
        current_custom_level_id_ = level_id;
        load_custom_level_textures();
        std::cout << "[Game] Loaded custom level config: " << custom_level_config_->name
                  << std::endl;
    } else {
        std::cerr << "[Game] Failed to load custom level: " << level_id << std::endl;
        custom_level_config_ = std::nullopt;
        current_custom_level_id_.clear();
    }
}

void Game::load_custom_level_textures() {
    if (!custom_level_config_)
        return;

    auto& texture_mgr = managers::TextureManager::instance();

    for (const auto& path : custom_level_config_->get_all_texture_paths()) {
        if (!path.empty() && !texture_mgr.has(path)) {
            try {
                texture_mgr.load(path);
                std::cout << "[Game] Loaded custom texture: " << path << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Game] Failed to load custom texture " << path << ": " << e.what()
                          << std::endl;
            }
        }
    }
}

void Game::run() {}
