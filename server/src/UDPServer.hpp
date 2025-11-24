
#pragma once

#include <asio.hpp>
#include <map>
#include <queue>
#include <mutex>
#include <memory>
#include <vector>
#include <iostream>

#include "SafeQueue.hpp"
#include "NetworkPacket.hpp"
#include "ClientEndpoint.hpp"

class UDPServer {
private:
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::map<int, ClientEndpoint> clients_;
    std::mutex clients_mutex_;
    ThreadSafeQueue<NetworkPacket> input_queue_;
    ThreadSafeQueue<NetworkPacket> output_queue_;
    std::array<uint8_t, 1024> recv_buffer_;
    int next_client_id_;
    bool running_;

public:
    UDPServer(asio::io_context& io_context, unsigned short port);
    ~UDPServer();
    void start_receive();
    void handle_receive(std::error_code ec, std::size_t bytes_received);
    int register_client(const asio::ip::udp::endpoint& endpoint);
    void send_to_all(const std::vector<uint8_t>& data);
    void send_to_client(int client_id, const std::vector<uint8_t>& data);
    void send_to_endpoint(const asio::ip::udp::endpoint& endpoint, const std::vector<uint8_t>& data);
    bool get_input_packet(NetworkPacket& packet);
    void queue_output_packet(const NetworkPacket& packet);
    void process_output_queue();
    void remove_inactive_clients(std::chrono::seconds timeout);
    size_t get_client_count();
    size_t get_input_queue_size() const;
    size_t get_output_queue_size() const;
    std::map<int, ClientEndpoint> get_clients();
    void run_network_loop();
    void stop();
};
