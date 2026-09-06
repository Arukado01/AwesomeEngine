#pragma once

#include "Core/EngineVisitor.h"
#include <SFML/Graphics.hpp>

class Engine {
private:
  sf::RenderWindow window_;

public:
  Engine();

  bool IsRunning() const;

  void ProcessEvent();
  void Update();
  void Render();

private:
  friend EngineVisitor;
  void EventWindowClose();
  void EventWindowResized(sf::Vector2u size);
  void EventWindowFocusLost();
  void EventWindowFocusGained();
};
