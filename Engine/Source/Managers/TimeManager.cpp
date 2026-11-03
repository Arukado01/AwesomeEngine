#include "Managers/TimeManager.h"
#include "Core/EngineConfig.h"
#include <algorithm>

/* Calcula el tiempo transcurrido desde la última actualización del motor. */
void TimeManager::Update() {
  // El reloj real sigue avanzando incluso cuando la escena está pausada o sin foco.
  const sf::Time currentTime = clock_.getElapsedTime();
  // Limita el delta para evitar saltos grandes en la simulación tras una demora.
  deltaTime_ = std::min(currentTime - previousTime_, gConfig.maximumDeltaTime);
  // Guarda el instante actual para calcular la diferencia en el siguiente frame.
  previousTime_ = currentTime;
}

/* Devuelve en segundos el delta calculado en la última actualización. */
float TimeManager::GetDeltaTime() const { return deltaTime_.asSeconds(); }

/* Devuelve en segundos el tiempo real transcurrido desde que se creó el gestor. */
float TimeManager::GetElapsedTime() const {
  return clock_.getElapsedTime().asSeconds();
}

/* Devuelve el tiempo de juego, que se detiene durante la pausa. */
sf::Time TimeManager::GetGameTime() const { return gameClock_.getElapsedTime(); }

/* Devuelve el tiempo real, que continúa mientras el juego está pausado. */
sf::Time TimeManager::GetRealTime() const { return clock_.getElapsedTime(); }

/* Pausa o reanuda el reloj de juego sin afectar el reloj real. */
void TimeManager::SetPaused(bool paused) {
  if (paused) {
    // Detiene solo el reloj de juego para conservar el tiempo acumulado.
    gameClock_.stop();
  } else {
    // Continúa desde el instante en que se pausó, sin reiniciar el reloj.
    gameClock_.start();
  }
}

/* Crea un temporizador vinculado a este gestor y al dominio elegido. */
Cooldown TimeManager::CreateCooldown(float duration, TimeDomain domain) const { return Cooldown(duration, *this, domain); }
