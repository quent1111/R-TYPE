#include "rendering/GameRenderer.hpp"

#include "common/Settings.hpp"
#include "managers/TextureManager.hpp"

#include <cmath>

#include <chrono>

namespace rendering {

GameRenderer::GameRenderer() : bg_scroll_offset_(0.0f) {
    transition_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    transition_overlay_.setFillColor(sf::Color(0, 0, 0, 0));
    ruins_background_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    ruins_background_.setFillColor(sf::Color(25, 20, 35));
}

void GameRenderer::init(sf::RenderWindow& window) {
    auto& texture_mgr = managers::TextureManager::instance();

    sf::Texture& bg_texture = texture_mgr.load("assets/bg.png");
    bg_texture.setRepeated(true);

    bg_sprite1_.setTexture(bg_texture);
    bg_sprite1_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(WINDOW_WIDTH), static_cast<int>(WINDOW_HEIGHT)));
    bg_sprite1_.setPosition(0, 0);

    bg_sprite2_.setTexture(bg_texture);
    bg_sprite2_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(WINDOW_WIDTH), static_cast<int>(WINDOW_HEIGHT)));
    bg_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH), 0);
    sf::Texture& ruins_bg_texture = texture_mgr.load("assets/ruins-background.png");
    ruins_bg_texture.setRepeated(true);

    const float BG_SCALE = 1.5f;

    ruins_bg_sprite1_.setTexture(ruins_bg_texture);
    ruins_bg_sprite1_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(WINDOW_WIDTH / BG_SCALE), static_cast<int>(WINDOW_HEIGHT / BG_SCALE)));
    ruins_bg_sprite1_.setScale(BG_SCALE, BG_SCALE);
    ruins_bg_sprite1_.setPosition(0, 0);

    ruins_bg_sprite2_.setTexture(ruins_bg_texture);
    ruins_bg_sprite2_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(WINDOW_WIDTH / BG_SCALE), static_cast<int>(WINDOW_HEIGHT / BG_SCALE)));
    ruins_bg_sprite2_.setScale(BG_SCALE, BG_SCALE);
    ruins_bg_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH), 0);

    sf::Texture& ruins_bg2_texture = texture_mgr.load("assets/ruins-background2.png");
    ruins_bg2_texture.setRepeated(true);

    ruins_bg2_sprite1_.setTexture(ruins_bg2_texture);
    ruins_bg2_sprite1_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(WINDOW_WIDTH / BG_SCALE), static_cast<int>(WINDOW_HEIGHT / BG_SCALE)));
    ruins_bg2_sprite1_.setScale(BG_SCALE, BG_SCALE);
    ruins_bg2_sprite1_.setPosition(0, 0);

    ruins_bg2_sprite2_.setTexture(ruins_bg2_texture);
    ruins_bg2_sprite2_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(WINDOW_WIDTH / BG_SCALE), static_cast<int>(WINDOW_HEIGHT / BG_SCALE)));
    ruins_bg2_sprite2_.setScale(BG_SCALE, BG_SCALE);
    ruins_bg2_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH), 0);

    sf::Texture& boss_fight_texture = texture_mgr.load("assets/worm_bossfightbg.png");
    boss_fight_bg_sprite_.setTexture(boss_fight_texture);
    sf::Vector2u tex_size = boss_fight_texture.getSize();
    float scale_x = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(tex_size.x);
    float scale_y = static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(tex_size.y);
    boss_fight_bg_sprite_.setScale(scale_x, scale_y);
    boss_fight_bg_sprite_.setPosition(0, 0);

    std::vector<std::string> top_files = {
        "assets/ruins-top1.png", "assets/ruins-top2.png", "assets/ruins-top3.png",
        "assets/ruins-top4.png", "assets/ruins-top5.png"
    };
    std::vector<std::string> bottom_files = {
        "assets/ruins-bottom1.png", "assets/ruins-bottom2.png",
        "assets/ruins-bottom3.png", "assets/ruins-bottom4.png"
    };
    const float RUIN_SCALE = 5.0f;
    float x_pos = 0.0f;
    int top_index = 0;
    for (const auto& file : top_files) {
        sf::Sprite sprite;
        sprite.setTexture(texture_mgr.load(file));
        sprite.setScale(RUIN_SCALE, RUIN_SCALE);
        sprite.setOrigin(0, 0);
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float y_offset = -30.0f;
        sprite.setPosition(x_pos, y_offset);
        ruins_top_base_positions_.push_back(x_pos);
        ruins_top_base_y_positions_.push_back(y_offset);
        x_pos += bounds.width;
        ruins_top_sprites_.push_back(sprite);
        top_index++;
    }
    float total_top_width = x_pos;
    ruins_top_total_width_ = total_top_width;
    for (size_t i = 0; i < top_files.size(); ++i) {
        sf::Sprite sprite = ruins_top_sprites_[i];
        float base_x = ruins_top_base_positions_[i] + total_top_width;
        float y_offset = -30.0f;
        sprite.setPosition(base_x, y_offset);
        ruins_top_base_positions_.push_back(base_x);
        ruins_top_base_y_positions_.push_back(y_offset);
        ruins_top_sprites_.push_back(sprite);
    }
    x_pos = 0.0f;
    int bottom_index = 0;
    for (const auto& file : bottom_files) {
        sf::Sprite sprite;
        sprite.setTexture(texture_mgr.load(file));
        sprite.setScale(RUIN_SCALE, RUIN_SCALE);
        sprite.setOrigin(0, 0);
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float extra_offset = 40.0f;
        float y_pos = static_cast<float>(WINDOW_HEIGHT) - (bounds.height - extra_offset);
        sprite.setPosition(x_pos, y_pos);
        ruins_bottom_base_positions_.push_back(x_pos);
        ruins_bottom_base_y_positions_.push_back(y_pos);
        x_pos += bounds.width;
        ruins_bottom_sprites_.push_back(sprite);
        bottom_index++;
    }
    float total_bottom_width = x_pos;
    ruins_bottom_total_width_ = total_bottom_width;
    for (size_t i = 0; i < bottom_files.size(); ++i) {
        sf::Sprite sprite = ruins_bottom_sprites_[i];
        float base_x = ruins_bottom_base_positions_[i] + total_bottom_width;
        float y_pos = ruins_bottom_base_y_positions_[i];
        sprite.setPosition(base_x, y_pos);
        ruins_bottom_base_positions_.push_back(base_x);
        ruins_bottom_base_y_positions_.push_back(y_pos);
        ruins_bottom_sprites_.push_back(sprite);
    }
    if (!transition_font_.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[GameRenderer] Failed to load transition font" << std::endl;
    }
    transition_text_.setFont(transition_font_);
    transition_text_.setCharacterSize(60);
    transition_text_.setFillColor(sf::Color::White);
    transition_text_.setStyle(sf::Text::Bold);
    transition_text_.setOutlineColor(sf::Color::Black);
    transition_text_.setOutlineThickness(3.0f);

    game_view_.setSize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    game_view_.setCenter(static_cast<float>(WINDOW_WIDTH) / 2.0f,
                         static_cast<float>(WINDOW_HEIGHT) / 2.0f);
    window.setView(game_view_);
}

