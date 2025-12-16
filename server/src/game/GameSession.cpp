#include "game/GameSession.hpp"

#include <chrono>
#include <iostream>
#include <thread>

extern std::atomic<bool> server_running;

namespace server {

GameSession::GameSession() {
    _registry.register_component<position>();
    _registry.register_component<velocity>();
    _registry.register_component<health>();
    _registry.register_component<collider>();
    _registry.register_component<entity_tag>();
    _registry.register_component<network_id>();
    _registry.register_component<controllable>();
    _registry.register_component<weapon>();
    _registry.register_component<collision_box>();
    _registry.register_component<multi_hitbox>();
    _registry.register_component<damage_on_contact>();
    _registry.register_component<player_tag>();
    _registry.register_component<enemy_tag>();
    _registry.register_component<boss_tag>();
    _registry.register_component<projectile_tag>();
    _registry.register_component<bounded_movement>();
    _registry.register_component<explosion_tag>();
    _registry.register_component<wave_manager>();
    _registry.register_component<level_manager>();
    _registry.register_component<sprite_component>();
    _registry.register_component<animation_component>();
    _registry.register_component<power_cannon>();
    _registry.register_component<shield>();
    _registry.register_component<damage_flash_component>();
    auto level_mgr_entity = _registry.spawn_entity();
    _registry.emplace_component<level_manager>(level_mgr_entity);
}

GameSession::~GameSession() {}

void GameSession::handle_player_ready(int client_id, bool ready) {
    _client_ready_status[client_id] = ready;
    std::cout << "[Game] Client " << client_id << " is " << (ready ? "READY" : "NOT READY")
              << std::endl;
}

void GameSession::check_start_game(UDPServer& server) {
    if (_game_phase != GamePhase::Lobby) {
        return;
    }
    if (_client_ready_status.empty()) {
        return;
    }
    bool all_ready = true;
    for (const auto& [client_id, ready] : _client_ready_status) {
        if (!ready) {
            all_ready = false;
            break;
        }
    }
    if (all_ready) {
        std::cout << "[Game] All players ready! Starting game..." << std::endl;
        start_game(server);
    }
}

void GameSession::start_game(UDPServer& server) {
    std::cout << "[Game] Starting game..." << std::endl;

    _game_broadcaster.broadcast_start_game(server);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    _game_phase = GamePhase::InGame;

    float start_x = 100.0f;
    for (const auto& [client_id, ready] : _client_ready_status) {
        _player_manager.create_player(_registry, _client_entity_ids, client_id, start_x, 300.0f);
        start_x += 50.0f;
    }

    std::cout << "[Game] Game started with " << _client_ready_status.size() << " players"
              << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    _game_broadcaster.broadcast_level_start(server, 1);
}

void GameSession::process_network_events(UDPServer& server) {
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
                    if (_game_phase != GamePhase::InGame) {
                        break;
                    }
                    auto player_opt =
                        _player_manager.get_player_entity(_registry, _client_entity_ids, client_id);
                    if (!player_opt.has_value()) {
                        float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                        float start_y = 300.0f;
                        auto player = _player_manager.create_player(_registry, _client_entity_ids,
                                                                    client_id, start_x, start_y);
                        std::cout << "[Game] New player connected (Implicit): Client " << client_id
                                  << " (Entity " << player.id() << ")" << std::endl;
                    }

                    std::vector<uint8_t> payload(
                        deserializer.data().begin() +
                            static_cast<std::ptrdiff_t>(deserializer.read_position()),
                        deserializer.data().end());
                    _input_handler.handle_player_input(_registry, _client_entity_ids, client_id,
                                                       payload);
                    break;
                }
                case RType::OpCode::Login: {
                    std::cout << "[Game] Login request received from client " << client_id
                              << std::endl;
                    auto player_opt =
                        _player_manager.get_player_entity(_registry, _client_entity_ids, client_id);
                    if (!player_opt.has_value()) {
                        _client_ready_status[client_id] = false;
                        std::cout << "[Game] Client " << client_id << " joined lobby" << std::endl;

                        RType::BinarySerializer ack_serializer;
                        ack_serializer << RType::MagicNumber::VALUE;
                        ack_serializer << RType::OpCode::LoginAck;
                        ack_serializer << client_id;

                        server.send_to_client(client_id, ack_serializer.data());
                        std::cout << "[Game] Sent LoginAck with network ID " << client_id
                                  << " to client" << std::endl;
                    }
                    break;
                }
                case RType::OpCode::PlayerReady: {
                    uint8_t ready_byte;
                    try {
                        deserializer >> ready_byte;
                        bool ready = (ready_byte != 0);
                        handle_player_ready(client_id, ready);
                        check_start_game(server);
                    } catch (...) {
                        std::cerr << "[Game] Failed to parse PlayerReady payload" << std::endl;
                    }
                    break;
                }
                case RType::OpCode::WeaponUpgradeChoice: {
                    uint8_t upgrade_choice;
                    try {
                        deserializer >> upgrade_choice;
                        bool all_ready = _weapon_handler.handle_weapon_upgrade_choice(
                            _registry, _client_entity_ids, client_id, upgrade_choice);
                        if (all_ready) {
                            advance_level(server);
                        }
                    } catch (...) {
                        std::cerr << "[Game] Failed to parse WeaponUpgradeChoice payload"
                                  << std::endl;
                    }
                    break;
                }
                case RType::OpCode::PowerUpChoice: {
                    uint8_t powerup_choice;
                    try {
                        deserializer >> powerup_choice;
                        _powerup_handler.handle_powerup_choice(_registry, _client_entity_ids,
                                                               _players_who_chose_powerup,
                                                               client_id, powerup_choice);

                        int alive_players =
                            _powerup_handler.count_alive_players(_registry, _client_entity_ids);
                        if (static_cast<int>(_players_who_chose_powerup.size()) >= alive_players) {
                            std::cout << "[Game] All players have chosen! Advancing level..."
                                      << std::endl;
                            _waiting_for_powerup_choice = false;
                            _level_complete_waiting = false;
                            _players_who_chose_powerup.clear();

                            _powerup_broadcaster.broadcast_powerup_status(server, _registry,
                                                                          _client_entity_ids);
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            advance_level(server);
                        }
                    } catch (...) {
                        std::cerr << "[Game] Failed to parse PowerUpChoice payload" << std::endl;
                    }
                    break;
                }
                case RType::OpCode::PowerUpActivate: {
                    uint8_t powerup_type = 0;
                    deserializer >> powerup_type;
                    _powerup_handler.handle_powerup_activate(_registry, _client_entity_ids,
                                                             client_id, powerup_type);
                    _powerup_broadcaster.broadcast_powerup_status(server, _registry,
                                                                  _client_entity_ids);
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

void GameSession::update_game_state(UDPServer& server, float dt) {
    if (_game_phase != GamePhase::InGame) {
        return;
    }

    if (_waiting_for_game_over_reset) {
        _game_over_timer += dt;
        _game_over_broadcast_accumulator += dt;
        if (_game_over_broadcast_accumulator >= 0.1f) {
            _game_broadcaster.broadcast_game_over(server);
            _game_over_broadcast_accumulator = 0.0f;
        }
        if (_game_over_timer >= 2.0f) {
            reset_game(server);
            _waiting_for_game_over_reset = false;
            _game_over_timer = 0.0f;
            _game_over_broadcast_accumulator = 0.0f;
        }
        return;
    }

    if (_player_manager.check_all_players_dead(_registry, _client_entity_ids)) {
        _waiting_for_game_over_reset = true;
        _game_over_timer = 0.0f;
        _game_over_broadcast_accumulator = 0.0f;
        _game_broadcaster.broadcast_game_over(server);
        return;
    }

    shootingSystem(_registry, dt);
    enemyShootingSystem(_registry, dt);
    waveSystem(_registry, dt);
    movementSystem(_registry, dt);
    collisionSystem(_registry);

    _boss_manager.update_boss_behavior(
        _registry, _boss_entity, _client_entity_ids, _boss_animation_timer, _boss_shoot_timer,
        _boss_shoot_cooldown, _boss_animation_complete, _boss_entrance_complete, _boss_target_x,
        _boss_shoot_counter, dt);

    _boss_manager.update_homing_enemies(_registry, _client_entity_ids, dt);

    auto& cannons = _registry.get_components<power_cannon>();
    auto& shields = _registry.get_components<shield>();
    for (std::size_t i = 0; i < cannons.size(); ++i) {
        if (cannons[i].has_value()) {
            cannons[i]->update(dt);
        }
    }
    for (std::size_t i = 0; i < shields.size(); ++i) {
        if (shields[i].has_value()) {
            shields[i]->update(dt);
        }
    }

    auto& damage_flashes = _registry.get_components<damage_flash_component>();
    for (std::size_t i = 0; i < damage_flashes.size(); ++i) {
        if (damage_flashes[i].has_value()) {
            damage_flashes[i]->update(dt);
        }
    }

    auto& explosion_tags = _registry.get_components<explosion_tag>();

    for (std::size_t i = 0; i < explosion_tags.size(); ++i) {
        if (explosion_tags[i].has_value()) {
            explosion_tags[i].value().elapsed += dt;
        }
    }

    cleanupSystem(_registry);
}

void GameSession::send_periodic_updates(UDPServer& server, float dt) {
    // Increased from 20Hz (0.05s) to 30Hz (~0.033s) for smoother movement
    const float position_broadcast_interval = 0.033f;
    const float lobby_broadcast_interval = 0.5f;
    const float cleanup_interval = 1.0f;
    const float level_broadcast_interval = 0.5f;

    if (_waiting_for_game_over_reset) {
        return;
    }

    if (_game_phase == GamePhase::Lobby) {
        _lobby_broadcast_accumulator += dt;
        if (_lobby_broadcast_accumulator >= lobby_broadcast_interval) {
            _lobby_broadcaster.broadcast_lobby_status(server, _client_ready_status);
            _lobby_broadcast_accumulator -= lobby_broadcast_interval;
        }
    } else {
        _pos_broadcast_accumulator += dt;
        if (_pos_broadcast_accumulator >= position_broadcast_interval) {
            _entity_broadcaster.broadcast_entity_positions(server, _registry, _client_entity_ids);
            _pos_broadcast_accumulator -= position_broadcast_interval;
        }
        _level_broadcast_accumulator += dt;
        if (_level_broadcast_accumulator >= level_broadcast_interval) {
            _game_broadcaster.broadcast_level_info(server, _registry);
            _level_broadcast_accumulator -= level_broadcast_interval;
        }
        check_level_completion(server);
        _powerup_broadcast_accumulator += dt;
        if (_powerup_broadcast_accumulator >= 0.2f) {
            _powerup_broadcaster.broadcast_powerup_status(server, _registry, _client_entity_ids);
            _powerup_broadcast_accumulator -= 0.2f;
        }
    }

    _cleanup_accumulator += dt;
    if (_cleanup_accumulator >= cleanup_interval) {
        auto dead_clients = server.remove_inactive_clients(std::chrono::seconds(90));
        for (int id : dead_clients) {
            _client_ready_status.erase(id);
            _player_manager.remove_player(_registry, _client_entity_ids, id);
        }
        _cleanup_accumulator -= cleanup_interval;
    }
}

void GameSession::check_level_completion(UDPServer& server) {
    auto& level_managers = _registry.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            if (lvl_mgr.level_completed && !_level_complete_waiting &&
                !_waiting_for_powerup_choice) {
                std::cout << "[Game] Level " << static_cast<int>(lvl_mgr.current_level)
                          << " completed! Clearing enemies..." << std::endl;

                _players_who_chose_powerup.clear();
                _level_manager.clear_enemies_and_projectiles(_registry, _boss_entity);

                _game_broadcaster.broadcast_level_complete(server, _registry);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                _powerup_broadcaster.broadcast_powerup_selection(server);
                _level_complete_waiting = true;
                _waiting_for_powerup_choice = true;
                _level_complete_timer = 0.0f;
            }
        }
    }
}

