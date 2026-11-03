#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <functional>

struct SpriteAnimationClip {
  int firstFrame;
  int frameCount;
  float framesPerSecond;
};

enum class SpritePlaybackMode {
  Loop,
  Once
};

class SpriteAnimator {
private:
  double position_ = 0;
  SpritePlaybackMode mode_ = SpritePlaybackMode::Loop;
  bool paused_ = false;
  bool finished_ = false;
  std::uint64_t revision_ = 0;

public:
  /* Elige si la animación se repite o termina y reinicia el ciclo al cambiar de modo. */
  void SetMode(SpritePlaybackMode mode);

  /* Devuelve el modo actual de reproducción. */
  SpritePlaybackMode GetMode() const;

  /* Vuelve al primer cuadro y deja la animación lista para avanzar; úsalo al cambiar de clip. */
  void Reset();

  /* Pausa el avance sin perder el cuadro ni la posición entre cuadros. */
  void Pause();

  /* Continúa una animación pausada; una animación terminada necesita Reset. */
  void Resume();

  /* Indica si la animación puede avanzar. */
  bool IsPlaying() const;

  /* Indica si una reproducción única completó el tiempo de su último cuadro. */
  bool IsFinished() const;

  /* Devuelve el cuadro actual dentro del clip, empezando por cero. */
  int CurrentFrame(int frameCount) const;

  /* Avanza según el tiempo y la velocidad; avisa cada cuadro cruzado y devuelve el cuadro visible. */
  /* El callback recibe índices locales desde cero; el cuadro inicial no se notifica hasta cerrar un bucle. */
  int Advance(float deltaTime, int frameCount, float framesPerSecond, const std::function<void(int)> &onFrame = {});

  /* Calcula la casilla de una hoja de sprites para el cuadro indicado. */
  static sf::IntRect FrameRect(const SpriteAnimationClip &clip, int frame, int columns, sf::Vector2i frameSize);
};