void GameRenderer::update(float dt) {
    bool is_boss_level = (current_bg_level_ == 5 || current_bg_level_ == 10 || current_bg_level_ == 15);

    if (current_bg_level_ >= 6) {
        if (bg_fade_active_) {
            bg_fade_timer_ += dt;
            if (bg_fade_timer_ >= bg_fade_duration_) {
                bg_fade_active_ = false;
                bg_fade_timer_ = bg_fade_duration_;
            }
        }

        if (!is_boss_level) {
            ruins_bg_scroll_offset_ += bg_scroll_speed_ * dt;
            if (ruins_bg_scroll_offset_ >= static_cast<float>(WINDOW_WIDTH)) {
                ruins_bg_scroll_offset_ -= static_cast<float>(WINDOW_WIDTH);
            }
        }

        if (current_bg_level_ >= 10) {
            ruins_bg2_sprite1_.setPosition(-ruins_bg_scroll_offset_, 0);
            ruins_bg2_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH) - ruins_bg_scroll_offset_, 0);
        } else {
            ruins_bg_sprite1_.setPosition(-ruins_bg_scroll_offset_, 0);
            ruins_bg_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH) - ruins_bg_scroll_offset_, 0);
        }

        if (!is_boss_level) {
            const float RUIN_SPEED = 300.0f;
            ruins_top_offset_ += RUIN_SPEED * dt;
            ruins_bottom_offset_ += RUIN_SPEED * dt;
            if (ruins_top_offset_ >= ruins_top_total_width_) {
                ruins_top_offset_ -= ruins_top_total_width_;
            }
            if (ruins_bottom_offset_ >= ruins_bottom_total_width_) {
                ruins_bottom_offset_ -= ruins_bottom_total_width_;
            }
        }

        for (size_t i = 0; i < ruins_top_sprites_.size(); ++i) {
            float x = ruins_top_base_positions_[i] - ruins_top_offset_;
            float y = ruins_top_base_y_positions_[i];
            while (x < -ruins_top_sprites_[i].getGlobalBounds().width) {
                x += ruins_top_total_width_ * 2;
            }
            ruins_top_sprites_[i].setPosition(x, y);
        }
        for (size_t i = 0; i < ruins_bottom_sprites_.size(); ++i) {
            float x = ruins_bottom_base_positions_[i] - ruins_bottom_offset_;
            float y = ruins_bottom_base_y_positions_[i];
            while (x < -ruins_bottom_sprites_[i].getGlobalBounds().width) {
                x += ruins_bottom_total_width_ * 2;
            }
            ruins_bottom_sprites_[i].setPosition(x, y);
        }
    } else {
        if (!is_boss_level) {
            bg_scroll_offset_ += bg_scroll_speed_ * dt;
            if (bg_scroll_offset_ >= static_cast<float>(WINDOW_WIDTH)) {
                bg_scroll_offset_ -= static_cast<float>(WINDOW_WIDTH);
            }
        }
        bg_sprite1_.setPosition(-bg_scroll_offset_, 0);
        bg_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH) - bg_scroll_offset_, 0);
    }
}

