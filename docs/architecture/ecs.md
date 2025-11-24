# Entity Component System (ECS)

Deep dive into R-TYPE's custom ECS implementation.

## What is ECS?

ECS is a software architectural pattern that separates data (Components) from logic (Systems) and uses Entities as simple identifiers.

### Traditional OOP vs ECS

=== "Traditional OOP"

    ```cpp
    class GameObject {
        Position position;
        Sprite sprite;
        Health health;
        
        void update();
        void render();
        void takeDamage(int amount);
    };
    
    class Enemy : public GameObject { ... };
    class Player : public GameObject { ... };
    ```

=== "ECS Approach"

    ```cpp
    // Components (pure data)
    struct Position { float x, y; };
    struct Sprite { std::string texture; };
    struct Health { int current, max; };
    
    // Systems (pure logic)
    class RenderSystem {
        void update(Registry& reg) { ... }
    };
    
    class HealthSystem {
        void update(Registry& reg) { ... }
    };
    
    // Entities (just IDs)
    Entity player = registry.spawn_entity();
    ```

## Core Components

### Entity

An entity is simply a unique identifier:

```cpp
class Entity {
    friend class Registry;
    
private:
    size_t _index;
    
public:
    explicit Entity(size_t index) : _index(index) {}
    operator size_t() const { return _index; }
};
```

### Component

Components are plain data structures:

```cpp
// Base ECS components (engine/ecs/components.hpp)
struct position {
    float x;
    float y;
    constexpr position(float pos_x = 0.0f, float pos_y = 0.0f) noexcept 
        : x(pos_x), y(pos_y) {}
};

struct velocity {
    float vx;
    float vy;
    constexpr velocity(float vel_x = 0.0f, float vel_y = 0.0f) noexcept 
        : vx(vel_x), vy(vel_y) {}
};

// Game-specific components (game/include/components/game_components.hpp)
struct sprite_component {
    std::string texture_path;
    int texture_rect_x;
    int texture_rect_y;
    int texture_rect_w;
    int texture_rect_h;
    float scale;
};

struct animation_component {
    std::vector<sf::IntRect> frames;
    size_t current_frame;
    float frame_duration;
    float time_accumulator;
    bool loop;
    
    void update(float dt);
    sf::IntRect get_current_frame() const;
};

struct health {
    int current;
    int maximum;
    bool is_dead() const { return current <= 0; }
    float health_percentage() const;
};

struct explosion_tag {
    float lifetime;
    float elapsed;
};

// Entity tags
struct player_tag {};
struct enemy_tag {};
struct projectile_tag {};
```

### System

Systems contain game logic and operate on entities with specific components:

```cpp
// Movement System (game/src/systems/movement_system.cpp)
void movementSystem(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();
    
    for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        if (positions[i] && velocities[i]) {
            positions[i]->x += velocities[i]->vx * dt;
            positions[i]->y += velocities[i]->vy * dt;
        }
    }
}

// Collision System (game/src/systems/collision_system.cpp)
void collisionSystem(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& projectile_tags = reg.get_components<projectile_tag>();
    auto& enemy_tags = reg.get_components<enemy_tag>();
    auto& healths = reg.get_components<health>();
    
    // Check projectile-enemy collisions
    for (size_t proj_id = 0; proj_id < positions.size(); ++proj_id) {
        if (!positions[proj_id] || !projectile_tags[proj_id]) continue;
        
        for (size_t enemy_id = 0; enemy_id < positions.size(); ++enemy_id) {
            if (!positions[enemy_id] || !enemy_tags[enemy_id]) continue;
            
            // Simple circle collision
            float dx = positions[proj_id]->x - positions[enemy_id]->x;
            float dy = positions[proj_id]->y - positions[enemy_id]->y;
            float distance_sq = dx * dx + dy * dy;
            
            if (distance_sq < COLLISION_RADIUS_SQ) {
                // Damage enemy
                if (healths[enemy_id]) {
                    healths[enemy_id]->current -= PROJECTILE_DAMAGE;
                }
                // Mark projectile for cleanup
                healths[proj_id]->current = 0;
            }
        }
    }
}
```

## Registry

The Registry manages entities and components:

