#include "Core/Engine.h"
#include "Core/EngineVistor.h"

void EngineVisitor::operator()(const sf::Event::Closed &) {
  engine.EventWindowClose();
}