void GameRenderer::render_background(sf::RenderWindow& window) {
    if (current_bg_level_ == 10) {
        window.draw(boss_fight_bg_sprite_);
        return;
    }

    if (current_bg_level_ >= 6) {
        ruins_bg_sprite1_.setColor(sf::Color(255, 255, 255, 255));
        ruins_bg_sprite2_.setColor(sf::Color(255, 255, 255, 255));
        window.draw(ruins_bg_sprite1_);
        window.draw(ruins_bg_sprite2_);

        for (const auto& sprite : ruins_top_sprites_) {
            window.draw(sprite);
        }
        for (const auto& sprite : ruins_bottom_sprites_) {
            window.draw(sprite);
        }
    } else {
        window.draw(bg_sprite1_);
        window.draw(bg_sprite2_);
    }
}

void GameRenderer::set_background_level(uint8_t level) {
    if (current_bg_level_ == level) return;
    if (level == 6 && current_bg_level_ == 5) {
        target_bg_level_ = level;
        transition_active_ = true;
        transition_timer_ = 0.0f;
    } else if (level == 10 && current_bg_level_ == 9) {
        bg_fade_active_ = true;
        bg_fade_timer_ = 0.0f;
        current_bg_level_ = level;
    } else {
        current_bg_level_ = level;
    }
    std::cout << "[GameRenderer] Starting transition to level " << static_cast<int>(level) << std::endl;
}

void GameRenderer::render_level_transition(sf::RenderWindow& window) {
    if (!transition_active_) return;
    transition_timer_ += 1.0f / 60.0f;
    float progress = transition_timer_ / transition_duration_;
    if (progress < 0.33f) {
        float fade = progress / 0.33f;
        transition_overlay_.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(fade * 255)));
        window.draw(transition_overlay_);
    } else if (progress < 0.67f) {
        transition_overlay_.setFillColor(sf::Color(0, 0, 0, 255));
        window.draw(transition_overlay_);
        if (target_bg_level_ >= 6) {
            transition_text_.setString("Entering Ancient Ruins...");
        } else {
            transition_text_.setString("Level " + std::to_string(target_bg_level_));
        }
        sf::FloatRect textBounds = transition_text_.getLocalBounds();
        transition_text_.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        transition_text_.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
        float pulse_progress = (progress - 0.33f) / 0.34f;
        float pulse = 1.0f + 0.2f * std::sin(pulse_progress * 3.14159f * 4.0f);
        transition_text_.setScale(pulse, pulse);
        window.draw(transition_text_);
        if (progress >= 0.5f && current_bg_level_ != target_bg_level_) {
            current_bg_level_ = target_bg_level_;
        }
    } else {
        float fade = 1.0f - ((progress - 0.67f) / 0.33f);
        transition_overlay_.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(fade * 255)));
        window.draw(transition_overlay_);
    }
    if (progress >= 1.0f) {
        transition_active_ = false;
        transition_timer_ = 0.0f;
        std::cout << "[GameRenderer] Transition completed" << std::endl;
    }
}