```cpp
class Registry {
public:
    // Entity management
    Entity spawn_entity();
    void kill_entity(Entity const& entity);
    
    // Component management
    template<typename Component>
    SparseArray<Component>& register_component();
    
    template<typename Component>
    SparseArray<Component>& get_components();
    
    template<typename Component>
    typename SparseArray<Component>::reference_type
    add_component(Entity const& entity, Component&& component);
    
    template<typename Component>
    void remove_component(Entity const& entity);
};
```

## SparseArray

Efficient component storage:

```cpp
template<typename Component>
class SparseArray {
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type&;
    using const_reference_type = value_type const&;
    
    reference_type insert_at(size_t pos, Component const& component);
    reference_type insert_at(size_t pos, Component&& component);
    void erase(size_t pos);
    
    reference_type operator[](size_t idx);
    const_reference_type operator[](size_t idx) const;
    
    size_t size() const;
};
```

**Benefits:**
- O(1) component access by entity ID
- Cache-friendly iteration
- Automatic memory management with `std::optional`

## Usage Examples

### Creating Entities

```cpp
registry reg;

// Register components
reg.register_component<position>();
reg.register_component<velocity>();
reg.register_component<sprite_component>();
reg.register_component<health>();
reg.register_component<player_tag>();

// Create player entity using factory
entity player = createPlayer(reg, 200.0f, 540.0f);

// Alternatively, create manually
entity enemy = reg.spawn_entity();
reg.add_component(enemy, position{500.0f, 300.0f});
reg.add_component(enemy, velocity{-100.0f, 0.0f});
reg.add_component(enemy, sprite_component{
    .texture_path = "assets/r-typesheet26.png",
    .texture_rect_x = 0,
    .texture_rect_y = 0,
    .texture_rect_w = 65,
    .texture_rect_h = 50,
    .scale = 2.0f
});
reg.add_component(enemy, health{100, 100});
reg.add_component(enemy, enemy_tag{});
```

### System Implementation

```cpp
class CollisionSystem {
public:
    void update(Registry& registry) {
        auto& positions = registry.get_components<Position>();
        auto& colliders = registry.get_components<Collider>();
        
        // Check all entities against each other
        for (size_t i = 0; i < positions.size(); ++i) {
            if (!positions[i] || !colliders[i]) continue;
            
            for (size_t j = i + 1; j < positions.size(); ++j) {
                if (!positions[j] || !colliders[j]) continue;
                
                if (check_collision(positions[i].value(), colliders[i].value(),
                                   positions[j].value(), colliders[j].value())) {
                    handle_collision(Entity(i), Entity(j));
                }
            }
        }
    }
    
private:
    bool check_collision(const Position& p1, const Collider& c1,
                        const Position& p2, const Collider& c2) {
        // AABB collision detection
        return (p1.x < p2.x + c2.width &&
                p1.x + c1.width > p2.x &&
                p1.y < p2.y + c2.height &&
                p1.y + c1.height > p2.y);
    }
};
```

### Game Loop Integration

```cpp
class Game {
    Registry registry_;
    MovementSystem movement_system_;
    CollisionSystem collision_system_;
    RenderSystem render_system_;
    
public:
    void update(float deltaTime) {
        // Update all systems
        movement_system_.update(registry_, deltaTime);
        collision_system_.update(registry_);
    }
    
    void render() {
        render_system_.update(registry_);
    }
};
```

## Advanced Features

### Entity Reuse

Dead entities are recycled to avoid memory fragmentation:

```cpp
void Registry::kill_entity(Entity const& entity) {
    // Remove all components
    // Add entity index to free list for reuse
    _dead_entities.push_back(entity);
}

Entity Registry::spawn_entity() {
    if (!_dead_entities.empty()) {
        Entity reused = _dead_entities.back();
        _dead_entities.pop_back();
        return reused;
    }
    return Entity(_next_entity_id++);
}
```

### Component Queries

Helper functions to query entities:

```cpp
template<typename... Components>
std::vector<Entity> Registry::view() {
    std::vector<Entity> entities;
    // Find all entities with specified components
    return entities;
}

// Usage
auto players = registry.view<Position, Player>();
```

## Performance Tips

!!! tip "Component Access"
    Cache component arrays when iterating multiple times in a frame.

!!! warning "Component Size"
    Keep components small and focused. Large components hurt cache performance.

!!! info "System Order"
    Order systems carefully - some systems depend on others completing first.

## Next Steps

- 🏗️ [Architecture Overview](overview.md)
- 🌐 [Network Integration](network.md)
- 💻 [API Reference](../api/engine.md)
