#pragma once

#include "UDPServer.hpp"
#include <atomic>

class Game
{
private:
    /* data */
public:
    Game(/* args */);
    ~Game();
    void runGameLoop(UDPServer& server);
};