void GameRenderer::update_ship_tilt(Entity& entity, float /*dt*/) {
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

void GameRenderer::update_ally_tilt(Entity& entity, float /*dt*/) {
    if (entity.type != 0x0A || entity.frames.size() != 2) {
        return;
    }

    int target_frame = 0;
    const float velocity_threshold = 50.0f;

    if (std::abs(entity.vy) > velocity_threshold) {
        target_frame = 1;
    } else {
        target_frame = 0;
    }

    if (entity.current_frame_index != static_cast<size_t>(target_frame)) {
        entity.current_frame_index = static_cast<size_t>(target_frame);
        entity.sprite.setTextureRect(entity.frames[entity.current_frame_index]);
    }
}

void GameRenderer::render_entities(sf::RenderWindow& window, std::map<uint32_t, Entity>& entities,
                                    uint32_t my_network_id, float dt, float predicted_x, float predicted_y) {
    const auto interp_delay = std::chrono::milliseconds(50);
    const auto render_time = std::chrono::steady_clock::now() - interp_delay;

    Entity* serpent_nest = nullptr;

    for (auto& pair : entities) {
        Entity& e = pair.second;
        uint32_t entity_id = pair.first;

        if (e.type == 0x10) {
            serpent_nest = &e;
            continue;
        }

        if (e.type == 0x01) {
            update_ship_tilt(e, dt);
        } else if (e.type == 0x0A || e.type == 0x0B || e.type == 0x0C || e.type == 0x0D) {
            update_ally_tilt(e, dt);
        } else {
            e.update_animation(dt);
        }

        float draw_x = e.x;
        float draw_y = e.y;

        if (entity_id == my_network_id && e.type == 0x01 && predicted_x >= 0.0f && predicted_y >= 0.0f) {
            draw_x = predicted_x;
            draw_y = predicted_y;
        } else {
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

                if (alpha > 1.0f) {
                    float overshoot_ms = elapsed_ms - total_ms;
                    float overshoot_frames = overshoot_ms / 16.67f;

                    if (overshoot_frames > 12.0f) {
                        overshoot_frames = 12.0f;
                    }

                    float extrapolation_time = overshoot_frames / 60.0f;
                    draw_x = e.x + e.vx * extrapolation_time;
                    draw_y = e.y + e.vy * extrapolation_time;
                } else {
                    alpha = std::max(0.0f, alpha);
                    draw_x = e.prev_x + (e.x - e.prev_x) * alpha;
                    draw_y = e.prev_y + (e.y - e.prev_y) * alpha;
                }
            }
        }
        e.sprite.setPosition(draw_x, draw_y);


        if (e.type == 0x03) {
            if (e.vx != 0.0f || e.vy != 0.0f) {
                float angle_rad = std::atan2(e.vy, e.vx);
                float angle_deg = angle_rad * 180.0f / 3.14159265f;
                e.sprite.setRotation(angle_deg);
            }
        }

        if (e.type == 0x11 || e.type == 0x12 || e.type == 0x14) {
            e.sprite.setRotation(e.rotation + 180.0f);
        } else if (e.type == 0x13) {
            e.sprite.setRotation(e.rotation);
        }

        if (e.type == 0x08 && e.damage_flash_timer > 0.0f) {
            e.sprite.setColor(sf::Color(255, 100, 100, 255));
        } else if (e.grayscale) {
            e.sprite.setColor(sf::Color(128, 128, 128, 255));
        } else {
            e.sprite.setColor(sf::Color(255, 255, 255, 255));
        }

        if (e.type == 0x01 && e.health <= 0) {
            continue;
        }

        if (e.type == 0x0B) {
            continue;
        }

        window.draw(e.sprite);
    }

    if (serpent_nest != nullptr) {
        serpent_nest->update_animation(dt);
        serpent_nest->sprite.setPosition(serpent_nest->x, serpent_nest->y);
        serpent_nest->sprite.setColor(sf::Color(255, 255, 255, 255));
        window.draw(serpent_nest->sprite);
    }
}

