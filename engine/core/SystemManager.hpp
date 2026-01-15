#pragma once

#include "ISystem.hpp"

#include <memory>
#include <vector>

namespace engine {

/**
 * @brief Manages registration and execution of systems in order.
 *
 * SystemManager enforces modular architecture by:
 * - Allowing systems to be registered in explicit order
 * - Providing lifecycle hooks (init, update, shutdown)
 * - Decoupling game logic from the main loop
 */
class SystemManager {
public:
    SystemManager() = default;
    ~SystemManager() = default;

    /**
     * @brief Register a system to be executed.
     * Systems are executed in the order they are registered.
     * @param system Unique pointer to the system (takes ownership).
     */
    void register_system(std::unique_ptr<ISystem> system);

    /**
     * @brief Initialize all registered systems.
     * @param reg Reference to the ECS registry.
     */
    void init_all(registry& reg);

    /**
     * @brief Update all registered systems in order.
     * @param reg Reference to the ECS registry.
     * @param dt Delta time since last update (seconds).
     */
    void update_all(registry& reg, float dt);

    /**
     * @brief Shutdown all registered systems.
     * @param reg Reference to the ECS registry.
     */
    void shutdown_all(registry& reg);

    /**
     * @brief Get the number of registered systems.
     */
    size_t count() const { return _systems.size(); }

private:
    std::vector<std::unique_ptr<ISystem>> _systems;
};

}  // namespace engine
