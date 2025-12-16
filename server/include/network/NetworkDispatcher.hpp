#pragma once

#include "INetworkCommand.hpp"
#include "UDPServer.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include <queue>
#include <memory>

namespace server {

/**
 * @brief Dispatches incoming network packets into command objects.
 * 
 * NetworkDispatcher separates concerns:
 * - Network layer: parses bytes into commands
 * - Game layer: executes commands via GameSession
 * 
 * This makes the codebase modular and testable (can mock commands).
 */
class NetworkDispatcher {
public:
    NetworkDispatcher() = default;
    
    /**
     * @brief Poll network packets and convert to commands.
     * @param server UDP server to read packets from.
     * @return Queue of commands ready to execute.
     */
    std::queue<std::unique_ptr<INetworkCommand>> poll_commands(UDPServer& server);

private:
    /**
     * @brief Parse a single packet into a command.
     * @param packet Raw network packet.
     * @param server UDP server reference.
     * @return Command object or nullptr if parsing failed.
     */
    std::unique_ptr<INetworkCommand> parse_packet(
        const NetworkPacket& packet,
        UDPServer& server
    );
};

}  // namespace server
