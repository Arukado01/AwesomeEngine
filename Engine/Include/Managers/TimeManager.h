#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

class TimeManager {
private:
  sf::Clock clock_;
  sf::Time deltaTime_;
  sf::Time previousTime_;

public:
  float GetDeltaTime() const;
  float GetElapsedTime() const;

private:
  // Solo la clase Engine puede acceder a la funcion privada Update
  friend class Engine;
  void Update();
};
