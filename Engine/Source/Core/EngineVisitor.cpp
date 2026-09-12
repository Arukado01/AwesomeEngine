#include "Core/EngineVisitor.h"
#include "Core/Engine.h"

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
  if (key.control && key.shift && key.scancode == sf::Keyboard::Scan::S) {
    engine.EventWindowScreenshot();
  }
}
