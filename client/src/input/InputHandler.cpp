#include "input/InputHandler.hpp"

#include <iostream>

namespace input {

InputHandler::InputHandler()
    : has_focus_(true),
      show_powerup_selection_(false),
      powerup_type_(0),
      powerup_time_remaining_(0.0f),
      was_shooting_(false),
      x_pressed_last_frame_(false) {}

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
            }
        }
    }
}

void InputHandler::handle_input() {
    if (!has_focus_) {
        return;
    }

    if (show_powerup_selection_) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
            if (powerup_choice_callback_) {
                powerup_choice_callback_(1);
            }
            show_powerup_selection_ = false;
            powerup_type_ = 1;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
            if (powerup_choice_callback_) {
                powerup_choice_callback_(2);
            }
            show_powerup_selection_ = false;
            powerup_type_ = 2;
        }
        return;
    }

    bool x_pressed_now = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
    if (x_pressed_now && !x_pressed_last_frame_) {
        if (powerup_type_ != 0 && powerup_time_remaining_ <= 0.0f) {
            if (powerup_activate_callback_) {
                powerup_activate_callback_();
            }
        }
    }
    x_pressed_last_frame_ = x_pressed_now;

    uint8_t input_mask = 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
        input_mask |= KEY_Z;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        input_mask |= KEY_Q;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        input_mask |= KEY_S;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        input_mask |= KEY_D;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        input_mask |= KEY_SPACE;
        if (!was_shooting_) {
            if (shoot_sound_callback_) {
                shoot_sound_callback_();
            }
            was_shooting_ = true;
        }
    } else {
        was_shooting_ = false;
    }

    if (input_mask != 0) {
        if (input_callback_) {
            input_callback_(input_mask);
        }
    }
}

}  // namespace input
