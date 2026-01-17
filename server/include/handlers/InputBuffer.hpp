#pragma once

#include <cstdint>

#include <chrono>
#include <deque>
#include <vector>

namespace server {

struct InputDelayConfig {
    static constexpr int INPUT_DELAY_MS = 50;

    static constexpr size_t MAX_BUFFERED_INPUTS = 100;

    static constexpr int INPUT_TIMEOUT_MS = 5000;
};


struct InputEntry {
    uint32_t client_timestamp;
    uint8_t input_mask;
    std::chrono::steady_clock::time_point receive_time;

    InputEntry(uint32_t timestamp, uint8_t mask)
        : client_timestamp(timestamp),
          input_mask(mask),
          receive_time(std::chrono::steady_clock::now()) {}

    bool is_ready_to_apply(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - receive_time);
        return elapsed.count() >= InputDelayConfig::INPUT_DELAY_MS;
    }

    bool is_expired(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - receive_time);
        return elapsed.count() >= InputDelayConfig::INPUT_TIMEOUT_MS;
    }
};

class ClientInputBuffer {
public:
    ClientInputBuffer() = default;


    bool add_input(uint32_t timestamp, uint8_t input_mask) {
        if (buffered_inputs_.size() >= InputDelayConfig::MAX_BUFFERED_INPUTS) {
            buffered_inputs_.pop_front();
        }

        buffered_inputs_.emplace_back(timestamp, input_mask);
        return true;
    }

    std::vector<InputEntry> get_ready_inputs() {
        auto now = std::chrono::steady_clock::now();
        std::vector<InputEntry> ready;

        while (!buffered_inputs_.empty() && buffered_inputs_.front().is_expired(now)) {
            buffered_inputs_.pop_front();
        }

        while (!buffered_inputs_.empty() && buffered_inputs_.front().is_ready_to_apply(now)) {
            ready.push_back(buffered_inputs_.front());
            buffered_inputs_.pop_front();
        }

        return ready;
    }


    void clear() { buffered_inputs_.clear(); }

    size_t size() const { return buffered_inputs_.size(); }

    bool empty() const { return buffered_inputs_.empty(); }

private:
    std::deque<InputEntry> buffered_inputs_;
};

}  // namespace server
