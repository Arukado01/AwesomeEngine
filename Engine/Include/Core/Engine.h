#pragma once

#include "Core/EngineVistor.h"
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
};
