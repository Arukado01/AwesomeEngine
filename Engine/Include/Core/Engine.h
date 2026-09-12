#pragma once

#include "Core/EngineContext.h"
#include "Core/EngineVisitor.h"
#include <SFML/Graphics.hpp>

class Engine {
private:
  sf::RenderWindow window_;
  EngineContext context_;

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
  void EventGamepadConnected(int id);
  void EventGamepadDisconnected(int id);
  void EventWindowScreenshot() const;
};
