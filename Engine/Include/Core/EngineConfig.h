#pragma once
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

struct EngineConfig {
  std::string windowTitle;
  sf::Vector2f windowSize;
  bool disableSfmlLogs;
  sf::Time maximumDeltaTime;
  float globalVolume;

  EngineConfig();
};

inline const EngineConfig gConfig;
