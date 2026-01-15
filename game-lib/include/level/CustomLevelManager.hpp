#pragma once

#include "LevelConfig.hpp"
#include "LevelConfigParser.hpp"
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace rtype::level {

struct LoadedLevel {
    LevelConfig config;
    std::filesystem::path source_path;
    bool is_builtin = false;
};

class CustomLevelManager {
public:
    static CustomLevelManager& instance();
    
    void setLevelsDirectory(const std::filesystem::path& path);
    void setCustomLevelsDirectory(const std::filesystem::path& path);
    
    bool loadBuiltinLevels();
    bool loadCustomLevels();
    bool reloadAllLevels();
    
    bool loadLevel(const std::filesystem::path& path, bool is_builtin = false);
    bool unloadLevel(const std::string& level_id);
    
    const LoadedLevel* getLevel(const std::string& level_id) const;
    std::vector<std::string> getAvailableLevelIds() const;
    std::vector<const LoadedLevel*> getAllLevels() const;
    std::vector<const LoadedLevel*> getBuiltinLevels() const;
    std::vector<const LoadedLevel*> getCustomLevels() const;
    
    bool validateLevel(const std::string& level_id) const;
    std::vector<std::string> getLevelWarnings(const std::string& level_id) const;
    
    using LevelLoadCallback = std::function<void(const std::string& level_id, bool success)>;
    void setOnLevelLoaded(LevelLoadCallback callback);

private:
    CustomLevelManager() = default;
    ~CustomLevelManager() = default;
    CustomLevelManager(const CustomLevelManager&) = delete;
    CustomLevelManager& operator=(const CustomLevelManager&) = delete;
    
    bool loadLevelsFromDirectory(const std::filesystem::path& dir, bool is_builtin);
    
    std::filesystem::path levels_directory_;
    std::filesystem::path custom_levels_directory_;
    std::unordered_map<std::string, LoadedLevel> loaded_levels_;
    std::unordered_map<std::string, std::vector<std::string>> level_warnings_;
    LevelLoadCallback on_level_loaded_;
};

}
