#pragma once
#include <SFML/System/Vector2.hpp>
#include <string>

struct EngineConfig {
  std::string windowTitle;
  sf::Vector2f windowSize;

  EngineConfig();
};

inline const EngineConfig gConfig;
