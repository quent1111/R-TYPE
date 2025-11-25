#include "UDPServer.hpp"

#include <cstring>

UDPServer::UDPServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
      work_guard_(io_context.get_executor()),
      socket_(io_context),
      next_client_id_(1),
      running_(true) {
    try {
        socket_.open(asio::ip::udp::v6());
        socket_.set_option(asio::ip::v6_only(false));
        socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v6(), port));
        std::cout << "[Network] UDP Server listening on port " << port << " (Dual Stack)"
                  << std::endl;
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
            uint16_t magic_number;
            std::memcpy(&magic_number, recv_buffer_.data(), sizeof(uint16_t));

            if (magic_number == 0xB542) {
                std::vector<uint8_t> data(recv_buffer_.begin(),
                                          recv_buffer_.begin() + bytes_received);
                NetworkPacket packet(data, remote_endpoint_);

                register_client(remote_endpoint_);
                input_queue_.push(packet);

                // std::cout << "[Network] Received " << bytes_received << " bytes." << std::endl;
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

void UDPServer::queue_output_packet(const NetworkPacket& packet) {
    asio::post(io_context_, [this, packet]() {
        socket_.async_send_to(asio::buffer(packet.data), packet.sender,
                              [](std::error_code ec, std::size_t /*bytes_sent*/) {
                                  if (ec) {
                                      std::cerr << "[Error] Send failed: " << ec.message()
                                                << std::endl;
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

// Ensure we use a single, consistent output pipeline: queue packets and
// process them from the network loop. This avoids racing async_send_to calls
// from multiple threads and centralizes error handling.

void UDPServer::queue_output_packet(const NetworkPacket& packet) {
    output_queue_.push(packet);
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

void UDPServer::process_output_queue() {
    NetworkPacket packet;
    while (output_queue_.try_pop(packet)) {
        socket_.async_send_to(asio::buffer(packet.data), packet.sender,
                              [](std::error_code ec, std::size_t /*bytes_sent*/) {
                                  if (ec) {
                                      std::cerr << "[Error] Send failed: " << ec.message()
                                                << std::endl;
                                  }
                              });
    }
}
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto now = std::chrono::steady_clock::now();

    for (auto it = clients_.begin(); it != clients_.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_seen);
        if (elapsed > timeout) {
            std::cout << "[Network] Client timed out: ID=" << it->first << std::endl;
            it = clients_.erase(it);
        } else {
            ++it;
        }
    }
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
    io_context_.stop();
    socket_.close();
}