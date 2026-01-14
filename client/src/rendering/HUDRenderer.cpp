#include "rendering/HUDRenderer.hpp"

#include "managers/EffectsManager.hpp"
#include <iostream>

namespace rendering {

HUDRenderer::HUDRenderer() {}

void HUDRenderer::init(const sf::Font& font) {
    score_text_.setFont(font);
    score_text_.setCharacterSize(36);
    score_text_.setFillColor(sf::Color(255, 215, 0));
    score_text_.setStyle(sf::Text::Bold);
    score_text_.setString("SCORE: 0");
    score_text_.setPosition(WINDOW_WIDTH - 300, 20);

    timer_text_.setFont(font);
    timer_text_.setCharacterSize(28);
    timer_text_.setFillColor(sf::Color(50, 255, 50));
    timer_text_.setStyle(sf::Text::Bold);
    timer_text_.setString("00:00.000");
    sf::FloatRect timer_bounds = timer_text_.getLocalBounds();
    timer_text_.setPosition((WINDOW_WIDTH - timer_bounds.width) / 2.0f, 15);

    health_bar_bg_.setSize(sf::Vector2f(300.0f, 30.0f));
    health_bar_bg_.setPosition(10.0f, 160.0f);
    health_bar_bg_.setFillColor(sf::Color(50, 50, 50, 200));
    health_bar_bg_.setOutlineColor(sf::Color::White);
    health_bar_bg_.setOutlineThickness(2.0f);

    health_bar_fill_.setSize(sf::Vector2f(296.0f, 26.0f));
    health_bar_fill_.setPosition(12.0f, 162.0f);
    health_bar_fill_.setFillColor(sf::Color(0, 255, 0));

    health_text_.setFont(font);
    health_text_.setCharacterSize(20);
    health_text_.setFillColor(sf::Color::White);
    health_text_.setPosition(20.0f, 165.0f);
    health_text_.setStyle(sf::Text::Bold);
    health_text_.setString("HP: 100 / 100");

    level_text_.setFont(font);
    level_text_.setCharacterSize(24);
    level_text_.setFillColor(sf::Color::Yellow);
    level_text_.setPosition(10, 10);
    level_text_.setStyle(sf::Text::Bold);

    progress_text_.setFont(font);
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

    combo_text_.setFont(font);
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

    const float boss_bar_width = 800.0f;
    const float boss_bar_height = 30.0f;
    const float boss_bar_x = (WINDOW_WIDTH - boss_bar_width) / 2.0f;
    const float boss_bar_y = 90.0f;
    
    boss_health_bar_border_.setSize(sf::Vector2f(boss_bar_width + 6.0f, boss_bar_height + 6.0f));
    boss_health_bar_border_.setPosition(boss_bar_x - 3.0f, boss_bar_y - 3.0f);
    boss_health_bar_border_.setFillColor(sf::Color(80, 0, 0, 255));
    boss_health_bar_border_.setOutlineColor(sf::Color(200, 50, 50));
    boss_health_bar_border_.setOutlineThickness(3.0f);
    
    boss_health_bar_bg_.setSize(sf::Vector2f(boss_bar_width, boss_bar_height));
    boss_health_bar_bg_.setPosition(boss_bar_x, boss_bar_y);
    boss_health_bar_bg_.setFillColor(sf::Color(30, 30, 30, 220));
    
    boss_health_bar_fill_.setSize(sf::Vector2f(boss_bar_width, boss_bar_height));
    boss_health_bar_fill_.setPosition(boss_bar_x, boss_bar_y);
    boss_health_bar_fill_.setFillColor(sf::Color(200, 0, 0));
    
    boss_name_text_.setFont(font);
    boss_name_text_.setCharacterSize(28);
    boss_name_text_.setFillColor(sf::Color(255, 200, 50));
    boss_name_text_.setStyle(sf::Text::Bold);
    boss_name_text_.setOutlineColor(sf::Color::Black);
    boss_name_text_.setOutlineThickness(2.0f);
}

void HUDRenderer::update_score(uint32_t score) {
    score_text_.setString("SCORE: " + std::to_string(score));
}

void HUDRenderer::update_timer(float game_time) {
    int minutes = static_cast<int>(game_time) / 60;
    int seconds = static_cast<int>(game_time) % 60;
    int millis = static_cast<int>((game_time - static_cast<int>(game_time)) * 1000);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d.%03d", minutes, seconds, millis);
    timer_text_.setString(buffer);
}

void HUDRenderer::update_level(uint8_t level, uint16_t enemies_killed, uint16_t enemies_needed) {
    current_level_ = level;
    enemies_killed_ = enemies_killed;
    enemies_needed_ = enemies_needed;
}

void HUDRenderer::render_score(sf::RenderWindow& window) {
    float score_scale = managers::EffectsManager::instance().get_score_scale();
    score_text_.setScale(score_scale, score_scale);

    sf::FloatRect bounds = score_text_.getLocalBounds();
    score_text_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    score_text_.setPosition(WINDOW_WIDTH - 150, 40);

    window.draw(score_text_);

    score_text_.setOrigin(0, 0);
    score_text_.setScale(1.0f, 1.0f);
}

void HUDRenderer::render_timer(sf::RenderWindow& window) {
    window.draw(timer_text_);
}

void HUDRenderer::render_health_bar(sf::RenderWindow& window,
                                    const std::map<uint32_t, Entity>& entities,
                                    uint32_t my_network_id,
                                    bool show_level_intro) {
    if (show_level_intro) {
        return;
    }

    for (const auto& [id, entity] : entities) {
        if (entity.type == 0x01 && id == my_network_id) {
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

            window.draw(health_bar_bg_);
            window.draw(health_bar_fill_);
            window.draw(health_text_);
            break;
        }
    }
}

void HUDRenderer::render_level_hud(sf::RenderWindow& window, bool show_level_intro) {
    if (show_level_intro) {
        return;
    }

    bool is_boss_wave = (current_level_ == 5 || current_level_ == 10 || current_level_ == 15);

    if (is_boss_wave) {
        level_text_.setFillColor(sf::Color(255, 50, 50));
        level_text_.setString("BOSS WAVE " + std::to_string(current_level_));
    } else {
        level_text_.setFillColor(sf::Color::Yellow);
        level_text_.setString("Level " + std::to_string(current_level_));
    }
    window.draw(level_text_);

    if (is_boss_wave) {
        uint16_t boss_killed = (enemies_killed_ >= 1) ? 1 : 0;
        progress_text_.setFillColor(sf::Color(255, 100, 100));
        progress_text_.setString("Boss: " + std::to_string(boss_killed) + " / 1");
    } else {
        progress_text_.setFillColor(sf::Color::White);
        progress_text_.setString("Enemies: " + std::to_string(enemies_killed_) + " / " +
                                 std::to_string(enemies_needed_));
    }
    window.draw(progress_text_);

    window.draw(progress_bar_bg_);

    float progress = 0.0f;
    if (is_boss_wave) {
        progress = (enemies_killed_ >= 1) ? 1.0f : 0.0f;
        progress_bar_fill_.setFillColor(sf::Color(255, 50, 50));
    } else {
        if (enemies_needed_ > 0) {
            progress = static_cast<float>(enemies_killed_) / static_cast<float>(enemies_needed_);
        }
        progress_bar_fill_.setFillColor(sf::Color(0, 200, 0));
    }
    progress_bar_fill_.setSize(sf::Vector2f(296.0f * progress, 21.0f));
    window.draw(progress_bar_fill_);
}

void HUDRenderer::render_combo_bar(sf::RenderWindow& window) {
    int combo_mult = managers::EffectsManager::instance().get_combo_multiplier();
    float combo_progress = managers::EffectsManager::instance().get_combo_progress();
    float combo_timer = managers::EffectsManager::instance().get_combo_timer();

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
        default:
            combo_color = sf::Color(255, 50, 50);
            break;
    }

