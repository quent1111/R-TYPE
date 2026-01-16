#include "managers/AudioManager.hpp"

#include <algorithm>
#include <iostream>

namespace managers {

AudioManager& AudioManager::instance() {
    static AudioManager instance;
    return instance;
}

#if RTYPE_NO_AUDIO

bool AudioManager::load_sounds() {
    std::cout << "[AudioManager] Audio disabled (no-audio build)" << std::endl;
    return true;
}

void AudioManager::play_sound([[maybe_unused]] SoundType type) {
}

void AudioManager::play_music([[maybe_unused]] const std::string& music_path,
                              [[maybe_unused]] bool loop) {
}

void AudioManager::stop_music() {
    current_music_path_.clear();
}

void AudioManager::pause_music() {
}

void AudioManager::resume_music() {
}

bool AudioManager::is_music_playing() const {
    return false;
}

std::string AudioManager::get_current_music() const {
    return current_music_path_;
}

void AudioManager::set_sound_volume(float volume) {
    sound_volume_ = std::clamp(volume, 0.0f, 100.0f);
}

void AudioManager::set_music_volume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 100.0f);
}

void AudioManager::set_master_volume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 100.0f);
}

float AudioManager::get_sound_volume() const {
    return sound_volume_;
}

float AudioManager::get_music_volume() const {
    return music_volume_;
}

float AudioManager::get_master_volume() const {
    return master_volume_;
}

void AudioManager::set_music_muted(bool muted) {
    music_muted_ = muted;
}

void AudioManager::set_sound_muted(bool muted) {
    sound_muted_ = muted;
}

bool AudioManager::is_music_muted() const {
    return music_muted_;
}

bool AudioManager::is_sound_muted() const {
    return sound_muted_;
}

float AudioManager::get_effective_volume(float base_volume) const {
    return (base_volume * master_volume_) / 100.0f;
}

#else

AudioManager::AudioManager() {
    sound_pool_.resize(SOUND_POOL_SIZE);
}

bool AudioManager::load_sounds() {
    bool success = true;

    const std::map<SoundType, std::string> sound_paths = {
        {SoundType::Laser, "assets/sounds/laser.mp3"},
        {SoundType::Explosion, "assets/sounds/explosion.wav"},
        {SoundType::HitSound, "assets/sounds/hit-sound.mp3"},
        {SoundType::PlayerHit, "assets/sounds/player-hit.mp3"},
        {SoundType::LevelUp, "assets/sounds/level-up.mp3"},
        {SoundType::Plop, "assets/sounds/plop.wav"},
        {SoundType::Coin, "assets/sounds/coin.wav"},
        {SoundType::BossRoar, "assets/sounds/monster-roar.mp3"},
        {SoundType::BossExplosion, "assets/sounds/multiexplosion.mp3"}};

    for (const auto& [type, path] : sound_paths) {
        if (!sound_buffers_[type].loadFromFile(path)) {
            std::cerr << "[AudioManager] Failed to load sound: " << path << std::endl;
            success = false;
        } else {
            std::cout << "[AudioManager] Loaded sound: " << path << std::endl;
        }
    }

    return success;
}

void AudioManager::play_sound(SoundType type) {
    if (sound_muted_) {
        return;
    }

    auto it = sound_buffers_.find(type);
    if (it == sound_buffers_.end()) {
        return;
    }

    float volume_multiplier = 1.0f;
    switch (type) {
        case SoundType::Laser:
            volume_multiplier = 0.4f;
            break;
        case SoundType::Explosion:
            volume_multiplier = 0.5f;
            break;
        case SoundType::HitSound:
            volume_multiplier = 0.5f;
            break;
        case SoundType::PlayerHit:
            volume_multiplier = 4.0f;
            break;
        case SoundType::Coin:
            volume_multiplier = 0.5f;
            break;
        case SoundType::BossRoar:
            volume_multiplier = 2.0f;
            break;
        default:
            volume_multiplier = 1.0f;
            break;
    }

    sf::Sound& sound = get_next_sound();
    sound.setBuffer(it->second);
    sound.setVolume(get_effective_volume(sound_volume_) * volume_multiplier);
    sound.play();
}

sf::Sound& AudioManager::get_next_sound() {
    for (size_t i = 0; i < SOUND_POOL_SIZE; ++i) {
        size_t index = (current_sound_index_ + i) % SOUND_POOL_SIZE;
        if (sound_pool_[index].getStatus() == sf::Sound::Stopped) {
            current_sound_index_ = (index + 1) % SOUND_POOL_SIZE;
            return sound_pool_[index];
        }
    }

    sf::Sound& sound = sound_pool_[current_sound_index_];
    current_sound_index_ = (current_sound_index_ + 1) % SOUND_POOL_SIZE;
    return sound;
}

void AudioManager::play_music(const std::string& music_path, bool loop) {
    if (music_muted_) {
        current_music_path_ = music_path;
        return;
    }

    if (current_music_path_ == music_path && music_.getStatus() == sf::Music::Playing) {
        return;
    }

    if (!music_.openFromFile(music_path)) {
        std::cerr << "[AudioManager] Failed to load music: " << music_path << std::endl;
        return;
    }

    current_music_path_ = music_path;
    music_.setLoop(loop);
    music_.setVolume(get_effective_volume(music_volume_));
    music_.play();
    std::cout << "[AudioManager] Playing music: " << music_path << std::endl;
}

void AudioManager::stop_music() {
    music_.stop();
    current_music_path_.clear();
}

void AudioManager::pause_music() {
    music_.pause();
}

void AudioManager::resume_music() {
    if (music_.getStatus() == sf::Music::Paused) {
        music_.play();
    }
}

bool AudioManager::is_music_playing() const {
    return music_.getStatus() == sf::Music::Playing;
}

std::string AudioManager::get_current_music() const {
    return current_music_path_;
}

void AudioManager::set_sound_volume(float volume) {
    sound_volume_ = std::clamp(volume, 0.0f, 100.0f);
}

void AudioManager::set_music_volume(float volume) {
    music_volume_ = std::clamp(volume, 0.0f, 100.0f);
    music_.setVolume(get_effective_volume(music_volume_));
}

void AudioManager::set_master_volume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 100.0f);
    music_.setVolume(get_effective_volume(music_volume_));
}

float AudioManager::get_sound_volume() const {
    return sound_volume_;
}

float AudioManager::get_music_volume() const {
    return music_volume_;
}

float AudioManager::get_master_volume() const {
    return master_volume_;
}

void AudioManager::set_music_muted(bool muted) {
    music_muted_ = muted;
    if (muted) {
        music_.pause();
    } else {
        if (!current_music_path_.empty() && music_.getStatus() == sf::Music::Paused) {
            music_.play();
        }
    }
}

void AudioManager::set_sound_muted(bool muted) {
    sound_muted_ = muted;
}

bool AudioManager::is_music_muted() const {
    return music_muted_;
}

bool AudioManager::is_sound_muted() const {
    return sound_muted_;
}

float AudioManager::get_effective_volume(float base_volume) const {
    return (base_volume * master_volume_) / 100.0f;
}

#endif

}  // namespace managers
