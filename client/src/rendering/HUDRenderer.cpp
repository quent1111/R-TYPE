#include "rendering/HUDRenderer.hpp"

#include "managers/EffectsManager.hpp"

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
}

void HUDRenderer::update_score(uint32_t score) {
    score_text_.setString("SCORE: " + std::to_string(score));
}

void HUDRenderer::update_timer(float game_time) {
    int minutes = static_cast<int>(game_time) / 60;
    int seconds = static_cast<int>(game_time) % 60;
    int millis = static_cast<int>((game_time - static_cast<int>(game_time)) * 1000);
    char buffer[16];
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
                                     uint32_t my_network_id) {
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

    level_text_.setString("Level " + std::to_string(current_level_));
    window.draw(level_text_);

    progress_text_.setString("Enemies: " + std::to_string(enemies_killed_) + " / " +
                             std::to_string(enemies_needed_));
    window.draw(progress_text_);

    window.draw(progress_bar_bg_);

    float progress = 0.0f;
    if (enemies_needed_ > 0) {
        progress = static_cast<float>(enemies_killed_) / static_cast<float>(enemies_needed_);
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

}  // namespace rendering
