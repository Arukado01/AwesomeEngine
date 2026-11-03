#include "Managers/AudioManager.h"
#include <SFML/Audio/Listener.hpp>
#include <algorithm>

/* Crea un grupo que la escena puede usar para controlar sus fuentes de audio. */
AudioGroup AudioManager::CreateGroup() const { return AudioGroup(mixer_); }

/* Ajusta una categoría en todos los grupos, respetando el volumen propio de cada sonido. */
void AudioManager::SetChannelVolume(AudioChannel channel, float volume) {
  const float clamped = std::clamp(volume, 0.f, 100.f);
  (channel == AudioChannel::Music ? mixer_->musicVolume : mixer_->effectVolume) = clamped;
  std::erase_if(mixer_->groups, [](const auto &group) { return group.expired(); });
  for (const auto &weakGroup : mixer_->groups) {
    if (const auto group = weakGroup.lock()) {
      for (auto &entry : group->sources) {
        if (entry.channel == channel) {
          AudioGroup::ApplyVolume(entry, *mixer_);
        }
      }
    }
  }
}

/* Devuelve el volumen configurado para la música o los efectos. */
float AudioManager::GetChannelVolume(AudioChannel channel) const {
  return channel == AudioChannel::Music ? mixer_->musicVolume : mixer_->effectVolume;
}

void AudioManager::SetGlobalVolume(float volume) {
  sf::Listener::setGlobalVolume(volume);
}

float AudioManager::GetGlobalVolume() const {
  return sf::Listener::getGlobalVolume();
}

void AudioManager::SetListenerPosition(sf::Vector3f position) {
  sf::Listener::setPosition(position);
}

sf::Vector3f AudioManager::GetListenerPosition() const {
  return sf::Listener::getPosition();
}

void AudioManager::SetListenerDirection(sf::Vector3f direction) {
  sf::Listener::setDirection(direction);
}

sf::Vector3f AudioManager::GetListenerDirection() const {
  return sf::Listener::getDirection();
}
