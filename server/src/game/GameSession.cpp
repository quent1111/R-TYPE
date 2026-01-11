#include "game/GameSession.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <memory>

extern std::atomic<bool> server_running;

namespace server {

GameSession::GameSession() {

    auto& reg = _engine.get_registry();
    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<collider>();
    reg.register_component<entity_tag>();
    reg.register_component<network_id>();
    reg.register_component<controllable>();
    reg.register_component<weapon>();
    reg.register_component<multishot>();
    reg.register_component<laser_beam>();
    reg.register_component<collision_box>();
    reg.register_component<multi_hitbox>();
    reg.register_component<damage_on_contact>();
    reg.register_component<player_tag>();
    reg.register_component<enemy_tag>();
    reg.register_component<boss_tag>();
    reg.register_component<projectile_tag>();
    reg.register_component<bounded_movement>();
    reg.register_component<explosion_tag>();
    reg.register_component<wave_manager>();
    reg.register_component<level_manager>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<power_cannon>();
    reg.register_component<shield>();
    reg.register_component<little_friend>();
    reg.register_component<missile_drone>();
    reg.register_component<damage_flash_component>();

    _engine.register_system(std::make_unique<ShootingSystem>());
    _engine.register_system(std::make_unique<EnemyShootingSystem>());
    _engine.register_system(std::make_unique<WaveSystem>());
    _engine.register_system(std::make_unique<MovementSystem>());
    _engine.register_system(std::make_unique<CollisionSystem>());
    _engine.register_system(std::make_unique<CleanupSystem>());

    _engine.init();

    auto level_mgr_entity = reg.spawn_entity();
    reg.emplace_component<level_manager>(level_mgr_entity);
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

    _game_broadcaster.broadcast_start_game(server, _lobby_client_ids);
    _game_phase = GamePhase::InGame;

    float start_x = 100.0f;
    for (const auto& [client_id, ready] : _client_ready_status) {
        _player_manager.create_player(_engine.get_registry(), _client_entity_ids, client_id, start_x, 300.0f);
        start_x += 50.0f;
    }

    std::cout << "[Game] Game started with " << _client_ready_status.size() << " players"
              << std::endl;

    _game_broadcaster.broadcast_level_start(server, 1, _lobby_client_ids);
}

void GameSession::broadcast_lobby_status(UDPServer& server) {
    _lobby_broadcaster.broadcast_lobby_status(server, _client_ready_status, _lobby_client_ids);
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
                    if (_waiting_for_powerup_choice) {
                        break;
                    }
                    auto player_opt =
                        _player_manager.get_player_entity(_engine.get_registry(), _client_entity_ids, client_id);
                    if (!player_opt.has_value()) {
                        float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                        float start_y = 300.0f;
                        auto player = _player_manager.create_player(_engine.get_registry(), _client_entity_ids,
                                                                    client_id, start_x, start_y);
                        std::cout << "[Game] New player connected (Implicit): Client " << client_id
                                  << " (Entity " << player.id() << ")" << std::endl;
                    }

                    std::vector<uint8_t> payload(
                        deserializer.data().begin() +
                            static_cast<std::ptrdiff_t>(deserializer.read_position()),
                        deserializer.data().end());
                    _input_handler.handle_player_input(_engine.get_registry(), _client_entity_ids, client_id,
                                                       payload);
                    break;
                }
                case RType::OpCode::Login: {
                    std::cout << "[Game] Login request received from client " << client_id
                              << std::endl;
                    auto player_opt =
                        _player_manager.get_player_entity(_engine.get_registry(), _client_entity_ids, client_id);
                    if (!player_opt.has_value()) {
                        _client_ready_status[client_id] = false;
                        std::cout << "[Game] Client " << client_id << " joined lobby" << std::endl;

                        RType::CompressionSerializer ack_serializer;
                        ack_serializer << RType::MagicNumber::VALUE;
                        ack_serializer << RType::OpCode::LoginAck;
                        ack_serializer << client_id;
                        ack_serializer.compress();

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
                            _engine.get_registry(), _client_entity_ids, client_id, upgrade_choice);
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
                        _powerup_handler.handle_powerup_choice(_engine.get_registry(), _client_entity_ids,
                                                               _players_who_chose_powerup,
                                                               client_id, powerup_choice);

                        int alive_players =
                            _powerup_handler.count_alive_players(_engine.get_registry(), _client_entity_ids);
                        if (static_cast<int>(_players_who_chose_powerup.size()) >= alive_players) {
                            std::cout << "[Game] All players have chosen! Advancing level..."
                                      << std::endl;
                            _waiting_for_powerup_choice = false;
                            _level_complete_waiting = false;
                            _players_who_chose_powerup.clear();

                            _powerup_broadcaster.broadcast_powerup_status(server, _engine.get_registry(),
                                                                          _client_entity_ids, _lobby_client_ids);
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
                    _powerup_handler.handle_powerup_activate(_engine.get_registry(), _client_entity_ids,
                                                             client_id, powerup_type);
                    _powerup_broadcaster.broadcast_powerup_status(server, _engine.get_registry(),
                                                                  _client_entity_ids, _lobby_client_ids);
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
            _game_broadcaster.broadcast_game_over(server, _lobby_client_ids);
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

    if (_player_manager.check_all_players_dead(_engine.get_registry(), _client_entity_ids)) {
        _waiting_for_game_over_reset = true;
        _game_over_timer = 0.0f;
        _game_over_broadcast_accumulator = 0.0f;
        _game_broadcaster.broadcast_game_over(server, _lobby_client_ids);
        return;
    }

    if (_waiting_for_powerup_choice) {
        auto& velocities = _engine.get_registry().get_components<velocity>();
        auto& player_tags = _engine.get_registry().get_components<player_tag>();
        for (size_t i = 0; i < velocities.size(); ++i) {
            if (velocities[i].has_value() && player_tags[i].has_value()) {
                velocities[i]->vx = 0.0f;
                velocities[i]->vy = 0.0f;
            }
        }
    }

    _engine.update(dt);

    _boss_manager.update_boss_behavior(
        _engine.get_registry(), _boss_entity, _client_entity_ids, _boss_animation_timer, _boss_shoot_timer,
        _boss_shoot_cooldown, _boss_animation_complete, _boss_entrance_complete, _boss_target_x,
        _boss_shoot_counter, dt);

    _boss_manager.update_homing_enemies(_engine.get_registry(), _client_entity_ids, dt);

    auto& cannons = _engine.get_registry().get_components<power_cannon>();
    auto& shields = _engine.get_registry().get_components<shield>();
    auto& laser_beams = _engine.get_registry().get_components<laser_beam>();
    auto& little_friends = _engine.get_registry().get_components<little_friend>();
    auto& positions = _engine.get_registry().get_components<position>();
    auto& player_powerups = _engine.get_registry().get_components<player_powerups_component>();

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

    for (std::size_t i = 0; i < laser_beams.size(); ++i) {
        if (laser_beams[i].has_value() && i < positions.size() && positions[i].has_value()) {
            auto& laser = laser_beams[i].value();
            auto& player_pos = positions[i].value();
            laser.update(dt);

            if (laser.active) {
                if (!laser.laser_entity.has_value()) {
                    entity laser_ent = _engine.get_registry().spawn_entity();

                    _engine.get_registry().add_component(laser_ent,
                        position{player_pos.x + 50.0f, player_pos.y});

                    _engine.get_registry().add_component(laser_ent, velocity{0.0f, 0.0f});

                    sprite_component sprite;
                    sprite.texture_path = "assets/laserbeam.png";
                    sprite.texture_rect_x = 0;
                    sprite.texture_rect_y = 0;
                    sprite.texture_rect_w = 2000;
                    sprite.texture_rect_h = 100;
                    sprite.scale = 1.0f;
                    _engine.get_registry().add_component(laser_ent, sprite);

                    _engine.get_registry().add_component(laser_ent, 
                        entity_tag{RType::EntityType::LaserBeam});

                    laser.laser_entity = laser_ent;
                }

                auto& laser_pos_opt = _engine.get_registry().get_component<position>(laser.laser_entity.value());
                if (laser_pos_opt.has_value()) {
                    laser_pos_opt->x = player_pos.x + 50.0f;
                    laser_pos_opt->y = player_pos.y;
                }

                if (laser.can_damage()) {
                    auto& enemies = _engine.get_registry().get_components<enemy_tag>();
                    auto& enemy_positions = _engine.get_registry().get_components<position>();
                    auto& healths = _engine.get_registry().get_components<health>();

                    for (std::size_t j = 0; j < enemies.size(); ++j) {
                        if (enemies[j].has_value() && j < enemy_positions.size() && 
                            enemy_positions[j].has_value() && j < healths.size() && healths[j].has_value()) {

                            auto& enemy_pos = enemy_positions[j].value();

                            if (enemy_pos.x >= player_pos.x && 
                                enemy_pos.x <= player_pos.x + 2000.0f &&
                                std::abs(enemy_pos.y - player_pos.y) <= 50.0f) {

                                int damage = static_cast<int>(laser.damage_per_second * 0.1f);
                                healths[j]->current -= damage;
                            }
                        }
                    }

                    laser.reset_damage_timer();
                }
            } else {
                if (laser.laser_entity.has_value()) {
                    try {
                        _engine.get_registry().kill_entity(laser.laser_entity.value());
                    } catch (...) {}
                    laser.laser_entity = std::nullopt;
                }
            }
        }
    }

    for (std::size_t i = 0; i < player_powerups.size(); ++i) {
        if (player_powerups[i].has_value()) {
            player_powerups[i]->update(dt);
        }
    }

    for (std::size_t i = 0; i < little_friends.size(); ++i) {
        if (little_friends[i].has_value() && i < positions.size() && positions[i].has_value()) {
            auto& lf = little_friends[i].value();
            auto& player_pos = positions[i].value();
            lf.update(dt);

            if (lf.is_active()) {
                if (lf.friend_entities.size() != static_cast<size_t>(lf.num_drones)) {
                    while (lf.friend_entities.size() > static_cast<size_t>(lf.num_drones)) {
                        if (lf.friend_entities.back().has_value()) {
                            try {
                                _engine.get_registry().kill_entity(lf.friend_entities.back().value());
                            } catch (...) {}
                        }
                        lf.friend_entities.pop_back();
                    }
                    while (lf.friend_entities.size() < static_cast<size_t>(lf.num_drones)) {
                        lf.friend_entities.push_back(std::nullopt);
                    }
                }

                for (std::size_t drone_idx = 0; drone_idx < static_cast<std::size_t>(lf.num_drones); ++drone_idx) {
                    if (!lf.friend_entities[drone_idx].has_value()) {
                        entity friend_ent = _engine.get_registry().spawn_entity();

                        _engine.get_registry().add_component(friend_ent, 
                            position{-200.0f, player_pos.y + 5.0f});

                        _engine.get_registry().add_component(friend_ent, velocity{0.0f, 0.0f});

                        sprite_component sprite;
                        sprite.texture_path = "assets/r-typesheet5.gif";
                        sprite.texture_rect_x = 495;
                        sprite.texture_rect_y = 0;
                        sprite.texture_rect_w = 33;
                        sprite.texture_rect_h = 32;
                        sprite.scale = 2.5f;
                        _engine.get_registry().add_component(friend_ent, sprite);

                        _engine.get_registry().add_component(friend_ent, 
                            entity_tag{RType::EntityType::SupportDrone});

                        lf.friend_entities[drone_idx] = friend_ent;
                    }
                }
            }

            if (lf.is_active()) {
                for (std::size_t drone_idx = 0; drone_idx < static_cast<std::size_t>(lf.num_drones); ++drone_idx) {
                    if (drone_idx >= lf.friend_entities.size() || !lf.friend_entities[drone_idx].has_value()) {
                        continue;
                    }

                    auto& friend_pos_opt = _engine.get_registry().get_component<position>(lf.friend_entities[drone_idx].value());
                    auto& friend_vel_opt = _engine.get_registry().get_component<velocity>(lf.friend_entities[drone_idx].value());
                    auto& friend_sprite_opt = _engine.get_registry().get_component<sprite_component>(lf.friend_entities[drone_idx].value());

                    float vertical_offset = 0.0f;
                    if (lf.num_drones == 2) {
                        vertical_offset = (drone_idx == 0) ? -40.0f : 40.0f;
                    }

                    float final_target_x = player_pos.x - 100.0f;
                    float final_target_y = player_pos.y + 5.0f + lf.get_vertical_offset() + vertical_offset;

                    float target_x, target_y;

                    if (!lf.entry_animation_complete) {
                        // Entry animation: lerp from far left to final position
                        float progress = lf.get_entry_progress();
                        float eased_progress = 1.0f - (1.0f - progress) * (1.0f - progress);

                        float start_x = -200.0f;
                        target_x = start_x + (final_target_x - start_x) * eased_progress;
                        target_y = final_target_y;
                    } else if (lf.exit_animation_started) {
                        // Exit animation: lerp from current position to off-screen left
                        float progress = lf.get_exit_progress();
                        float eased_progress = progress * progress;

                        float end_x = -200.0f;
                        target_x = final_target_x + (end_x - final_target_x) * eased_progress;
                        target_y = final_target_y;
                    } else {
                        target_x = final_target_x;
                        target_y = final_target_y;
                    }
                    
                    if (friend_pos_opt.has_value() && friend_vel_opt.has_value()) {
                        float old_x = friend_pos_opt->x;
                        float old_y = friend_pos_opt->y;
                        
                        float vx = (target_x - old_x) / dt;
                        float vy = (target_y - old_y) / dt;
                        
                        const float max_vel = 1000.0f;
                        if (std::abs(vx) > max_vel) vx = (vx > 0 ? max_vel : -max_vel);
                        if (std::abs(vy) > max_vel) vy = (vy > 0 ? max_vel : -max_vel);

                        friend_vel_opt->vx = vx;
                        friend_vel_opt->vy = vy;

                        friend_pos_opt->x = target_x;
                        friend_pos_opt->y = target_y;
                    }

                    if (friend_sprite_opt.has_value() && friend_vel_opt.has_value()) {
                        float vertical_velocity = friend_vel_opt->vy;

                        if (std::abs(vertical_velocity) > 10.0f) {
                            friend_sprite_opt->texture_rect_x = 462;
                        } else {
                            friend_sprite_opt->texture_rect_x = 495;
                        }
                    }

                    if (lf.can_shoot() && friend_pos_opt.has_value()) {
                    float nearest_dist = -1.0f;
                    float enemy_target_x = -1.0f;
                    float enemy_target_y = -1.0f;
                    bool found_enemy = false;

                    auto& enemy_tags = _engine.get_registry().get_components<enemy_tag>();
                    auto& enemy_positions = _engine.get_registry().get_components<position>();

                    for (std::size_t j = 0; j < enemy_tags.size(); ++j) {
                        if (enemy_tags[j].has_value() && j < enemy_positions.size() && 
                            enemy_positions[j].has_value()) {
                            auto& enemy_pos = enemy_positions[j].value();

                            float dx = enemy_pos.x - friend_pos_opt->x;
                            float dy = enemy_pos.y - friend_pos_opt->y;

                            if (dx > 0) {
                                if (std::abs(dy) < dx * 1.5f) {
                                    float dist = std::sqrt(dx * dx + dy * dy);

                                    if (nearest_dist < 0.0f || dist < nearest_dist) {
                                        nearest_dist = dist;
                                        enemy_target_x = enemy_pos.x;
                                        enemy_target_y = enemy_pos.y;
                                        found_enemy = true;
                                    }
                                }
                            }
                        }
                    }

                    auto& boss_tags = _engine.get_registry().get_components<boss_tag>();
                    auto& boss_positions = _engine.get_registry().get_components<position>();

                    for (std::size_t j = 0; j < boss_tags.size(); ++j) {
                        if (boss_tags[j].has_value() && j < boss_positions.size() && 
                            boss_positions[j].has_value()) {
                            auto& boss_pos = boss_positions[j].value();

                            float dx = boss_pos.x - friend_pos_opt->x;
                            float dy = boss_pos.y - friend_pos_opt->y;

                            if (dx > 0 && std::abs(dy) < dx * 1.5f) {
                                float dist = std::sqrt(dx * dx + dy * dy);

                                if (nearest_dist < 0.0f || dist < nearest_dist) {
                                    nearest_dist = dist;
                                    enemy_target_x = boss_pos.x;
                                    enemy_target_y = boss_pos.y;
                                    found_enemy = true;
                                }
                            }
                        }
                    }

                    if (found_enemy) {
                        float dx = enemy_target_x - friend_pos_opt->x;
                        float dy = enemy_target_y - friend_pos_opt->y;
                        float magnitude = std::sqrt(dx * dx + dy * dy);

                        if (magnitude > 0.0f) {
                            float projectile_speed = 500.0f;
                            float vx = (dx / magnitude) * projectile_speed;
                            float vy = (dy / magnitude) * projectile_speed;
                            ::createProjectile(_engine.get_registry(), 
                                friend_pos_opt->x + 20.0f, friend_pos_opt->y, 
                                vx, vy, lf.damage, WeaponUpgradeType::AllyMissile, false);
                        }
                    }

                    if (drone_idx == static_cast<std::size_t>(lf.num_drones) - 1) {
                        lf.reset_shoot_timer();
                    }
                    }
                }
            }

            if (!lf.is_active()) {
                for (auto& friend_entity_opt : lf.friend_entities) {
                    if (friend_entity_opt.has_value()) {
                        try {
                            _engine.get_registry().kill_entity(friend_entity_opt.value());
                            std::cout << "[LittleFriend] Despawned ally ship for player " << i << std::endl;
                        } catch (...) {}
                        friend_entity_opt = std::nullopt;
                    }
                }
                lf.friend_entities.clear();
            }
        }
    }

    auto& missile_drones = _engine.get_registry().get_components<missile_drone>();
    for (std::size_t i = 0; i < missile_drones.size(); ++i) {
        if (missile_drones[i].has_value() && i < positions.size() && positions[i].has_value()) {
            auto& md = missile_drones[i].value();
            auto& player_pos = positions[i].value();
            md.update(dt);

            if (md.is_active()) {
                if (md.drone_entities.size() != static_cast<size_t>(md.num_drones)) {
                    while (md.drone_entities.size() > static_cast<size_t>(md.num_drones)) {
                        if (md.drone_entities.back().has_value()) {
                            try {
                                _engine.get_registry().kill_entity(md.drone_entities.back().value());
                            } catch (...) {}
                        }
                        md.drone_entities.pop_back();
                    }
                    while (md.drone_entities.size() < static_cast<size_t>(md.num_drones)) {
                        md.drone_entities.push_back(std::nullopt);
                    }
                }

                for (std::size_t drone_idx = 0; drone_idx < static_cast<std::size_t>(md.num_drones); ++drone_idx) {
                    if (!md.drone_entities[drone_idx].has_value()) {
                        entity drone_ent = _engine.get_registry().spawn_entity();

                        _engine.get_registry().add_component(drone_ent, 
                            position{-200.0f, player_pos.y + 5.0f});

                        _engine.get_registry().add_component(drone_ent, velocity{0.0f, 0.0f});

                        sprite_component sprite;
                        sprite.texture_path = "assets/r-typesheet5.gif";
                        sprite.texture_rect_x = 495;
                        sprite.texture_rect_y = 0;
                        sprite.texture_rect_w = 33;
                        sprite.texture_rect_h = 32;
                        sprite.scale = 2.0f;
                        sprite.grayscale = true;
                        _engine.get_registry().add_component(drone_ent, sprite);

                        _engine.get_registry().add_component(drone_ent, 
                            entity_tag{RType::EntityType::MissileDrone});

                        md.drone_entities[drone_idx] = drone_ent;
                    }

                    auto& drone_pos_opt = _engine.get_registry().get_component<position>(md.drone_entities[drone_idx].value());

                    if (drone_pos_opt.has_value()) {
                        float vertical_offset = md.get_vertical_offset();

                        float target_x, target_y;

                        if (md.num_drones == 1) {
                            target_x = player_pos.x;
                            target_y = player_pos.y - 70.0f + vertical_offset;
                        } else if (md.num_drones == 2) {
                            float offset_x = (drone_idx == 0) ? -60.0f : 60.0f;
                            target_x = player_pos.x + offset_x;
                            target_y = player_pos.y - 70.0f + vertical_offset;
                        } else {
                            if (drone_idx == 0) {
                                target_x = player_pos.x;
                                target_y = player_pos.y - 100.0f + vertical_offset;
                            } else if (drone_idx == 1) {
                                target_x = player_pos.x - 60.0f;
                                target_y = player_pos.y - 60.0f + vertical_offset;
                            } else {
                                target_x = player_pos.x + 60.0f;
                                target_y = player_pos.y - 60.0f + vertical_offset;
                            }
                        }

                        drone_pos_opt->x += (target_x - drone_pos_opt->x) * dt * 5.0f;
                        drone_pos_opt->y += (target_y - drone_pos_opt->y) * dt * 5.0f;
                    }
                }

                if (md.can_shoot()) {
                    for (std::size_t drone_idx = 0; drone_idx < static_cast<std::size_t>(md.num_drones); ++drone_idx) {
                        if (md.drone_entities[drone_idx].has_value()) {
                            auto& drone_pos_opt = _engine.get_registry().get_component<position>(md.drone_entities[drone_idx].value());

                            if (drone_pos_opt.has_value()) {
                                auto& enemies = _engine.get_registry().get_components<enemy_tag>();
                                auto& enemy_positions = _engine.get_registry().get_components<position>();

                                std::vector<std::pair<float, float>> targets;

                                for (std::size_t j = 0; j < enemies.size() && targets.size() < static_cast<size_t>(md.missiles_per_volley); ++j) {
                                    if (enemies[j].has_value() && j < enemy_positions.size() && 
                                        enemy_positions[j].has_value()) {
                                        auto& enemy_pos = enemy_positions[j].value();

                                        if (enemy_pos.x > drone_pos_opt->x) {
                                            targets.push_back({enemy_pos.x, enemy_pos.y});
                                        }
                                    }
                                }

                                auto& boss_tags = _engine.get_registry().get_components<boss_tag>();
                                auto& boss_positions = _engine.get_registry().get_components<position>();

                                for (std::size_t j = 0; j < boss_tags.size() && targets.size() < static_cast<size_t>(md.missiles_per_volley); ++j) {
                                    if (boss_tags[j].has_value() && j < boss_positions.size() && 
                                        boss_positions[j].has_value()) {
                                        auto& boss_pos = boss_positions[j].value();

                                        if (boss_pos.x > drone_pos_opt->x) {
                                            targets.push_back({boss_pos.x, boss_pos.y});
                                        }
                                    }
                                }

                                for (const auto& [target_x, target_y] : targets) {
                                    float dx = target_x - drone_pos_opt->x;
                                    float dy = target_y - drone_pos_opt->y;
                                    float magnitude = std::sqrt(dx * dx + dy * dy);

                                    if (magnitude > 0.0f) {
                                        float projectile_speed = 600.0f;
                                        float vx = (dx / magnitude) * projectile_speed;
                                        float vy = (dy / magnitude) * projectile_speed;
                                        ::createProjectile(_engine.get_registry(), 
                                            drone_pos_opt->x + 20.0f, drone_pos_opt->y, 
                                            vx, vy, 20, WeaponUpgradeType::AllyMissile, false);
                                    }
                                }
                            }
                        }

                        if (drone_idx == static_cast<std::size_t>(md.num_drones) - 1) {
                            md.reset_shoot_timer();
                        }
                    }
                }
            } else {
                for (auto& drone_entity_opt : md.drone_entities) {
                    if (drone_entity_opt.has_value()) {
                        try {
                            _engine.get_registry().kill_entity(drone_entity_opt.value());
                        } catch (...) {}
                        drone_entity_opt = std::nullopt;
                    }
                }
                md.drone_entities.clear();
            }
        }
    }

    auto& damage_flashes = _engine.get_registry().get_components<damage_flash_component>();
    for (std::size_t i = 0; i < damage_flashes.size(); ++i) {
        if (damage_flashes[i].has_value()) {
            damage_flashes[i]->update(dt);
        }
    }

    auto& explosion_tags = _engine.get_registry().get_components<explosion_tag>();

    for (std::size_t i = 0; i < explosion_tags.size(); ++i) {
        if (explosion_tags[i].has_value()) {
            explosion_tags[i].value().elapsed += dt;
        }
    }

}

void GameSession::send_periodic_updates(UDPServer& server, float dt) {
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
            _lobby_broadcaster.broadcast_lobby_status(server, _client_ready_status, _lobby_client_ids);
            _lobby_broadcast_accumulator -= lobby_broadcast_interval;
        }
    } else {
        _pos_broadcast_accumulator += dt;
        if (_pos_broadcast_accumulator >= position_broadcast_interval) {
            _entity_broadcaster.broadcast_entity_positions(server, _engine.get_registry(), _client_entity_ids, _lobby_client_ids);
            _pos_broadcast_accumulator -= position_broadcast_interval;
        }
        _level_broadcast_accumulator += dt;
        if (_level_broadcast_accumulator >= level_broadcast_interval) {
            _game_broadcaster.broadcast_level_info(server, _engine.get_registry(), _lobby_client_ids);
            _level_broadcast_accumulator -= level_broadcast_interval;
        }
        check_level_completion(server);
        _powerup_broadcast_accumulator += dt;
        if (_powerup_broadcast_accumulator >= 0.2f) {
            _powerup_broadcaster.broadcast_powerup_status(server, _engine.get_registry(), _client_entity_ids, _lobby_client_ids);

            for (const auto& [client_id, entity_id] : _client_entity_ids) {
                auto player = _engine.get_registry().entity_from_index(entity_id);
                auto& powerups_opt = _engine.get_registry().get_component<player_powerups_component>(player);
                if (powerups_opt.has_value()) {
                    _powerup_broadcaster.broadcast_activable_slots(server, client_id, powerups_opt->activable_slots);
                }
            }

            _powerup_broadcast_accumulator -= 0.2f;
        }
    }

