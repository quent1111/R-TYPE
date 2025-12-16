#include "network/UDPServer.hpp"

#include <cstring>

#include <iostream>

namespace server {

UDPServer::UDPServer(asio::io_context& io_context, const std::string& bind_address,
                     unsigned short port)
    : io_context_(io_context),
      work_guard_(io_context.get_executor()),
      socket_(io_context),
      next_client_id_(1),
      running_(true) {
    try {
        if (bind_address.empty()) {
            socket_.open(asio::ip::udp::v6());
            socket_.set_option(asio::ip::v6_only(false));
            socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v6(), port));
            std::cout << "[Network] UDP Server listening on port " << port << " (Dual Stack)"
                      << std::endl;
        } else {
            asio::ip::address addr = asio::ip::make_address(bind_address);
            if (addr.is_v6()) {
                socket_.open(asio::ip::udp::v6());
                socket_.set_option(asio::ip::v6_only(false));
                socket_.bind(asio::ip::udp::endpoint(addr, port));
                std::cout << "[Network] UDP Server listening on " << bind_address << ":" << port
                          << " (IPv6 / dual-stack)" << std::endl;
            } else {
                socket_.open(asio::ip::udp::v4());
                socket_.bind(asio::ip::udp::endpoint(addr, port));
                std::cout << "[Network] UDP Server listening on " << bind_address << ":" << port
                          << " (IPv4)" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        try {
            socket_.close();
            socket_.open(asio::ip::udp::v4());
            socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
            std::cout << "[Network] UDP Server listening on port " << port << " (IPv4 Only)"
                      << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "[Error] Failed to bind UDP socket: " << ex.what() << std::endl;
            throw;
        }
    }

    start_receive();
}

UDPServer::~UDPServer() {
    stop();
}

void UDPServer::start_receive() {
    socket_.async_receive_from(
        asio::buffer(recv_buffer_), remote_endpoint_,
        [this](std::error_code ec, std::size_t bytes_recvd) { handle_receive(ec, bytes_recvd); });
}

void UDPServer::handle_receive(std::error_code ec, std::size_t bytes_received) {
    if (!ec && bytes_received > 0) {
        if (bytes_received >= 2) {
            uint16_t magic_number = static_cast<uint16_t>(recv_buffer_[0]) |
                                    (static_cast<uint16_t>(recv_buffer_[1]) << 8);
            if (magic_number == 0xB542) {
                std::vector<uint8_t> data(recv_buffer_.begin(),
                                          recv_buffer_.begin() + bytes_received);
                NetworkPacket packet(std::move(data), remote_endpoint_);

                register_client(remote_endpoint_);
                input_queue_.push(std::move(packet));
            } else {
                std::cerr << "[Security] Ignored packet with bad Magic Number from "
                          << remote_endpoint_ << std::endl;
            }
        }
    } else if (ec != asio::error::operation_aborted) {
        std::cerr << "[Error] Receive error: " << ec.message() << std::endl;
    }

    if (running_) {
        start_receive();
    }
}

void UDPServer::queue_output_packet(NetworkPacket packet) {
    asio::post(io_context_, [this, packet = std::move(packet)]() {
        socket_.async_send_to(
            asio::buffer(packet.data), packet.sender, [](std::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "[Error] Send failed: " << ec.message() << std::endl;
                }
            });
    });
}

void UDPServer::send_to_all(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (const auto& [id, client] : clients_) {
        NetworkPacket packet(data, client.endpoint);
        queue_output_packet(packet);
    }
}

void UDPServer::send_to_client(int client_id, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        NetworkPacket packet(data, it->second.endpoint);
        queue_output_packet(packet);
    }
}

void UDPServer::send_to_endpoint(const asio::ip::udp::endpoint& endpoint,
                                 const std::vector<uint8_t>& data) {
    NetworkPacket packet(data, endpoint);
    queue_output_packet(packet);
}

int UDPServer::register_client(const asio::ip::udp::endpoint& endpoint) {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    for (auto& [id, client] : clients_) {
        if (client.endpoint == endpoint) {
            client.last_seen = std::chrono::steady_clock::now();
            return id;
        }
    }

    int client_id = next_client_id_++;
    clients_[client_id] = ClientEndpoint(endpoint, client_id);
    std::cout << "[Network] New client registered: ID=" << client_id << " ("
              << endpoint.address().to_string() << ")" << std::endl;
    return client_id;
}

std::vector<int> UDPServer::remove_inactive_clients(std::chrono::seconds timeout) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    std::vector<int> removed_ids;
    auto now = std::chrono::steady_clock::now();

    for (auto it = clients_.begin(); it != clients_.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_seen);
        if (elapsed > timeout) {
            std::cout << "[Network] Client timed out: ID=" << it->first << std::endl;
            removed_ids.push_back(it->first);
            it = clients_.erase(it);
        } else {
            ++it;
        }
    }
    return removed_ids;
}

bool UDPServer::get_input_packet(NetworkPacket& packet) {
    return input_queue_.try_pop(packet);
}

size_t UDPServer::get_client_count() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

size_t UDPServer::get_input_queue_size() const {
    return input_queue_.size();
}

std::map<int, ClientEndpoint> UDPServer::get_clients() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_;
}

void UDPServer::run_network_loop() {
    std::cout << "[System] Network thread started." << std::endl;
    try {
        io_context_.run();
    } catch (const std::exception& e) {
        std::cerr << "[Fatal] Network loop crashed: " << e.what() << std::endl;
    }
    std::cout << "[System] Network thread stopped." << std::endl;
}

void UDPServer::stop() {
    running_ = false;
    work_guard_.reset();
    io_context_.stop();
    socket_.close();
}

}  // namespace server
