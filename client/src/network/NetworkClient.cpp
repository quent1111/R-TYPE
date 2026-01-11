#include "network/NetworkClient.hpp"
#include "../../src/Common/QuantizedSerializer.hpp"

NetworkClient::NetworkClient(const std::string& host, unsigned short port,
                             ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
                             ThreadSafeQueue<NetworkToGame::Message>& net_to_game)
    : io_context_(),
      socket_(io_context_),
      running_(true),
      game_to_network_queue_(game_to_net),
      network_to_game_queue_(net_to_game) {
    try {
        asio::ip::udp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port));
        server_endpoint_ = *endpoints.begin();
        socket_.open(asio::ip::udp::v4());

        std::cout << "[Client] Connecté au serveur " << host << ":" << port << std::endl;

        network_thread_ = std::thread(&NetworkClient::run, this);

        game_to_network_queue_.push(GameToNetwork::Message(GameToNetwork::MessageType::SendLogin));
    } catch (std::exception& e) {
        throw std::runtime_error("Impossible de se connecter au serveur: " + std::string(e.what()));
    }
}

void NetworkClient::start_receive() {
    socket_.async_receive_from(asio::buffer(recv_buffer_), server_endpoint_,
                               [this](std::error_code ec, std::size_t bytes_received) {
                                   handle_receive(ec, bytes_received);
                               });
}

void NetworkClient::handle_receive(std::error_code ec, std::size_t bytes_received) {
    if (!ec && bytes_received >= 4) {
        uint16_t magic =
            static_cast<uint16_t>(recv_buffer_[0]) | (static_cast<uint16_t>(recv_buffer_[1]) << 8);

        if (magic == MAGIC_NUMBER) {
            uint8_t opcode = recv_buffer_[2];
            std::vector<uint8_t> buffer(recv_buffer_.begin(),
                                        recv_buffer_.begin() + bytes_received);

            if (opcode == 0x02) {
                decode_login_ack(buffer, bytes_received);
            } else if (opcode == 0x13) {
                decode_entities(buffer, bytes_received);
            } else if (opcode == 0x21) {
                decode_lobby_status(buffer, bytes_received);
            } else if (opcode == 0x22) {
                decode_start_game(buffer, bytes_received);
            } else if (opcode == 0x23) {
                std::cout << "[NetworkClient] ListLobbies packet: bytes_received=" << bytes_received
                         << ", buffer.size()=" << buffer.size() << std::endl;
                std::cout << "[NetworkClient] Packet data: ";
                for (size_t i = 0; i < bytes_received && i < buffer.size(); ++i) {
                    std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
                }
                std::cout << std::dec << std::endl;
                std::vector<uint8_t> data(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(bytes_received));
                NetworkToGame::Message msg(NetworkToGame::MessageType::LobbyListUpdate);
                msg.raw_lobby_data = data;
                network_to_game_queue_.push(msg);
            } else if (opcode == 0x27) {
                try {
                    RType::BinarySerializer deserializer(buffer);
                    uint16_t magic_num;
                    uint8_t op;
                    deserializer >> magic_num >> op;
                    uint8_t success = 0;
                    int32_t lobby_id = -1;
                    deserializer >> success;
                    deserializer >> lobby_id;
                    std::cout << "[NetworkClient] LobbyJoined received (success=" << static_cast<int>(success) << ", id=" << lobby_id << ")" << std::endl;
                    NetworkToGame::Message msg(NetworkToGame::MessageType::LobbyJoined);
                    msg.lobby_join_success = (success != 0);
                    msg.lobby_joined_id = static_cast<int>(lobby_id);
                    network_to_game_queue_.push(msg);
                } catch (const std::exception& e) {
                    std::cerr << "[NetworkClient] Error decoding LobbyJoined: " << e.what() << std::endl;
                }
            } else if (opcode == 0x28) {
                std::cout << "[NetworkClient] LobbyLeft received" << std::endl;
            } else if (opcode == 0x30) {
                decode_level_start(buffer, bytes_received);
            } else if (opcode == 0x33) {
                decode_level_progress(buffer, bytes_received);
            } else if (opcode == 0x31) {
                decode_level_complete(buffer, bytes_received);
            } else if (opcode == 0x34) {
                decode_powerup_selection(buffer, bytes_received);
            } else if (opcode == 0x37) {
                decode_powerup_cards(buffer, bytes_received);
            } else if (opcode == 0x38) {
                decode_activable_slots(buffer, bytes_received);
            } else if (opcode == 0x36) {
                decode_powerup_status(buffer, bytes_received);
            } else if (opcode == 0x50) {
                decode_boss_spawn(buffer, bytes_received);
            } else if (opcode == 0x40) {
                decode_game_over(buffer, bytes_received);
            } else {
                std::cerr << "[NetworkClient] Warning: Unknown opcode 0x" << std::hex
                          << static_cast<int>(opcode) << std::dec << std::endl;
            }
        } else {
            std::cerr << "[NetworkClient] Error: Bad magic number 0x" << std::hex << magic
                      << std::dec << std::endl;
        }
    }

    if (running_) {
        start_receive();
    }
}