    _cleanup_accumulator += dt;
    if (_cleanup_accumulator >= cleanup_interval) {
        auto dead_clients = server.remove_inactive_clients(std::chrono::seconds(90));
        for (int id : dead_clients) {
            _client_ready_status.erase(id);
            _player_manager.remove_player(_engine.get_registry(), _client_entity_ids, id);
        }
        _cleanup_accumulator -= cleanup_interval;
    }
}

void GameSession::check_level_completion(UDPServer& server) {
    auto& level_managers = _engine.get_registry().get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            if (lvl_mgr.level_completed && !_level_complete_waiting &&
                !_waiting_for_powerup_choice) {
                std::cout << "[Game] Level " << static_cast<int>(lvl_mgr.current_level)
                          << " completed! Clearing enemies..." << std::endl;

                _players_who_chose_powerup.clear();
                _level_manager.clear_enemies_and_projectiles(_engine.get_registry(), _boss_entity);

                _game_broadcaster.broadcast_level_complete(server, _engine.get_registry(), _lobby_client_ids);
                for (const auto& [client_id, entity_id] : _client_entity_ids) {
                    auto player = _engine.get_registry().entity_from_index(entity_id);
                    auto health_opt = _engine.get_registry().get_component<health>(player);
                    if (health_opt.has_value() && health_opt->current > 0) {
                        auto cards = _powerup_handler.generate_card_choices(
                            _engine.get_registry(), _client_entity_ids, client_id);
                        _powerup_broadcaster.broadcast_powerup_cards(server, client_id, cards);
                    }
                }
                _powerup_broadcaster.broadcast_powerup_selection(server, _lobby_client_ids);
                _level_complete_waiting = true;
                _waiting_for_powerup_choice = true;
                _level_complete_timer = 0.0f;
            }
        }
    }
}

