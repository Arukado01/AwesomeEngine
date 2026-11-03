#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include "Utils/Cooldown.h"

class TimeManager {
private:
  sf::Clock clock_;
  sf::Clock gameClock_;
  sf::Time deltaTime_;
  sf::Time previousTime_;

public:
  float GetDeltaTime() const;
  /* Conserva la consulta anterior de tiempo real en segundos. */
  float GetElapsedTime() const;
  /* Devuelve el tiempo que avanza solamente mientras la partida no está pausada. */
  sf::Time GetGameTime() const;
  /* Devuelve el tiempo transcurrido desde la creación del gestor, incluso durante la pausa. */
  sf::Time GetRealTime() const;
  void SetPaused(bool paused);
  /* El temporizador debe dejar de usarse antes de destruir este gestor. */
  Cooldown CreateCooldown(float duration, TimeDomain domain = TimeDomain::Game) const;

private:
  // Solo la clase Engine puede acceder a la funcion privada Update
  friend class Engine;
  void Update();
};
