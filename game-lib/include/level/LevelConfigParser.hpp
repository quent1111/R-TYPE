#pragma once

#include "LevelConfig.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <variant>

namespace rtype::level {

enum class ParseError {
    FileNotFound,
    InvalidJson,
    MissingRequiredField,
    InvalidValue,
    InvalidEnemyReference
};

struct ParseResult {
    LevelConfig config;
    std::vector<std::string> warnings;
};

using ParseReturn = std::variant<ParseResult, std::pair<ParseError, std::string>>;

class LevelConfigParser {
public:
    static ParseReturn parse(const std::filesystem::path& path);
    static ParseReturn parseFromString(const std::string& json_content);
    static std::string serialize(const LevelConfig& config);
    
    static bool isSuccess(const ParseReturn& result);
    static ParseResult& getResult(ParseReturn& result);
    static const ParseResult& getResult(const ParseReturn& result);
    static std::pair<ParseError, std::string>& getError(ParseReturn& result);

private:
    static bool validateConfig(const LevelConfig& config, std::vector<std::string>& warnings);
};

}
