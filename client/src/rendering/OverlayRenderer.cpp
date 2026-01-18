#include "rendering/OverlayRenderer.hpp"

#include "managers/TextureManager.hpp"

#include <iostream>
#include <optional>

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

    boss_warning_text_.setFont(font);
    boss_warning_text_.setCharacterSize(120);
    boss_warning_text_.setFillColor(sf::Color(255, 50, 50));
    boss_warning_text_.setStyle(sf::Text::Bold);
    boss_warning_text_.setOutlineColor(sf::Color::Black);
    boss_warning_text_.setOutlineThickness(4.0f);
    boss_warning_text_.setString("⚠ WARNING ⚠");

    boss_wave_text_.setFont(font);
    boss_wave_text_.setCharacterSize(90);
    boss_wave_text_.setFillColor(sf::Color(255, 200, 50));
    boss_wave_text_.setStyle(sf::Text::Bold);
    boss_wave_text_.setOutlineColor(sf::Color(100, 0, 0));
    boss_wave_text_.setOutlineThickness(3.0f);
    boss_wave_text_.setString("BOSS WAVE");

    boss_warning_stripe1_.setSize(sf::Vector2f(WINDOW_WIDTH, 80.0f));
    boss_warning_stripe1_.setFillColor(sf::Color(180, 0, 0, 200));

    boss_warning_stripe2_.setSize(sf::Vector2f(WINDOW_WIDTH, 80.0f));
    boss_warning_stripe2_.setFillColor(sf::Color(180, 0, 0, 200));

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
    powerup_card1_sprite_.setScale(1.2f, 1.2f);
    powerup_card1_sprite_.setPosition(560.0f, 500.0f);

    powerup_card2_sprite_.setTexture(bonus_texture);
    powerup_card2_sprite_.setTextureRect(sf::IntRect(card_width * 1, 0, card_width, card_height));
    powerup_card2_sprite_.setScale(1.2f, 1.2f);
    powerup_card2_sprite_.setPosition(960.0f, 500.0f);

    powerup_card3_sprite_.setTexture(bonus_texture);
    powerup_card3_sprite_.setTextureRect(sf::IntRect(card_width * 0, 0, card_width, card_height));
    powerup_card3_sprite_.setScale(1.2f, 1.2f);
    powerup_card3_sprite_.setPosition(1360.0f, 500.0f);

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
    powerup_number2_text_.setPosition(1080.0f, 720.0f);

    powerup_number3_text_.setFont(font);
    powerup_number3_text_.setCharacterSize(50);
    powerup_number3_text_.setFillColor(sf::Color::Yellow);
    powerup_number3_text_.setStyle(sf::Text::Bold);
    powerup_number3_text_.setString("3");
    powerup_number3_text_.setPosition(1480.0f, 720.0f);

    powerup_level1_text_.setFont(font);
    powerup_level1_text_.setCharacterSize(32);
    powerup_level1_text_.setFillColor(sf::Color(255, 215, 0));
    powerup_level1_text_.setStyle(sf::Text::Bold);
    powerup_level1_text_.setOutlineColor(sf::Color::Black);
    powerup_level1_text_.setOutlineThickness(2.0f);

    powerup_level2_text_.setFont(font);
    powerup_level2_text_.setCharacterSize(32);
    powerup_level2_text_.setFillColor(sf::Color(255, 215, 0));
    powerup_level2_text_.setStyle(sf::Text::Bold);
    powerup_level2_text_.setOutlineColor(sf::Color::Black);
    powerup_level2_text_.setOutlineThickness(2.0f);

    powerup_level3_text_.setFont(font);
    powerup_level3_text_.setCharacterSize(32);
    powerup_level3_text_.setFillColor(sf::Color(255, 215, 0));
    powerup_level3_text_.setStyle(sf::Text::Bold);
    powerup_level3_text_.setOutlineColor(sf::Color::Black);
    powerup_level3_text_.setOutlineThickness(2.0f);

    powerup_desc1_text_.setFont(font);
    powerup_desc1_text_.setCharacterSize(18);
    powerup_desc1_text_.setFillColor(sf::Color(220, 220, 220));
    powerup_desc1_text_.setStyle(sf::Text::Regular);
    powerup_desc1_text_.setOutlineColor(sf::Color::Black);
    powerup_desc1_text_.setOutlineThickness(1.0f);

    powerup_desc2_text_.setFont(font);
    powerup_desc2_text_.setCharacterSize(18);
    powerup_desc2_text_.setFillColor(sf::Color(220, 220, 220));
    powerup_desc2_text_.setStyle(sf::Text::Regular);
    powerup_desc2_text_.setOutlineColor(sf::Color::Black);
    powerup_desc2_text_.setOutlineThickness(1.0f);

    powerup_desc3_text_.setFont(font);
    powerup_desc3_text_.setCharacterSize(18);
    powerup_desc3_text_.setFillColor(sf::Color(220, 220, 220));
    powerup_desc3_text_.setStyle(sf::Text::Regular);
    powerup_desc3_text_.setOutlineColor(sf::Color::Black);
    powerup_desc3_text_.setOutlineThickness(1.0f);

    powerup_instruction_.setFont(font);
    powerup_instruction_.setCharacterSize(25);
    powerup_instruction_.setFillColor(sf::Color::Cyan);
    powerup_instruction_.setString("Press 1, 2 or 3 to choose (or click on a card)");
    powerup_instruction_.setPosition(620.0f, 850.0f);

    friend_hint_text_.setFont(font);
    friend_hint_text_.setCharacterSize(24);
    friend_hint_text_.setFillColor(sf::Color::White);
    friend_hint_text_.setStyle(sf::Text::Bold);

    friend_hint_bg_.setSize(sf::Vector2f(600.0f, 50.0f));
    friend_hint_bg_.setPosition(10, 810);
    friend_hint_bg_.setFillColor(sf::Color(50, 50, 50, 180));
    friend_hint_bg_.setOutlineColor(sf::Color(100, 200, 255));
    friend_hint_bg_.setOutlineThickness(2.0f);

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

    activable_slot1_frame_.setSize(sf::Vector2f(120.0f, 120.0f));
    activable_slot1_frame_.setFillColor(sf::Color(40, 40, 40, 200));
    activable_slot1_frame_.setOutlineColor(sf::Color(255, 255, 255, 150));
    activable_slot1_frame_.setOutlineThickness(3.0f);
    activable_slot1_frame_.setPosition(50.0f, 880.0f);

    activable_slot2_frame_.setSize(sf::Vector2f(120.0f, 120.0f));
    activable_slot2_frame_.setFillColor(sf::Color(40, 40, 40, 200));
    activable_slot2_frame_.setOutlineColor(sf::Color(255, 255, 255, 150));
    activable_slot2_frame_.setOutlineThickness(3.0f);
    activable_slot2_frame_.setPosition(190.0f, 880.0f);

    activable_slot1_bar_.setSize(sf::Vector2f(120.0f, 10.0f));
    activable_slot1_bar_.setFillColor(sf::Color(0, 255, 0, 200));
    activable_slot1_bar_.setPosition(50.0f, 1005.0f);

    activable_slot2_bar_.setSize(sf::Vector2f(120.0f, 10.0f));
    activable_slot2_bar_.setFillColor(sf::Color(0, 255, 0, 200));
    activable_slot2_bar_.setPosition(190.0f, 1005.0f);

    activable_slot1_key_.setFont(font);
    activable_slot1_key_.setCharacterSize(24);
    activable_slot1_key_.setFillColor(sf::Color::White);
    activable_slot1_key_.setString("Skill 1");
    activable_slot1_key_.setPosition(60.0f, 1020.0f);

    activable_slot2_key_.setFont(font);
    activable_slot2_key_.setCharacterSize(24);
    activable_slot2_key_.setFillColor(sf::Color::White);
    activable_slot2_key_.setString("Skill 2");
    activable_slot2_key_.setPosition(200.0f, 1020.0f);
}

