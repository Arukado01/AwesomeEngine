#pragma once

#include "Graphics/Effect.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <array>

// Bloom: las zonas brillantes deslumbran y derraman luz como un halo.
class EffectBloom : public Effect {
private:
  sf::Shader downsampleShader_; // Paso 1: promedia vecinos y baja resolución.
  sf::Shader blurShader_; // Pasos 2 y 3: blur gaussiano (se reusa con distinta dirección).
  sf::Shader additiveShader_; // Paso 4: suma bloom + imagen original.

  // Ping-pong rendering A -> B -> A, mitad de tamaño, resultados de cada pasada.
  std::array<sf::RenderTexture, 2> textures_;

public:
  EffectBloom();

  // Dibuja input con bloom en output.
  void Apply(const sf::Texture &input, sf::RenderTarget &output) override;

private:
  // Ejecuta el shader sobre cada píxel de la textura 'output'.
  void Render(const sf::Shader &shader, sf::RenderTexture &output);
};
