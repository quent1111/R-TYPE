#include "systems/input_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include <SFML/Window/Keyboard.hpp>

void inputSystem(registry& reg) {
    auto& velocities = reg.get_components<velocity>();
    auto& controllables = reg.get_components<controllable>();
    auto& player_tags = reg.get_components<player_tag>();

    for (std::size_t i = 0; i < velocities.size() && i < controllables.size() &&
                            i < player_tags.size();
         ++i) {
        auto& vel_opt = velocities[i];
        auto& ctrl_opt = controllables[i];
        auto& player_opt = player_tags[i];

        if (vel_opt && ctrl_opt && player_opt) {
            auto& vel = vel_opt.value();
            auto& ctrl = ctrl_opt.value();

            vel.vx = 0.0f;
            vel.vy = 0.0f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                vel.vx = -ctrl.speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                vel.vx = ctrl.speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
                vel.vy = -ctrl.speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                vel.vy = ctrl.speed;
            }
        }
    }
}
