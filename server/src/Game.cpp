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
    _registry.register_component<level_manager>();
    _registry.register_component<sprite_component>();
    _registry.register_component<animation_component>();
    _registry.register_component<power_cannon>();
    _registry.register_component<shield>();
    _broadcast_serializer.reserve(65536);
    auto level_mgr_entity = _registry.spawn_entity();
    _registry.emplace_component<level_manager>(level_mgr_entity);
}
Game::~Game() {}

entity Game::create_player(int client_id, float start_x, float start_y) {
    if (_client_entity_ids.find(client_id) != _client_entity_ids.end()) {
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

    // Joueurs
    for (const auto& [client_id, entity_id] : _client_entity_ids) {
        auto player = _registry.entity_from_index(entity_id);
        auto pos_opt = _registry.get_component<position>(player);
        auto vel_opt = _registry.get_component<velocity>(player);
        auto health_opt = _registry.get_component<health>(player);

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

            int current_health = health_opt.has_value() ? health_opt->current : 100;
            int max_health = health_opt.has_value() ? health_opt->maximum : 100;
            _broadcast_serializer << static_cast<int32_t>(current_health);
            _broadcast_serializer << static_cast<int32_t>(max_health);

            entity_count++;
        }
    }

    for (size_t i = 0; i < tags.size(); ++i) {
        if (!tags[i].has_value())
            continue;
        if (i >= positions.size() || !positions[i].has_value())
            continue;

        if (tags[i]->type == RType::EntityType::Enemy ||
            tags[i]->type == RType::EntityType::Projectile ||
            tags[i]->type == RType::EntityType::Obstacle) {
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

    if (entity_count == 0)
        return;

    _broadcast_serializer.data()[count_position] = static_cast<uint8_t>(entity_count);
    server.send_to_all(_broadcast_serializer.data());
}

void Game::handle_player_input(int client_id, const std::vector<uint8_t>& data) {
    auto player_opt = get_player_entity(client_id);
    if (!player_opt.has_value())
        return;

    auto player = player_opt.value();

    auto player_tag_opt = _registry.get_component<player_tag>(player);
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

    auto& pos_opt = _registry.get_component<position>(player);
    auto& vel_opt = _registry.get_component<velocity>(player);
    auto& wpn_opt = _registry.get_component<weapon>(player);
    auto& power_cannon_opt = _registry.get_component<power_cannon>(player);

    if (pos_opt.has_value() && vel_opt.has_value()) {
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
                    if (power_cannon_opt.has_value() && power_cannon_opt->is_active()) {
                        damage = power_cannon_opt->damage;
                        visual_type = WeaponUpgradeType::PowerShot;
                    }
                    if (wpn.upgrade_type == WeaponUpgradeType::TripleShot) {
                        ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f,
                                           500.0f, 0.0f, damage, visual_type);
                        ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f,
                                           500.0f, -100.0f, damage, visual_type);
                        ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f,
                                           500.0f, 100.0f, damage, visual_type);
                    } else {
                        ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f,
                                           500.0f, 0.0f, damage, visual_type);
                    }
                    wpn.reset_shot_timer();
                }
            } else {
                ::createProjectile(_registry, pos_opt->x + 50.0f, pos_opt->y + 10.0f, 500.0f, 0.0f,
                                   10);
            }
        }
    }
}

