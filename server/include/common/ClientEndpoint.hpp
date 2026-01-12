#pragma once

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include <chrono>

namespace server {

struct ClientEndpoint {
    asio::ip::udp::endpoint endpoint;
    int client_id;
    std::chrono::steady_clock::time_point last_seen;

    ClientEndpoint() = default;
    ClientEndpoint(const asio::ip::udp::endpoint& ep, int id)
        : endpoint(ep), client_id(id), last_seen(std::chrono::steady_clock::now()) {}
};

}  // namespace server
