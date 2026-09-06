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
