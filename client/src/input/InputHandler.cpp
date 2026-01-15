#include "input/InputHandler.hpp"

#include "common/Settings.hpp"

#include <iostream>

namespace input {

InputHandler::InputHandler()
    : has_focus_(true),
      show_powerup_selection_(false),
      powerup_type_(0),
      powerup_time_remaining_(0.0f),
      was_shooting_(false),
      num1_was_pressed_(false),
      num2_was_pressed_(false),
      num3_was_pressed_(false) {}

void InputHandler::handle_event(const sf::Event& event, sf::RenderWindow& window) {
    if (!has_focus_) {
        return;
    }

    if (show_powerup_selection_ && event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pixel_pos(event.mouseButton.x, event.mouseButton.y);
            sf::Vector2f mouse_pos = window.mapPixelToCoords(pixel_pos);

            if (powerup_card1_bounds_.contains(mouse_pos)) {
                if (powerup_choice_callback_) {
                    powerup_choice_callback_(1);
                }
                show_powerup_selection_ = false;
                powerup_type_ = 1;
                std::cout << "[Game] Powerup 1 selected via click" << std::endl;
            } else if (powerup_card2_bounds_.contains(mouse_pos)) {
                if (powerup_choice_callback_) {
                    powerup_choice_callback_(2);
                }
                show_powerup_selection_ = false;
                powerup_type_ = 2;
                std::cout << "[Game] Powerup 2 selected via click" << std::endl;
            } else if (powerup_card3_bounds_.contains(mouse_pos)) {
                if (powerup_choice_callback_) {
                    powerup_choice_callback_(3);
                }
                show_powerup_selection_ = false;
                powerup_type_ = 3;
                std::cout << "[Game] Powerup 3 selected via click" << std::endl;
            }
        }
    }
}

void InputHandler::handle_input(float dt) {
    if (!has_focus_) {
        return;
    }

    if (show_powerup_selection_) {
        bool num1_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
        bool num2_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
        bool num3_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);

        if (num1_pressed && !num1_was_pressed_) {
            if (powerup_choice_callback_) {
                powerup_choice_callback_(1);
                std::cout << "[Game] Powerup 1 selected via keyboard" << std::endl;
            }
            show_powerup_selection_ = false;
            powerup_type_ = 1;
        } else if (num2_pressed && !num2_was_pressed_) {
            if (powerup_choice_callback_) {
                powerup_choice_callback_(2);
                std::cout << "[Game] Powerup 2 selected via keyboard" << std::endl;
            }
            show_powerup_selection_ = false;
            powerup_type_ = 2;
        } else if (num3_pressed && !num3_was_pressed_) {
            if (powerup_choice_callback_) {
                powerup_choice_callback_(3);
                std::cout << "[Game] Powerup 3 selected via keyboard" << std::endl;
            }
            show_powerup_selection_ = false;
            powerup_type_ = 3;
        }

        num1_was_pressed_ = num1_pressed;
        num2_was_pressed_ = num2_pressed;
        num3_was_pressed_ = num3_pressed;

        return;
    }

    num1_was_pressed_ = false;
    num2_was_pressed_ = false;
    num3_was_pressed_ = false;

    uint8_t input_mask = 0;

    // Bloquer les mouvements et le tir pendant la sélection de power-up
    if (!show_powerup_selection_) {
        auto& s = Settings::instance();
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_up))) {
            input_mask |= KEY_Z;
        }
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_left))) {
            input_mask |= KEY_Q;
        }
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_down))) {
            input_mask |= KEY_S;
        }
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_right))) {
            input_mask |= KEY_D;
        }

        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_powerup1))) {
            if (!was_activating_cannon_ && powerup_activate_callback_) {
                powerup_activate_callback_(0);
                was_activating_cannon_ = true;
            }
        } else {
            was_activating_cannon_ = false;
        }

        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_powerup2))) {
            if (!was_activating_shield_ && powerup_activate_callback_) {
                powerup_activate_callback_(1);
                was_activating_shield_ = true;
            }
        } else {
            was_activating_shield_ = false;
        }

        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_powerup3))) {
            if (!was_activating_friend_ && powerup_activate_callback_) {
                powerup_activate_callback_(3);
                was_activating_friend_ = true;
            }
        } else {
            was_activating_friend_ = false;
        }

        bool is_shooting = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(s.key_shoot)) || s.auto_fire_enabled;
        
        if (is_shooting) {
            input_mask |= KEY_SPACE;

            if (!was_shooting_) {
                if (shoot_sound_callback_) {
                    shoot_sound_callback_();
                }
                shoot_sound_timer_ = 0.0f;
            } else {
                shoot_sound_timer_ += dt;
                if (shoot_sound_timer_ >= shoot_sound_interval_) {
                    if (shoot_sound_callback_) {
                        shoot_sound_callback_();
                    }
                    shoot_sound_timer_ = 0.0f;
                }
            }

            was_shooting_ = true;
        } else {
            was_shooting_ = false;
            shoot_sound_timer_ = 0.0f;
        }
    } else {
        // Réinitialiser l'état de tir quand on est en sélection de power-up
        was_shooting_ = false;
        shoot_sound_timer_ = 0.0f;
    }

    if (input_callback_) {
        input_callback_(input_mask);
    }
}

}  // namespace input
