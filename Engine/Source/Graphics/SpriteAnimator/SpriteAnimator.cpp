#include "Graphics/SpriteAnimator/SpriteAnimator.h"

#include <algorithm>
#include <cmath>
#include <limits>

/* Cambia el modo desde el primer cuadro para no mezclar estados de reproducción distintos. */
void SpriteAnimator::SetMode(SpritePlaybackMode mode) {
  if (mode_ != mode) {
    mode_ = mode;
    Reset();
  }
}

/* Permite consultar si el ciclo se repite o termina. */
SpritePlaybackMode SpriteAnimator::GetMode() const { return mode_; }

/* Limpia la posición y los estados de pausa y finalización para empezar de nuevo. */
void SpriteAnimator::Reset() {
  ++revision_;
  position_ = 0;
  paused_ = false;
  finished_ = false;
}

/* Detiene el avance hasta que se solicite continuar. */
void SpriteAnimator::Pause() {
  ++revision_;
  paused_ = true;
}

/* Quita la pausa sin reiniciar una animación que ya terminó. */
void SpriteAnimator::Resume() {
  ++revision_;
  paused_ = false;
}

/* Comprueba que el avance no esté pausado ni haya terminado. */
bool SpriteAnimator::IsPlaying() const { return !paused_ && !finished_; }

/* Informa si se completó una reproducción configurada para ejecutarse una vez. */
bool SpriteAnimator::IsFinished() const { return finished_; }

/* Devuelve el cuadro visible con el índice relativo al clip. */
int SpriteAnimator::CurrentFrame(int frameCount) const {
  if (frameCount <= 0) {
    return 0;
  }
  // El último cuadro permanece visible después de finalizar una reproducción única.
  if (mode_ == SpritePlaybackMode::Once) {
    return static_cast<int>(std::min(position_, static_cast<double>(frameCount - 1)));
  }
  return static_cast<int>(std::fmod(position_, static_cast<double>(frameCount)));
}

/* Convierte segundos en cuadros y avisa los cuadros que se alcanzan durante el avance. */
int SpriteAnimator::Advance(float deltaTime, int frameCount, float framesPerSecond, const std::function<void(int)> &onFrame) {
  // Sin tiempo o velocidad válidos, o durante la pausa, se conserva el cuadro actual.
  if (frameCount <= 0 || !std::isfinite(deltaTime) || !std::isfinite(framesPerSecond) ||
      deltaTime <= 0 || framesPerSecond <= 0 || !IsPlaying()) {
    return CurrentFrame(frameCount);
  }

  // Detecta si un callback pausa, reinicia o vuelve a avanzar este animador.
  const auto revision = ++revision_;
  // Una posición decimal conserva la parte del tiempo que aún no completa otro cuadro.
  const double next = position_ + static_cast<double>(deltaTime) * framesPerSecond;
  const double end = mode_ == SpritePlaybackMode::Once ? std::min(next, static_cast<double>(frameCount)) : next;

  // Al saltar varios cuadros, se notifica cada uno en orden para no perder eventos como pisadas.
  if (onFrame && end < static_cast<double>(std::numeric_limits<long long>::max())) {
    const auto last = static_cast<long long>(std::floor(end));
    for (auto crossed = static_cast<long long>(std::floor(position_)) + 1; crossed <= last; ++crossed) {
      // En reproducción única no existe un cuadro adicional al terminar el último.
      if (mode_ == SpritePlaybackMode::Once && crossed >= frameCount) {
        break;
      }
      // El callback observa el cuadro que acaba de alcanzarse, incluso si pausa el ciclo.
      position_ = mode_ == SpritePlaybackMode::Once ? static_cast<double>(crossed)
                                                     : static_cast<double>(crossed % frameCount);
      onFrame(static_cast<int>(crossed % frameCount));
      if (revision_ != revision) {
        // Una orden recibida durante el evento reemplaza el resto del avance anterior.
        return CurrentFrame(frameCount);
      }
    }
  }

  if (mode_ == SpritePlaybackMode::Once) {
    // El último cuadro se muestra durante su duración completa antes de marcar el fin.
    finished_ = next >= frameCount;
    position_ = std::min(next, static_cast<double>(frameCount));
  } else {
    // En bucle, conservar solo la posición dentro del ciclo evita que crezca indefinidamente.
    position_ = std::fmod(next, static_cast<double>(frameCount));
  }
  return CurrentFrame(frameCount);
}

/* Traduce el número del cuadro a su posición y tamaño dentro de la imagen. */
sf::IntRect SpriteAnimator::FrameRect(const SpriteAnimationClip &clip, int frame, int columns, sf::Vector2i frameSize) {
  if (clip.firstFrame < 0 || clip.frameCount <= 0 || frame < 0 || frame >= clip.frameCount ||
      columns <= 0 || frameSize.x <= 0 || frameSize.y <= 0) {
    return {};
  }
  const int sheetFrame = clip.firstFrame + frame;
  // La columna se obtiene con el resto y la fila con la división entera.
  return {{(sheetFrame % columns) * frameSize.x, (sheetFrame / columns) * frameSize.y}, frameSize};
}
