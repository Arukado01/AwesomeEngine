#include "Managers/AudioGroup.h"

#include <SFML/Audio/SoundBuffer.hpp>
#include <algorithm>

AudioGroup::AudioGroup(std::shared_ptr<MixerState> mixer) : mixer_(std::move(mixer)), group_(std::make_shared<GroupState>()) {
  // Retira grupos cerrados y conserva solo referencias débiles a las escenas activas.
  std::erase_if(mixer_->groups, [](const auto &group) { return group.expired(); });
  mixer_->groups.push_back(group_);
}

/* Detiene el audio de la escena incluso si alguna fuente sigue compartida fuera del grupo. */
AudioGroup::~AudioGroup() {
  if (group_) {
    StopAll();
  }
}

/* Combina el volumen del sonido con el de su categoría, sin alterar el volumen maestro. */
void AudioGroup::ApplyVolume(Entry &entry, const MixerState &mixer) {
  // El volumen maestro se aplica por separado mediante el listener de SFML.
  const float category = entry.channel == AudioChannel::Music ? mixer.musicVolume : mixer.effectVolume;
  entry.source->setVolume(entry.volume * category / 100.f);
}

/* Registra una fuente compartida y conserva su volumen individual. */
void AudioGroup::Add(std::shared_ptr<sf::SoundSource> source, AudioChannel channel, float volume) {
  // Una fuente que ya sonaba también debe respetar la pausa del grupo.
  const bool resume = group_->paused && source->getStatus() == sf::SoundSource::Status::Playing;
  if (resume) {
    source->pause();
  }
  group_->sources.push_back({std::move(source), channel, std::clamp(volume, 0.f, 100.f), resume});
  ApplyVolume(group_->sources.back(), *mixer_);
}

/* Crea un efecto cuyo buffer permanece vivo mientras exista el sonido. */
std::shared_ptr<sf::Sound> AudioGroup::AddSound(std::shared_ptr<const sf::SoundBuffer> buffer, float volume, bool looping) {
  // Un recurso ausente o vacío no debe detener la escena.
  if (!buffer || buffer->getSampleCount() == 0) {
    return {};
  }
  // El destructor compartido conserva el buffer hasta que desaparezca el último sonido que lo usa.
  auto sound = std::shared_ptr<sf::Sound>(new sf::Sound(*buffer), [buffer](sf::Sound *source) { delete source; });
  sound->setLooping(looping);
  Add(sound, AudioChannel::Effect, volume);
  return sound;
}

/* Crea una pista para la escena y permite conservar su volumen individual. */
std::shared_ptr<sf::Music> AudioGroup::AddMusic(sf::Music music, float volume, bool looping) {
  // La pista se mueve al grupo para que no dependa de una variable temporal.
  auto source = std::make_shared<sf::Music>(std::move(music));
  source->setLooping(looping);
  Add(source, AudioChannel::Music, volume);
  return source;
}

/* Cambia el volumen de una fuente sin perder el ajuste global de su categoría. */
void AudioGroup::SetSourceVolume(sf::SoundSource &source, float volume) {
  // Se guarda el nivel individual para poder combinarlo de nuevo si cambia la categoría.
  const auto found = std::ranges::find_if(group_->sources, [&](const Entry &entry) { return entry.source.get() == &source; });
  if (found != group_->sources.end()) {
    found->volume = std::clamp(volume, 0.f, 100.f);
    ApplyVolume(*found, *mixer_);
  }
}

/* Reproduce una fuente del grupo y permite reiniciarla para sonidos puntuales. */
void AudioGroup::Play(sf::SoundSource &source, bool restart) {
  const auto found = std::ranges::find_if(group_->sources, [&](const Entry &entry) { return entry.source.get() == &source; });
  // Los bucles activos conservan su posición; restart permite repetir un efecto puntual.
  if (!group_->paused && found != group_->sources.end() && (restart || source.getStatus() == sf::SoundSource::Status::Stopped)) {
    source.play();
  }
}

/* Detiene una fuente y cancela cualquier reanudación pendiente. */
void AudioGroup::Stop(sf::SoundSource &source) {
  const auto found = std::ranges::find_if(group_->sources, [&](const Entry &entry) { return entry.source.get() == &source; });
  if (found != group_->sources.end()) {
    // Una fuente detenida no debe revivir al quitar la pausa.
    found->resume = false;
    if (source.getStatus() != sf::SoundSource::Status::Stopped) {
      source.stop();
    }
  }
}

/* Detiene solo las fuentes de la categoría indicada. */
void AudioGroup::Stop(AudioChannel channel) {
  for (auto &entry : group_->sources) {
    // Las demás categorías conservan su estado, como la música durante Game Over.
    if (entry.channel == channel) {
      entry.resume = false;
      if (entry.source->getStatus() != sf::SoundSource::Status::Stopped) {
        entry.source->stop();
      }
    }
  }
}

/* Detiene todas las fuentes y deja el grupo listo para una nueva partida. */
void AudioGroup::StopAll() {
  for (auto &entry : group_->sources) {
    // Cancela también las reanudaciones pendientes antes de salir de la escena.
    entry.resume = false;
    if (entry.source->getStatus() != sf::SoundSource::Status::Stopped) {
      entry.source->stop();
    }
  }
  group_->paused = false;
}

/* Pausa las fuentes activas y reanuda únicamente las que este grupo pausó. */
void AudioGroup::SetPaused(bool paused) {
  // Repetir la misma orden no debe reiniciar ni volver a pausar los sonidos.
  if (group_->paused == paused) {
    return;
  }
  group_->paused = paused;
  for (auto &entry : group_->sources) {
    if (paused) {
      // Solo se recuerdan las fuentes que estaban sonando al comenzar la pausa.
      entry.resume = entry.source->getStatus() == sf::SoundSource::Status::Playing;
      if (entry.resume) {
        entry.source->pause();
      }
    } else {
      // Una fuente pausada por otra causa no se reanuda desde este grupo.
      if (entry.resume && entry.source->getStatus() == sf::SoundSource::Status::Paused) {
        entry.source->play();
      }
      entry.resume = false;
    }
  }
}
