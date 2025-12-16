#pragma once

#include "SystemManager.hpp"
#include "../ecs/registry.hpp"

namespace engine {

/**
 * @brief Core game engine that manages ECS registry and systems.
 * 
 * GameEngine provides:
 * - Central registry for entities/components
 * - System registration and lifecycle management
 * - Clean separation between engine infrastructure and game logic
 * 
 * Usage:
 *   GameEngine engine;
 *   engine.register_system(std::make_unique<MovementSystem>());
 *   engine.init();
 *   while (running) {
 *       engine.update(dt);
 *   }
 *   engine.shutdown();
 */
class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    /**
     * @brief Get reference to the ECS registry.
     */
    registry& get_registry() { return _registry; }

    /**
     * @brief Register a system to the engine.
     * @param system Unique pointer to system (takes ownership).
     */
    void register_system(std::unique_ptr<ISystem> system);

    /**
     * @brief Initialize all systems.
     */
    void init();

    /**
     * @brief Update all systems for one frame.
     * @param dt Delta time in seconds.
     */
    void update(float dt);

    /**
     * @brief Shutdown all systems.
     */
    void shutdown();

private:
    registry _registry;
    SystemManager _system_manager;
};

}  // namespace engine
