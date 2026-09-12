#include "Managers/RenderManager.h"
#include "Core/EngineConfig.h"

RenderManager::RenderManager()
    : target_(sf::Vector2u(gConfig.windowSize)),
      backgroundTexture_("Content/Textures/Background.png"),
      background_(gConfig.windowSize) {
  target_.setSmooth(true);

  background_.setTexture(&backgroundTexture_);
  background_.setFillColor(gConfig.backgroundColor);
}

void RenderManager::Draw(const sf::Drawable &drawable) {
  target_.draw(drawable);
}

void RenderManager::SetView(const sf::View &view) { target_.setView(view); }

void RenderManager::ResetView() { SetView(target_.getDefaultView()); }

void RenderManager::BeginDrawing() {
  target_.clear();
  Draw(background_);
}

const sf::Texture &RenderManager::FinishDrawing() {
  target_.display();
  return target_.getTexture();
}
