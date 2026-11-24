#pragma once
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

class ResourceManager;
class RenderManager;

class CursorManager {
private:
  sf::RenderWindow &window_;
  RenderManager &renderer_; // Dueño de la vista letterbox compartida con el render
  sf::RectangleShape shape_;
  float speed_;
  bool visible_;

public:
  CursorManager(sf::RenderWindow &window, ResourceManager &resources,
                RenderManager &renderer);

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
