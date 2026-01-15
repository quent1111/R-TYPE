#pragma once

#include "../ecs/registry.hpp"

namespace engine {

/**
 * @brief Pure interface for game systems in the ECS architecture.
 *
 * Systems contain logic (no data) and operate on components via the registry.
 * This enforces separation: Components are POD (data), Systems are behavior.
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /**
     * @brief Initialize the system (called once at startup).
     * @param reg Reference to the ECS registry.
     */
    virtual void init(registry& reg) = 0;

    /**
     * @brief Update the system logic each frame.
     * @param reg Reference to the ECS registry.
     * @param dt Delta time since last update (seconds).
     */
    virtual void update(registry& reg, float dt) = 0;

    /**
     * @brief Cleanup/shutdown the system (called once before destruction).
     * @param reg Reference to the ECS registry.
     */
    virtual void shutdown(registry& reg) = 0;
};

}  // namespace engine
