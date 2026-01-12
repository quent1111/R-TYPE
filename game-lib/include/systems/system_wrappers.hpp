#pragma once

#include "../../engine/core/ISystem.hpp"
#include "shooting_system.hpp"
#include "movement_system.hpp"
#include "collision_system.hpp"
#include "wave_system.hpp"
#include "cleanup_system.hpp"


class ShootingSystem : public engine::ISystem {
public:
    void init([[maybe_unused]] registry& reg) override {}
    void update(registry& reg, float dt) override {
        shootingSystem(reg, dt);
    }
    void shutdown([[maybe_unused]] registry& reg) override {}
};

class EnemyShootingSystem : public engine::ISystem {
public:
    void init([[maybe_unused]] registry& reg) override {}
    void update(registry& reg, float dt) override {
        enemyShootingSystem(reg, dt);
    }
    void shutdown([[maybe_unused]] registry& reg) override {}
};

class MovementSystem : public engine::ISystem {
public:
    void init([[maybe_unused]] registry& reg) override {}
    void update(registry& reg, float dt) override {
        movementSystem(reg, dt);
    }
    void shutdown([[maybe_unused]] registry& reg) override {}
};

class CollisionSystem : public engine::ISystem {
public:
    void init([[maybe_unused]] registry& reg) override {}
    void update(registry& reg, [[maybe_unused]] float dt) override {
        collisionSystem(reg);
    }
    void shutdown([[maybe_unused]] registry& reg) override {}
};

class WaveSystem : public engine::ISystem {
public:
    void init([[maybe_unused]] registry& reg) override {}
    void update(registry& reg, float dt) override {
        waveSystem(reg, dt);
    }
    void shutdown([[maybe_unused]] registry& reg) override {}
};

class CleanupSystem : public engine::ISystem {
public:
    void init([[maybe_unused]] registry& reg) override {}
    void update(registry& reg, [[maybe_unused]] float dt) override {
        cleanupSystem(reg);
    }
    void shutdown([[maybe_unused]] registry& reg) override {}
};
