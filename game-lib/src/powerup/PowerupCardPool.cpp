#include "powerup/PowerupCardPool.hpp"
#include <set>

namespace powerup {

std::vector<PowerupCard> PowerupCardPool::generate_card_choices(
    const PlayerPowerups& player_powerups, int count) {
    
    std::vector<PowerupCard> available = get_available_cards(player_powerups);
    
    if (available.empty()) {
        auto all_ids = PowerupRegistry::instance().get_all_powerup_ids();
        for (auto id : all_ids) {
            available.emplace_back(id, 1);
        }
    }
    
    std::shuffle(available.begin(), available.end(), rng_);
    
    int actual_count = std::min(count, static_cast<int>(available.size()));
    std::vector<PowerupCard> choices;
    choices.reserve(actual_count);
    
    for (int i = 0; i < actual_count; ++i) {
        choices.push_back(available[i]);
    }
    
    return choices;
}

std::vector<PowerupCard> PowerupCardPool::get_available_cards(
    const PlayerPowerups& player_powerups) {
    
    std::vector<PowerupCard> cards;
    auto all_ids = PowerupRegistry::instance().get_all_powerup_ids();
    
    for (auto id : all_ids) {
        auto* def = PowerupRegistry::instance().get_powerup(id);
        if (!def) continue;
        
        uint8_t current_level = player_powerups.get_level(id);
        
        if (current_level == 0) {
            cards.emplace_back(id, 1);
        } else if (current_level < def->max_level) {
            cards.emplace_back(id, current_level + 1);
        }
    }
    
    return cards;
}

}
