#include "Managers/ScreenshotManager.h"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <chrono>
#include <ctime>
#include <format>
#include <string>

#include "Utils/Log.h"

ScreenshotManager::ScreenshotManager(const sf::RenderWindow &window)
    : window_(window) {}

void ScreenshotManager::Take() const {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm *local = std::localtime(&t);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  std::string filename =
      std::format("Screenshot_{:04d}{:02d}{:02d}_{:02d}{:02d}{:02d}_{:03d}.png",
                  local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                  local->tm_hour, local->tm_min, local->tm_sec, ms.count());

  sf::Texture screenshot(window_.getSize());
  screenshot.update(window_);

  if (screenshot.copyToImage().saveToFile("Content/Screenshots/" + filename)) {
    LOG_INFO("Screenshot saved as {}", filename);
  } else {
    LOG_WARNING("Failed to save screenshot to {}", filename);
  }
}
