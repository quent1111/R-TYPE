#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>

struct PlayerInfo {
    int id;
    std::string address;
    int port;
};

struct LobbyInfo {
    int id;
    std::string name;
    int current_players;
    int max_players;
    int state;
};

struct ServerStatus {
    std::string uptime;
    int player_count;
    int lobby_count;
};

class AdminClient {
public:
    AdminClient(const std::string& host, unsigned short port);
    ~AdminClient();

    bool connect();
    bool authenticate(const std::string& password);
    bool is_authenticated() const { return _authenticated; }
    bool is_connected() const { return _connected; }

    void send_command(const std::string& command);
    bool has_response();
    std::string get_response();

    std::vector<PlayerInfo> get_players();
    std::vector<LobbyInfo> get_lobbies();
    ServerStatus get_server_status();

    void disconnect();

private:
    sf::UdpSocket _socket;
    sf::IpAddress _server_address;
    unsigned short _server_port;

    bool _connected;
    bool _authenticated;
    std::atomic<bool> _running;

    std::queue<std::string> _response_queue;
    std::mutex _response_mutex;

    std::thread _receive_thread;

    void receive_loop();
    void parse_response(const std::string& response);
};
