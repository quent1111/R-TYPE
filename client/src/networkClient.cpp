#include "NetworkClient.hpp"

NetworkClient::NetworkClient(const std::string& host, unsigned short port,
                             ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
                             ThreadSafeQueue<NetworkToGame::Message>& net_to_game)
    : running_(true), game_to_network_queue_(game_to_net), network_to_game_queue_(net_to_game) {
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        throw std::runtime_error("Impossible de créer le socket");
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct hostent* server = gethostbyname(host.c_str());
    if (server == nullptr) {
        close(socket_fd_);
        throw std::runtime_error("Hôte introuvable: " + host);
    }

    std::memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    std::memcpy(&server_addr_.sin_addr.s_addr, server->h_addr,
                static_cast<size_t>(server->h_length));
    server_addr_.sin_port = htons(port);

    std::cout << "[Client] Connecté au serveur " << host << ":" << port << std::endl;

    receiver_thread_ = std::thread(&NetworkClient::receive_loop, this);
    sender_thread_ = std::thread(&NetworkClient::send_loop, this);

    game_to_network_queue_.push(GameToNetwork::Message(GameToNetwork::MessageType::SendLogin));
}

void NetworkClient::receive_loop() {
    std::vector<uint8_t> buffer(65536);
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (running_) {
        ssize_t received = recvfrom(socket_fd_, buffer.data(), buffer.size(), 0,
                        reinterpret_cast<struct sockaddr*>(&from_addr),
                        &from_len);

        if (received > 0 && received >= 4) {
            uint16_t magic =
                static_cast<uint16_t>(buffer[0]) | (static_cast<uint16_t>(buffer[1]) << 8);

            if (magic == MAGIC_NUMBER && buffer[2] == 0x13) {
                decode_entities(buffer, received);
            }
        }
    }
}
void NetworkClient::send_loop() {
    while (running_) {
        GameToNetwork::Message msg(GameToNetwork::MessageType::SendLogin);

        game_to_network_queue_.wait_and_pop(msg);

        switch (msg.type) {
            case GameToNetwork::MessageType::SendLogin:
                send_login();
                break;

            case GameToNetwork::MessageType::SendInput:
                send_input(msg.input_mask);
                break;

            case GameToNetwork::MessageType::Disconnect:
                running_ = false;
                break;
        }
    }
}
void NetworkClient::decode_entities(const std::vector<uint8_t>& buffer, ssize_t received) {
    if (received < 4) return;

    try {
        RType::BinarySerializer deserializer(buffer);

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

            deserializer >> entity_id >> type_val >> x >> y >> vx >> vy;

            Entity entity;
            entity.id = entity_id;
            entity.type = type_val;
            entity.x = x;
            entity.y = y;
            entity.vx = vx;
            entity.vy = vy;

            entity.shape.setSize(sf::Vector2f(30, 30));
            entity.shape.setPosition(entity.x, entity.y);

            if (type_val == 0x01) {
                entity.shape.setFillColor(sf::Color::Green);
            } else if (type_val == 0x02) {
                entity.shape.setFillColor(sf::Color::Red);
            } else if (type_val == 0x03) {
                entity.shape.setFillColor(sf::Color::Yellow);
            } else {
                entity.shape.setFillColor(sf::Color::White);
            }

            new_entities[entity_id] = entity;
        }

        network_to_game_queue_.push(
            NetworkToGame::Message(NetworkToGame::MessageType::EntityUpdate, new_entities));

    } catch (const std::exception& e) {
        std::cerr << "[NetworkClient] Erreur de décodage paquet: " << e.what() << std::endl;
    }
}

void NetworkClient::send_login() {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Login;

    serializer << std::string("Player");

    sendto(socket_fd_, serializer.raw_data(), serializer.size(), 0,
           reinterpret_cast<const struct sockaddr*>(&server_addr_), sizeof(server_addr_));
    std::cout << "[Client] Asking connexion..." << std::endl;
}

void NetworkClient::send_input(uint8_t input_mask) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Input;
    serializer << input_mask;
    serializer << static_cast<uint32_t>(0);

    sendto(socket_fd_, serializer.raw_data(), serializer.size(), 0,
           reinterpret_cast<const struct sockaddr*>(&server_addr_), sizeof(server_addr_));
}

NetworkClient::~NetworkClient() {
    running_ = false;

    game_to_network_queue_.push(
        GameToNetwork::Message(GameToNetwork::MessageType::Disconnect)
    );

    if (sender_thread_.joinable()) {
        sender_thread_.join();
    }

    if (socket_fd_ >= 0) {
        shutdown(socket_fd_, SHUT_RDWR);
        close(socket_fd_);
        socket_fd_ = -1;
    }

    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }

    std::cout << "[Client] Disconnect from server" << std::endl;
}