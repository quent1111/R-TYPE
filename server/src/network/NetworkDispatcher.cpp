#include "network/NetworkDispatcher.hpp"
#include "network/NetworkCommands.hpp"
#include <iostream>

namespace server {

std::queue<std::unique_ptr<INetworkCommand>> NetworkDispatcher::poll_commands(UDPServer& server) {
    std::queue<std::unique_ptr<INetworkCommand>> commands;

    NetworkPacket packet;
    while (server.get_input_packet(packet)) {
        auto cmd = parse_packet(packet, server);
        if (cmd) {
            commands.push(std::move(cmd));
        }
    }

    return commands;
}

std::unique_ptr<INetworkCommand> NetworkDispatcher::parse_packet(
    const NetworkPacket& packet,
    UDPServer& server
) {
    if (packet.data.empty() || packet.data.size() < 3) {
        return nullptr;
    }

    try {
        RType::BinarySerializer deserializer(packet.data);

        uint16_t magic;
        deserializer >> magic;

        if (!RType::MagicNumber::is_valid(magic)) {
            std::cerr << "[NetworkDispatcher] Invalid magic number from " << packet.sender << std::endl;
            return nullptr;
        }

        RType::OpCode opcode;
        deserializer >> opcode;

        int client_id = server.register_client(packet.sender);

        std::cerr << "[NetworkDispatcher] Opcode parsing not yet implemented: " 
                  << static_cast<int>(opcode) << std::endl;
        return nullptr;

    } catch (const RType::SerializationException& e) {
        std::cerr << "[NetworkDispatcher] Serialization error: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "[NetworkDispatcher] Error parsing packet: " << e.what() << std::endl;
        return nullptr;
    }
}

}  // namespace server
