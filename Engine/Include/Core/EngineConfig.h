#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

/* Juego registrado para el menú principal, cargado desde Config.json. */
struct GameEntry {
  std::string name;     // nombre de la escena registrado en SceneFactory
  std::string title;    // título visible en el botón del menú
  std::string controls; // controles mostrados en la pantalla de ayuda
};

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
  std::vector<GameEntry> games;

  EngineConfig();
};

inline const EngineConfig gConfig;
