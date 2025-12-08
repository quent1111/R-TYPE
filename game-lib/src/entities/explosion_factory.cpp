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
        {330, 289, 28, 34},
        {362, 289, 28, 34},
        {394, 289, 28, 34},
        {426, 289, 28, 34},
        {0, 0, 1, 1}
    };

    reg.add_component(explosion,
                      sprite_component{"assets/r-typesheet1.png", 330, 289, 28, 34, 2.0f});
    
    animation_component anim(explosion_frames, 0.08f, false);

    reg.add_component(explosion, explosion_tag{0.5f});
    reg.add_component(explosion, entity_tag{RType::EntityType::Obstacle});

    return explosion;
}