void Game::process_network_events(UDPServer& server) {
    if (_game_phase == GamePhase::InGame) {
        auto& velocities = _registry.get_components<velocity>();
        auto& player_tags = _registry.get_components<player_tag>();
        for (std::size_t i = 0; i < velocities.size() && i < player_tags.size(); ++i) {
            if (velocities[i].has_value() && player_tags[i].has_value()) {
                velocities[i]->vx = 0.0f;
                velocities[i]->vy = 0.0f;
            }
        }
    }

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
                        handle_weapon_upgrade_choice(client_id, upgrade_choice, server);
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
                        handle_powerup_choice(client_id, powerup_choice, server);
                    } catch (...) {
                        std::cerr << "[Game] Failed to parse PowerUpChoice payload" << std::endl;
                    }
                    break;
                }
                case RType::OpCode::PowerUpActivate: {
                    handle_powerup_activate(client_id, server);
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

void Game::handle_player_ready(int client_id, bool ready) {
    _client_ready_status[client_id] = ready;
    std::cout << "[Game] Client " << client_id << " is " << (ready ? "READY" : "NOT READY")
              << std::endl;
}

void Game::check_start_game(UDPServer& server) {
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

void Game::start_game(UDPServer& server) {
    std::cout << "[Game] ===== STARTING GAME =====" << std::endl;

    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::StartGame;
    server.send_to_all(serializer.data());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    _game_phase = GamePhase::InGame;

    float start_x = 100.0f;
    for (const auto& [client_id, ready] : _client_ready_status) {
        create_player(client_id, start_x, 300.0f);
        start_x += 50.0f;
    }

    std::cout << "[Game] Game started with " << _client_ready_status.size() << " players"
              << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    RType::BinarySerializer level_start;
    level_start << RType::MagicNumber::VALUE;
    level_start << RType::OpCode::LevelStart;
    level_start << static_cast<uint8_t>(1);
    server.send_to_all(level_start.data());
    std::cout << "[Game] Sent Level 1 start screen (opcode: 0x30, level: 1)" << std::endl;
}

void Game::broadcast_lobby_status(UDPServer& server) {
    _broadcast_serializer.clear();

    _broadcast_serializer << RType::MagicNumber::VALUE;
    _broadcast_serializer << RType::OpCode::LobbyStatus;

    uint8_t total_players = static_cast<uint8_t>(_client_ready_status.size());
    uint8_t ready_players = 0;

    for (const auto& [client_id, ready] : _client_ready_status) {
        if (ready)
            ready_players++;
    }

    _broadcast_serializer << total_players;
    _broadcast_serializer << ready_players;

    server.send_to_all(_broadcast_serializer.data());
}

void Game::update_game_state(float dt) {
    if (_game_phase != GamePhase::InGame) {
        return;
    }

    shootingSystem(_registry, dt);
    enemyShootingSystem(_registry, dt);
    waveSystem(_registry, dt);
    movementSystem(_registry, dt);
    collisionSystem(_registry);
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

    auto& explosion_tags = _registry.get_components<explosion_tag>();

    for (std::size_t i = 0; i < explosion_tags.size(); ++i) {
        if (explosion_tags[i].has_value()) {
            explosion_tags[i].value().elapsed += dt;
        }
    }

    cleanupSystem(_registry);
}

void Game::send_periodic_updates(UDPServer& server, float dt) {
    const float position_broadcast_interval = 0.05f;
    const float lobby_broadcast_interval = 0.5f;
    const float cleanup_interval = 1.0f;
    const float level_broadcast_interval = 0.5f;

    if (_game_phase == GamePhase::Lobby) {
        _lobby_broadcast_accumulator += dt;
        if (_lobby_broadcast_accumulator >= lobby_broadcast_interval) {
            broadcast_lobby_status(server);
            _lobby_broadcast_accumulator -= lobby_broadcast_interval;
        }
    } else {
        _pos_broadcast_accumulator += dt;
        if (_pos_broadcast_accumulator >= position_broadcast_interval) {
            broadcast_entity_positions(server);
            _pos_broadcast_accumulator -= position_broadcast_interval;
        }
        _level_broadcast_accumulator += dt;
        if (_level_broadcast_accumulator >= level_broadcast_interval) {
            broadcast_level_info(server);
            _level_broadcast_accumulator -= level_broadcast_interval;
        }
        check_level_completion(server);
        _powerup_broadcast_accumulator += dt;
        if (_powerup_broadcast_accumulator >= 0.2f) {
            broadcast_powerup_status(server);
            _powerup_broadcast_accumulator -= 0.2f;
        }
    }

    _cleanup_accumulator += dt;
    if (_cleanup_accumulator >= cleanup_interval) {
        auto dead_clients = server.remove_inactive_clients(std::chrono::seconds(90));
        for (int id : dead_clients) {
            _client_ready_status.erase(id);
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

void Game::check_level_completion(UDPServer& server) {
    auto& level_managers = _registry.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            if (lvl_mgr.level_completed && !_level_complete_waiting &&
                !_waiting_for_powerup_choice) {
                broadcast_level_complete(server);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                broadcast_powerup_selection(server);
                _level_complete_waiting = true;
                _waiting_for_powerup_choice = true;
                _level_complete_timer = 0.0f;
            }
        }
    }
}

void Game::broadcast_level_info(UDPServer& server) {
    auto& level_managers = _registry.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::LevelProgress;
            serializer << static_cast<uint8_t>(lvl_mgr.current_level);
            serializer << static_cast<uint16_t>(lvl_mgr.enemies_killed_this_level);
            serializer << static_cast<uint16_t>(lvl_mgr.enemies_needed_for_next_level);
            server.send_to_all(serializer.data());
            break;
        }
    }
}

void Game::handle_powerup_choice(int client_id, uint8_t powerup_choice, UDPServer& server) {
    auto player_opt = get_player_entity(client_id);
    if (!player_opt.has_value()) {
        std::cerr << "[Game] Cannot apply power-up: player not found for client " << client_id
                  << std::endl;
        return;
    }
    auto player = player_opt.value();
    if (powerup_choice == 1) {
        _registry.emplace_component<power_cannon>(player);
    } else if (powerup_choice == 2) {
        _registry.emplace_component<shield>(player);
    }
    _waiting_for_powerup_choice = false;
    broadcast_powerup_status(server);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    advance_level(server);
}

void Game::handle_powerup_activate(int client_id, UDPServer& server) {
    auto player_opt = get_player_entity(client_id);
    if (!player_opt.has_value()) {
        return;
    }
    auto player = player_opt.value();
    auto& cannon_opt = _registry.get_component<power_cannon>(player);
    if (cannon_opt.has_value()) {
        if (!cannon_opt->is_active()) {
            cannon_opt->activate();
            broadcast_powerup_status(server);
        }
        return;
    }
    auto& shield_opt = _registry.get_component<shield>(player);
    if (shield_opt.has_value()) {
        if (!shield_opt->is_active()) {
            shield_opt->activate();
            broadcast_powerup_status(server);
        }
        return;
    }
}

void Game::broadcast_powerup_selection(UDPServer& server) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpChoice;
    serializer << static_cast<uint8_t>(1);
    server.send_to_all(serializer.data());
}

