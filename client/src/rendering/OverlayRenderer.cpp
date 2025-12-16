#include "rendering/OverlayRenderer.hpp"

#include "managers/TextureManager.hpp"

namespace rendering {

OverlayRenderer::OverlayRenderer() {}

void OverlayRenderer::init(const sf::Font& font) {
    auto& texture_mgr = managers::TextureManager::instance();

    level_intro_title_.setFont(font);
    level_intro_title_.setCharacterSize(80);
    level_intro_title_.setFillColor(sf::Color::Yellow);
    level_intro_title_.setStyle(sf::Text::Bold);

    level_intro_subtitle_.setFont(font);
    level_intro_subtitle_.setCharacterSize(40);
    level_intro_subtitle_.setFillColor(sf::Color::White);

    level_intro_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    level_intro_overlay_.setFillColor(sf::Color(0, 0, 0, 150));

    powerup_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    powerup_overlay_.setFillColor(sf::Color(0, 0, 0, 180));

    powerup_title_.setFont(font);
    powerup_title_.setCharacterSize(60);
    powerup_title_.setFillColor(sf::Color::Yellow);
    powerup_title_.setStyle(sf::Text::Bold);
    powerup_title_.setString("CHOOSE YOUR POWER-UP");

    auto& bonus_texture = texture_mgr.load("assets/bonus.png");
    const int card_width = 459;
    const int card_height = 759;

    powerup_card1_sprite_.setTexture(bonus_texture);
    powerup_card1_sprite_.setTextureRect(sf::IntRect(card_width * 2, 0, card_width, card_height));
    powerup_card1_sprite_.setScale(0.6f, 0.6f);
    powerup_card1_sprite_.setPosition(560.0f, 300.0f);

    powerup_card2_sprite_.setTexture(bonus_texture);
    powerup_card2_sprite_.setTextureRect(sf::IntRect(card_width * 1, 0, card_width, card_height));
    powerup_card2_sprite_.setScale(0.6f, 0.6f);
    powerup_card2_sprite_.setPosition(1180.0f, 300.0f);

    powerup_number1_text_.setFont(font);
    powerup_number1_text_.setCharacterSize(50);
    powerup_number1_text_.setFillColor(sf::Color::Yellow);
    powerup_number1_text_.setStyle(sf::Text::Bold);
    powerup_number1_text_.setString("1");
    powerup_number1_text_.setPosition(670.0f, 720.0f);

    powerup_number2_text_.setFont(font);
    powerup_number2_text_.setCharacterSize(50);
    powerup_number2_text_.setFillColor(sf::Color::Yellow);
    powerup_number2_text_.setStyle(sf::Text::Bold);
    powerup_number2_text_.setString("2");
    powerup_number2_text_.setPosition(1305.0f, 720.0f);

    powerup_instruction_.setFont(font);
    powerup_instruction_.setCharacterSize(25);
    powerup_instruction_.setFillColor(sf::Color::Cyan);
    powerup_instruction_.setString("Press 1 or 2 to choose (or click on a card)");
    powerup_instruction_.setPosition(660.0f, 850.0f);

    powerup_hint_text_.setFont(font);
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

    cannon_hint_text_.setFont(font);
    cannon_hint_text_.setCharacterSize(28);
    cannon_hint_text_.setFillColor(sf::Color(255, 255, 0, 255));
    cannon_hint_text_.setStyle(sf::Text::Bold);
    cannon_hint_text_.setOutlineColor(sf::Color::Black);
    cannon_hint_text_.setOutlineThickness(3.0f);
    cannon_hint_text_.setPosition(20, 880);

    cannon_hint_bg_.setSize(sf::Vector2f(600.0f, 50.0f));
    cannon_hint_bg_.setPosition(10, 870);
    cannon_hint_bg_.setFillColor(sf::Color(0, 0, 0, 180));
    cannon_hint_bg_.setOutlineColor(sf::Color::Yellow);
    cannon_hint_bg_.setOutlineThickness(2.0f);

    shield_frames_ = {
        {0, 0, 27, 27}, {27, 0, 34, 34}, {61, 0, 42, 42}, {103, 0, 51, 51}, {154, 0, 55, 55}};
    shield_visual_.setTextureRect(shield_frames_[0]);
    shield_visual_.setScale(2.0f, 2.0f);

    game_over_overlay_.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    game_over_overlay_.setFillColor(sf::Color(0, 0, 0, 200));

    texture_mgr.load("assets/gameover2.png");
    if (texture_mgr.has("assets/gameover2.png")) {
        game_over_sprite_.setTexture(*texture_mgr.get("assets/gameover2.png"));
        auto bounds = game_over_sprite_.getLocalBounds();
        game_over_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        game_over_sprite_.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    }
}

void OverlayRenderer::render_level_intro(sf::RenderWindow& window, bool show, uint8_t level,
                                         uint16_t enemies_needed) {
    if (!show) {
        return;
    }

    window.draw(level_intro_overlay_);

    level_intro_title_.setString("LEVEL " + std::to_string(level));
    sf::FloatRect title_bounds = level_intro_title_.getLocalBounds();
    level_intro_title_.setPosition(WINDOW_WIDTH / 2 - title_bounds.width / 2,
                                   WINDOW_HEIGHT / 2 - 80.0f);
    window.draw(level_intro_title_);

    level_intro_subtitle_.setString("KILL " + std::to_string(enemies_needed) + " ENEMIES");
    sf::FloatRect sub_bounds = level_intro_subtitle_.getLocalBounds();
    level_intro_subtitle_.setPosition(WINDOW_WIDTH / 2 - sub_bounds.width / 2,
                                      WINDOW_HEIGHT / 2 + 20.0f);
    window.draw(level_intro_subtitle_);
}

void OverlayRenderer::render_powerup_selection(sf::RenderWindow& window, bool show) {
    if (!show) {
        return;
    }

    window.draw(powerup_overlay_);

    sf::FloatRect title_bounds = powerup_title_.getLocalBounds();
    powerup_title_.setPosition(WINDOW_WIDTH / 2 - title_bounds.width / 2, 200.0f);
    window.draw(powerup_title_);

    window.draw(powerup_card1_sprite_);
    window.draw(powerup_card2_sprite_);

    window.draw(powerup_number1_text_);
    window.draw(powerup_number2_text_);

    sf::FloatRect inst_bounds = powerup_instruction_.getLocalBounds();
    powerup_instruction_.setPosition(WINDOW_WIDTH / 2 - inst_bounds.width / 2, 850.0f);
    window.draw(powerup_instruction_);
}

void OverlayRenderer::render_powerup_active(
    sf::RenderWindow& window,
    const std::map<std::pair<uint32_t, uint8_t>, float>& player_powerups,
    const std::map<uint32_t, Entity>& entities,
    const std::map<uint32_t, int>& player_shield_frame,
    uint32_t my_network_id) {
    
    float cannon_time = 0.0f;
    float shield_time = 0.0f;
    
    auto cannon_it = player_powerups.find({my_network_id, 1});
    if (cannon_it != player_powerups.end()) {
        cannon_time = cannon_it->second;
    }
    
    auto shield_it = player_powerups.find({my_network_id, 2});
    if (shield_it != player_powerups.end()) {
        shield_time = shield_it->second;
    }
    
    if (cannon_time > 0.0f) {
        int seconds = static_cast<int>(cannon_time);
        std::string hint_text = "Power Cannon actif: " + std::to_string(seconds) + "s";
        cannon_hint_text_.setString(hint_text);
        cannon_hint_text_.setFillColor(sf::Color::Yellow);
        cannon_hint_bg_.setOutlineColor(sf::Color::Yellow);
        
        window.draw(cannon_hint_bg_);
        window.draw(cannon_hint_text_);
    }
    
    if (shield_time > 0.0f) {
        int seconds = static_cast<int>(shield_time);
        std::string hint_text = "Protection active: " + std::to_string(seconds) + "s";
        powerup_hint_text_.setString(hint_text);
        powerup_hint_text_.setFillColor(sf::Color::Cyan);
        powerup_hint_bg_.setOutlineColor(sf::Color::Cyan);
        
        window.draw(powerup_hint_bg_);
        window.draw(powerup_hint_text_);
    }

    auto& texture_mgr = managers::TextureManager::instance();

    for (const auto& [key, time] : player_powerups) {
        uint32_t player_id = key.first;
        uint8_t type = key.second;

        if (type == 2 && time > 0.0f) {
            auto it = entities.find(player_id);
            if (it != entities.end() && it->second.type == 0x01) {
                auto frame_it = player_shield_frame.find(player_id);
                if (frame_it != player_shield_frame.end()) {
                    int frame_index = frame_it->second;
                    if (frame_index >= 0 &&
                        static_cast<size_t>(frame_index) < shield_frames_.size()) {
                        if (texture_mgr.has("assets/shield.png")) {
                            shield_visual_.setTexture(*texture_mgr.get("assets/shield.png"));
                        }
                        shield_visual_.setTextureRect(
                            shield_frames_[static_cast<size_t>(frame_index)]);
                        sf::FloatRect bounds = shield_visual_.getLocalBounds();
                        shield_visual_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                        shield_visual_.setPosition(it->second.x, it->second.y);
                        window.draw(shield_visual_);
                    }
                }
            }
        }
    }
}

void OverlayRenderer::render_game_over(sf::RenderWindow& window, bool show) {
    if (!show) {
        return;
    }
    window.draw(game_over_overlay_);
    window.draw(game_over_sprite_);
}

void OverlayRenderer::update_shield_animation(
    float dt, const std::map<std::pair<uint32_t, uint8_t>, float>& player_powerups,
    std::map<uint32_t, int>& player_shield_frame,
    std::map<uint32_t, float>& player_shield_anim_timer) {
    for (const auto& [key, time] : player_powerups) {
        uint32_t player_id = key.first;
        uint8_t type = key.second;
        if (type == 2 && time > 0.0f) {
            player_shield_anim_timer[player_id] += dt;

            constexpr float frame_duration = 0.1f;
            if (player_shield_anim_timer[player_id] >= frame_duration) {
                player_shield_anim_timer[player_id] -= frame_duration;
                player_shield_frame[player_id] =
                    (player_shield_frame[player_id] + 1) % static_cast<int>(shield_frames_.size());
            }
        }
    }
}

}  // namespace rendering
