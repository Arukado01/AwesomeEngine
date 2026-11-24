#pragma once

#include "Core/EngineContext.h"
#include "Core/EngineVisitor.h"
#include "Core/Overlay.h"
#include "Scene/SceneFactory.h"
#include <SFML/Graphics.hpp>
#include <optional>

class Engine {
private:
  sf::RenderWindow window_;
  EngineContext context_;

  SceneFactory::Scenes scenes_;
  Scene *currentScene_;

  Overlay overlay_;
  bool cursorWasVisible_;
  bool scenePaused_ = false;

  bool fullscreen_ = false; // Estado actual: ventana normal o pantalla completa
  std::optional<sf::Vector2u> lastWindowedSize_; // Tamaño de la ventana antes de ir a pantalla completa

public:
  Engine();

  bool IsRunning() const;
  bool HasFocus() const;

  void ProcessEvent();
  void Update();
  void Render();

private:
  friend EngineVisitor;
  void EventWindowClose();
  void EventWindowResized(sf::Vector2u size);
  void EventWindowFocusLost();
  void EventWindowFocusGained();
  void SetScenePaused(bool paused);
  void SyncScenePause();
  void EventGamepadConnected(int id);
  void EventGamepadDisconnected(int id);
  void EventWindowScreenshot() const;
  void EventSceneChange(const std::string &name);
  void EventSceneRestart();
  void EventSceneMenuReturn();
  void EventOverlayPauseToggle();
  void EventOverlaySelect(OverlaySelection selection);
  void EventFullscreenToggle();
  void ApplyWindowSettings();
};
