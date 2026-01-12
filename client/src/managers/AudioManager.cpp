// Audio disabled - stub implementation (openal-soft incompatible with GCC 15)
#include "managers/AudioManager.hpp"

#include <algorithm>
#include <iostream>

namespace managers {

AudioManager& AudioManager::instance() {
    static AudioManager instance;
    return instance;
}

bool AudioManager::load_sounds() {
    std::cout << "[AudioManager] Audio disabled (no-audio build)" << std::endl;
    return true;
}

void AudioManager::play_sound([[maybe_unused]] SoundType type) {
    // No-op: audio disabled
}

void AudioManager::play_music([[maybe_unused]] const std::string& music_path, [[maybe_unused]] bool loop) {
    // No-op: audio disabled
}

void AudioManager::stop_music() {
    current_music_path_.clear();
}

void AudioManager::pause_music() {
    // No-op: audio disabled
}

void AudioManager::resume_music() {
    // No-op: audio disabled
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

float AudioManager::get_effective_volume(float base_volume) const {
    return (base_volume * master_volume_) / 100.0f;
}

}  // namespace managers
