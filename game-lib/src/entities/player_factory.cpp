#include "entities/player_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"

entity createPlayer(registry& reg, float x, float y, int player_index) {
    entity player = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<controllable>();
    reg.register_component<health>();
    reg.register_component<weapon>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<player_tag>();
    reg.register_component<bounded_movement>();
    reg.register_component<player_index_component>();

    std::vector<sf::IntRect> player_frames = {
        {99, 0, 33, 17},
        {132, 0, 33, 17},
        {165, 0, 33, 17}
    };

    reg.add_component(player, position{x, y});
    reg.add_component(player, velocity{0.0f, 0.0f});
    reg.add_component(player, controllable{300.0f});
    reg.add_component(player, health{100});
    reg.add_component(player, weapon{3.0f, 600.0f, 20});
    reg.add_component(player,
                      sprite_component{"assets/r-typesheet1.png", 99, 0, 33, 17, 2.0f});
    reg.add_component(player, animation_component{player_frames, 0.15f, true});
    reg.add_component(player, collision_box{48.0f, 24.0f});
    reg.add_component(player, player_tag{});
    reg.add_component(player, bounded_movement{0.0f, 1920.0f, 0.0f, 1080.0f});
    reg.add_component(player, player_index_component{player_index});

    return player;
}
