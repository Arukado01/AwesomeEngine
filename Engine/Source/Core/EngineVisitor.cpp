#include "Core/EngineVisitor.h"
#include "Core/Engine.h"
#include "Utils/InputBindings.h"

void EngineVisitor::operator()(const sf::Event::Closed &) {
  engine.EventWindowClose();
}

void EngineVisitor::operator()(const sf::Event::Resized &resized) {
  engine.EventWindowResized(resized.size);
}

void EngineVisitor::operator()(const sf::Event::FocusLost &) {
  engine.EventWindowFocusLost();
}

void EngineVisitor::operator()(const sf::Event::FocusGained &) {
  engine.EventWindowFocusGained();
}

void EngineVisitor::operator()(const sf::Event::JoystickConnected &joystick) {
  engine.EventGamepadConnected(joystick.joystickId);
}

void EngineVisitor::operator()(
    const sf::Event::JoystickDisconnected &joystick) {
  engine.EventGamepadDisconnected(joystick.joystickId);
}

void EngineVisitor::operator()(const sf::Event::KeyPressed &key) {
  if (key.scancode == sf::Keyboard::Scan::PrintScreen) {
    engine.EventWindowScreenshot();
  }

  else if (key.scancode == sf::Keyboard::Scan::R) {
    engine.EventSceneRestart();
  }

  else if (key.scancode == sf::Keyboard::Scan::M) {
    engine.EventSceneMenuReturn();
  }

  else if (key.scancode == sf::Keyboard::Scan::Escape) {
    engine.EventOverlayPauseToggle();
  }
}

void EngineVisitor::operator()(
    const sf::Event::JoystickButtonPressed &joystick) {
  if (const auto button =
          Input::HardwareToLogical(joystick.button, joystick.joystickId)) {

    if (*button == GamepadButton::Select) {
      engine.EventSceneMenuReturn();
    } else if (*button == GamepadButton::Start) {
      engine.EventOverlayPauseToggle();
    }
  }
}
