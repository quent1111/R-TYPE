#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <thread>
#include <atomic>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "SafeQueue.hpp"
#include "Messages.hpp"
#include "Entity.hpp"

class NetworkClient {
private:
    int socket_fd_;
    struct sockaddr_in server_addr_;
    static constexpr uint16_t MAGIC_NUMBER = 0xB542;
    std::atomic<bool> running_;
    std::thread receiver_thread_;
    std::thread sender_thread_;

    ThreadSafeQueue<GameToNetwork::Message>& game_to_network_queue_;
    ThreadSafeQueue<NetworkToGame::Message>& network_to_game_queue_;

public:
    NetworkClient(const std::string& host, unsigned short port,
                  ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
                  ThreadSafeQueue<NetworkToGame::Message>& net_to_game);

    void receive_loop();
    void send_loop();
    void decode_entities(const std::vector<uint8_t>& buffer, ssize_t received);
    void send_login();
    void send_input(uint8_t input_mask);

    ~NetworkClient();
};
