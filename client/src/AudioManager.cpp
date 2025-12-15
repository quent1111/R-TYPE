#include "AudioManager.hpp"

#include <iostream>

AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager() {
    sound_pool_.resize(SOUND_POOL_SIZE);
}

bool AudioManager::loadSounds() {
    bool success = true;

    const std::map<SoundType, std::string> sound_paths = {
        {SoundType::Laser, "assets/sounds/laser.mp3"},
        {SoundType::Explosion, "assets/sounds/explosion.wav"},
        {SoundType::HitSound, "assets/sounds/hit-sound.mp3"},
        {SoundType::PlayerHit, "assets/sounds/player-hit.mp3"},
        {SoundType::LevelUp, "assets/sounds/level-up.mp3"},
        {SoundType::Plop, "assets/sounds/plop.wav"},
        {SoundType::Coin, "assets/sounds/coin.wav"}};

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

void AudioManager::playSound(SoundType type) {
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
        default:
            volume_multiplier = 1.0f;
            break;
    }

    sf::Sound& sound = getNextSound();
    sound.setBuffer(it->second);
    sound.setVolume(getEffectiveVolume(sound_volume_) * volume_multiplier);
    sound.play();
}

sf::Sound& AudioManager::getNextSound() {
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

void AudioManager::playMusic(const std::string& musicPath, bool loop) {
    if (current_music_path_ == musicPath && music_.getStatus() == sf::Music::Playing) {
        return;
    }

    if (!music_.openFromFile(musicPath)) {
        std::cerr << "[AudioManager] Failed to load music: " << musicPath << std::endl;
        return;
    }

    current_music_path_ = musicPath;
    music_.setLoop(loop);
    music_.setVolume(getEffectiveVolume(music_volume_));
    music_.play();
    std::cout << "[AudioManager] Playing music: " << musicPath << std::endl;
}

void AudioManager::stopMusic() {
    music_.stop();
    current_music_path_.clear();
}

void AudioManager::pauseMusic() {
    music_.pause();
}

void AudioManager::resumeMusic() {
    if (music_.getStatus() == sf::Music::Paused) {
        music_.play();
    }
}

bool AudioManager::isMusicPlaying() const {
    return music_.getStatus() == sf::Music::Playing;
}

void AudioManager::setSoundVolume(float volume) {
    sound_volume_ = std::max(0.0f, std::min(100.0f, volume));
}

void AudioManager::setMusicVolume(float volume) {
    music_volume_ = std::max(0.0f, std::min(100.0f, volume));
    music_.setVolume(getEffectiveVolume(music_volume_));
}

void AudioManager::setMasterVolume(float volume) {
    master_volume_ = std::max(0.0f, std::min(100.0f, volume));
    music_.setVolume(getEffectiveVolume(music_volume_));
}

float AudioManager::getEffectiveVolume(float base_volume) const {
    return (base_volume * master_volume_) / 100.0f;
}
