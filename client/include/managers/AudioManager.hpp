#pragma once

#include <string>

// Audio disabled on Windows due to openal-soft incompatibility with GCC 15
#ifdef _WIN32
#define RTYPE_NO_AUDIO 1
#else
#define RTYPE_NO_AUDIO 0
#endif

#if !RTYPE_NO_AUDIO
#include <SFML/Audio.hpp>

#include <map>
#include <vector>
#endif

namespace managers {

class AudioManager {
public:
    enum class SoundType { Laser, Explosion, HitSound, PlayerHit, LevelUp, Plop, Coin, BossRoar };

    static AudioManager& instance();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool load_sounds();

    void play_sound(SoundType type);

    void play_music(const std::string& music_path, bool loop = true);
    void stop_music();
    void pause_music();
    void resume_music();
    bool is_music_playing() const;
    std::string get_current_music() const;

    void set_sound_volume(float volume);
    void set_music_volume(float volume);
    void set_master_volume(float volume);

    float get_sound_volume() const;
    float get_music_volume() const;
    float get_master_volume() const;

private:
#if RTYPE_NO_AUDIO
    // Stub implementation (no audio)
    AudioManager() = default;
    ~AudioManager() = default;
    std::string current_music_path_;
    float sound_volume_ = 70.0f;
    float music_volume_ = 50.0f;
    float master_volume_ = 100.0f;
    float get_effective_volume(float base_volume) const;
#else
    // Full SFML Audio implementation
    AudioManager();
    ~AudioManager() = default;
    static constexpr size_t SOUND_POOL_SIZE = 16;
    std::map<SoundType, sf::SoundBuffer> sound_buffers_;
    std::vector<sf::Sound> sound_pool_;
    size_t current_sound_index_ = 0;
    sf::Music music_;
    std::string current_music_path_;
    float sound_volume_ = 70.0f;
    float music_volume_ = 50.0f;
    float master_volume_ = 100.0f;
    sf::Sound& get_next_sound();
    float get_effective_volume(float base_volume) const;
#endif
};

}  // namespace managers