void GameSession::advance_level(UDPServer& server) {
    _level_manager.advance_level(_engine.get_registry());

    _player_manager.respawn_dead_players(_engine.get_registry(), _client_entity_ids);

    _entity_broadcaster.broadcast_entity_positions(server, _engine.get_registry(), _client_entity_ids, _lobby_client_ids);

    uint8_t current_level = _level_manager.get_current_level(_engine.get_registry());

    if (current_level == 5) {
        std::cout << "[SERVER] *** Level 5 - Spawning BOSS! ***" << std::endl;
        _boss_manager.spawn_boss_level_5(_engine.get_registry(), _boss_entity, _boss_animation_timer,
                                         _boss_shoot_timer, _boss_animation_complete,
                                         _boss_entrance_complete, _boss_target_x);

        _game_broadcaster.broadcast_boss_spawn(server, _lobby_client_ids);
    }

    _game_broadcaster.broadcast_level_start(server, current_level, _lobby_client_ids);
}

void GameSession::reset_game([[maybe_unused]] UDPServer& server) {
    auto& level_managers = _engine.get_registry().get_components<level_manager>();
    std::optional<size_t> level_mgr_idx = _level_manager.get_level_manager_index(_engine.get_registry());

    _client_entity_ids.clear();

    for (auto& [client_id, ready] : _client_ready_status) {
        ready = false;
    }

    auto& positions = _engine.get_registry().get_components<position>();
    auto& velocities = _engine.get_registry().get_components<velocity>();
    auto& healths = _engine.get_registry().get_components<health>();

    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i].has_value() || velocities[i].has_value() || healths[i].has_value()) {
            if (level_mgr_idx.has_value() && i == level_mgr_idx.value()) {
                continue;
            }
            auto ent = _engine.get_registry().entity_from_index(i);
            _engine.get_registry().kill_entity(ent);
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
            _engine.get_registry().remove_component<entity_tag>(boss_entity);
            _engine.get_registry().kill_entity(boss_entity);
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

void GameSession::update(UDPServer& server, float dt) {
    if (_game_phase != GamePhase::InGame) {
        return;
    }
    update_game_state(server, dt);
    send_periodic_updates(server, dt);
}

void GameSession::process_inputs(UDPServer& server) {
    (void)server;
}

void GameSession::remove_player(int client_id) {
    std::cout << "[GameSession] Removing player " << client_id << " from game session" << std::endl;

    _client_ready_status.erase(client_id);

    _players_who_chose_powerup.erase(client_id);

    auto it = _client_entity_ids.find(client_id);
    if (it != _client_entity_ids.end()) {
        auto entity = _engine.get_registry().entity_from_index(it->second);
        _engine.get_registry().kill_entity(entity);
        _client_entity_ids.erase(it);
    }
}

void GameSession::handle_packet(UDPServer& server, int client_id, const std::vector<uint8_t>& data) {
    if (data.empty() || data.size() < 3) {
        return;
    }

    try {
        RType::BinarySerializer deserializer(data);

        uint16_t magic;
        deserializer >> magic;

        if (!RType::MagicNumber::is_valid(magic)) {
            std::cerr << "[GameSession] Invalid magic number from client " << client_id << std::endl;
            return;
        }

        RType::OpCode opcode;
        deserializer >> opcode;

        switch (opcode) {
            case RType::OpCode::Input: {
                if (_game_phase != GamePhase::InGame) {
                    break;
                }
                if (_waiting_for_powerup_choice) {
                    break;
                }
                auto player_opt =
                    _player_manager.get_player_entity(_engine.get_registry(), _client_entity_ids, client_id);
                if (!player_opt.has_value()) {
                    float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                    float start_y = 300.0f;
                    auto player = _player_manager.create_player(_engine.get_registry(), _client_entity_ids,
                                                                client_id, start_x, start_y);
                    std::cout << "[GameSession] New player connected (Implicit): Client " << client_id
                              << " (Entity " << player.id() << ")" << std::endl;
                }

                std::vector<uint8_t> payload(
                    deserializer.data().begin() +
                        static_cast<std::ptrdiff_t>(deserializer.read_position()),
                    deserializer.data().end());
                _input_handler.handle_player_input(_engine.get_registry(), _client_entity_ids, client_id,
                                                   payload);
                break;
            }
            case RType::OpCode::PlayerReady: {
                uint8_t ready_val;
                deserializer >> ready_val;
                bool ready = (ready_val != 0);
                handle_player_ready(client_id, ready);
                _lobby_broadcaster.broadcast_lobby_status(server, _client_ready_status, _lobby_client_ids);
                check_start_game(server);
                break;
            }
            case RType::OpCode::WeaponUpgradeChoice: {
                uint8_t choice;
                deserializer >> choice;
                std::cout << "[GameSession] Client " << client_id << " chose weapon upgrade: "
                          << static_cast<int>(choice) << std::endl;
                _weapon_handler.handle_weapon_upgrade_choice(_engine.get_registry(), _client_entity_ids,
                                                            client_id, choice);
                break;
            }
            case RType::OpCode::PowerUpChoice: {
                uint8_t choice;
                deserializer >> choice;
                std::cout << "[GameSession] Client " << client_id << " chose powerup: "
                          << static_cast<int>(choice) << std::endl;
                _powerup_handler.handle_powerup_choice(_engine.get_registry(),
                                                      _client_entity_ids,
                                                      _players_who_chose_powerup, client_id, choice);
                int alive_players = _powerup_handler.count_alive_players(_engine.get_registry(), _client_entity_ids);
                if (static_cast<int>(_players_who_chose_powerup.size()) >= alive_players) {
                    std::cout << "[GameSession] All players have chosen! Advancing level..." << std::endl;
                    _waiting_for_powerup_choice = false;
                    _level_complete_waiting = false;
                    _players_who_chose_powerup.clear();
                    _powerup_broadcaster.broadcast_powerup_status(server, _engine.get_registry(),
                                                                  _client_entity_ids, _lobby_client_ids);
                    advance_level(server);
                }
                break;
            }
            case RType::OpCode::PowerUpActivate: {
                uint8_t powerup_type;
                deserializer >> powerup_type;
                std::cout << "[GameSession] Client " << client_id << " activated powerup: "
                          << static_cast<int>(powerup_type) << std::endl;
                _powerup_handler.handle_powerup_activate(_engine.get_registry(),
                                                        _client_entity_ids, client_id, powerup_type);
                break;
            }
            default:
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameSession] Error handling packet from client " << client_id << ": "
                  << e.what() << std::endl;
    }
}

}  // namespace server
