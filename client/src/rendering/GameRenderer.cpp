#include "rendering/GameRenderer.hpp"

#include "common/Settings.hpp"
#include "managers/TextureManager.hpp"

#include <cmath>

#include <chrono>

namespace rendering {

GameRenderer::GameRenderer() : bg_scroll_offset_(0.0f) {}

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

    game_view_.setSize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
    game_view_.setCenter(static_cast<float>(WINDOW_WIDTH) / 2.0f,
                         static_cast<float>(WINDOW_HEIGHT) / 2.0f);
    window.setView(game_view_);
}

void GameRenderer::update(float dt) {
    bg_scroll_offset_ += bg_scroll_speed_ * dt;
    if (bg_scroll_offset_ >= static_cast<float>(WINDOW_WIDTH)) {
        bg_scroll_offset_ -= static_cast<float>(WINDOW_WIDTH);
    }
    bg_sprite1_.setPosition(-bg_scroll_offset_, 0);
    bg_sprite2_.setPosition(static_cast<float>(WINDOW_WIDTH) - bg_scroll_offset_, 0);
}

void GameRenderer::render_background(sf::RenderWindow& window) {
    window.draw(bg_sprite1_);
    window.draw(bg_sprite2_);
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

void GameRenderer::render_entities(sf::RenderWindow& window, std::map<uint32_t, Entity>& entities,
                                   uint32_t /*my_network_id*/, float dt) {
    const auto interp_delay = std::chrono::milliseconds(50);
    const auto render_time = std::chrono::steady_clock::now() - interp_delay;

    for (auto& pair : entities) {
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
            alpha = std::max(0.0f, std::min(1.0f, alpha));

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

        if (e.type == 0x08 && e.damage_flash_timer > 0.0f) {
            e.sprite.setColor(sf::Color(255, 100, 100, 255));
        } else {
            e.sprite.setColor(sf::Color(255, 255, 255, 255));
        }

        window.draw(e.sprite);
    }
}

void GameRenderer::render_effects(sf::RenderWindow& window) {
    managers::EffectsManager::instance().render(window);
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
