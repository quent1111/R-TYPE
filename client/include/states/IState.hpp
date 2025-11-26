#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

namespace rtype {

class IState {
public:
    virtual ~IState() = default;
    virtual void on_enter() = 0;
    virtual void on_exit() = 0;
    virtual void handle_event(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual std::string get_next_state() const = 0;
    virtual bool is_overlay() const { return false; }
};

} // namespace rtype
