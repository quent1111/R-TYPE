#pragma once

#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../game-lib/include/entities/projectile_factory.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "common/InputKey.hpp"
#include "handlers/InputBuffer.hpp"

#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

namespace server {

class InputHandler {
public:
    InputHandler() = default;
    ~InputHandler() = default;

    void handle_player_input(registry& reg,
                             const std::unordered_map<int, std::size_t>& client_entity_ids,
                             int client_id, const std::vector<uint8_t>& data);

    void apply_buffered_inputs(registry& reg,
                               const std::unordered_map<int, std::size_t>& client_entity_ids);

    void clear_client_buffer(int client_id);

private:
    std::optional<entity>
    get_player_entity(registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
                      int client_id);

    void apply_input_to_player(registry& reg, entity player, uint8_t input_mask);

    std::unordered_map<int, ClientInputBuffer> client_input_buffers_;
};

}  // namespace server
