#include "network/UDPServer.hpp"

#include "../../src/Common/CompressionSerializer.hpp"

#include <cstring>

#include <iostream>

namespace server {

UDPServer::UDPServer(asio::io_context& io_context, const std::string& bind_address,
                     unsigned short port)
    : io_context_(io_context), next_client_id_(1), running_(true) {
    try {
        recv_buffer_ = std::make_unique<std::vector<uint8_t>>(65536);
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to allocate recv_buffer: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[Error] Unknown error allocating recv_buffer" << std::endl;
        throw;
    }

    try {
        work_guard_ = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
            io_context.get_executor());
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to create work_guard: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[Error] Unknown error creating work_guard" << std::endl;
        throw;
    }

    try {
        socket_ = std::make_unique<asio::ip::udp::socket>(io_context);

        if (bind_address.empty()) {
            socket_->open(asio::ip::udp::v6());
            socket_->set_option(asio::ip::v6_only(false));
            socket_->bind(asio::ip::udp::endpoint(asio::ip::udp::v6(), port));
            std::cout << "[Network] UDP Server listening on port " << port << " (Dual Stack)"
                      << std::endl;
        } else {
            asio::ip::address addr = asio::ip::make_address(bind_address);
            if (addr.is_v6()) {
                socket_->open(asio::ip::udp::v6());
                socket_->set_option(asio::ip::v6_only(false));
                socket_->bind(asio::ip::udp::endpoint(addr, port));
                std::cout << "[Network] UDP Server listening on " << bind_address << ":" << port
                          << " (IPv6 / dual-stack)" << std::endl;
            } else {
                socket_->open(asio::ip::udp::v4());
                socket_->bind(asio::ip::udp::endpoint(addr, port));
                std::cout << "[Network] UDP Server listening on " << bind_address << ":" << port
                          << " (IPv4)" << std::endl;
            }
        }
    } catch (const std::system_error& e) {
        std::cerr << "[Error] System error in socket creation/bind: " << e.what()
                  << " (code: " << e.code() << ")" << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "[Error] Exception in socket creation/bind: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[Error] Unknown error in socket creation/bind" << std::endl;
        throw;
    }

    start_receive();

    retry_thread_ = std::thread(&UDPServer::retry_thread_loop, this);
}

UDPServer::~UDPServer() {
    stop();

    if (retry_thread_.joinable()) {
        retry_thread_.join();
    }
}

void UDPServer::start_receive() {
    socket_->async_receive_from(
        asio::buffer(*recv_buffer_), remote_endpoint_,
        [this](std::error_code ec, std::size_t bytes_recvd) { handle_receive(ec, bytes_recvd); });
}

