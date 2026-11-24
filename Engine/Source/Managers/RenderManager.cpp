#include "Managers/RenderManager.h"

#include <utility>

#include "Graphics/Effects/EffectBloom.h"
#include "Graphics/Effects/EffectMonitor.h"
#include "Graphics/Effects/EffectGlitch.h"

#include "Core/EngineConfig.h"
#include "Utils/Verify.h"

RenderManager::RenderManager()
    : target_(sf::Vector2u(gConfig.windowSize)),
      backgroundTexture_("Content/Textures/Background.png"),
      background_(gConfig.windowSize) {
  target_.setSmooth(true);
  backgroundTexture_.setSmooth(true); // El fondo se escala a la ventana completa: evita pixelado

  background_.setTexture(&backgroundTexture_);
  background_.setFillColor(gConfig.backgroundColor);

  if (sf::Shader::isAvailable()) {
    VERIFY(effectsTarget_.resize(sf::Vector2u(gConfig.windowSize)));

    // Post Process Effects
    effects_.emplace_back(std::make_unique<EffectBloom>());
    effects_.emplace_back(std::make_unique<EffectMonitor>());
    // effects_.emplace_back(std::make_unique<EffectGlitch>());
  }
}

void RenderManager::Draw(const sf::Drawable &drawable) {
  target_.draw(drawable);
}

void RenderManager::SetView(const sf::View &view) { target_.setView(view); }

void RenderManager::ResetView() { SetView(target_.getDefaultView()); }

/* Vista del área de contenido: el mundo es el rectángulo de diseño
   (0,0,windowSize) y el viewport es el sub-rectángulo central de la ventana
   con el mismo aspecto. Así el contenido se escala por igual en ambos ejes
   sin deformarse nunca; el resto de la ventana lo cubre el fondo. */
sf::View RenderManager::GetFitView(sf::Vector2u windowSize) const {
  sf::View view(sf::FloatRect({0.f, 0.f}, gConfig.windowSize));

  const sf::Vector2f size(windowSize);
  if (size.x <= 0.f || size.y <= 0.f) {
    return view; // Ventana minimizada: sin área útil, viewport por defecto
  }

  const float windowAspect = size.x / size.y;
  const float designAspect = gConfig.windowSize.x / gConfig.windowSize.y;

  sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});
  if (windowAspect > designAspect) { // Ventana más ancha que el diseño: barras a los lados
    viewport.size = {designAspect / windowAspect, 1.f};
    viewport.position = {(1.f - viewport.size.x) / 2.f, 0.f};
  } else { // Ventana más alta que el diseño: barras arriba y abajo
    viewport.size = {1.f, windowAspect / designAspect};
    viewport.position = {0.f, (1.f - viewport.size.y) / 2.f};
  }

  view.setViewport(viewport);
  return view;
}

void RenderManager::BeginDrawing() {
  target_.clear();
  target_.setView(target_.getDefaultView()); // El fondo se dibuja en coords de ventana: cubre todo el sobrante
  Draw(background_);

  SetView(GetFitView(target_.getSize())); // El contenido de diseño se encaja dentro sin deformar
}

/* Ajusta el render al tamaño de la ventana. El sobrante del aspecto se rellena
   con el fondo (nunca con franjas negras) y el contenido se dibuja encima. */
void RenderManager::Resize(sf::Vector2u size) {
  if (size == target_.getSize() || size.x == 0 || size.y == 0) {
    return; // Mismo tamaño o ventana minimizada: nada que hacer
  }

  VERIFY(target_.resize(size));
  target_.setSmooth(true); // resize() restablece el smoothing: se reaplica

  if (!effects_.empty()) { // Los efectos post-proceso comparten el tamaño del render
    VERIFY(effectsTarget_.resize(size));
  }

  background_.setSize(sf::Vector2f(size)); // El fondo estira su textura a la ventana completa
}

const sf::Texture &RenderManager::FinishDrawing() {
  target_.display();

  sf::RenderTexture *input = &target_;
  sf::RenderTexture *output = &effectsTarget_;

  for (auto &effect : effects_) {
    output->clear();
    // Cada pase dibuja la textura completa 1:1: sin la vista por defecto, el
    // pase heredaría la vista letterbox de la escena y ampliaría el frame.
    output->setView(output->getDefaultView());
    effect->Apply(input->getTexture(), *output);
    output->display();

    std::swap(input, output);
  }

  return input->getTexture();
}
