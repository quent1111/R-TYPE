#pragma once

#include <asio.hpp>

#include <vector>

struct NetworkPacket {
    std::vector<uint8_t> data;
    asio::ip::udp::endpoint sender;

    NetworkPacket() = default;

    NetworkPacket(const std::vector<uint8_t>& d, const asio::ip::udp::endpoint& s)
        : data(d), sender(s) {}

    NetworkPacket(std::vector<uint8_t>&& d, const asio::ip::udp::endpoint& s)
        : data(std::move(d)), sender(s) {}
};