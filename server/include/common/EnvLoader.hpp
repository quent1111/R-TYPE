#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace server {

class EnvLoader {
public:
    static std::unordered_map<std::string, std::string> load(const std::string& filepath = ".env") {
        std::unordered_map<std::string, std::string> env_vars;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            return env_vars;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }

            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                env_vars[key] = value;
            }
        }

        return env_vars;
    }

    static std::string get(const std::unordered_map<std::string, std::string>& env_vars,
                           const std::string& key, const std::string& default_value = "") {
        auto it = env_vars.find(key);
        if (it != env_vars.end()) {
            return it->second;
        }
        return default_value;
    }
};

}  // namespace server
