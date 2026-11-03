#include "Managers/ResourceManager.h"
#include "Utils/Log.h"

sf::Texture *ResourceManager::FetchTexture(const std::string &filename) {
  if (!textures_.contains(filename)) {
    if (!textures_[filename].loadFromFile("Content/Textures/" + filename)) {
      LOG_ERROR("Failed to load texture: {}", filename);
      return nullptr;
    }
  }

  return &textures_.at(filename);
}

sf::SoundBuffer *ResourceManager::FetchSound(const std::string &filename) {
  return AcquireSound(filename).get();
}

/* Comparte el buffer cargado para que los sonidos puedan conservarlo durante su reproducción. */
std::shared_ptr<sf::SoundBuffer> ResourceManager::AcquireSound(const std::string &filename) {
  if (!sounds_.contains(filename)) {
    auto buffer = std::make_shared<sf::SoundBuffer>();
    if (!buffer->loadFromFile("Content/Sounds/" + filename)) {
      LOG_ERROR("Failed to load sound: {}", filename);
      return {};
    }
    sounds_.emplace(filename, std::move(buffer));
  }
  return sounds_.at(filename);
}

sf::Font *ResourceManager::FetchFont(const std::string &filename) {
  if (!fonts_.contains(filename)) {
    if (!fonts_[filename].openFromFile("Content/Fonts/" + filename)) {
      LOG_ERROR("Failed to load font: {}", filename);
      return nullptr;
    }
  }

  return &fonts_.at(filename);
}

std::optional<sf::Music>
ResourceManager::FetchMusic(const std::string &filename) const {
  sf::Music music;

  if (!music.openFromFile("Content/Musics/" + filename)) {
    LOG_ERROR("Failed to load music: {}", filename);
    return std::nullopt;
  }

  return music;
}