void NetworkClient::receive_loop() {}

void NetworkClient::send_loop() {
    while (running_) {
        GameToNetwork::Message msg(GameToNetwork::MessageType::SendLogin);

        game_to_network_queue_.wait_and_pop(msg);

        if (!running_) {
            break;
        }

        switch (msg.type) {
            case GameToNetwork::MessageType::SendLogin:
                send_login();
                break;

            case GameToNetwork::MessageType::SendInput:
                send_input(msg.input_mask);
                break;

            case GameToNetwork::MessageType::SendReady:
                send_ready(msg.ready_status);
                break;

            case GameToNetwork::MessageType::SendPowerUpChoice:
                send_powerup_choice(msg.powerup_choice_value);
                break;

            case GameToNetwork::MessageType::SendPowerUpActivate:
                send_powerup_activate(msg.powerup_activate_type);
                break;

            case GameToNetwork::MessageType::RawPacket:
                if (!msg.raw_data.empty()) {
                    socket_.send_to(asio::buffer(msg.raw_data), server_endpoint_);
                }
                break;

            case GameToNetwork::MessageType::Disconnect:
                running_ = false;
                break;
        }
    }
}

void NetworkClient::decode_login_ack(const std::vector<uint8_t>& buffer, std::size_t received) {
    if (received < 7)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);

        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint32_t network_id;
        deserializer >> network_id;

        my_network_id_ = network_id;
        std::cout << "[NetworkClient] Received my network ID: " << my_network_id_ << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Erreur de décodage LoginAck: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_entities(const std::vector<uint8_t>& buffer, std::size_t received) {
    if (received < 4)
        return;

    try {
        RType::QuantizedSerializer deserializer(buffer);

        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint8_t entity_count;
        deserializer >> entity_count;

        std::map<uint32_t, Entity> new_entities;

        for (int i = 0; i < entity_count; ++i) {
            uint32_t entity_id;
            uint8_t type_val;
            float x, y, vx, vy;

            deserializer >> entity_id >> type_val;

            deserializer.read_position(x, y);
            deserializer.read_velocity(vx, vy);

            Entity entity;
            entity.id = entity_id;
            entity.type = type_val;
            entity.x = x;
            entity.y = y;
            entity.vx = vx;
            entity.vy = vy;

            if (type_val == 0x01) {  // Player
                int current_health, max_health;
                deserializer.read_quantized_health(current_health, max_health);  // 2 bytes au lieu de 8
                entity.health = current_health;
                entity.max_health = max_health;
            } else if (type_val == 0x08) {  // Boss
                int current_health, max_health;
                deserializer.read_quantized_health(current_health, max_health);  // 2 bytes au lieu de 8
                entity.health = current_health;
                entity.max_health = max_health;
            }

            new_entities[entity_id] = entity;
        }

        auto msg = NetworkToGame::Message(NetworkToGame::MessageType::EntityUpdate, new_entities);
        msg.my_network_id = my_network_id_;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Erreur de décodage paquet: " << e.what() << std::endl;
    }
}

