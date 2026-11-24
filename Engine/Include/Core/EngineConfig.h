#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

struct EngineConfig {
  std::string windowTitle;
  sf::Vector2f windowSize;
  unsigned int frameRateLimit;
  bool disableSfmlLogs;
  sf::Time maximumDeltaTime;
  float globalVolume;
  sf::Color backgroundColor;
  float cursorSize;
  float cursorSpeed;
  float joystickDeadzone;

  EngineConfig();
};

inline const EngineConfig gConfig;
