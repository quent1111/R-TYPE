#include "Game.hpp"

#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "inputKey.hpp"

#include <cstdint>
#include <cstring>

#include <chrono>
#include <iostream>
#include <optional>
#include <random>
#include <thread>

extern std::atomic<bool> server_running;

Game::Game() {
    _registry.register_component<position>();
    _registry.register_component<velocity>();
    _registry.register_component<health>();
    _registry.register_component<collider>();
    _registry.register_component<entity_tag>();
    _registry.register_component<network_id>();
    _registry.register_component<controllable>();
    _registry.register_component<weapon>();
    _registry.register_component<collision_box>();
    _registry.register_component<damage_on_contact>();
    _registry.register_component<player_tag>();
    _registry.register_component<enemy_tag>();
    _registry.register_component<projectile_tag>();
    _registry.register_component<bounded_movement>();
    _registry.register_component<explosion_tag>();
    _registry.register_component<wave_manager>();
    _registry.register_component<sprite_component>();
    _registry.register_component<animation_component>();
    _broadcast_serializer.reserve(65536);
}

Game::~Game() {}

entity Game::create_player(int client_id, float start_x, float start_y) {
    if (_client_entity_ids.find(client_id) != _client_entity_ids.end()) {
        std::cout << "[Game] Player for client " << client_id << " already exists" << std::endl;
        return _registry.entity_from_index(_client_entity_ids[client_id]);
    }

    auto player = ::createPlayer(_registry, start_x, start_y);
    _registry.emplace_component<network_id>(player, client_id);

    _client_entity_ids[client_id] = player.id();
    std::cout << "[Game] Player created for client " << client_id << " (Entity ID: " << player.id()
              << ")" << std::endl;
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

void Game::broadcast_entity_positions(UDPServer& server) {
    _broadcast_serializer.clear();

    _broadcast_serializer << RType::MagicNumber::VALUE;
    _broadcast_serializer << RType::OpCode::EntityPosition;

    size_t count_position = _broadcast_serializer.data().size();
    _broadcast_serializer << static_cast<uint8_t>(0);

    size_t entity_count = 0;
    auto& tags = _registry.get_components<entity_tag>();
    auto& positions = _registry.get_components<position>();

    for (const auto& [client_id, entity_id] : _client_entity_ids) {
        auto player = _registry.entity_from_index(entity_id);
        auto pos_opt = _registry.get_component<position>(player);
        auto vel_opt = _registry.get_component<velocity>(player);

        if (pos_opt.has_value()) {
            const auto& pos = pos_opt.value();
            _broadcast_serializer << static_cast<uint32_t>(entity_id);
            _broadcast_serializer << static_cast<uint8_t>(RType::EntityType::Player);
            _broadcast_serializer << pos.x;
            _broadcast_serializer << pos.y;
            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            _broadcast_serializer << vx;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            _broadcast_serializer << vy;
            entity_count++;
        }
    }

    for (size_t i = 0; i < tags.size(); ++i) {
        if (!tags[i].has_value()) continue;
        if (i >= positions.size() || !positions[i].has_value()) continue;

        if (tags[i]->type == RType::EntityType::Enemy ||
            tags[i]->type == RType::EntityType::Projectile) {

            const auto& pos = positions[i].value();
            auto entity_obj = _registry.entity_from_index(i);
            auto vel_opt = _registry.get_component<velocity>(entity_obj);

            _broadcast_serializer << static_cast<uint32_t>(i);
            _broadcast_serializer << static_cast<uint8_t>(tags[i]->type);
            _broadcast_serializer << pos.x;
            _broadcast_serializer << pos.y;
            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            _broadcast_serializer << vx;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            _broadcast_serializer << vy;
            entity_count++;
        }
    }

    if (entity_count == 0) {
        return;
    }

    _broadcast_serializer.data()[count_position] = static_cast<uint8_t>(entity_count);

    server.send_to_all(_broadcast_serializer.data());
}

void Game::handle_player_input(int client_id, const std::vector<uint8_t>& data) {
    auto player_opt = get_player_entity(client_id);
    if (!player_opt.has_value())
        return;

    RType::BinarySerializer deserializer(data);
    uint8_t input_mask;
    uint32_t timestamp;

    try {
        deserializer >> input_mask >> timestamp;
    } catch (...) {
        std::cerr << "[Game] Failed to parse input payload" << std::endl;
        return;
    }

    auto player = player_opt.value();
    auto& pos_opt = _registry.get_component<position>(player);
    // auto& vel_opt = _registry.get_component<velocity>(player);
    auto& wpn_opt = _registry.get_component<weapon>(player);

    if (pos_opt.has_value()) {
        float speed = 10.0f;
        if (input_mask & KEY_Z)
            pos_opt->y -= speed;
        if (input_mask & KEY_S)
            pos_opt->y += speed;
        if (input_mask & KEY_Q)
            pos_opt->x -= speed;
        if (input_mask & KEY_D)
            pos_opt->x += speed;
        if (input_mask & KEY_SPACE) {
             if (wpn_opt.has_value()) {
                 auto& wpn = wpn_opt.value();
                 if (wpn.can_shoot()) {
                     std::cout << "[Game] Client " << client_id << " (Entity " << player.id() << ") SHOOTS!" << std::endl;
                     ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f, 0.0f, 10);
                     wpn.reset_shot_timer();
                 }
             } else {
                 std::cout << "[Game] Client " << client_id << " (Entity " << player.id() << ") SHOOTS (Fallback)!" << std::endl;
                 ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f, 0.0f, 10);
             }
        }
    }
}

