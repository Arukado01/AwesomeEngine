#pragma once
#include <SFML/Graphics.hpp>

#include <vector>
#include <memory>

#include "Graphics/Effect.h"

class RenderManager {
private:
  sf::RenderTexture target_;
  sf::Texture backgroundTexture_;
  sf::RectangleShape background_;

  std::vector<std::unique_ptr<Effect>> effects_;
  sf::RenderTexture effectsTarget_;

public:
  RenderManager();
  void Draw(const sf::Drawable &drawable);

  void SetView(const sf::View &view);
  void ResetView();

  /** Vista que encaja el área de diseño en la ventana conservando el aspecto
      (escala uniforme, centrada, con bandas negras en el sobrante). */
  sf::View GetFitView(sf::Vector2u windowSize) const;

private:
  friend class Engine;

  /** Ajusta el render al tamaño de la ventana: el fondo cubre toda la ventana
      y el contenido de diseño se encaja dentro sin deformar. */
  void Resize(sf::Vector2u size);

  void BeginDrawing();
  const sf::Texture &FinishDrawing();
};