void Game::broadcast_powerup_status(UDPServer& server) {
    for (const auto& [client_id, entity_id] : _client_entity_ids) {
        auto player = _registry.entity_from_index(entity_id);
        uint8_t powerup_type = 0;
        float time_remaining = 0.0f;
        auto& cannon_opt = _registry.get_component<power_cannon>(player);
        if (cannon_opt.has_value()) {
            powerup_type = 1;
            if (cannon_opt->is_active()) {
                time_remaining = cannon_opt->time_remaining;
            } else {
                time_remaining = 0.0f;
            }
        }
        auto& shield_opt = _registry.get_component<shield>(player);
        if (shield_opt.has_value()) {
            powerup_type = 2;
            if (shield_opt->is_active()) {
                time_remaining = shield_opt->time_remaining;
            } else {
                time_remaining = 0.0f;
            }
        }
        RType::BinarySerializer serializer;
        serializer << RType::MagicNumber::VALUE;
        serializer << RType::OpCode::PowerUpStatus;
        serializer << powerup_type;
        serializer << time_remaining;
        server.send_to_client(client_id, serializer.data());
    }
}

void Game::broadcast_level_complete(UDPServer& server) {
    auto& level_managers = _registry.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::LevelComplete;
            serializer << static_cast<uint8_t>(lvl_mgr.current_level);
            serializer << static_cast<uint8_t>(lvl_mgr.current_level + 1);
            server.send_to_all(serializer.data());
            break;
        }
    }
}

void Game::handle_weapon_upgrade_choice(int client_id, uint8_t upgrade_choice, UDPServer& server) {
    auto player_opt = get_player_entity(client_id);
    if (!player_opt.has_value()) {
        std::cerr << "[Game] Cannot apply upgrade: player not found for client " << client_id
                  << std::endl;
        return;
    }
    auto player = player_opt.value();
    auto& wpn_opt = _registry.get_component<weapon>(player);
    if (!wpn_opt.has_value()) {
        std::cerr << "[Game] Cannot apply upgrade: player has no weapon component" << std::endl;
        return;
    }
    WeaponUpgradeType upgrade_type = WeaponUpgradeType::None;
    std::string upgrade_name = "None";
    if (upgrade_choice == 1) {
        upgrade_type = WeaponUpgradeType::PowerShot;
        upgrade_name = "Power Shot";
    } else if (upgrade_choice == 2) {
        upgrade_type = WeaponUpgradeType::TripleShot;
        upgrade_name = "Triple Shot";
    }
    wpn_opt->apply_upgrade(upgrade_type);
    std::cout << "[Game] Client " << client_id << " chose upgrade: " << upgrade_name << std::endl;
    int players_ready = 0;
    int total_players = 0;
    for (const auto& [cid, entity_id] : _client_entity_ids) {
        total_players++;
        auto ent = _registry.entity_from_index(entity_id);
        auto wpn = _registry.get_component<weapon>(ent);
        if (wpn.has_value() && wpn->upgrade_type != WeaponUpgradeType::None) {
            players_ready++;
        }
    }
    if (players_ready == total_players && total_players > 0) {
        advance_level(server);
    }
}

void Game::advance_level(UDPServer& server) {
    auto& level_managers = _registry.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            lvl_mgr.advance_to_next_level();
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::LevelStart;
            serializer << static_cast<uint8_t>(lvl_mgr.current_level);
            server.send_to_all(serializer.data());
            break;
        }
    }
}
