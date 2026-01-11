#pragma once

#include "PowerupRegistry.hpp"
#include "PlayerPowerups.hpp"
#include <vector>
#include <random>
#include <algorithm>

namespace powerup {

struct PowerupCard {
    PowerupId id;
    uint8_t level;
    
    PowerupCard() : id(PowerupId::PowerCannon), level(1) {}
    PowerupCard(PowerupId _id, uint8_t _level) : id(_id), level(_level) {}
    
    bool operator==(const PowerupCard& other) const {
        return id == other.id && level == other.level;
    }
};

class PowerupCardPool {
public:
    PowerupCardPool() : rng_(std::random_device{}()) {}
    
    std::vector<PowerupCard> generate_card_choices(const PlayerPowerups& player_powerups, int count = 3);
    
    void seed(unsigned int seed) {
        rng_.seed(seed);
    }

private:
    std::mt19937 rng_;
    
    std::vector<PowerupCard> get_available_cards(const PlayerPowerups& player_powerups);
};

}
