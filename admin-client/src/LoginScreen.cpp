#include "LoginScreen.hpp"
#include <iostream>

LoginScreen::LoginScreen()
    : _login_requested(false), _cursor_timer(0.f), _cursor_visible(true) {

    if (!_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[LoginScreen] Failed to load font" << std::endl;
    }

    _title.setFont(_font);
    _title.setString("R-TYPE SERVER ADMIN");
    _title.setCharacterSize(48);
    _title.setFillColor(sf::Color::White);
    _title.setPosition(400, 200);

    _password_label.setFont(_font);
    _password_label.setString("Admin Password:");
    _password_label.setCharacterSize(24);
    _password_label.setFillColor(sf::Color::White);
    _password_label.setPosition(400, 350);

    _password_box.setSize(sf::Vector2f(400, 50));
    _password_box.setPosition(400, 390);
    _password_box.setFillColor(sf::Color(50, 50, 50));
    _password_box.setOutlineColor(sf::Color::White);
    _password_box.setOutlineThickness(2);

    _password_text.setFont(_font);
    _password_text.setCharacterSize(24);
    _password_text.setFillColor(sf::Color::White);
    _password_text.setPosition(410, 400);

    _login_button.setSize(sf::Vector2f(200, 50));
    _login_button.setPosition(500, 480);
    _login_button.setFillColor(sf::Color(30, 150, 50));
    _login_button.setOutlineColor(sf::Color::White);
    _login_button.setOutlineThickness(2);

    _login_button_text.setFont(_font);
    _login_button_text.setString("LOGIN");
    _login_button_text.setCharacterSize(24);
    _login_button_text.setFillColor(sf::Color::White);
    _login_button_text.setPosition(560, 490);

    _error_text.setFont(_font);
    _error_text.setCharacterSize(20);
    _error_text.setFillColor(sf::Color::Red);
    _error_text.setPosition(400, 550);
}

void LoginScreen::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == '\b' && !_password.empty()) {
            _password.pop_back();
        } else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
            _login_requested = true;
        } else if (event.text.unicode < 128 && event.text.unicode != '\b') {
            _password += static_cast<char>(event.text.unicode);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mouse_pos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (_login_button.getGlobalBounds().contains(mouse_pos)) {
            _login_requested = true;
        }
    }
}

void LoginScreen::update(float dt) {
    _cursor_timer += dt;
    if (_cursor_timer >= 0.5f) {
        _cursor_visible = !_cursor_visible;
        _cursor_timer = 0.f;
    }

    std::string display_password(_password.length(), '*');
    if (_cursor_visible) {
        display_password += "|";
    }
    _password_text.setString(display_password);
}

void LoginScreen::render(sf::RenderWindow& window) {
    window.draw(_title);
    window.draw(_password_label);
    window.draw(_password_box);
    window.draw(_password_text);
    window.draw(_login_button);
    window.draw(_login_button_text);
    window.draw(_error_text);
}

void LoginScreen::set_error_message(const std::string& message) {
    _error_text.setString(message);
}
