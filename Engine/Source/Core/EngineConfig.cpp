#include "Core/EngineConfig.h"
#include <cassert>
#include <fstream>
#include <nlohmann/json.hpp>

EngineConfig::EngineConfig() {
  std::ifstream file("Content/Config.json");
  assert(file);

  nlohmann::json json = nlohmann::json::parse(file);
  windowTitle = json["windowTitle"];
  windowSize = {json["windowSize"][0], json["windowSize"][1]};
  frameRateLimit = json.value("frameRateLimit", 120u);
  disableSfmlLogs = json["disableSfmlLogs"];
  maximumDeltaTime = sf::seconds(json["maximumDeltaTime"]);
  globalVolume = json["globalVolume"];
  backgroundColor = {
      json["backgroundColor"][0],
      json["backgroundColor"][1],
      json["backgroundColor"][2],
  };
  cursorSize = json.value("cursorSize", 24.f);
  cursorSpeed = json["cursorSpeed"];
  joystickDeadzone = json["joystickDeadzone"];

  // Juegos del menú principal: sin la lista, el menú queda vacío pero el motor arranca.
  if (json.contains("games")) {
    for (const auto &entry : json["games"]) {
      games.push_back({entry["name"], entry["title"], entry["controls"]});
    }
  }
}
