#pragma once
#include <SFML/Window/Event.hpp>

struct EngineVisitor {
  class Engine &engine;

  void operator()(const sf::Event::Closed &);
  void operator()(const auto &) {}
};
