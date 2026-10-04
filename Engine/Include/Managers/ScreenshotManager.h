#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

class ScreenshotManager {
private:
  const sf::RenderWindow &window_;
  sf::SoundBuffer soundBuffer_;
  mutable sf::Sound sound_;

public:
  ScreenshotManager(const sf::RenderWindow &window);
  void Take() const;

private:
  [[nodiscard]] static sf::Sound MakeSound(sf::SoundBuffer &buffer);
};
