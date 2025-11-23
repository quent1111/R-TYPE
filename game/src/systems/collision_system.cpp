#include "systems/collision_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include <vector>

struct CollisionData {
    std::size_t entity_id;
    float x, y;
    float width, height;
    bool has_damage;
    int damage;
    bool destroy_on_hit;
};

static bool checkAABB(float x1, float y1, float w1, float h1, float x2, float y2, float w2,
                      float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void collisionSystem(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& collision_boxes = reg.get_components<collision_box>();
    auto& damages = reg.get_components<damage_on_contact>();
    auto& healths = reg.get_components<health>();
    auto& projectile_tags = reg.get_components<projectile_tag>();
    auto& enemy_tags = reg.get_components<enemy_tag>();

    std::vector<CollisionData> colliders;
    for (std::size_t i = 0; i < positions.size() && i < collision_boxes.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& box_opt = collision_boxes[i];

        if (pos_opt && box_opt) {
            auto& pos = pos_opt.value();
            auto& box = box_opt.value();

            CollisionData data;
            data.entity_id = i;
            data.x = pos.x + box.offset_x;
            data.y = pos.y + box.offset_y;
            data.width = box.width;
            data.height = box.height;
            data.has_damage = false;
            data.damage = 0;
            data.destroy_on_hit = false;

            if (i < damages.size() && damages[i]) {
                auto& dmg = damages[i].value();
                data.has_damage = true;
                data.damage = dmg.damage_amount;
                data.destroy_on_hit = dmg.destroy_on_hit;
            }

            colliders.push_back(data);
        }
    }

    for (std::size_t i = 0; i < colliders.size(); ++i) {
        std::size_t entity_i = colliders[i].entity_id;

        if (entity_i >= projectile_tags.size() || !projectile_tags[entity_i]) {
            continue;
        }

        for (std::size_t j = 0; j < colliders.size(); ++j) {
            if (i == j) continue;

            std::size_t entity_j = colliders[j].entity_id;

            if (entity_j >= enemy_tags.size() || !enemy_tags[entity_j]) {
                continue;
            }

            if (checkAABB(colliders[i].x, colliders[i].y, colliders[i].width, colliders[i].height,
                          colliders[j].x, colliders[j].y, colliders[j].width,
                          colliders[j].height)) {
                if (colliders[i].has_damage && entity_j < healths.size() && healths[entity_j]) {
                    healths[entity_j].value().current -= colliders[i].damage;
                }

                if (colliders[i].destroy_on_hit && entity_i < healths.size()) {
                    if (!healths[entity_i]) {
                        reg.add_component(reg.entity_from_index(entity_i), health{0, 1});
                    } else {
                        healths[entity_i].value().current = 0;
                    }
                }
            }
        }
    }
}
