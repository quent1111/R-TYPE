#pragma once

#include "states/IState.hpp"

#include <SFML/Graphics.hpp>

#include <functional>
#include <map>
#include <memory>
#include <stack>
#include <string>

namespace rtype {

class StateManager {
public:
    StateManager() = default;
    ~StateManager() = default;

    void register_state(const std::string& name, std::function<std::unique_ptr<IState>()> factory);
    void push_state(const std::string& name);
    void pop_state();
    void change_state(const std::string& name);
    void handle_event(const sf::Event& event);
    void update(float dt);
    void render(sf::RenderWindow& window);
    bool has_states() const { return !m_states.empty(); }
    void process_transitions();

private:
    std::stack<std::unique_ptr<IState>> m_states;
    std::map<std::string, std::function<std::unique_ptr<IState>()>> m_factories;
    std::string m_pending_transition;
    enum class TransitionType { None, Push, Pop, Change };
    TransitionType m_transition_type = TransitionType::None;
};

}  // namespace rtype