void UDPServer::handle_receive(std::error_code ec, std::size_t bytes_received) {
    if (!ec && bytes_received > 0) {
        if (bytes_received >= 2) {
            std::vector<uint8_t> data(recv_buffer_->begin(),
                                      recv_buffer_->begin() +
                                          static_cast<std::ptrdiff_t>(bytes_received));

            if (!data.empty() && (data[0] == 0x00 || data[0] == 0x01)) {
                try {
                    RType::CompressionSerializer decompressor(data);
                    decompressor.decompress();
                    data = decompressor.data();
                } catch (const RType::CompressionException& e) {
                    std::cerr << "[Security] Decompression error from " << remote_endpoint_ << ": "
                              << e.what() << std::endl;
                    if (running_) {
                        start_receive();
                    }
                    return;
                }
            }

            if (data.size() >= 2) {
                uint16_t magic_number = static_cast<uint16_t>(
                    static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8));
                if (magic_number == 0xB542) {
                    int client_id = register_client(remote_endpoint_);

                    if (data.size() >= 3 && data[2] == 0x60) {
                        if (data.size() >= 7) {
                            uint32_t seq_id = static_cast<uint32_t>(data[3]) |
                                              (static_cast<uint32_t>(data[4]) << 8) |
                                              (static_cast<uint32_t>(data[5]) << 16) |
                                              (static_cast<uint32_t>(data[6]) << 24);
                            handle_ack(client_id, seq_id);
                        }
                    } else if (data.size() >= 7) {
                        uint8_t opcode = data[2];
                        bool is_reliable_opcode = (opcode == 0x02 ||
                                                   opcode == 0x30 ||
                                                   opcode == 0x50 ||
                                                   opcode == 0x40 ||
                                                   opcode == 0x37);

                        if (is_reliable_opcode) {
                            uint32_t seq_id = static_cast<uint32_t>(data[3]) |
                                              (static_cast<uint32_t>(data[4]) << 8) |
                                              (static_cast<uint32_t>(data[5]) << 16) |
                                              (static_cast<uint32_t>(data[6]) << 24);

                            std::vector<uint8_t> payload(data.begin() + 7, data.end());

                            std::lock_guard<std::mutex> lock(reliability_mutex_);
                            auto& state = client_reliability_[client_id];
                            auto ready_packets = state.process_received_packet(seq_id, payload);

                            send_ack(client_id, seq_id);

                            for (auto& pkt : ready_packets) {
                                std::vector<uint8_t> complete_packet;
                                complete_packet.push_back(0x42);
                                complete_packet.push_back(0xB5);
                                complete_packet.push_back(opcode);
                                complete_packet.insert(complete_packet.end(), pkt.begin(),
                                                       pkt.end());

                                NetworkPacket packet(std::move(complete_packet), remote_endpoint_);
                                input_queue_.push(std::move(packet));
                            }
                        } else {
                            NetworkPacket packet(std::move(data), remote_endpoint_);
                            input_queue_.push(std::move(packet));
                        }
                    } else {
                        NetworkPacket packet(std::move(data), remote_endpoint_);
                        input_queue_.push(std::move(packet));
                    }
                } else {
                    std::cerr << "[Security] Ignored packet with bad Magic Number from "
                              << remote_endpoint_ << std::endl;
                }
            }
        }
    } else if (ec) {
        std::cerr << "[Error] Receive error: " << ec.message() << std::endl;
    }

    if (running_) {
        start_receive();
    }
}

