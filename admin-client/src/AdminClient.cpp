#include "AdminClient.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include <iostream>
#include <sstream>
#include <cstring>

AdminClient::AdminClient(const std::string& host, unsigned short port)
    : _server_address(host), _server_port(port),
      _connected(false), _authenticated(false), _running(true) {
}

AdminClient::~AdminClient() {
    disconnect();
}

bool AdminClient::connect() {
    _socket.setBlocking(false);

    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::AdminLogin;
    serializer << std::string("PING");

    if (_socket.send(serializer.data().data(), serializer.data().size(),
                     _server_address, _server_port) != sf::Socket::Done) {
        std::cerr << "[AdminClient] Failed to send ping" << std::endl;
        return false;
    }

    _connected = true;
    _running = true;
    _receive_thread = std::thread(&AdminClient::receive_loop, this);

    std::cout << "[AdminClient] Connected to server" << std::endl;
    return true;
}

bool AdminClient::authenticate(const std::string& password) {
    {
        std::lock_guard<std::mutex> lock(_response_mutex);
        while (!_response_queue.empty()) {
            _response_queue.pop();
        }
    }

    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::AdminLogin;
    serializer << password;

    if (_socket.send(serializer.data().data(), serializer.data().size(),
                     _server_address, _server_port) != sf::Socket::Done) {
        std::cerr << "[AdminClient] Failed to send auth packet" << std::endl;
        return false;
    }

    std::cout << "[AdminClient] Auth packet sent, waiting for response..." << std::endl;

    for (int i = 0; i < 40; ++i) {
        sf::sleep(sf::milliseconds(50));

        if (has_response()) {
            std::string response = get_response();
            std::cout << "[AdminClient] Received response: " << response << std::endl;
            _authenticated = (response.find("OK") != std::string::npos);
            return _authenticated;
        }
    }

    std::cout << "[AdminClient] Auth timeout - no response received" << std::endl;
    return false;
}

void AdminClient::send_command(const std::string& command) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::AdminCommand;
    serializer << command;

    _socket.send(serializer.data().data(), serializer.data().size(),
                 _server_address, _server_port);
}

bool AdminClient::has_response() {
    std::lock_guard<std::mutex> lock(_response_mutex);
    return !_response_queue.empty();
}

std::string AdminClient::get_response() {
    std::lock_guard<std::mutex> lock(_response_mutex);
    if (_response_queue.empty()) {
        return "";
    }
    std::string response = _response_queue.front();
    _response_queue.pop();
    return response;
}

void AdminClient::receive_loop() {
    char buffer[65536];
    std::size_t received;
    sf::IpAddress sender;
    unsigned short port;

    while (_running) {
        sf::Socket::Status status = _socket.receive(buffer, sizeof(buffer), received, sender, port);

        if (status == sf::Socket::Done && received > 0) {
            try {
                RType::BinarySerializer deserializer(std::vector<uint8_t>(buffer, buffer + received));

                uint16_t magic;
                deserializer >> magic;

                if (!RType::MagicNumber::is_valid(magic)) {
                    continue;
                }

                RType::OpCode opcode;
                deserializer >> opcode;

                if (opcode == RType::OpCode::AdminLoginAck ||
                    opcode == RType::OpCode::AdminResponse) {
                    std::string response;
                    deserializer >> response;

                    std::lock_guard<std::mutex> lock(_response_mutex);
                    _response_queue.push(response);
                }
            } catch (...) {
                // Ignore invalid packets
            }
        }
        sf::sleep(sf::milliseconds(10));
    }
}

std::vector<PlayerInfo> AdminClient::get_players() {
    send_command("list-players");
    sf::sleep(sf::milliseconds(200));

    std::vector<PlayerInfo> players;
    if (has_response()) {
        std::string response = get_response();

        std::istringstream iss(response);
        std::string token;

        if (!std::getline(iss, token, '|')) return players;
        if (token != "PLAYERS") return players;

        if (!std::getline(iss, token, '|')) return players;

        while (std::getline(iss, token, '|')) {
            if (token.empty()) continue;

            std::istringstream player_stream(token);
            std::string part;
            PlayerInfo player;

            if (std::getline(player_stream, part, ';')) {
                player.id = std::stoi(part);
            }
            if (std::getline(player_stream, part, ';')) {
                player.address = part;
            }
            if (std::getline(player_stream, part, ';')) {
                player.port = std::stoi(part);
            }

            players.push_back(player);
        }
    }
    return players;
}

std::vector<LobbyInfo> AdminClient::get_lobbies() {
    send_command("list-lobbies");
    sf::sleep(sf::milliseconds(200));

    std::vector<LobbyInfo> lobbies;
    if (has_response()) {
        std::string response = get_response();

        std::istringstream iss(response);
        std::string token;

        if (!std::getline(iss, token, '|')) return lobbies;
        if (token != "LOBBIES") return lobbies;

        if (!std::getline(iss, token, '|')) return lobbies;

        while (std::getline(iss, token, '|')) {
            if (token.empty()) continue;

            std::istringstream lobby_stream(token);
            std::string part;
            LobbyInfo lobby;

            if (std::getline(lobby_stream, part, ';')) {
                lobby.id = std::stoi(part);
            }
            if (std::getline(lobby_stream, part, ';')) {
                lobby.name = part;
            }
            if (std::getline(lobby_stream, part, ';')) {
                lobby.current_players = std::stoi(part);
            }
            if (std::getline(lobby_stream, part, ';')) {
                lobby.max_players = std::stoi(part);
            }
            if (std::getline(lobby_stream, part, ';')) {
                lobby.state = std::stoi(part);
            }

            lobbies.push_back(lobby);
        }
    }
    return lobbies;
}

ServerStatus AdminClient::get_server_status() {
    send_command("status");
    sf::sleep(sf::milliseconds(200));

    ServerStatus status;
    status.uptime = "0h 0m 0s";
    status.player_count = 0;
    status.lobby_count = 0;

    if (has_response()) {
        std::string response = get_response();

        std::istringstream iss(response);
        std::string token;

        if (!std::getline(iss, token, '|')) return status;
        if (token != "STATUS") return status;

        if (std::getline(iss, token, '|')) {
            status.uptime = token;
        }

        if (std::getline(iss, token, '|')) {
            status.player_count = std::stoi(token);
        }

        if (std::getline(iss, token, '|')) {
            status.lobby_count = std::stoi(token);
        }
    }
    return status;
}

void AdminClient::disconnect() {
    _running = false;
    if (_receive_thread.joinable()) {
        _receive_thread.join();
    }
    _socket.unbind();
    _connected = false;
    _authenticated = false;
}

void AdminClient::parse_response(const std::string& response) {
    // TODO: Implement response parsing
}
