#pragma once
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class ResourceManager;

class CursorManager {
private:
  sf::RenderWindow &window_;
  sf::RectangleShape shape_;
  float speed_;
  bool visible_;

public:
  CursorManager(sf::RenderWindow &window, ResourceManager &resources);

  void SetSpeed(float speed);
  float GetSpeed() const;

  void SetVisible(bool visible);
  bool IsVisible() const;

  void SetPosition(sf::Vector2f position);
  sf::Vector2f GetPosition() const;
  sf::Vector2f GetPosition(const sf::View &view) const;

private:
  friend class Engine;
  void Update(float deltaTime);
  void Render() const;
};