void GameSession::advance_level(UDPServer& server) {
    _level_manager.advance_level(_registry);

    _player_manager.respawn_dead_players(_registry, _client_entity_ids);

    uint8_t current_level = _level_manager.get_current_level(_registry);

    if (current_level == 5) {
        std::cout << "[SERVER] *** Level 5 - Spawning BOSS! ***" << std::endl;
        _boss_manager.spawn_boss_level_5(_registry, _boss_entity, _boss_animation_timer,
                                         _boss_shoot_timer, _boss_animation_complete,
                                         _boss_entrance_complete, _boss_target_x);

        _game_broadcaster.broadcast_boss_spawn(server);
    }

    _game_broadcaster.broadcast_level_start(server, current_level);
}

void GameSession::reset_game([[maybe_unused]] UDPServer& server) {
    auto& level_managers = _registry.get_components<level_manager>();
    std::optional<size_t> level_mgr_idx = _level_manager.get_level_manager_index(_registry);

    _client_entity_ids.clear();

    for (auto& [client_id, ready] : _client_ready_status) {
        ready = false;
    }

    auto& positions = _registry.get_components<position>();
    auto& velocities = _registry.get_components<velocity>();
    auto& healths = _registry.get_components<health>();

    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i].has_value() || velocities[i].has_value() || healths[i].has_value()) {
            if (level_mgr_idx.has_value() && i == level_mgr_idx.value()) {
                continue;
            }
            auto ent = _registry.entity_from_index(i);
            _registry.kill_entity(ent);
        }
    }

    if (level_mgr_idx.has_value()) {
        auto& lvl_mgr = level_managers[level_mgr_idx.value()].value();
        lvl_mgr.current_level = 1;
        lvl_mgr.enemies_killed_this_level = 0;
    }

    _game_phase = GamePhase::Lobby;
    _level_complete_waiting = false;
    _waiting_for_powerup_choice = false;

    if (_boss_entity.has_value()) {
        try {
            auto boss_entity = _boss_entity.value();
            _registry.remove_component<entity_tag>(boss_entity);
            _registry.kill_entity(boss_entity);
        } catch (...) {}
    }
    _boss_entity = std::nullopt;
    _boss_animation_timer = 0.0f;
    _boss_shoot_timer = 0.0f;
    _boss_animation_complete = false;
    _boss_entrance_complete = false;
    _boss_shoot_counter = 0;

    std::cout << "[Game] Game reset. Back to lobby." << std::endl;
}

void GameSession::runGameLoop(UDPServer& server) {
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
            update_game_state(server, dt);
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

}  // namespace server
