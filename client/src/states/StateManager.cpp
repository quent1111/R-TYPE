#include "states/StateManager.hpp"
#include <iostream>

namespace rtype {

void StateManager::register_state(
    const std::string& name, std::function<std::unique_ptr<IState>()> factory) {
    m_factories[name] = std::move(factory);
}

void StateManager::push_state(const std::string& name) {
    auto it = m_factories.find(name);
    if (it == m_factories.end()) {
        std::cerr << "Error: State '" << name << "' not registered!\n";
        return;
    }

    auto state = it->second();
    state->on_enter();
    m_states.push(std::move(state));
}

void StateManager::pop_state() {
    if (!m_states.empty()) {
        m_states.top()->on_exit();
        m_states.pop();
    }
}

void StateManager::change_state(const std::string& name) {
    if (!m_states.empty()) {
        m_states.top()->on_exit();
        m_states.pop();
    }
    push_state(name);
}

void StateManager::handle_event(const sf::Event& event) {
    if (!m_states.empty()) {
        m_states.top()->handle_event(event);
    }
}

void StateManager::update(float dt) {
    if (!m_states.empty()) {
        m_states.top()->update(dt);
    }
}

void StateManager::render(sf::RenderWindow& window) {
    if (!m_states.empty()) {
        m_states.top()->render(window);
    }
}

void StateManager::process_transitions() {
    if (!m_states.empty()) {
        std::string next_state = m_states.top()->get_next_state();
        if (!next_state.empty()) {
            change_state(next_state);
        }
    }
}

} // namespace rtype
