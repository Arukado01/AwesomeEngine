#include "Core/Engine.h"

Engine::Engine() : window_(sf::VideoMode({800, 800}), "Awesome Engine") {
  window_.setIcon(sf::Image("Content/Textures/Icon.png"));
  window_.setMinimumSize(window_.getSize() / 2u);
}

bool Engine::IsRunning() const { return window_.isOpen(); }

void Engine::ProcessEvent() {
  while (const std::optional<sf::Event> event = window_.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      window_.close();
    }
  }
}

void Engine::Update() {}

void Engine::Render() {
  window_.clear();

  window_.display();
}