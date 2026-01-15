#include "handlers/InputHandler.hpp"

#include <cmath>

namespace server {

std::optional<entity> InputHandler::get_player_entity(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id) {
    auto it = client_entity_ids.find(client_id);
    if (it != client_entity_ids.end()) {
        return reg.entity_from_index(it->second);
    }
    return std::nullopt;
}

void InputHandler::handle_player_input(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id,
    const std::vector<uint8_t>& data) {
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value())
        return;

    auto player = player_opt.value();

    auto player_tag_opt = reg.get_component<player_tag>(player);
    if (!player_tag_opt.has_value()) {
        return;
    }

    RType::BinarySerializer deserializer(data);
    uint8_t input_mask;
    uint32_t timestamp;

    try {
        deserializer >> input_mask >> timestamp;
    } catch (...) {
        std::cerr << "[Game] Failed to parse input payload" << std::endl;
        return;
    }

    auto& buffer = client_input_buffers_[client_id];
    if (!buffer.add_input(timestamp, input_mask)) {
        std::cerr << "[InputHandler] Warning: Failed to buffer input for client " << client_id
                  << std::endl;
    }
}

void InputHandler::apply_buffered_inputs(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids) {
    for (auto& [client_id, buffer] : client_input_buffers_) {
        auto ready_inputs = buffer.get_ready_inputs();

        if (ready_inputs.empty()) {
            continue;
        }

        auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
        if (!player_opt.has_value()) {
            continue;
        }

        for (const auto& input : ready_inputs) {
            apply_input_to_player(reg, player_opt.value(), input.input_mask);
        }
    }
}

void InputHandler::apply_input_to_player(registry& reg, entity player, uint8_t input_mask) {
    auto& pos_opt = reg.get_component<position>(player);
    auto& vel_opt = reg.get_component<velocity>(player);
    auto& wpn_opt = reg.get_component<weapon>(player);
    auto& power_cannon_opt = reg.get_component<power_cannon>(player);

    if (pos_opt.has_value() && vel_opt.has_value()) {
        vel_opt->vx = 0.0f;
        vel_opt->vy = 0.0f;

        float speed = 300.0f;
        if (input_mask & KEY_Z)
            vel_opt->vy = -speed;
        if (input_mask & KEY_S)
            vel_opt->vy = speed;
        if (input_mask & KEY_Q)
            vel_opt->vx = -speed;
        if (input_mask & KEY_D)
            vel_opt->vx = speed;
        if (input_mask & KEY_SPACE) {
            if (wpn_opt.has_value()) {
                auto& wpn = wpn_opt.value();
                if (wpn.can_shoot()) {
                    int damage = wpn.damage;
                    WeaponUpgradeType visual_type = wpn.upgrade_type;
                    bool power_cannon_active =
                        power_cannon_opt.has_value() && power_cannon_opt->is_active();
                    if (power_cannon_active) {
                        damage = power_cannon_opt->damage;
                        visual_type = WeaponUpgradeType::PowerShot;
                    }

                    auto& multishot_opt = reg.get_component<multishot>(player);
                    int total_projectiles =
                        multishot_opt.has_value() ? multishot_opt->extra_projectiles : 1;

                    if (wpn.upgrade_type == WeaponUpgradeType::TripleShot &&
                        total_projectiles == 1) {
                        ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                           0.0f, damage, visual_type, power_cannon_active);
                        ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                           -100.0f, damage, visual_type, power_cannon_active);
                        ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                           100.0f, damage, visual_type, power_cannon_active);
                    } else if (total_projectiles > 1) {
                        float angle_step = 15.0f;
                        if (total_projectiles == 4) {
                            angle_step = 10.0f;
                        }

                        int half_count = total_projectiles / 2;
                        bool has_center = (total_projectiles % 2 == 1);

                        if (has_center) {
                            ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                               0.0f, damage, visual_type, power_cannon_active);
                        }

                        for (int i = 1; i <= half_count; ++i) {
                            float angle_deg = angle_step * static_cast<float>(i);
                            float angle_rad = angle_deg * 3.14159265f / 180.0f;
                            float vy = 500.0f * std::sin(angle_rad);

                            ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                               vy, damage, visual_type, power_cannon_active);
                            ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                               -vy, damage, visual_type, power_cannon_active);
                        }
                    } else {
                        ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f,
                                           0.0f, damage, visual_type, power_cannon_active);
                    }
                    wpn.reset_shot_timer();
                }
            } else {
                ::createProjectile(reg, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f, 0.0f, 10);
            }
        }
    }
}

void InputHandler::clear_client_buffer(int client_id) {
    auto it = client_input_buffers_.find(client_id);
    if (it != client_input_buffers_.end()) {
        it->second.clear();
        client_input_buffers_.erase(it);
        std::cout << "[InputHandler] Cleared input buffer for client " << client_id << std::endl;
    }
}

}  // namespace server
