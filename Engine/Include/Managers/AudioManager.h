#pragma once
#include "Managers/AudioGroup.h"
#include <SFML/System/Vector3.hpp>

class AudioManager {
private:
  std::shared_ptr<AudioGroup::MixerState> mixer_ = std::make_shared<AudioGroup::MixerState>();

public:
  AudioGroup CreateGroup() const;
  void SetChannelVolume(AudioChannel channel, float volume);
  float GetChannelVolume(AudioChannel channel) const;
  void SetGlobalVolume(float volume);
  float GetGlobalVolume() const;

  void SetListenerPosition(sf::Vector3f position);
  sf::Vector3f GetListenerPosition() const;

  void SetListenerDirection(sf::Vector3f direction);
  sf::Vector3f GetListenerDirection() const;
};
