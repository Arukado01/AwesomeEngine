#include "Core/Engine.h"
#include "Core/EngineConfig.h"

Engine::Engine()
    : window_(sf::VideoMode(sf::Vector2u(gConfig.windowSize)),
              gConfig.windowTitle) {
  window_.setIcon(sf::Image("Content/Textures/Icon.png"));
  window_.setMinimumSize(window_.getSize() / 2u);
}

bool Engine::IsRunning() const { return window_.isOpen(); }

void Engine::ProcessEvent() {
  while (const auto event = window_.pollEvent()) {
    event->visit(EngineVisitor{*this});
  }
}

void Engine::Update() {}

void Engine::Render() {
  window_.clear();

  window_.display();
}

void Engine::EventWindowClose() { window_.close(); }