void OverlayRenderer::render_level_intro(sf::RenderWindow& window, bool show, uint8_t level,
                                         uint16_t enemies_needed, bool is_custom_level) {
    if (!show) {
        return;
    }

    bool is_boss_wave = !is_custom_level && (level == 5 || level == 10 || level == 15);

    if (is_boss_wave) {
        level_intro_overlay_.setFillColor(sf::Color(40, 0, 0, 200));
        window.draw(level_intro_overlay_);

        boss_warning_stripe1_.setPosition(0, WINDOW_HEIGHT / 2.0f - 200.0f);
        window.draw(boss_warning_stripe1_);

        boss_warning_stripe2_.setPosition(0, WINDOW_HEIGHT / 2.0f + 120.0f);
        window.draw(boss_warning_stripe2_);

        boss_warning_text_.setString("!! WARNING !!");
        sf::FloatRect warning_bounds = boss_warning_text_.getLocalBounds();
        boss_warning_text_.setOrigin(warning_bounds.width / 2.0f, warning_bounds.height / 2.0f);
        boss_warning_text_.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 100.0f);
        window.draw(boss_warning_text_);

        sf::FloatRect wave_bounds = boss_wave_text_.getLocalBounds();
        boss_wave_text_.setOrigin(wave_bounds.width / 2.0f, wave_bounds.height / 2.0f);
        boss_wave_text_.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 50.0f);
        window.draw(boss_wave_text_);
    } else {
        level_intro_overlay_.setFillColor(sf::Color(0, 0, 0, 150));
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
}

