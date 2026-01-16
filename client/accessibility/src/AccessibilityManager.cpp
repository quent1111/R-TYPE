#include "../include/AccessibilityManager.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace accessibility {

AccessibilityManager& AccessibilityManager::instance() {
    static AccessibilityManager instance;
    return instance;
}

AccessibilityManager::AccessibilityManager()
    : current_mode_(ColorBlindnessMode::Normal),
      projectile_shapes_enabled_(true),
      cache_valid_(false) {}

void AccessibilityManager::setColorBlindMode(ColorBlindnessMode mode) {
    if (current_mode_ != mode) {
        current_mode_ = mode;
        invalidateCache();
        std::cout << "[Accessibility] Mode changed to: " << colorBlindnessModeToString(mode)
                  << std::endl;
    }
}

sf::Color AccessibilityManager::transformColor(const sf::Color& original) const {
    return ColorTransform::transform(original, current_mode_);
}

sf::Color AccessibilityManager::getPlayerProjectileColor(const sf::Color& /*original*/) const {
    if (cache_valid_) {
        return cached_player_projectile_color_;
    }

    sf::Color playerColor = sf::Color(0, 200, 255);

    playerColor = transformColor(playerColor);

    sf::Color background(0, 0, 0);
    playerColor = ColorTransform::enhanceContrast(playerColor, background, 1.3f);

    cached_player_projectile_color_ = playerColor;

    return playerColor;
}

sf::Color AccessibilityManager::getEnemyProjectileColor(const sf::Color& original) const {
    if (cache_valid_) {
        return cached_enemy_projectile_color_;
    }

    sf::Color enemyColor = sf::Color(255, 80, 0);

    enemyColor = transformColor(enemyColor);

    sf::Color background(0, 0, 0);
    enemyColor = ColorTransform::enhanceContrast(enemyColor, background, 1.3f);

    sf::Color playerColor = getPlayerProjectileColor(original);
    if (!ColorTransform::areColorsDistinguishable(playerColor, enemyColor, current_mode_, 60.0f)) {
        enemyColor = sf::Color(255, 200, 0);
        enemyColor = transformColor(enemyColor);
    }

    cached_enemy_projectile_color_ = enemyColor;

    return enemyColor;
}

sf::Color AccessibilityManager::getBorderColor(const sf::Color& fillColor) const {
    float luminance = ColorTransform::calculateLuminance(fillColor);

    if (luminance > 128.0f) {
        return transformColor(sf::Color::Black);
    } else {
        return transformColor(sf::Color::White);
    }
}

bool AccessibilityManager::loadSettings(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Accessibility] Could not open settings file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    bool in_accessibility_section = false;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (line == "[Accessibility]") {
            in_accessibility_section = true;
            continue;
        } else if (line[0] == '[') {
            in_accessibility_section = false;
            continue;
        }

        if (in_accessibility_section) {
            size_t equal_pos = line.find('=');
            if (equal_pos != std::string::npos) {
                std::string key = line.substr(0, equal_pos);
                std::string value = line.substr(equal_pos + 1);

                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (key == "ColorBlindMode") {
                    setColorBlindMode(stringToColorBlindnessMode(value));
                } else if (key == "ProjectileShapes") {
                    projectile_shapes_enabled_ = (value == "true" || value == "1");
                }
            }
        }
    }

    file.close();
    std::cout << "[Accessibility] Settings loaded from " << filepath << std::endl;
    return true;
}

bool AccessibilityManager::saveSettings(const std::string& filepath) const {
    // Lire le fichier existant
    std::ifstream in_file(filepath);
    std::vector<std::string> lines;
    std::string line;

    if (in_file.is_open()) {
        while (std::getline(in_file, line)) {
            lines.push_back(line);
        }
        in_file.close();
    }

    // Chercher ou créer la section [Accessibility]
    bool found_section = false;
    size_t section_start = 0;

    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i] == "[Accessibility]") {
            found_section = true;
            section_start = i;
            break;
        }
    }

    // Si la section n'existe pas, l'ajouter
    if (!found_section) {
        lines.push_back("");
        lines.push_back("[Accessibility]");
        section_start = lines.size() - 1;
    }

    // Mettre à jour ou ajouter les valeurs
    std::string mode_line =
        "ColorBlindMode=" + std::string(colorBlindnessModeToString(current_mode_));
    std::string shapes_line =
        "ProjectileShapes=" + std::string(projectile_shapes_enabled_ ? "true" : "false");

    // Chercher et mettre à jour les lignes existantes
    bool found_mode = false;
    bool found_shapes = false;

    for (size_t i = section_start + 1; i < lines.size(); ++i) {
        if (!lines[i].empty() && lines[i][0] == '[') {
            break;
        }

        if (lines[i].find("ColorBlindMode=") == 0) {
            lines[i] = mode_line;
            found_mode = true;
        } else if (lines[i].find("ProjectileShapes=") == 0) {
            lines[i] = shapes_line;
            found_shapes = true;
        }
    }

    if (!found_mode) {
        lines.insert(lines.begin() + section_start + 1, mode_line);
    }
    if (!found_shapes) {
        lines.insert(lines.begin() + section_start + 2, shapes_line);
    }

    std::ofstream out_file(filepath);
    if (!out_file.is_open()) {
        std::cerr << "[Accessibility] Could not write settings file: " << filepath << std::endl;
        return false;
    }

    for (const auto& l : lines) {
        out_file << l << "\n";
    }

    out_file.close();
    std::cout << "[Accessibility] Settings saved to " << filepath << std::endl;
    return true;
}

void AccessibilityManager::resetToDefaults() {
    current_mode_ = ColorBlindnessMode::Normal;
    projectile_shapes_enabled_ = true;
    invalidateCache();
    std::cout << "[Accessibility] Reset to default settings" << std::endl;
}

void AccessibilityManager::invalidateCache() {
    cache_valid_ = false;
}

}  // namespace accessibility
