#include "Utils/Cooldown.h"
#include "Managers/TimeManager.h"

/* Crea un temporizador independiente que puede vincularse después al gestor de tiempo. */
Cooldown::Cooldown(float duration) : duration_(duration) {}

/* Crea un temporizador vinculado directamente al reloj de juego o al reloj real. */
Cooldown::Cooldown(float duration, const TimeManager &time, TimeDomain domain)
    : duration_(duration), time_(&time), domain_(domain) { startedAt_ = Now(); }

/* Devuelve el instante del reloj elegido o del reloj independiente si aún no se ha vinculado. */
sf::Time Cooldown::Now() const {
  if (!time_) {
    return standaloneClock_.getElapsedTime();
  }
  return domain_ == TimeDomain::Game ? time_->GetGameTime() : time_->GetRealTime();
}

/* Vincula otro reloj conservando el tiempo acumulado y el estado activo o detenido. */
void Cooldown::Bind(const TimeManager &time, TimeDomain domain) {
  if (running_) {
    elapsed_ += Now() - startedAt_;
  }
  time_ = &time;
  domain_ = domain;
  startedAt_ = Now();
}

/* Continúa desde el tiempo acumulado únicamente si estaba detenido. */
void Cooldown::Start() {
  if (!running_) {
    startedAt_ = Now();
    running_ = true;
  }
}

/* Conserva el tiempo transcurrido y detiene este temporizador. */
void Cooldown::Stop() {
  if (running_) {
    elapsed_ += Now() - startedAt_;
    running_ = false;
  }
}

/* Vuelve a cero y comienza a contar. */
void Cooldown::Restart() { elapsed_ = sf::Time::Zero; startedAt_ = Now(); running_ = true; }

/* Vuelve a cero y deja el temporizador detenido. */
void Cooldown::Reset() { elapsed_ = sf::Time::Zero; running_ = false; }
void Cooldown::SetDuration(float duration) { duration_ = duration; }
float Cooldown::GetDuration() const { return duration_; }
float Cooldown::GetElapsedTime() const { return (elapsed_ + (running_ ? Now() - startedAt_ : sf::Time::Zero)).asSeconds(); }
bool Cooldown::IsRunning() const { return running_; }
bool Cooldown::IsOver() const { return GetElapsedTime() > duration_; }
