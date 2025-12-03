#include "entities/explosion_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"

entity createExplosion(registry& reg, float x, float y) {
    entity explosion = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<explosion_tag>();

    reg.add_component(explosion, position{x, y});

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet1.png";
    sprite.texture_rect_x = 330;
    sprite.texture_rect_y = 289;
    sprite.texture_rect_w = 28;
    sprite.texture_rect_h = 34;
    sprite.scale = 2.0f;
    reg.add_component(explosion, sprite);

    animation_component anim;
    anim.frames.push_back(sf::IntRect(330, 289, 28, 34));
    anim.frames.push_back(sf::IntRect(362, 289, 28, 34));
    anim.frames.push_back(sf::IntRect(394, 289, 28, 34));
    anim.frames.push_back(sf::IntRect(426, 289, 28, 34));
    anim.current_frame = 0;
    anim.frame_duration = 0.08f;
    anim.time_accumulator = 0.0f;
    anim.loop = false;
    reg.add_component(explosion, anim);

    reg.add_component(explosion, explosion_tag{0.35f});

    return explosion;
}
