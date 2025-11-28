#include "Game.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <optional>
#include <cstdint>
#include <cstring>

extern std::atomic<bool> server_running;

Game::Game(/* args */)
{
    _registry.register_component<position>();
    _registry.register_component<velocity>();
    _registry.register_component<health>();
    _registry.register_component<collider>();
    _registry.register_component<entity_tag>();
    _registry.register_component<network_id>();
}

Game::~Game()
{
}

entity Game::create_player(int client_id, float start_x, float start_y) {
    if (_client_entity_ids.find(client_id) != _client_entity_ids.end()) {
        std::cout << "[Game] Player for client " << client_id << " already exists" << std::endl;
        return _registry.entity_from_index(_client_entity_ids[client_id]);
    }
    auto player = _registry.spawn_entity();

    _registry.emplace_component<position>(player, start_x, start_y);
    _registry.emplace_component<velocity>(player, 1.0f, 0.0f);
    _registry.emplace_component<health>(player, 100);
    _registry.emplace_component<collider>(player, 32.0f, 32.0f);
    _registry.emplace_component<entity_tag>(player, entity_type::PLAYER);
    _registry.emplace_component<network_id>(player, client_id);

    _client_entity_ids[client_id] = player.id();
    std::cout << "[Game] Player created for client " << client_id
              << " (Entity ID: " << player.id() << ")" << std::endl;
    return player;
}

std::optional<entity> Game::get_player_entity(int client_id) {
    auto it = _client_entity_ids.find(client_id);
    if (it != _client_entity_ids.end()) {
        return _registry.entity_from_index(it->second);
    }
    return std::nullopt;
}

void Game::remove_player(int client_id) {
    auto it = _client_entity_ids.find(client_id);
    if (it != _client_entity_ids.end()) {
        auto player = _registry.entity_from_index(it->second);
        _registry.kill_entity(player);
        _client_entity_ids.erase(it);
        std::cout << "[Game] Player removed for client " << client_id << std::endl;
    }
}

void Game::broadcast_player_positions(UDPServer& server) {
    if (_client_entity_ids.empty()) {
        return;
    }

    std::vector<uint8_t> data;
    data.push_back(0x42);
    data.push_back(0xB5);
    data.push_back(0x01); // msg type: player_positions
    data.push_back(static_cast<uint8_t>(_client_entity_ids.size()));

    for (const auto& [client_id, entity_id] : _client_entity_ids) {
        auto player = _registry.entity_from_index(entity_id);
        auto pos_opt = _registry.get_component<position>(player);

        if (pos_opt.has_value()) {
            const auto& pos = pos_opt.value();

            // Client ID (2 bytes)
            data.push_back(static_cast<uint8_t>(client_id & 0xFF));
            data.push_back(static_cast<uint8_t>((client_id >> 8) & 0xFF));

            // Position X (4 bytes - float as bytes)
            uint32_t x_int = *reinterpret_cast<const uint32_t*>(&pos.x);
            data.push_back(static_cast<uint8_t>(x_int & 0xFF));
            data.push_back(static_cast<uint8_t>((x_int >> 8) & 0xFF));
            data.push_back(static_cast<uint8_t>((x_int >> 16) & 0xFF));
            data.push_back(static_cast<uint8_t>((x_int >> 24) & 0xFF));

            // Position Y (4 bytes - float as bytes)
            uint32_t y_int = *reinterpret_cast<const uint32_t*>(&pos.y);
            data.push_back(static_cast<uint8_t>(y_int & 0xFF));
            data.push_back(static_cast<uint8_t>((y_int >> 8) & 0xFF));
            data.push_back(static_cast<uint8_t>((y_int >> 16) & 0xFF));
            data.push_back(static_cast<uint8_t>((y_int >> 24) & 0xFF));
        }
    }

    server.send_to_all(data);
}

void Game::process_network_events(UDPServer& server) {
    NetworkPacket packet;
    while (server.get_input_packet(packet)) {
        if (!packet.data.empty()) {
            int client_id = server.register_client(packet.sender);
            auto player_opt = get_player_entity(client_id);

            if (!player_opt.has_value()) {
                float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                float start_y = 300.0f;
                create_player(client_id, start_x, start_y);

                std::string welcome_msg = "[Game] welcome player " + std::to_string(client_id);
                std::vector<uint8_t> welcome_data;
                welcome_data.push_back(0x42);
                welcome_data.push_back(0xB5);
                welcome_data.insert(welcome_data.end(), welcome_msg.begin(), welcome_msg.end());
                NetworkPacket welcome_packet(std::move(welcome_data), packet.sender);
                server.queue_output_packet(std::move(welcome_packet));
            }

            NetworkPacket response_packet(std::move(packet.data), packet.sender);
            server.queue_output_packet(std::move(response_packet));
        }
    }
}

void Game::update_game_state(float dt)
{
    // 1. Logique (IA, Tirs auto)
    // ai_system(_registry);

    // 2. Physique
    position_system(_registry, dt);

    // 3. Collisions & Dégâts
    // collision_system(_registry);
    // damage_system(_registry);

    // 4. Nettoyage
    // boundary_system(_registry, 4000.0f, 1000.0f);
    // cleanup_system(_registry);
}

void Game::send_periodic_updates(UDPServer& server, float dt) {
    const float position_broadcast_interval = 0.1f; // 100ms
    const float cleanup_interval = 1.0f;            // 1s

    _pos_broadcast_accumulator += dt;
    if (_pos_broadcast_accumulator >= position_broadcast_interval) {
        broadcast_player_positions(server);
        _pos_broadcast_accumulator -= position_broadcast_interval;
    }

    _cleanup_accumulator += dt;
    if (_cleanup_accumulator >= cleanup_interval) {
        auto dead_clients = server.remove_inactive_clients(std::chrono::seconds(30));
        for (int id : dead_clients) {
            remove_player(id);
        }
        _cleanup_accumulator -= cleanup_interval;
    }
}

void Game::runGameLoop(UDPServer& server) {
    std::cout << "[Core] Game loop started at 60 TPS (fixed timestep)" << std::endl;

    const double target_ticks = 60.0;
    const std::chrono::duration<double> fixed_timestep(1.0 / target_ticks);
    const float dt = static_cast<float>(fixed_timestep.count());

    auto previous_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> lag(0.0);

    while (server_running) {
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed;

        while (lag >= fixed_timestep) {
            process_network_events(server);
            update_game_state(dt);
            send_periodic_updates(server, dt);
            lag -= fixed_timestep;
        }
        auto frame_duration = std::chrono::steady_clock::now() - current_time;
        if (frame_duration < fixed_timestep) {
            std::this_thread::sleep_for(fixed_timestep - frame_duration);
        }
    }
    std::cout << "[Core] Game loop stopped" << std::endl;
}