void NetworkClient::send_login() {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Login;
    serializer << std::string("Player");

    socket_.async_send_to(asio::buffer(serializer.raw_data(), serializer.size()), server_endpoint_,
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr << "[Client] Error sending login: " << ec.message()
                                            << std::endl;
                              }
                          });
    std::cout << "[Client] Asking connexion..." << std::endl;
}

void NetworkClient::send_input(uint8_t input_mask) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Input;
    serializer << input_mask;
    serializer << static_cast<uint32_t>(0);

    socket_.async_send_to(asio::buffer(serializer.raw_data(), serializer.size()), server_endpoint_,
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr << "[Client] Error sending input: " << ec.message()
                                            << std::endl;
                              }
                          });
}

void NetworkClient::send_ready(bool ready) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PlayerReady;
    serializer << static_cast<uint8_t>(ready ? 1 : 0);

    socket_.async_send_to(asio::buffer(serializer.raw_data(), serializer.size()), server_endpoint_,
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr << "[Client] Error sending ready: " << ec.message()
                                            << std::endl;
                              }
                          });
}

void NetworkClient::decode_lobby_status(const std::vector<uint8_t>& buffer, std::size_t received) {
    if (received < 5)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);
        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint8_t total_players, ready_players;
        deserializer >> total_players >> ready_players;

        NetworkToGame::Message msg(NetworkToGame::MessageType::LobbyStatus);
        msg.total_players = total_players;
        msg.ready_players = ready_players;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding lobby status: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_start_game(const std::vector<uint8_t>& /*buffer*/,
                                      std::size_t /*received*/) {
    NetworkToGame::Message msg(NetworkToGame::MessageType::StartGame);
    network_to_game_queue_.push(msg);
}

void NetworkClient::decode_level_start(const std::vector<uint8_t>& buffer, std::size_t received) {
    if (received < 4)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);
        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint8_t level;
        deserializer >> level;

        NetworkToGame::Message msg(NetworkToGame::MessageType::LevelStart);
        msg.level = level;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding level start: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_level_progress(const std::vector<uint8_t>& buffer,
                                          std::size_t received) {
    if (received < 8)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);
        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint8_t level;
        uint16_t kills, needed;
        deserializer >> level >> kills >> needed;

        NetworkToGame::Message msg(NetworkToGame::MessageType::LevelProgress);
        msg.level = level;
        msg.kills = kills;
        msg.enemies_needed = needed;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding level progress: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_level_complete(const std::vector<uint8_t>& buffer,
                                          std::size_t received) {
    if (received < 4)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);
        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint8_t level;
        deserializer >> level;

        NetworkToGame::Message msg(NetworkToGame::MessageType::LevelComplete);
        msg.level = level;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding level complete: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_powerup_selection([[maybe_unused]] const std::vector<uint8_t>& buffer,
                                             std::size_t received) {
    if (received < 4)
        return;

    try {
        NetworkToGame::Message msg(NetworkToGame::MessageType::PowerUpSelection);
        msg.show_powerup_selection = true;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding power-up selection: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_powerup_cards(const std::vector<uint8_t>& buffer,
                                         std::size_t received) {
    if (received < 4)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);

        uint16_t magic;
        uint8_t opcode;
        uint8_t count;

        deserializer >> magic;
        deserializer >> opcode;
        deserializer >> count;

        NetworkToGame::Message msg(NetworkToGame::MessageType::PowerUpCards);
        msg.powerup_cards.clear();

        for (uint8_t i = 0; i < count && i < 3; ++i) {
            uint8_t card_id, card_level;
            deserializer >> card_id;
            deserializer >> card_level;

            NetworkToGame::Message::PowerUpCard card;
            card.id = card_id;
            card.level = card_level;
            msg.powerup_cards.push_back(card);
        }

        msg.show_powerup_selection = true;
        network_to_game_queue_.push(msg);

        std::cout << "[NetworkClient] Received " << static_cast<int>(count) 
                  << " power-up cards" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding power-up cards: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_activable_slots(const std::vector<uint8_t>& buffer,
                                           std::size_t received) {
    if (received < 5)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);
        uint16_t magic;
        uint8_t opcode;

        deserializer >> magic;
        deserializer >> opcode;

        NetworkToGame::Message msg(NetworkToGame::MessageType::ActivableSlots);
        msg.activable_slots.clear();

        for (int slot_idx = 0; slot_idx < 2; ++slot_idx) {
            NetworkToGame::Message::ActivableSlotData slot_data;

            bool has_powerup;
            deserializer >> has_powerup;
            slot_data.has_powerup = has_powerup;

            if (has_powerup) {
                uint8_t powerup_id, level;
                float time_remaining, cooldown_remaining;
                bool is_active;

                deserializer >> powerup_id;
                deserializer >> level;
                deserializer >> time_remaining;
                deserializer >> cooldown_remaining;
                deserializer >> is_active;

                slot_data.powerup_id = powerup_id;
                slot_data.level = level;
                slot_data.time_remaining = time_remaining;
                slot_data.cooldown_remaining = cooldown_remaining;
                slot_data.is_active = is_active;
            }

            msg.activable_slots.push_back(slot_data);
        }

        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding activable slots: " << e.what() << std::endl;
    }
}

