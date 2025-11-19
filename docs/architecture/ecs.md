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
// Position component
struct Position {
    float x;
    float y;
};

// Velocity component
struct Velocity {
    float dx;
    float dy;
};

// Sprite component
struct Sprite {
    std::string texture_path;
    int width;
    int height;
};

// Health component
struct Health {
    int current;
    int maximum;
};
```

### System

Systems contain game logic:

```cpp
class MovementSystem {
public:
    void update(Registry& registry, float deltaTime) {
        auto& positions = registry.get_components<Position>();
        auto& velocities = registry.get_components<Velocity>();
        
        for (size_t i = 0; i < positions.size(); ++i) {
            if (positions[i] && velocities[i]) {
                positions[i]->x += velocities[i]->dx * deltaTime;
                positions[i]->y += velocities[i]->dy * deltaTime;
            }
        }
    }
};
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
Registry registry;

// Register components
registry.register_component<Position>();
registry.register_component<Velocity>();
registry.register_component<Sprite>();

// Create player entity
Entity player = registry.spawn_entity();
registry.add_component(player, Position{100.0f, 100.0f});
registry.add_component(player, Velocity{0.0f, 0.0f});
registry.add_component(player, Sprite{"player.png", 32, 32});

// Create enemy entity
Entity enemy = registry.spawn_entity();
registry.add_component(enemy, Position{500.0f, 300.0f});
registry.add_component(enemy, Velocity{-50.0f, 0.0f});
registry.add_component(enemy, Sprite{"enemy.png", 32, 32});
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
