#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class LoginScreen {
public:
    LoginScreen();

    void handle_event(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderWindow& window);

    bool is_login_requested() const { return _login_requested; }
    std::string get_password() const { return _password; }
    void reset_login_request() { _login_requested = false; }

    void set_error_message(const std::string& message);

private:
    sf::Font _font;
    sf::Text _title;
    sf::Text _password_label;
    sf::Text _password_text;
    sf::Text _error_text;
    sf::RectangleShape _password_box;
    sf::RectangleShape _login_button;
    sf::Text _login_button_text;

    std::string _password;
    bool _login_requested;
    float _cursor_timer;
    bool _cursor_visible;
};