void NetworkClient::decode_powerup_status(const std::vector<uint8_t>& buffer,
                                          std::size_t received) {
    if (received < 12)
        return;

    try {
        RType::BinarySerializer deserializer(buffer);
        uint16_t magic;
        uint8_t opcode;
        deserializer >> magic >> opcode;

        uint32_t player_id;
        uint8_t powerup_type;
        float time_remaining;
        deserializer >> player_id >> powerup_type >> time_remaining;

        NetworkToGame::Message msg(NetworkToGame::MessageType::PowerUpStatus);
        msg.powerup_player_id = player_id;
        msg.powerup_type = powerup_type;
        msg.powerup_time_remaining = time_remaining;
        network_to_game_queue_.push(msg);

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Error decoding power-up status: " << e.what() << std::endl;
    }
}

void NetworkClient::send_powerup_choice(uint8_t choice) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpChoice;
    serializer << choice;

    socket_.async_send_to(asio::buffer(serializer.raw_data(), serializer.size()), server_endpoint_,
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr
                                      << "[Client] Error sending powerup choice: " << ec.message()
                                      << std::endl;
                              }
                          });
}

void NetworkClient::send_powerup_activate(uint8_t powerup_type) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpActivate;
    serializer << powerup_type;

    socket_.async_send_to(asio::buffer(serializer.raw_data(), serializer.size()), server_endpoint_,
                          [](std::error_code ec, std::size_t) {
                              if (ec) {
                                  std::cerr
                                      << "[Client] Error sending powerup activate: " << ec.message()
                                      << std::endl;
                              }
                          });
}

void NetworkClient::decode_game_over([[maybe_unused]] const std::vector<uint8_t>& buffer,
                                     [[maybe_unused]] std::size_t received) {
    NetworkToGame::Message msg(NetworkToGame::MessageType::GameOver);
    network_to_game_queue_.push(msg);
}

void NetworkClient::decode_boss_spawn([[maybe_unused]] const std::vector<uint8_t>& buffer,
                                      [[maybe_unused]] std::size_t received) {
    std::cout << "[NetworkClient] Boss Spawn received! Triggering music & roar..." << std::endl;
    NetworkToGame::Message msg(NetworkToGame::MessageType::BossSpawn);
    network_to_game_queue_.push(msg);
}

void NetworkClient::run() {
    start_receive();

    std::thread send_thread([this]() { send_loop(); });

    io_context_.run();

    if (send_thread.joinable()) {
        send_thread.join();
    }
}

void NetworkClient::stop() {
    running_ = false;

    game_to_network_queue_.push(GameToNetwork::Message(GameToNetwork::MessageType::Disconnect));

    io_context_.stop();
}

NetworkClient::~NetworkClient() {
    running_ = false;

    game_to_network_queue_.push(GameToNetwork::Message(GameToNetwork::MessageType::Disconnect));

    io_context_.stop();

    if (network_thread_.joinable()) {
        network_thread_.join();
    }

    std::cout << "[Client] Disconnect from server" << std::endl;
}
