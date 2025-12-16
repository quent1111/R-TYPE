#include "SystemManager.hpp"

namespace engine {

void SystemManager::register_system(std::unique_ptr<ISystem> system) {
    _systems.push_back(std::move(system));
}

void SystemManager::init_all(registry& reg) {
    for (auto& system : _systems) {
        system->init(reg);
    }
}

void SystemManager::update_all(registry& reg, float dt) {
    for (auto& system : _systems) {
        system->update(reg, dt);
    }
}

void SystemManager::shutdown_all(registry& reg) {
    for (auto& system : _systems) {
        system->shutdown(reg);
    }
}

}  // namespace engine
