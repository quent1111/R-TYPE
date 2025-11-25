#pragma once

#include <vector>
#include <asio.hpp>

struct NetworkPacket {
    std::vector<uint8_t> data;
    asio::ip::udp::endpoint sender;

    NetworkPacket() = default;
    NetworkPacket(const std::vector<uint8_t>& d, const asio::ip::udp::endpoint& s)
        : data(d), sender(s) {}
};
