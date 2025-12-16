#include "game/Game.hpp"

#include "common/Settings.hpp"
#include "input/InputKey.hpp"

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

    auto& texture_mgr = managers::TextureManager::instance();
    try {
        texture_mgr.load("assets/bg.png");
        texture_mgr.load("assets/r-typesheet1.png");
        texture_mgr.load("assets/r-typesheet1.3.png");
        texture_mgr.load("assets/r-typesheet1.4.png");
        texture_mgr.load("assets/r-typesheet1.5.png");
        texture_mgr.load("assets/r-typesheet26.png");
        texture_mgr.load("assets/r-typesheet24.png");
        texture_mgr.load("assets/ennemi-projectile.png");
        texture_mgr.load("assets/shield.png");
        texture_mgr.load("assets/r-typesheet30.gif");
        texture_mgr.load("assets/r-typesheet30a.gif");
        std::cout << "[Game] Boss textures loaded: r-typesheet30.gif and r-typesheet30a.gif"
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Game] Failed to load textures: " << e.what() << std::endl;
    }

    game_renderer_.init(window_);
    hud_renderer_.init(font_);
    overlay_renderer_.init(font_);

    setup_input_handler();

    auto& audio = managers::AudioManager::instance();
    audio.load_sounds();
    audio.play_music("assets/sounds/game-loop.ogg", true);
}

Game::~Game() {
    std::cout << "[Game] Closing of the client..." << std::endl;
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
        powerup_card1_sprite_.setScale(0.6f, 0.6f);
        powerup_card1_sprite_.setPosition(560.0f, 300.0f);
        powerup_card2_sprite_.setTexture(*bonus_texture);
        powerup_card2_sprite_.setTextureRect(
            sf::IntRect(card_width * 1, 0, card_width, card_height));
        powerup_card2_sprite_.setScale(0.6f, 0.6f);
        powerup_card2_sprite_.setPosition(1180.0f, 300.0f);
    }

    managers::EffectsManager::instance().set_score_position(sf::Vector2f(WINDOW_WIDTH - 200, 40));
}

void Game::setup_input_handler() {
    input_handler_.set_input_callback([this](uint8_t input_mask) {
        game_to_network_queue_.push(
            GameToNetwork::Message(GameToNetwork::MessageType::SendInput, input_mask));
    });

    input_handler_.set_powerup_choice_callback([this](uint8_t choice) {
        game_to_network_queue_.push(GameToNetwork::Message::powerup_choice(choice));
        show_powerup_selection_ = false;
        powerup_type_ = choice;
    });

    input_handler_.set_powerup_activate_callback(
        [this]() { game_to_network_queue_.push(GameToNetwork::Message::powerup_activate()); });

    input_handler_.set_shoot_sound_callback([]() {
        managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Laser);
    });
}

void Game::handle_event(const sf::Event& event) {
    if (!has_focus_) {
        return;
    }

    input_handler_.set_focus(has_focus_);
    input_handler_.set_powerup_selection_active(show_powerup_selection_);
    input_handler_.set_powerup_card_bounds(powerup_card1_sprite_.getGlobalBounds(),
                                           powerup_card2_sprite_.getGlobalBounds());

    input_handler_.handle_event(event, window_);

    show_powerup_selection_ = input_handler_.is_powerup_selection_active();
    powerup_type_ = input_handler_.get_selected_powerup_type();
}

void Game::handle_input() {
    if (!has_focus_) {
        return;
    }

    input_handler_.set_focus(has_focus_);
    input_handler_.set_powerup_selection_active(show_powerup_selection_);
    input_handler_.set_powerup_state(powerup_type_, powerup_time_remaining_);

    input_handler_.handle_input();

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
    ;

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
    auto& texture_mgr = managers::TextureManager::instance();

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
    } else if (entity.type == 0x03) {
        bool is_enemy2_projectile = (entity.vx < 0 && std::abs(entity.vy) > 10.0f);

        if (is_enemy2_projectile && texture_mgr.has("assets/ennemi-projectile.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/ennemi-projectile.png"));
            entity.frames = {{0, 0, 18, 19}, {18, 0, 18, 19}};
            entity.frame_duration = 0.1F;
            entity.loop = true;
            entity.sprite.setTextureRect(entity.frames[0]);
            entity.sprite.setScale(2.0F, 2.0F);
        } else {
            std::string sprite_sheet =
                (entity.vx < 0) ? "assets/r-typesheet1.3.png" : "assets/r-typesheet1.png";

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
    } else if (entity.type == 0x05) {
        if (texture_mgr.has("assets/r-typesheet1.png")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet1.png"));
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
    } else if (entity.type == 0x07) {
        if (texture_mgr.has("assets/r-typesheet30a.gif")) {
            entity.sprite.setTexture(*texture_mgr.get("assets/r-typesheet30a.gif"));

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
    window_.clear(sf::Color(0, 0, 0));

    float dt = 1.0F / 60.0F;

    game_renderer_.apply_screen_shake(window_);
    game_renderer_.render_background(window_);

    game_renderer_.render_entities(window_, entities_, my_network_id_, dt);

    game_renderer_.render_effects(window_);

    game_renderer_.restore_view(window_);

    hud_renderer_.update_score(current_score_);
    hud_renderer_.update_timer(game_time_);
    hud_renderer_.update_level(current_level_, enemies_killed_, enemies_needed_);

    hud_renderer_.render_timer(window_);
    hud_renderer_.render_score(window_);
    hud_renderer_.render_health_bar(window_, entities_, my_network_id_);
    hud_renderer_.render_level_hud(window_, show_level_intro_);
    hud_renderer_.render_combo_bar(window_);

    overlay_renderer_.render_powerup_active(window_, powerup_type_, powerup_time_remaining_,
                                            player_powerups_, entities_, player_shield_frame_);
    overlay_renderer_.render_level_intro(window_, show_level_intro_, current_level_,
                                         enemies_needed_);
    overlay_renderer_.render_powerup_selection(window_, show_powerup_selection_);
    overlay_renderer_.render_game_over(window_, show_game_over_);

    game_renderer_.render_damage_flash(window_);
    game_renderer_.render_colorblind_overlay(window_);
}

void Game::run() {}
