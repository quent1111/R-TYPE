#pragma once

#include "ClientEndpoint.hpp"
#include "NetworkPacket.hpp"
#include "SafeQueue.hpp"

#include <asio.hpp>

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <string>

class UDPServer {
private:
    asio::io_context& io_context_;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::map<int, ClientEndpoint> clients_;
    std::mutex clients_mutex_;
    ThreadSafeQueue<NetworkPacket> input_queue_;
    std::array<uint8_t, 65536> recv_buffer_;
    int next_client_id_;
    bool running_;

public:
    UDPServer(asio::io_context& io_context, const std::string& bind_address, unsigned short port);
    ~UDPServer();

    void start_receive();
    void handle_receive(std::error_code ec, std::size_t bytes_received);

    int register_client(const asio::ip::udp::endpoint& endpoint);
    std::vector<int> remove_inactive_clients(std::chrono::seconds timeout);
    size_t get_client_count();
    std::map<int, ClientEndpoint> get_clients();

    void send_to_all(const std::vector<uint8_t>& data);
    void send_to_client(int client_id, const std::vector<uint8_t>& data);
    void send_to_endpoint(const asio::ip::udp::endpoint& endpoint,
                          const std::vector<uint8_t>& data);

    bool get_input_packet(NetworkPacket& packet);
    void queue_output_packet(NetworkPacket packet);

    size_t get_input_queue_size() const;

    void run_network_loop();
    void stop();
};
