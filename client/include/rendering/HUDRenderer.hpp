#pragma once

#include "game/Entity.hpp"
#include "managers/EffectsManager.hpp"

#include <SFML/Graphics.hpp>

#include <map>

namespace rendering {

class HUDRenderer {
public:
    static constexpr unsigned int WINDOW_WIDTH = 1920;
    static constexpr unsigned int WINDOW_HEIGHT = 1080;

    HUDRenderer();
    ~HUDRenderer() = default;

    void init(const sf::Font& font);

    void update_score(uint32_t score);
    void update_timer(float game_time);
    void update_level(uint8_t level, uint16_t enemies_killed, uint16_t enemies_needed);

    void render_score(sf::RenderWindow& window);
    void render_timer(sf::RenderWindow& window);
    void render_health_bar(sf::RenderWindow& window, const std::map<uint32_t, Entity>& entities,
                           uint32_t my_network_id);
    void render_level_hud(sf::RenderWindow& window, bool show_level_intro, bool is_custom_level);
    void render_combo_bar(sf::RenderWindow& window);
    void render_boss_health_bar(sf::RenderWindow& window,
                                const std::map<uint32_t, Entity>& entities);

    HUDRenderer(const HUDRenderer&) = delete;
    HUDRenderer& operator=(const HUDRenderer&) = delete;

private:
    sf::Text score_text_;
    sf::Text timer_text_;

    sf::RectangleShape health_bar_bg_;
    sf::RectangleShape health_bar_fill_;
    sf::Text health_text_;

    sf::Text level_text_;
    sf::Text progress_text_;
    sf::RectangleShape progress_bar_bg_;
    sf::RectangleShape progress_bar_fill_;

    sf::Text combo_text_;
    sf::RectangleShape combo_bar_bg_;
    sf::RectangleShape combo_bar_fill_;
    sf::RectangleShape combo_timer_bar_;

    sf::RectangleShape boss_health_bar_bg_;
    sf::RectangleShape boss_health_bar_fill_;
    sf::RectangleShape boss_health_bar_border_;
    sf::Text boss_name_text_;

    uint8_t current_level_ = 1;
    uint16_t enemies_killed_ = 0;
    uint16_t enemies_needed_ = 20;
};

}  // namespace rendering
