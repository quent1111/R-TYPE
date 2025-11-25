#include "UDPServer.hpp"

UDPServer::UDPServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
        socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
        next_client_id_(1),
        running_(true)
{
    std::cout << "UDP Server listening on port " << port << std::endl;
    start_receive();
}

UDPServer::~UDPServer()
{
    running_ = false;
    socket_.close();
}

void UDPServer::start_receive()
{
    socket_.async_receive_from(
        asio::buffer(recv_buffer_),
        remote_endpoint_,
        [this](std::error_code ec, std::size_t bytes_recvd) {
            handle_receive(ec, bytes_recvd);
        }
    );
}

void UDPServer::handle_receive(std::error_code ec, std::size_t bytes_received)
{
    if (!ec && bytes_received > 0) {
        // Créer un paquet avec les données reçues
        std::vector<uint8_t> data(recv_buffer_.begin(), recv_buffer_.begin() + bytes_received);
        NetworkPacket packet(data, remote_endpoint_);

        // Enregistrer ou mettre à jour le client
        register_client(remote_endpoint_);
        input_queue_.push(packet);

        std::cout << "Received " << bytes_received << " bytes from "
                    << remote_endpoint_.address().to_string() << ":"
                    << remote_endpoint_.port() << std::endl;
    } else if (ec) {
        std::cerr << "Receive error: " << ec.message() << std::endl;
    }
    if (running_) {
        start_receive();
    }
}

int UDPServer::register_client(const asio::ip::udp::endpoint& endpoint)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);

    // Chercher si le client existe déjà
    for (auto& [id, client] : clients_) {
        if (client.endpoint == endpoint) {
            client.last_seen = std::chrono::steady_clock::now();
            return id;
        }
    }

    // Nouveau client
    int client_id = next_client_id_++;
    clients_[client_id] = ClientEndpoint(endpoint, client_id);
    std::cout << "New client registered: ID=" << client_id
                << " from " << endpoint.address().to_string()
                << ":" << endpoint.port() << std::endl;
    return client_id;
}

void UDPServer::send_to_all(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (const auto& [id, client] : clients_) {
        socket_.async_send_to(
            asio::buffer(data),
            client.endpoint,
            [id](std::error_code ec, std::size_t /*bytes_sent*/) {
                if (ec) {
                    std::cerr << "Send error to client " << id << ": "
                                << ec.message() << std::endl;
                }
            }
        );
    }
}

void UDPServer::send_to_client(int client_id, const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        socket_.async_send_to(
            asio::buffer(data),
            it->second.endpoint,
            [client_id](std::error_code ec, std::size_t /*bytes_sent*/) {
                if (ec) {
                    std::cerr << "Send error to client " << client_id << ": "
                                << ec.message() << std::endl;
                }
            }
        );
    }
}

void UDPServer::send_to_endpoint(const asio::ip::udp::endpoint& endpoint, const std::vector<uint8_t>& data)
{
    socket_.async_send_to(
        asio::buffer(data),
        endpoint,
        [endpoint](std::error_code ec, std::size_t /*bytes_sent*/) {
            if (ec) {
                std::cerr << "Send error to " << endpoint.address().to_string()
                            << ":" << endpoint.port() << ": " << ec.message() << std::endl;
            }
        }
    );
}

bool UDPServer::get_input_packet(NetworkPacket& packet)
{
    return input_queue_.try_pop(packet);
}

void UDPServer::queue_output_packet(const NetworkPacket& packet)
{
    output_queue_.push(packet);
}

void UDPServer::process_output_queue()
{
    NetworkPacket packet;
    while (output_queue_.try_pop(packet)) {
        // Implémenter protocole RFC
        send_to_endpoint(packet.sender, packet.data);
    }
}

void UDPServer::remove_inactive_clients(std::chrono::seconds timeout)
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto now = std::chrono::steady_clock::now();
    for (auto it = clients_.begin(); it != clients_.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.last_seen
        );

        if (elapsed > timeout) {
            std::cout << "Removing inactive client: ID=" << it->first << std::endl;
            it = clients_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t UDPServer::get_client_count()
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

size_t UDPServer::get_input_queue_size() const
{
    return input_queue_.size();
}

size_t UDPServer::get_output_queue_size() const
{
    return output_queue_.size();
}

std::map<int, ClientEndpoint> UDPServer::get_clients()
{
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_;
}

void UDPServer::run_network_loop()
{
    while (running_) {
        try {
            io_context_.poll();
            process_output_queue();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } catch (const std::exception& e) {
            std::cerr << "Network loop error: " << e.what() << std::endl;
        }
    }
}

void UDPServer::stop()
{
    running_ = false;
}