void OverlayRenderer::render_powerup_selection(sf::RenderWindow& window, bool show) {
    if (!show) {
        return;
    }

    window.draw(powerup_overlay_);

    sf::FloatRect title_bounds = powerup_title_.getLocalBounds();
    powerup_title_.setPosition(WINDOW_WIDTH / 2 - title_bounds.width / 2, 150.0f);
    window.draw(powerup_title_);

    window.draw(powerup_card1_sprite_);
    window.draw(powerup_card2_sprite_);
    window.draw(powerup_card3_sprite_);

    window.draw(powerup_level1_text_);
    window.draw(powerup_level2_text_);
    window.draw(powerup_level3_text_);

    window.draw(powerup_desc1_text_);
    window.draw(powerup_desc2_text_);
    window.draw(powerup_desc3_text_);

    sf::FloatRect inst_bounds = powerup_instruction_.getLocalBounds();
    powerup_instruction_.setPosition(WINDOW_WIDTH / 2 - inst_bounds.width / 2, 850.0f);
    window.draw(powerup_instruction_);
}

void OverlayRenderer::render_powerup_active(
    sf::RenderWindow& window, const std::map<std::pair<uint32_t, uint8_t>, float>& player_powerups,
    const std::map<uint32_t, Entity>& entities, const std::map<uint32_t, int>& player_shield_frame,
    uint32_t) {
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

void OverlayRenderer::update_powerup_cards(
    const std::vector<NetworkToGame::Message::PowerUpCard>& cards) {
    auto& texture_mgr = managers::TextureManager::instance();
    const auto& registry = powerup::PowerupRegistry::instance();

    std::cout << "[OverlayRenderer] Updating " << cards.size() << " power-up cards" << std::endl;

    auto to_roman = [](uint8_t level) -> std::string {
        switch (level) {
            case 1:
                return "I";
            case 2:
                return "II";
            case 3:
                return "III";
            default:
                return "?";
        }
    };

    auto get_description = [](powerup::PowerupId id, uint8_t level) -> std::string {
        switch (id) {
            case powerup::PowerupId::PowerCannon:
                switch (level) {
                    case 1:
                        return "50 DMG per shot";
                    case 2:
                        return "+25 DMG and + 2s";
                    case 3:
                        return "+25 DMG and + 2s";
                    default:
                        return "";
                }
            case powerup::PowerupId::Shield:
                switch (level) {
                    case 1:
                        return "3s protection";
                    case 2:
                        return "5s protection";
                    case 3:
                        return "7s protection";
                    default:
                        return "";
                }
            case powerup::PowerupId::LittleFriend:
                switch (level) {
                    case 1:
                        return "+1 drone";
                    case 2:
                        return "+firerate +Damage";
                    case 3:
                        return "+1 drone +firerate +Damage";
                    default:
                        return "";
                }
            case powerup::PowerupId::Damage:
                switch (level) {
                    case 1:
                        return "+20% damage";
                    case 2:
                        return "+50% damage";
                    case 3:
                        return "+100% damage";
                    default:
                        return "";
                }
            case powerup::PowerupId::Speed:
                switch (level) {
                    case 1:
                        return "+20% speed";
                    case 2:
                        return "+40% speed";
                    case 3:
                        return "+70% speed";
                    default:
                        return "";
                }
            case powerup::PowerupId::Health:
                switch (level) {
                    case 1:
                        return "+30 max HP";
                    case 2:
                        return "+50 max HP";
                    case 3:
                        return "+80 max HP";
                    default:
                        return "";
                }
            case powerup::PowerupId::FireRate:
                switch (level) {
                    case 1:
                        return "+30% fire rate";
                    case 2:
                        return "+60% fire rate";
                    case 3:
                        return "+100% fire rate";
                    default:
                        return "";
                }
            case powerup::PowerupId::MultiShot:
                switch (level) {
                    case 1:
                        return "3 projectiles";
                    case 2:
                        return "4 projectiles";
                    case 3:
                        return "5 projectiles";
                    default:
                        return "";
                }
            case powerup::PowerupId::LaserBeam:
                switch (level) {
                    case 1:
                        return "3s beam, 100 DPS";
                    case 2:
                        return "5s beam, 150 DPS";
                    case 3:
                        return "8s beam, 200 DPS";
                    default:
                        return "";
                }
            case powerup::PowerupId::MissileDrone:
                switch (level) {
                    case 1:
                        return "1 drone, 5 missiles";
                    case 2:
                        return "2 drones, 7 missiles";
                    case 3:
                        return "3 drones, 10 missiles";
                    default:
                        return "";
                }
            default:
                return "";
        }
    };

    if (cards.size() > 0) {
        auto powerup_def = registry.get_powerup(static_cast<powerup::PowerupId>(cards[0].id));
        if (powerup_def) {
            const auto& def = *powerup_def;
            std::string asset_path = def.asset_path;

            std::cout << "[Card 1] ID=" << static_cast<int>(cards[0].id)
                      << " Level=" << static_cast<int>(cards[0].level) << " Name=" << def.name
                      << " Asset=" << asset_path << std::endl;

            texture_mgr.load(asset_path);
            if (texture_mgr.has(asset_path)) {
                powerup_card1_texture_ = *texture_mgr.get(asset_path);
                powerup_card1_sprite_.setTexture(powerup_card1_texture_, true);

                auto tex_size = powerup_card1_texture_.getSize();
                std::cout << "[Card 1] Texture size: " << tex_size.x << "x" << tex_size.y
                          << std::endl;

                auto bounds = powerup_card1_sprite_.getLocalBounds();
                powerup_card1_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                powerup_card1_sprite_.setPosition(560.0f, 500.0f);
                powerup_card1_sprite_.setScale(1.2f, 1.2f);
            }

            powerup_level1_text_.setString(to_roman(cards[0].level));
            sf::FloatRect level_bounds = powerup_level1_text_.getLocalBounds();
            powerup_level1_text_.setOrigin(level_bounds.width / 2.0f, level_bounds.height / 2.0f);
            powerup_level1_text_.setPosition(560.0f, 720.0f);

            std::string description =
                get_description(static_cast<powerup::PowerupId>(cards[0].id), cards[0].level);
            powerup_desc1_text_.setString(description);
            sf::FloatRect desc_bounds = powerup_desc1_text_.getLocalBounds();
            powerup_desc1_text_.setOrigin(desc_bounds.width / 2.0f, desc_bounds.height / 2.0f);
            powerup_desc1_text_.setPosition(560.0f, 755.0f);
        }
    }

    if (cards.size() > 1) {
        auto powerup_def = registry.get_powerup(static_cast<powerup::PowerupId>(cards[1].id));
        if (powerup_def) {
            const auto& def = *powerup_def;
            std::string asset_path = def.asset_path;

            std::cout << "[Card 2] ID=" << static_cast<int>(cards[1].id)
                      << " Level=" << static_cast<int>(cards[1].level) << " Name=" << def.name
                      << " Asset=" << asset_path << std::endl;

            texture_mgr.load(asset_path);
            if (texture_mgr.has(asset_path)) {
                powerup_card2_texture_ = *texture_mgr.get(asset_path);
                powerup_card2_sprite_.setTexture(powerup_card2_texture_, true);

                auto tex_size = powerup_card2_texture_.getSize();
                std::cout << "[Card 2] Texture size: " << tex_size.x << "x" << tex_size.y
                          << std::endl;

                auto bounds = powerup_card2_sprite_.getLocalBounds();
                powerup_card2_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                powerup_card2_sprite_.setPosition(960.0f, 500.0f);
                powerup_card2_sprite_.setScale(1.2f, 1.2f);
            }

            powerup_level2_text_.setString(to_roman(cards[1].level));
            sf::FloatRect level_bounds = powerup_level2_text_.getLocalBounds();
            powerup_level2_text_.setOrigin(level_bounds.width / 2.0f, level_bounds.height / 2.0f);
            powerup_level2_text_.setPosition(960.0f, 720.0f);

            std::string description =
                get_description(static_cast<powerup::PowerupId>(cards[1].id), cards[1].level);
            powerup_desc2_text_.setString(description);
            sf::FloatRect desc_bounds = powerup_desc2_text_.getLocalBounds();
            powerup_desc2_text_.setOrigin(desc_bounds.width / 2.0f, desc_bounds.height / 2.0f);
            powerup_desc2_text_.setPosition(960.0f, 755.0f);
        }
    }

    if (cards.size() > 2) {
        auto powerup_def = registry.get_powerup(static_cast<powerup::PowerupId>(cards[2].id));
        if (powerup_def) {
            const auto& def = *powerup_def;
            std::string asset_path = def.asset_path;

            std::cout << "[Card 3] ID=" << static_cast<int>(cards[2].id)
                      << " Level=" << static_cast<int>(cards[2].level) << " Name=" << def.name
                      << " Asset=" << asset_path << std::endl;

            texture_mgr.load(asset_path);
            if (texture_mgr.has(asset_path)) {
                powerup_card3_texture_ = *texture_mgr.get(asset_path);
                powerup_card3_sprite_.setTexture(powerup_card3_texture_, true);

                auto tex_size = powerup_card3_texture_.getSize();
                std::cout << "[Card 3] Texture size: " << tex_size.x << "x" << tex_size.y
                          << std::endl;

                auto bounds = powerup_card3_sprite_.getLocalBounds();
                powerup_card3_sprite_.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
                powerup_card3_sprite_.setPosition(1360.0f, 500.0f);
                powerup_card3_sprite_.setScale(1.2f, 1.2f);
            }

            powerup_level3_text_.setString(to_roman(cards[2].level));
            sf::FloatRect level_bounds = powerup_level3_text_.getLocalBounds();
            powerup_level3_text_.setOrigin(level_bounds.width / 2.0f, level_bounds.height / 2.0f);
            powerup_level3_text_.setPosition(1360.0f, 720.0f);

            std::string description =
                get_description(static_cast<powerup::PowerupId>(cards[2].id), cards[2].level);
            powerup_desc3_text_.setString(description);
            sf::FloatRect desc_bounds = powerup_desc3_text_.getLocalBounds();
            powerup_desc3_text_.setOrigin(desc_bounds.width / 2.0f, desc_bounds.height / 2.0f);
            powerup_desc3_text_.setPosition(1360.0f, 755.0f);
        }
    }
}

void OverlayRenderer::render_activable_slots(
    sf::RenderWindow& window,
    const std::vector<std::pair<std::optional<powerup::PowerupId>, uint8_t>>& slots,
    const std::vector<float>& slot_timers, const std::vector<float>& slot_cooldowns,
    const std::vector<bool>& slot_active) {
    auto& texture_mgr = managers::TextureManager::instance();

    for (std::size_t i = 0; i < 2 && i < slots.size(); ++i) {
        sf::RectangleShape& frame = (i == 0) ? activable_slot1_frame_ : activable_slot2_frame_;
        sf::RectangleShape& bar = (i == 0) ? activable_slot1_bar_ : activable_slot2_bar_;
        sf::Text& key_text = (i == 0) ? activable_slot1_key_ : activable_slot2_key_;

        window.draw(frame);

        if (slots[i].first.has_value()) {
            auto* def = powerup::PowerupRegistry::instance().get_powerup(slots[i].first.value());
            if (def) {
                sf::Sprite& slot_sprite =
                    (i == 0) ? activable_slot1_sprite_ : activable_slot2_sprite_;

                texture_mgr.load(def->asset_path);
                if (texture_mgr.has(def->asset_path)) {
                    slot_sprite.setTexture(*texture_mgr.get(def->asset_path));
                    auto tex_size = texture_mgr.get(def->asset_path)->getSize();
                    slot_sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(tex_size.x),
                                                           static_cast<int>(tex_size.y)));

                    float scale_x = 240.0f / static_cast<float>(tex_size.x);
                    float scale_y = 240.0f / static_cast<float>(tex_size.y);
                    slot_sprite.setScale(scale_x, scale_y);

                    sf::Vector2f pos = frame.getPosition();
                    float offset_x = (120.0f - 240.0f) / 2.0f;
                    float offset_y = (120.0f - 240.0f) / 2.0f;
                    slot_sprite.setPosition(pos.x + offset_x, pos.y + offset_y);

                    window.draw(slot_sprite);
                }

                float bar_width = 120.0f;
                float bar_height = 10.0f;
                float percentage = 0.0f;
                sf::Color bar_color;

                if (i < slot_active.size() && slot_active[i]) {
                    if (i < slot_timers.size()) {
                        auto* def_ptr = powerup::PowerupRegistry::instance().get_powerup(
                            slots[i].first.value());
                        if (def_ptr && slots[i].second > 0 &&
                            slots[i].second <= def_ptr->level_effects.size()) {
                            float max_duration =
                                def_ptr->level_effects[slots[i].second - 1].duration;
                            percentage =
                                (max_duration > 0.0f) ? (slot_timers[i] / max_duration) : 0.0f;
                        }
                    }
                    bar_color = sf::Color(0, 255, 0, 200);
                } else if (i < slot_cooldowns.size() && slot_cooldowns[i] > 0.0f) {
                    percentage = 1.0f - (slot_cooldowns[i] / 25.0f);
                    bar_color = sf::Color(255, 140, 0, 200);
                } else {
                    percentage = 1.0f;
                    bar_color = sf::Color(0, 255, 0, 200);
                }

                sf::Vector2f frame_pos = frame.getPosition();
                bar.setPosition(frame_pos.x, frame_pos.y + 120.0f + 5.0f);
                bar.setSize(sf::Vector2f(bar_width * percentage, bar_height));
                bar.setFillColor(bar_color);
                window.draw(bar);
            }
        }

        window.draw(key_text);
    }
}

}  // namespace rendering
