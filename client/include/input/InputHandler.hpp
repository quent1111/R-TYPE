#pragma once

#include "input/InputKey.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>

#include <functional>

namespace input {

class InputHandler {
public:
    using InputCallback = std::function<void(uint8_t input_mask)>;
    using PowerupChoiceCallback = std::function<void(uint8_t choice)>;
    using PowerupActivateCallback = std::function<void(uint8_t powerup_type)>;
    using SoundCallback = std::function<void()>;

    InputHandler();
    ~InputHandler() = default;

    void set_input_callback(InputCallback callback) { input_callback_ = std::move(callback); }
    void set_powerup_choice_callback(PowerupChoiceCallback callback) {
        powerup_choice_callback_ = std::move(callback);
    }
    void set_powerup_activate_callback(PowerupActivateCallback callback) {
        powerup_activate_callback_ = std::move(callback);
    }
    void set_shoot_sound_callback(SoundCallback callback) {
        shoot_sound_callback_ = std::move(callback);
    }

    void set_focus(bool focus) { has_focus_ = focus; }
    void set_powerup_selection_active(bool active) { show_powerup_selection_ = active; }
    void set_powerup_state(uint8_t type, float time_remaining) {
        powerup_type_ = type;
        powerup_time_remaining_ = time_remaining;
    }

    void set_powerup_card_bounds(const sf::FloatRect& card1, const sf::FloatRect& card2, const sf::FloatRect& card3) {
        powerup_card1_bounds_ = card1;
        powerup_card2_bounds_ = card2;
        powerup_card3_bounds_ = card3;
    }

    void handle_event(const sf::Event& event, sf::RenderWindow& window);
    void handle_input(float dt = 0.0f);

    bool is_powerup_selection_active() const { return show_powerup_selection_; }
    uint8_t get_selected_powerup_type() const { return powerup_type_; }

    InputHandler(const InputHandler&) = delete;
    InputHandler& operator=(const InputHandler&) = delete;

private:
    InputCallback input_callback_;
    PowerupChoiceCallback powerup_choice_callback_;
    PowerupActivateCallback powerup_activate_callback_;
    SoundCallback shoot_sound_callback_;

    bool has_focus_ = true;
    bool show_powerup_selection_ = false;
    uint8_t powerup_type_ = 0;
    float powerup_time_remaining_ = 0.0f;

    bool was_shooting_ = false;
    bool was_activating_cannon_ = false;
    bool was_activating_shield_ = false;
    bool was_activating_friend_ = false;

    bool num1_was_pressed_ = false;
    bool num2_was_pressed_ = false;
    bool num3_was_pressed_ = false;

    float shoot_sound_timer_ = 0.0f;
    const float shoot_sound_interval_ = 0.2f;

    sf::FloatRect powerup_card1_bounds_;
    sf::FloatRect powerup_card2_bounds_;
    sf::FloatRect powerup_card3_bounds_;
};

}  // namespace input
