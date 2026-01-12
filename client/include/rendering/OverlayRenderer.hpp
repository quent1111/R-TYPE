#pragma once

#include "game/Entity.hpp"
#include "managers/TextureManager.hpp"
#include "network/Messages.hpp"
#include "../../game-lib/include/powerup/PowerupRegistry.hpp"

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

    void update_powerup_cards(const std::vector<NetworkToGame::Message::PowerUpCard>& cards);

    void render_powerup_active(sf::RenderWindow& window,
                               const std::map<std::pair<uint32_t, uint8_t>, float>& player_powerups,
                               const std::map<uint32_t, Entity>& entities,
                               const std::map<uint32_t, int>& player_shield_frame,
                               uint32_t my_network_id);
    
    void render_activable_slots(sf::RenderWindow& window, 
                                const std::vector<std::pair<std::optional<powerup::PowerupId>, uint8_t>>& slots,
                                const std::vector<float>& slot_timers,
                                const std::vector<float>& slot_cooldowns,
                                const std::vector<bool>& slot_active);

    void render_game_over(sf::RenderWindow& window, bool show);

    void
    update_shield_animation(float dt,
                            const std::map<std::pair<uint32_t, uint8_t>, float>& player_powerups,
                            std::map<uint32_t, int>& player_shield_frame,
                            std::map<uint32_t, float>& player_shield_anim_timer);

    OverlayRenderer(const OverlayRenderer&) = delete;
    OverlayRenderer& operator=(const OverlayRenderer&) = delete;

private:
    sf::Text level_intro_title_;
    sf::Text level_intro_subtitle_;
    sf::RectangleShape level_intro_overlay_;

    sf::Text boss_warning_text_;
    sf::Text boss_wave_text_;
    sf::RectangleShape boss_warning_stripe1_;
    sf::RectangleShape boss_warning_stripe2_;

    sf::RectangleShape powerup_overlay_;
    sf::Text powerup_title_;
    sf::Sprite powerup_card1_sprite_;
    sf::Sprite powerup_card2_sprite_;
    sf::Sprite powerup_card3_sprite_;
    sf::Text powerup_number1_text_;
    sf::Text powerup_number2_text_;
    sf::Text powerup_number3_text_;
    sf::Text powerup_level1_text_;
    sf::Text powerup_level2_text_;
    sf::Text powerup_level3_text_;
    sf::Text powerup_desc1_text_;
    sf::Text powerup_desc2_text_;
    sf::Text powerup_desc3_text_;
    sf::Text powerup_instruction_;

    sf::Texture powerup_card1_texture_;
    sf::Texture powerup_card2_texture_;
    sf::Texture powerup_card3_texture_;

    sf::Text friend_hint_text_;
    sf::RectangleShape friend_hint_bg_;

    sf::Sprite shield_visual_;
    std::vector<sf::IntRect> shield_frames_;

    sf::Sprite game_over_sprite_;
    sf::RectangleShape game_over_overlay_;
    
    sf::Sprite activable_slot1_sprite_;
    sf::Sprite activable_slot2_sprite_;
    sf::Texture activable_slot1_texture_;
    sf::Texture activable_slot2_texture_;
    sf::RectangleShape activable_slot1_frame_;
    sf::RectangleShape activable_slot2_frame_;
    sf::RectangleShape activable_slot1_bar_;
    sf::RectangleShape activable_slot2_bar_;
    sf::Text activable_slot1_key_;
    sf::Text activable_slot2_key_;
};

}  // namespace rendering
