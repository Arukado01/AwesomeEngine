#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

class TimeManager;
enum class TimeDomain { Game, Real };

class Cooldown {
private:
  sf::Time startedAt_; // instante del reloj de juego en el que este temporizador comenzó o se reanudó.
  sf::Time elapsed_; // tiempo que el temporizador ya había acumulado antes de detenerse.
  float duration_; // duración que debe cumplirse, expresada en segundos.
  bool running_ = true; // indica si este temporizador está activo.
  const TimeManager *time_ = nullptr;
  TimeDomain domain_ = TimeDomain::Game;
  sf::Clock standaloneClock_;
  sf::Time Now() const;

public:
  /* Sin vincular, el temporizador usa su reloj independiente de tiempo real. */
  Cooldown(float duration = 0.f);

  /* El gestor indicado debe permanecer vivo durante el uso del temporizador. */
  Cooldown(float duration, const TimeManager &time, TimeDomain domain = TimeDomain::Game);
  void Bind(const TimeManager &time, TimeDomain domain = TimeDomain::Game);

  void Start();
  void Stop();

  void Restart();
  void Reset();

  void SetDuration(float duration);
  float GetDuration() const;

  float GetElapsedTime() const;

  bool IsRunning() const;
  bool IsOver() const;
};
