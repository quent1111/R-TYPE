#include "NetworkClient.hpp"

NetworkClient::NetworkClient(const std::string& host, unsigned short port,
                             ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
                             ThreadSafeQueue<NetworkToGame::Message>& net_to_game)
    : running_(true)
    , game_to_network_queue_(game_to_net)
    , network_to_game_queue_(net_to_game)
{
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
    std::memcpy(&server_addr_.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr_.sin_port = htons(port);

    std::cout << "[Client] Connecté au serveur " << host << ":" << port << std::endl;

    receiver_thread_ = std::thread(&NetworkClient::receive_loop, this);
    sender_thread_ = std::thread(&NetworkClient::send_loop, this);

    game_to_network_queue_.push(GameToNetwork::Message(GameToNetwork::MessageType::SendLogin));
}

void NetworkClient::receive_loop()
{
    std::vector<uint8_t> buffer(4096);
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (running_) {
        ssize_t received = recvfrom(socket_fd_, buffer.data(), buffer.size(), 0,
                                    (struct sockaddr*)&from_addr, &from_len);

        if (received > 0 && received >= 4) {
            uint16_t magic = static_cast<uint16_t>(buffer[0]) |
                            (static_cast<uint16_t>(buffer[1]) << 8);

            if (magic == MAGIC_NUMBER && buffer[2] == 0x13) {
                decode_entities(buffer, received);
            }
        }
    }
}

void NetworkClient::send_loop()
{
    while (running_) {
        GameToNetwork::Message msg(GameToNetwork::MessageType::SendLogin);

        if (game_to_network_queue_.try_pop(msg)) {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void NetworkClient::decode_entities(const std::vector<uint8_t>& buffer, ssize_t received)
{
    size_t offset = 3;
    uint8_t entity_count = buffer[offset];
    offset += 1;

    std::map<uint32_t, Entity> new_entities;

    for (int i = 0; i < entity_count && offset + 21 <= static_cast<size_t>(received); i++) {
        uint32_t entity_id = static_cast<uint32_t>(buffer[offset]) |
                            (static_cast<uint32_t>(buffer[offset + 1]) << 8) |
                            (static_cast<uint32_t>(buffer[offset + 2]) << 16) |
                            (static_cast<uint32_t>(buffer[offset + 3]) << 24);
        offset += 4;

        uint8_t type = buffer[offset];
        offset += 1;

        Entity entity;
        entity.id = entity_id;
        entity.type = type;

        std::memcpy(&entity.x, &buffer[offset], sizeof(float));
        offset += 4;
        std::memcpy(&entity.y, &buffer[offset], sizeof(float));
        offset += 4;
        std::memcpy(&entity.vx, &buffer[offset], sizeof(float));
        offset += 4;
        std::memcpy(&entity.vy, &buffer[offset], sizeof(float));
        offset += 4;

        entity.shape.setSize(sf::Vector2f(30, 30));
        entity.shape.setPosition(entity.x, entity.y);

        if (type == 0x01) { // PLAYER
            entity.shape.setFillColor(sf::Color::Green);
        } else if (type == 0x02) { // ENEMY
            entity.shape.setFillColor(sf::Color::Red);
        } else if (type == 0x03) { // PROJECTILE
            entity.shape.setFillColor(sf::Color::Yellow);
        }

        new_entities[entity_id] = entity;
    }

    network_to_game_queue_.push(NetworkToGame::Message(
        NetworkToGame::MessageType::EntityUpdate,
        new_entities
    ));
}

void NetworkClient::send_login()
{
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(MAGIC_NUMBER & 0xFF));
    packet.push_back(static_cast<uint8_t>((MAGIC_NUMBER >> 8) & 0xFF));
    packet.push_back(0x01);
    sendto(socket_fd_, packet.data(), packet.size(), 0,
            (struct sockaddr*)&server_addr_, sizeof(server_addr_));
    std::cout << "[Client] Asking connexion..." << std::endl;
}

void NetworkClient::send_input(uint8_t input_mask)
{
    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(MAGIC_NUMBER & 0xFF));
    packet.push_back(static_cast<uint8_t>((MAGIC_NUMBER >> 8) & 0xFF));
    packet.push_back(0x10);
    packet.push_back(input_mask);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    packet.push_back(0);
    sendto(socket_fd_, packet.data(), packet.size(), 0,
            (struct sockaddr*)&server_addr_, sizeof(server_addr_));
}

NetworkClient::~NetworkClient()
{
    running_ = false;

    if (sender_thread_.joinable()) {
        sender_thread_.join();
    }
    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }
    if (socket_fd_ >= 0) {
        close(socket_fd_);
    }

    std::cout << "[Client] Disconnect from server" << std::endl;
}