void Game::process_network_events(UDPServer& server) {
    NetworkPacket packet;
    while (server.get_input_packet(packet)) {
        if (packet.data.empty() || packet.data.size() < 3) {
            continue;
        }

        try {
            RType::BinarySerializer deserializer(packet.data);

            uint16_t magic;
            deserializer >> magic;

            if (!RType::MagicNumber::is_valid(magic)) {
                std::cerr << "[Game] Invalid magic number from " << packet.sender << std::endl;
                continue;
            }

            RType::OpCode opcode;
            deserializer >> opcode;
            int client_id = server.register_client(packet.sender);

            switch (opcode) {
                case RType::OpCode::Input: {
                    auto player_opt = get_player_entity(client_id);
                    if (!player_opt.has_value()) {
                        float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                        float start_y = 300.0f;
                        auto player = create_player(client_id, start_x, start_y);
                        std::cout << "[Game] New player connected (Implicit): Client " << client_id
                                  << " (Entity " << player.id() << ")" << std::endl;
                    }

                    std::vector<uint8_t> payload(
                        deserializer.data().begin() +
                            static_cast<std::ptrdiff_t>(deserializer.read_position()),
                        deserializer.data().end());
                    handle_player_input(client_id, payload);
                    break;
                }
                case RType::OpCode::Login: {
                    std::cout << "[Game] Login request received from client " << client_id
                              << std::endl;
                    auto player_opt = get_player_entity(client_id);
                    if (!player_opt.has_value()) {
                        float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                        float start_y = 300.0f;
                        create_player(client_id, start_x, start_y);
                    }
                    break;
                }
                default: {
                    std::cout << "[Game] Unhandled opcode: 0x" << std::hex
                              << static_cast<int>(opcode) << std::dec << " from client "
                              << client_id << std::endl;
                    break;
                }
            }

        } catch (const RType::SerializationException& e) {
            std::cerr << "[Game] Serialization error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Game] Error processing packet: " << e.what() << std::endl;
        }
    }
}

void Game::update_game_state(float dt) {
    shootingSystem(_registry, dt);
    waveSystem(_registry, dt);

    movementSystem(_registry, dt);

    cleanupSystem(_registry);
}

void Game::send_periodic_updates(UDPServer& server, float dt) {
    const float position_broadcast_interval = 0.1f;
    const float cleanup_interval = 1.0f;

    _pos_broadcast_accumulator += dt;
    if (_pos_broadcast_accumulator >= position_broadcast_interval) {
        broadcast_entity_positions(server);
        _pos_broadcast_accumulator -= position_broadcast_interval;
    }

    _cleanup_accumulator += dt;
    if (_cleanup_accumulator >= cleanup_interval) {
        auto dead_clients = server.remove_inactive_clients(std::chrono::seconds(90));
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
