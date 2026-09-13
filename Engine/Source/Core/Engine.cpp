#include "Core/Engine.h"
#include "Core/EngineConfig.h"
#include "Utils/Log.h"

Engine::Engine()
    : window_(sf::VideoMode(sf::Vector2u(gConfig.windowSize)),
              gConfig.windowTitle),
      context_(window_) {
  window_.setIcon(sf::Image("Content/Textures/Icon.png"));
  window_.setMinimumSize(window_.getSize() / 2u);
  window_.setKeyRepeatEnabled(false);
  window_.setMouseCursorVisible(false);

  /** Si disableSfmlLogs es true, desabilita los logs propios de SFML
  (Esto para usar el sistema propio de logs) **/
  if (gConfig.disableSfmlLogs) {
    sf::err().rdbuf(nullptr);
  }

  // Volumen Global
  context_.audio.SetGlobalVolume(gConfig.globalVolume);

  LOG_INFO("Window created");
}

bool Engine::IsRunning() const { return window_.isOpen(); }

void Engine::ProcessEvent() {
  while (const auto event = window_.pollEvent()) {

    event->visit(EngineVisitor{*this});
    context_.gui.ProcessEvent(*event);
  }
}

void Engine::Update() {
  context_.time.Update();
  context_.cursor.Update(context_.time.GetDeltaTime());
}

void Engine::Render() {
  window_.clear();

  context_.renderer.BeginDrawing();
  window_.draw(sf::Sprite(context_.renderer.FinishDrawing()));

  context_.gui.Render();
  context_.cursor.Render();

  window_.display();
}

void Engine::EventWindowClose() {
  window_.close();
  LOG_INFO("Window closed");
}

void Engine::EventWindowResized(sf::Vector2u size) {
  LOG_INFO("Window resized to: {}x{}", size.x, size.y);
}

void Engine::EventWindowFocusLost() { LOG_INFO("Window focus lost"); }
void Engine::EventWindowFocusGained() { LOG_INFO("Window focus gained"); }

void Engine::EventGamepadConnected(int id) {
  LOG_INFO("Gamepad {} connected", id);
}

void Engine::EventGamepadDisconnected(int id) {
  LOG_INFO("Gamepad {} disconnected", id);
}

void Engine::EventWindowScreenshot() const { context_.screenshot.Take(); }