void UDPServer::queue_output_packet(NetworkPacket packet) {
    asio::post(io_context_, [this, packet = std::move(packet)]() {
        socket_->async_send_to(
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

void UDPServer::send_to_clients(const std::vector<int>& client_ids,
                                const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (int client_id : client_ids) {
        auto it = clients_.find(client_id);
        if (it != clients_.end()) {
            NetworkPacket packet(data, it->second.endpoint);
            queue_output_packet(packet);
        }
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

std::map<int, asio::ip::udp::endpoint> UDPServer::get_all_clients() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    std::map<int, asio::ip::udp::endpoint> result;
    for (const auto& [id, client] : clients_) {
        result[id] = client.endpoint;
    }
    return result;
}

void UDPServer::disconnect_client(int client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        std::cout << "[Network] Disconnecting client: ID=" << client_id << " ("
                  << it->second.endpoint.address().to_string() << ":" << it->second.endpoint.port()
                  << ")" << std::endl;

        try {
            std::vector<uint8_t> disconnect_packet = {0x42, 0xB5, 0x40};
            if (socket_) {
                socket_->send_to(asio::buffer(disconnect_packet), it->second.endpoint);
                std::cout << "[Network] Disconnect notification sent to client " << client_id
                          << std::endl;
            } else {
                std::cerr
                    << "[Network] Socket is null, cannot send disconnect notification for client "
                    << client_id << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[Network] Failed to send disconnect notification: " << e.what()
                      << std::endl;
        }

        clients_.erase(it);
        std::cout << "[Network] Client " << client_id << " removed from server" << std::endl;

        cleanup_client_reliability(client_id);
    } else {
        std::cout << "[Network] Client " << client_id << " not found (already disconnected?)"
                  << std::endl;
    }
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
    if (socket_) {
        socket_->close();
    }
}

void UDPServer::send_reliable(int client_id, uint8_t opcode, const std::vector<uint8_t>& payload) {
    std::lock_guard<std::mutex> lock(reliability_mutex_);

    auto& state = client_reliability_[client_id];
    uint32_t seq_id = state.get_next_send_sequence();

    std::vector<uint8_t> packet;
    packet.reserve(3 + 4 + payload.size());

    packet.push_back(0x42);
    packet.push_back(0xB5);

    packet.push_back(opcode);

    packet.push_back(static_cast<uint8_t>(seq_id & 0xFF));
    packet.push_back(static_cast<uint8_t>((seq_id >> 8) & 0xFF));
    packet.push_back(static_cast<uint8_t>((seq_id >> 16) & 0xFF));
    packet.push_back(static_cast<uint8_t>((seq_id >> 24) & 0xFF));

    packet.insert(packet.end(), payload.begin(), payload.end());

    RType::CompressionSerializer compressor(packet);
    compressor.compress();

    send_to_client(client_id, compressor.data());

    state.pending_acks.emplace_back(seq_id, opcode, compressor.data());

    std::cout << "[Reliable] Sent packet seq=" << seq_id << " opcode=0x" << std::hex
              << static_cast<int>(opcode) << std::dec << " to client " << client_id << std::endl;
}

void UDPServer::send_ack(int client_id, uint32_t sequence_id) {
    std::vector<uint8_t> ack_packet;
    ack_packet.reserve(7);

    ack_packet.push_back(0x42);
    ack_packet.push_back(0xB5);

    ack_packet.push_back(0x60);

    ack_packet.push_back(static_cast<uint8_t>(sequence_id & 0xFF));
    ack_packet.push_back(static_cast<uint8_t>((sequence_id >> 8) & 0xFF));
    ack_packet.push_back(static_cast<uint8_t>((sequence_id >> 16) & 0xFF));
    ack_packet.push_back(static_cast<uint8_t>((sequence_id >> 24) & 0xFF));

    send_to_client(client_id, ack_packet);
}

void UDPServer::handle_ack(int client_id, uint32_t sequence_id) {
    std::lock_guard<std::mutex> lock(reliability_mutex_);

    auto it = client_reliability_.find(client_id);
    if (it == client_reliability_.end()) {
        return;
    }

    auto& state = it->second;

    for (auto pkt_it = state.pending_acks.begin(); pkt_it != state.pending_acks.end(); ++pkt_it) {
        if (pkt_it->sequence_id == sequence_id) {
            std::cout << "[Reliable] ACK received seq=" << sequence_id << " from client "
                      << client_id << " (retry_count=" << pkt_it->retry_count << ")" << std::endl;
            state.pending_acks.erase(pkt_it);
            return;
        }
    }
}

void UDPServer::retry_unacked_packets() {
    std::lock_guard<std::mutex> lock(reliability_mutex_);
    auto now = std::chrono::steady_clock::now();

    for (auto& [client_id, state] : client_reliability_) {
        for (auto it = state.pending_acks.begin(); it != state.pending_acks.end();) {
            if (it->should_retry(now)) {
                if (it->max_retries_reached()) {
                    std::cout << "[Warning] Packet seq=" << it->sequence_id << " to client "
                              << client_id << " max retries reached, dropping" << std::endl;
                    it = state.pending_acks.erase(it);
                } else {
                    std::cout << "[Reliable] Retrying packet seq=" << it->sequence_id
                              << " to client " << client_id << " (attempt " << (it->retry_count + 1)
                              << ")" << std::endl;
                    send_to_client(client_id, it->data);
                    it->mark_resent(now);
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }
}

void UDPServer::retry_thread_loop() {
    std::cout << "[Reliable] Retry thread started" << std::endl;

    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        retry_unacked_packets();
    }

    std::cout << "[Reliable] Retry thread stopped" << std::endl;
}

void UDPServer::cleanup_client_reliability(int client_id) {
    std::lock_guard<std::mutex> lock(reliability_mutex_);

    auto it = client_reliability_.find(client_id);
    if (it != client_reliability_.end()) {
        std::cout << "[Reliable] Cleaning up reliability state for client " << client_id
                  << std::endl;
        it->second.reset();
        client_reliability_.erase(it);
    }
}

}  // namespace server
