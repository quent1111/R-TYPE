#include "GameEngine.hpp"

namespace engine {

GameEngine::GameEngine() = default;

GameEngine::~GameEngine() = default;

void GameEngine::register_system(std::unique_ptr<ISystem> system) {
    _system_manager.register_system(std::move(system));
}

void GameEngine::init() {
    _system_manager.init_all(_registry);
}

void GameEngine::update(float dt) {
    _system_manager.update_all(_registry, dt);
}

void GameEngine::shutdown() {
    _system_manager.shutdown_all(_registry);
}

}  // namespace engine
