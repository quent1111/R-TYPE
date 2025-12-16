#include "entities/explosion_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"

entity createExplosion(registry& reg, float x, float y) {
    entity explosion = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<explosion_tag>();
    reg.register_component<entity_tag>();

    reg.add_component(explosion, position{x, y});

    std::vector<sf::IntRect> explosion_frames = {
        {0, 0, 34, 38},
        {34, 0, 33, 38},
        {67, 0, 35, 38},
        {102, 0, 34, 38},
        {136, 0, 35, 38},
        {171, 0, 35, 38}
    };

    reg.add_component(explosion,
                      sprite_component{"assets/explosion.gif", 0, 0, 34, 38, 2.0f});
    
    animation_component anim(explosion_frames, 0.08f, false);

    reg.add_component(explosion, explosion_tag{0.5f});
    reg.add_component(explosion, entity_tag{RType::EntityType::Obstacle});

    return explosion;
}
