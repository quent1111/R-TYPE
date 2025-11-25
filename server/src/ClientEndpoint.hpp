#pragma once

#include <asio.hpp>

#include <chrono>

struct ClientEndpoint {
    asio::ip::udp::endpoint endpoint;
    int client_id;
    std::chrono::steady_clock::time_point last_seen;

    ClientEndpoint() = default;
    ClientEndpoint(const asio::ip::udp::endpoint& ep, int id)
        : endpoint(ep), client_id(id), last_seen(std::chrono::steady_clock::now()) {}
};
