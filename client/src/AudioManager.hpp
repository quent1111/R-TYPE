#pragma once

#include <SFML/Audio.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

class AudioManager {
public:
    enum class SoundType { Laser, Explosion, HitSound, PlayerHit, LevelUp, Plop, Coin };

    static AudioManager& getInstance();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool loadSounds();

    void playSound(SoundType type);

    void playMusic(const std::string& musicPath, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    bool isMusicPlaying() const;
    std::string getCurrentMusic() const { return current_music_path_; }

    void setSoundVolume(float volume);
    void setMusicVolume(float volume);
    void setMasterVolume(float volume);

    float getSoundVolume() const { return sound_volume_; }
    float getMusicVolume() const { return music_volume_; }

private:
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

    sf::Sound& getNextSound();

    float getEffectiveVolume(float base_volume) const;
};