    combo_text_.setFillColor(combo_color);
    combo_bar_fill_.setFillColor(combo_color);

    window.draw(combo_bar_bg_);

    combo_bar_fill_.setSize(sf::Vector2f(142.0f * combo_progress, 16));
    window.draw(combo_bar_fill_);

    combo_timer_bar_.setSize(sf::Vector2f(150.0f * combo_timer, 4));
    window.draw(combo_timer_bar_);

    window.draw(combo_text_);
}

void HUDRenderer::render_boss_health_bar(sf::RenderWindow& window, const std::map<uint32_t, Entity>& entities) {
    bool is_boss_wave = (current_level_ == 5 || current_level_ == 10 || current_level_ == 15);
    if (!is_boss_wave) {
        return;
    }
    
    int boss_current_hp = 0;
    int boss_max_hp = 0;
    bool boss_found = false;
    std::string boss_name = "BOSS";
    
    for (const auto& [id, entity] : entities) {
        if (entity.type == 0x08 && current_level_ == 5) {
            boss_current_hp = entity.health;
            boss_max_hp = entity.max_health;
            boss_found = true;
            boss_name = "DESTROYER";
            break;
        }
        if (current_level_ == 10 && entity.type == 0x11) {
            boss_current_hp = entity.health;
            boss_max_hp = entity.max_health;
            boss_found = true;
            boss_name = "SERPENT GUARDIAN";
            break;
        }
    }
    
    if (!boss_found || boss_max_hp <= 0) {
        return;
    }
    
    boss_name_text_.setString(boss_name);
    sf::FloatRect name_bounds = boss_name_text_.getLocalBounds();
    boss_name_text_.setOrigin(name_bounds.width / 2.0f, name_bounds.height / 2.0f);
    boss_name_text_.setPosition(WINDOW_WIDTH / 2.0f, 70.0f);
    window.draw(boss_name_text_);
    
    window.draw(boss_health_bar_border_);
    window.draw(boss_health_bar_bg_);
    
    float health_pct = static_cast<float>(boss_current_hp) / static_cast<float>(boss_max_hp);
    health_pct = std::max(0.0f, std::min(1.0f, health_pct));
    
    sf::Color bar_color;
    if (health_pct > 0.6f) {
        bar_color = sf::Color(200, 50, 50);
    } else if (health_pct > 0.3f) {
        bar_color = sf::Color(255, 100, 50);
    } else {
        bar_color = sf::Color(255, 50, 50);
    }
    
    const float boss_bar_width = 800.0f;
    boss_health_bar_fill_.setSize(sf::Vector2f(boss_bar_width * health_pct, boss_health_bar_fill_.getSize().y));
    boss_health_bar_fill_.setFillColor(bar_color);
    window.draw(boss_health_bar_fill_);
}

}  // namespace rendering