void GameRenderer::render_effects(sf::RenderWindow& window) {
    managers::EffectsManager::instance().render(window);
}

void GameRenderer::render_laser_particles(sf::RenderWindow& window, std::map<uint32_t, Entity>& entities, float dt) {
    std::vector<uint32_t> to_remove;
    for (auto& [laser_id, system] : laser_particle_systems_) {
        if (entities.find(laser_id) == entities.end()) {
            to_remove.push_back(laser_id);
        }
    }
    for (uint32_t id : to_remove) {
        laser_particle_systems_.erase(id);
    }

    to_remove.clear();
    for (auto& [laser_id, system] : serpent_laser_systems_) {
        if (entities.find(laser_id) == entities.end()) {
            to_remove.push_back(laser_id);
        }
    }
    for (uint32_t id : to_remove) {
        serpent_laser_systems_.erase(id);
    }

    bool scream_active = false;
    float scream_x = 0.0f, scream_y = 0.0f;
    bool charge_active = false;
    float charge_x = 0.0f, charge_y = 0.0f, charge_progress = 0.0f;

    for (auto& [entity_id, entity] : entities) {
        if (entity.type == 0x0B) {
            if (laser_particle_systems_.find(entity_id) == laser_particle_systems_.end()) {
                laser_particle_systems_[entity_id] = LaserParticleSystem();
                laser_particle_systems_[entity_id].set_active(true);
            }

            auto& system = laser_particle_systems_[entity_id];
            system.set_active(true);

            float laser_length = 2000.0f;
            system.update(dt, entity.x, entity.y, laser_length);
            system.render(window);
        }
        else if (entity.type == 0x18) {
            scream_active = true;
            scream_x = entity.x;
            scream_y = entity.y;
        }
        else if (entity.type == 0x19) {
            charge_active = true;
            charge_x = entity.x;
            charge_y = entity.y;
            charge_progress = entity.vy / 100.0f;
        }
    }

    serpent_effects_.update_scream(dt, scream_x, scream_y, scream_active);
    serpent_effects_.update_laser_charge(dt, charge_x, charge_y, charge_active, charge_progress);
    serpent_effects_.render(window);

    for (auto& [entity_id, entity] : entities) {
        if (entity.type == 0x16) {
            if (serpent_laser_systems_.find(entity_id) == serpent_laser_systems_.end()) {
                serpent_laser_systems_[entity_id] = SerpentLaserSystem();
            }

            auto& system = serpent_laser_systems_[entity_id];
            system.set_active(true);

            float angle = entity.vx / 100.0f;
            float length = entity.vy;

            system.update(dt, entity.x, entity.y, angle, length);
            system.render(window);
        }
    }
}

void GameRenderer::render_damage_flash(sf::RenderWindow& window) {
    float flash_alpha = managers::EffectsManager::instance().get_damage_flash_alpha();
    if (flash_alpha > 0.0f) {
        sf::RectangleShape flash_overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        flash_overlay.setFillColor(sf::Color(255, 0, 0, static_cast<sf::Uint8>(flash_alpha)));
        window.draw(flash_overlay);
    }
}

void GameRenderer::render_colorblind_overlay(sf::RenderWindow& window) {
    if (Settings::instance().colorblind_mode) {
        sf::RectangleShape colorblind_overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        colorblind_overlay.setFillColor(sf::Color(255, 200, 100, 40));
        window.draw(colorblind_overlay);
    }
}

void GameRenderer::apply_screen_shake(sf::RenderWindow& window) {
    sf::View view = game_view_;
    sf::Vector2f shake_offset = managers::EffectsManager::instance().get_screen_shake_offset();
    view.move(shake_offset);
    window.setView(view);
}

void GameRenderer::restore_view(sf::RenderWindow& window) {
    window.setView(game_view_);
}

}  // namespace rendering
