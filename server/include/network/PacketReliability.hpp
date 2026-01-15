#pragma once

#include <cstdint>

#include <chrono>
#include <deque>
#include <map>
#include <set>
#include <vector>

namespace RType {

struct ReliabilityConfig {
    static constexpr int MAX_RETRIES = 3;
    static constexpr int RETRY_TIMEOUT_MS = 200;

    static constexpr uint32_t REORDER_WINDOW_SIZE = 64;
    static constexpr int REORDER_BUFFER_TIMEOUT_MS = 500;

    static constexpr uint32_t DUPLICATE_CACHE_SIZE = 256;
    static constexpr int DUPLICATE_CACHE_TTL_MS = 5000;
};

struct PendingPacket {
    uint32_t sequence_id;
    uint8_t opcode;
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point sent_time;
    int retry_count;

    PendingPacket(uint32_t seq, uint8_t op, std::vector<uint8_t> d)
        : sequence_id(seq),
          opcode(op),
          data(std::move(d)),
          sent_time(std::chrono::steady_clock::now()),
          retry_count(0) {}

    bool should_retry(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - sent_time);
        return elapsed.count() >= ReliabilityConfig::RETRY_TIMEOUT_MS;
    }

    void mark_resent(const std::chrono::steady_clock::time_point& now) {
        sent_time = now;
        retry_count++;
    }

    bool max_retries_reached() const { return retry_count >= ReliabilityConfig::MAX_RETRIES; }
};

struct BufferedPacket {
    uint32_t sequence_id;
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point received_time;

    BufferedPacket(uint32_t seq, std::vector<uint8_t> d)
        : sequence_id(seq), data(std::move(d)), received_time(std::chrono::steady_clock::now()) {}

    bool is_expired(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - received_time);
        return elapsed.count() >= ReliabilityConfig::REORDER_BUFFER_TIMEOUT_MS;
    }
};

struct DuplicateCacheEntry {
    std::chrono::steady_clock::time_point timestamp;

    DuplicateCacheEntry() : timestamp(std::chrono::steady_clock::now()) {}

    bool is_expired(const std::chrono::steady_clock::time_point& now) const {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp);
        return elapsed.count() >= ReliabilityConfig::DUPLICATE_CACHE_TTL_MS;
    }
};

struct ClientReliabilityState {
    uint32_t next_send_sequence = 1;
    std::deque<PendingPacket> pending_acks;

    uint32_t expected_recv_sequence = 1;
    std::map<uint32_t, BufferedPacket> reorder_buffer;

    std::set<uint32_t> duplicate_cache;
    std::map<uint32_t, DuplicateCacheEntry> cache_timestamps;

    uint32_t get_next_send_sequence() { return next_send_sequence++; }

    bool is_duplicate(uint32_t seq_id) {
        cleanup_duplicate_cache();

        if (duplicate_cache.find(seq_id) != duplicate_cache.end()) {
            return true;
        }

        duplicate_cache.insert(seq_id);
        cache_timestamps[seq_id] = DuplicateCacheEntry();

        if (duplicate_cache.size() > ReliabilityConfig::DUPLICATE_CACHE_SIZE) {
            auto oldest = duplicate_cache.begin();
            cache_timestamps.erase(*oldest);
            duplicate_cache.erase(oldest);
        }

        return false;
    }

    bool is_in_reorder_window(uint32_t seq_id) const {
        if (seq_id < expected_recv_sequence) {
            return false;
        }
        if (seq_id >= expected_recv_sequence + ReliabilityConfig::REORDER_WINDOW_SIZE) {
            return false;
        }
        return true;
    }

    std::vector<std::vector<uint8_t>> process_received_packet(uint32_t seq_id,
                                                              std::vector<uint8_t> data) {
        std::vector<std::vector<uint8_t>> ready_packets;

        if (is_duplicate(seq_id)) {
            return ready_packets;
        }

        if (!is_in_reorder_window(seq_id)) {
            return ready_packets;
        }

        if (seq_id == expected_recv_sequence) {
            ready_packets.push_back(std::move(data));
            expected_recv_sequence++;

            while (true) {
                auto it = reorder_buffer.find(expected_recv_sequence);
                if (it == reorder_buffer.end()) {
                    break;
                }
                ready_packets.push_back(std::move(it->second.data));
                reorder_buffer.erase(it);
                expected_recv_sequence++;
            }
        } else {
            reorder_buffer.emplace(seq_id, BufferedPacket(seq_id, std::move(data)));
        }

        cleanup_reorder_buffer();

        return ready_packets;
    }

    void cleanup_reorder_buffer() {
        auto now = std::chrono::steady_clock::now();
        for (auto it = reorder_buffer.begin(); it != reorder_buffer.end();) {
            if (it->second.is_expired(now)) {
                it = reorder_buffer.erase(it);
            } else {
                ++it;
            }
        }
    }

    void cleanup_duplicate_cache() {
        auto now = std::chrono::steady_clock::now();
        for (auto it = cache_timestamps.begin(); it != cache_timestamps.end();) {
            if (it->second.is_expired(now)) {
                duplicate_cache.erase(it->first);
                it = cache_timestamps.erase(it);
            } else {
                ++it;
            }
        }
    }

    void reset() {
        pending_acks.clear();
        reorder_buffer.clear();
        duplicate_cache.clear();
        cache_timestamps.clear();
        next_send_sequence = 1;
        expected_recv_sequence = 1;
    }
};

}  // namespace RType
