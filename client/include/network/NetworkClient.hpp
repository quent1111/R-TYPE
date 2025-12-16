#pragma once

#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "common/SafeQueue.hpp"
#include "game/Entity.hpp"
#include "network/Messages.hpp"

#include <SFML/Graphics.hpp>
#include <asio.hpp>
#include <cstdint>

#include <atomic>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class NetworkClient {
private:
    asio::io_context io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    std::array<uint8_t, 65536> recv_buffer_;
    static constexpr uint16_t MAGIC_NUMBER = 0xB542;
    std::atomic<bool> running_;
    std::thread network_thread_;

    ThreadSafeQueue<GameToNetwork::Message>& game_to_network_queue_;
    ThreadSafeQueue<NetworkToGame::Message>& network_to_game_queue_;

    uint32_t my_network_id_ = 0;

    void start_receive();
    void handle_receive(std::error_code ec, std::size_t bytes_received);

public:
    NetworkClient(const std::string& host, unsigned short port,
                  ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
                  ThreadSafeQueue<NetworkToGame::Message>& net_to_game);

    void receive_loop();
    void send_loop();
    void decode_entities(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_login_ack(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_lobby_status(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_start_game(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_level_start(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_level_progress(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_level_complete(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_powerup_selection(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_powerup_status(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_boss_spawn(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_game_over(const std::vector<uint8_t>& buffer, std::size_t received);
    void send_login();
    void send_input(uint8_t input_mask);
    void send_ready(bool ready);
    void send_powerup_choice(uint8_t choice);
    void send_powerup_activate();
    void stop();
    void run();

    uint32_t get_my_network_id() const { return my_network_id_; }

    ~NetworkClient();
};
