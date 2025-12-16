#pragma once

#include "game/Entity.hpp"
#include "managers/TextureManager.hpp"

#include <SFML/Graphics.hpp>

#include <map>
#include <vector>

namespace rendering {

class OverlayRenderer {
public:
    static constexpr unsigned int WINDOW_WIDTH = 1920;
    static constexpr unsigned int WINDOW_HEIGHT = 1080;

    OverlayRenderer();
    ~OverlayRenderer() = default;

    void init(const sf::Font& font);

    void render_level_intro(sf::RenderWindow& window, bool show, uint8_t level,
                            uint16_t enemies_needed);

    void render_powerup_selection(sf::RenderWindow& window, bool show);

    void render_powerup_active(sf::RenderWindow& window, uint8_t powerup_type, float time_remaining,
                               const std::map<uint32_t, std::pair<uint8_t, float>>& player_powerups,
                               const std::map<uint32_t, Entity>& entities,
                               const std::map<uint32_t, int>& player_shield_frame);

    void render_game_over(sf::RenderWindow& window, bool show);

    void
    update_shield_animation(float dt,
                            const std::map<uint32_t, std::pair<uint8_t, float>>& player_powerups,
                            std::map<uint32_t, int>& player_shield_frame,
                            std::map<uint32_t, float>& player_shield_anim_timer);

    OverlayRenderer(const OverlayRenderer&) = delete;
    OverlayRenderer& operator=(const OverlayRenderer&) = delete;

private:
    sf::Text level_intro_title_;
    sf::Text level_intro_subtitle_;
    sf::RectangleShape level_intro_overlay_;

    sf::RectangleShape powerup_overlay_;
    sf::Text powerup_title_;
    sf::Sprite powerup_card1_sprite_;
    sf::Sprite powerup_card2_sprite_;
    sf::Text powerup_number1_text_;
    sf::Text powerup_number2_text_;
    sf::Text powerup_instruction_;

    sf::Text powerup_hint_text_;
    sf::RectangleShape powerup_hint_bg_;

    sf::Sprite shield_visual_;
    std::vector<sf::IntRect> shield_frames_;

    sf::Sprite game_over_sprite_;
    sf::RectangleShape game_over_overlay_;
};

}  // namespace rendering
