#include "level/CustomLevelManager.hpp"
#include <algorithm>
#include <iostream>

namespace rtype::level {

CustomLevelManager& CustomLevelManager::instance() {
    static CustomLevelManager instance;
    return instance;
}

void CustomLevelManager::setLevelsDirectory(const std::filesystem::path& path) {
    levels_directory_ = path;
}

void CustomLevelManager::setCustomLevelsDirectory(const std::filesystem::path& path) {
    custom_levels_directory_ = path;
}

bool CustomLevelManager::loadBuiltinLevels() {
    if (levels_directory_.empty() || !std::filesystem::exists(levels_directory_)) {
        return false;
    }
    return loadLevelsFromDirectory(levels_directory_, true);
}

bool CustomLevelManager::loadCustomLevels() {
    if (custom_levels_directory_.empty() || !std::filesystem::exists(custom_levels_directory_)) {
        return false;
    }
    return loadLevelsFromDirectory(custom_levels_directory_, false);
}

bool CustomLevelManager::reloadAllLevels() {
    loaded_levels_.clear();
    level_warnings_.clear();
    
    bool success = true;
    if (!levels_directory_.empty() && std::filesystem::exists(levels_directory_)) {
        success &= loadBuiltinLevels();
    }
    if (!custom_levels_directory_.empty() && std::filesystem::exists(custom_levels_directory_)) {
        success &= loadCustomLevels();
    }
    return success;
}

bool CustomLevelManager::loadLevelsFromDirectory(const std::filesystem::path& dir, bool is_builtin) {
    bool any_loaded = false;
    
    std::cout << "[CustomLevelManager] Scanning directory: " << dir << std::endl;
    
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        std::cout << "[CustomLevelManager] Found entry: " << entry.path() << " (is_file: " << entry.is_regular_file() << ")" << std::endl;
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            std::cout << "[CustomLevelManager] Extension: '" << ext << "'" << std::endl;
            if (ext == ".json") {
                std::cout << "[CustomLevelManager] Loading JSON file: " << entry.path() << std::endl;
                if (loadLevel(entry.path(), is_builtin)) {
                    any_loaded = true;
                }
            }
        }
    }
    
    return any_loaded;
}

bool CustomLevelManager::loadLevel(const std::filesystem::path& path, bool is_builtin) {
    std::cout << "[CustomLevelManager] Loading level from: " << path << std::endl;
    
    auto result = LevelConfigParser::parse(path);
    
    if (!LevelConfigParser::isSuccess(result)) {
        std::cout << "[CustomLevelManager] Parse failed for: " << path << std::endl;
        auto& error = LevelConfigParser::getError(result);
        std::cout << "[CustomLevelManager] Error code: " << static_cast<int>(error.first) << ", message: " << error.second << std::endl;
        if (on_level_loaded_) {
            on_level_loaded_(path.stem().string(), false);
        }
        return false;
    }
    
    auto& parse_result = LevelConfigParser::getResult(result);
    // Copy the level_id BEFORE moving the config!
    const std::string level_id = parse_result.config.metadata.id;
    std::cout << "[CustomLevelManager] Parsed level ID: '" << level_id << "'" << std::endl;
    
    if (level_id.empty()) {
        std::cout << "[CustomLevelManager] Level ID is empty, skipping" << std::endl;
        if (on_level_loaded_) {
            on_level_loaded_(path.stem().string(), false);
        }
        return false;
    }
    
    LoadedLevel loaded;
    loaded.config = std::move(parse_result.config);
    loaded.source_path = path;
    loaded.is_builtin = is_builtin;
    
    loaded_levels_[level_id] = std::move(loaded);
    level_warnings_[level_id] = std::move(parse_result.warnings);
    
    std::cout << "[CustomLevelManager] Successfully loaded level: " << level_id << std::endl;
    
    if (on_level_loaded_) {
        on_level_loaded_(level_id, true);
    }
    
    return true;
}

bool CustomLevelManager::unloadLevel(const std::string& level_id) {
    auto it = loaded_levels_.find(level_id);
    if (it == loaded_levels_.end()) {
        return false;
    }
    
    loaded_levels_.erase(it);
    level_warnings_.erase(level_id);
    return true;
}

const LoadedLevel* CustomLevelManager::getLevel(const std::string& level_id) const {
    auto it = loaded_levels_.find(level_id);
    if (it == loaded_levels_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<std::string> CustomLevelManager::getAvailableLevelIds() const {
    std::vector<std::string> ids;
    ids.reserve(loaded_levels_.size());
    for (const auto& [id, _] : loaded_levels_) {
        ids.push_back(id);
    }
    return ids;
}

std::vector<const LoadedLevel*> CustomLevelManager::getAllLevels() const {
    std::vector<const LoadedLevel*> levels;
    levels.reserve(loaded_levels_.size());
    for (const auto& [_, level] : loaded_levels_) {
        levels.push_back(&level);
    }
    return levels;
}

std::vector<const LoadedLevel*> CustomLevelManager::getBuiltinLevels() const {
    std::vector<const LoadedLevel*> levels;
    for (const auto& [_, level] : loaded_levels_) {
        if (level.is_builtin) {
            levels.push_back(&level);
        }
    }
    return levels;
}

std::vector<const LoadedLevel*> CustomLevelManager::getCustomLevels() const {
    std::vector<const LoadedLevel*> levels;
    for (const auto& [_, level] : loaded_levels_) {
        if (!level.is_builtin) {
            levels.push_back(&level);
        }
    }
    return levels;
}

bool CustomLevelManager::validateLevel(const std::string& level_id) const {
    auto warnings = getLevelWarnings(level_id);
    return warnings.empty();
}

std::vector<std::string> CustomLevelManager::getLevelWarnings(const std::string& level_id) const {
    auto it = level_warnings_.find(level_id);
    if (it == level_warnings_.end()) {
        return {};
    }
    return it->second;
}

void CustomLevelManager::setOnLevelLoaded(LevelLoadCallback callback) {
    on_level_loaded_ = std::move(callback);
}

}
