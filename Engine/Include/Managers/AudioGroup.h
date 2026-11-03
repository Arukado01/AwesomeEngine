#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <memory>
#include <vector>

enum class AudioChannel { Music, Effect };

class AudioManager;

class AudioGroup {
  friend class AudioManager;

private:
  struct Entry {
    std::shared_ptr<sf::SoundSource> source;
    AudioChannel channel;
    float volume;
    bool resume = false;
  };
  struct GroupState {
    std::vector<Entry> sources;
    bool paused = false;
  };
  struct MixerState {
    float musicVolume = 100;
    float effectVolume = 100;
    std::vector<std::weak_ptr<GroupState>> groups;
  };

  std::shared_ptr<MixerState> mixer_;
  std::shared_ptr<GroupState> group_;

  explicit AudioGroup(std::shared_ptr<MixerState> mixer);
  static void ApplyVolume(Entry &entry, const MixerState &mixer);
  void Add(std::shared_ptr<sf::SoundSource> source, AudioChannel channel, float volume);

public:
  AudioGroup(const AudioGroup &) = delete;
  AudioGroup &operator=(const AudioGroup &) = delete;
  AudioGroup(AudioGroup &&) = default;
  AudioGroup &operator=(AudioGroup &&) = delete;
  ~AudioGroup();

  std::shared_ptr<sf::Sound> AddSound(std::shared_ptr<const sf::SoundBuffer> buffer, float volume, bool looping = false);
  std::shared_ptr<sf::Music> AddMusic(sf::Music music, float volume, bool looping = false);
  /* Ajusta el volumen individual para que los cambios de categoría respeten ese valor. */
  void SetSourceVolume(sf::SoundSource &source, float volume);
  void Play(sf::SoundSource &source, bool restart = false);
  void Stop(sf::SoundSource &source);
  void Stop(AudioChannel channel);
  void StopAll();
  void SetPaused(bool paused);
};
