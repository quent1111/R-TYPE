#include "AdminClient.hpp"
#include "AdminUI.hpp"
#include "LoginScreen.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    unsigned short port = 4242;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        }
    }

    std::cout << "=== R-TYPE SERVER ADMINISTRATION ===" << std::endl;
    std::cout << "Connecting to: " << host << ":" << port << std::endl;

    sf::RenderWindow window(sf::VideoMode(1280, 900), "R-Type Admin Panel");
    window.setFramerateLimit(60);

    auto client = std::make_shared<AdminClient>(host, port);

    if (!client->connect()) {
        std::cerr << "Failed to connect to server!" << std::endl;
        return 1;
    }

    enum class State {
        Login,
        Dashboard
    };

    State current_state = State::Login;
    LoginScreen login_screen;
    std::unique_ptr<AdminUI> admin_ui;

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }

            if (current_state == State::Login) {
                login_screen.handle_event(event);
            } else if (current_state == State::Dashboard && admin_ui) {
                admin_ui->handle_event(event);
            }
        }

        if (current_state == State::Login) {
            login_screen.update(dt);

            if (login_screen.is_login_requested()) {
                std::string password = login_screen.get_password();
                login_screen.reset_login_request();

                std::cout << "[Admin] Attempting authentication..." << std::endl;

                if (client->authenticate(password)) {
                    std::cout << "[Admin] Authentication successful!" << std::endl;
                    current_state = State::Dashboard;
                    admin_ui = std::make_unique<AdminUI>(window, client);
                } else {
                    std::cout << "[Admin] Authentication failed!" << std::endl;
                    login_screen.set_error_message("Authentication failed. Please try again.");
                }
            }
        } else if (current_state == State::Dashboard && admin_ui) {
            admin_ui->update(dt);
        }

        window.clear(sf::Color(20, 20, 25));

        if (current_state == State::Login) {
            login_screen.render(window);
        } else if (current_state == State::Dashboard && admin_ui) {
            admin_ui->render();
        }

        window.display();
    }

    client->disconnect();
    std::cout << "[Admin] Disconnected" << std::endl;

    return 0;
}
