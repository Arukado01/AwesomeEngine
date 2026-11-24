#include "Managers/CursorManager.h"

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>

#include "Core/EngineConfig.h"
#include "Managers/ResourceManager.h"

/* Usa el logo del motor como cursor y coloca el punto de clic en la punta de la A. */
CursorManager::CursorManager(sf::RenderWindow &window, ResourceManager &resources)
    : window_(window), speed_(gConfig.cursorSpeed), visible_(true) {
  if (auto *texture = resources.FetchTexture("Icon.png")) {
    const sf::Vector2f imageSize(texture->getSize());
    const float longestSide = std::max(imageSize.x, imageSize.y);
    if (longestSide > 0.f) {
      const auto cursorSize = imageSize * (gConfig.cursorSize / longestSide);
      shape_.setTexture(texture);
      shape_.setSize(cursorSize);
      // El centro del borde superior corresponde a la punta visible de la A.
      shape_.setOrigin({cursorSize.x / 2.f, 0.f});
      // Gira alrededor de la punta para que el borde izquierdo quede vertical.
      shape_.setRotation(sf::degrees(-29.5f));
    }
  }
  shape_.setPosition(gConfig.windowSize / 2.f);
}

void CursorManager::Update(float deltaTime) {
  const sf::Vector2f joystickDirection(
      sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) / 100,
      sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) / 100);

  if (joystickDirection.length() > gConfig.joystickDeadzone) {
    shape_.move(joystickDirection * speed_ * deltaTime);
    shape_.setPosition(
        {std::clamp(shape_.getPosition().x, 0.f, gConfig.windowSize.x - 1),
         std::clamp(shape_.getPosition().y, 0.f, gConfig.windowSize.y - 1)});

    SetPosition(shape_.getPosition());
  } else {
    shape_.setPosition(GetPosition());
  }
}

void CursorManager::Render() const {
  if (visible_) {
    window_.draw(shape_);
  }
}

void CursorManager::SetSpeed(float speed) { speed_ = speed; }

float CursorManager::GetSpeed() const { return speed_; }

void CursorManager::SetVisible(bool visible) { visible_ = visible; }

bool CursorManager::IsVisible() const { return visible_; }

void CursorManager::SetPosition(sf::Vector2f position) {
  sf::Mouse::setPosition(
      window_.mapCoordsToPixel(position, window_.getDefaultView()), window_);
}

sf::Vector2f CursorManager::GetPosition() const {
  return GetPosition(window_.getDefaultView());
}

sf::Vector2f CursorManager::GetPosition(const sf::View &view) const {
  return window_.mapPixelToCoords(sf::Mouse::getPosition(window_), view);
}
