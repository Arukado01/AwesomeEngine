#include "Graphics/Effects/EffectBloom.h"

#include <SFML/Graphics/Sprite.hpp>

#include <string>

#include "Core/EngineConfig.h"
#include "Utils/Verify.h"

EffectBloom::EffectBloom() {
  // Los tres comparten el mismo vertex shader; solo cambia el fragment.
  const std::string shadersPath = "Content/Shaders/";
  const std::string vertexShader = shadersPath + "Default.vert";

  VERIFY(downsampleShader_.loadFromFile(vertexShader, shadersPath + "Downsample.frag"));
  VERIFY(blurShader_.loadFromFile(vertexShader, shadersPath + "Blur.frag"));
  VERIFY(additiveShader_.loadFromFile(vertexShader, shadersPath + "Additive.frag"));

  // Mitad de tamaño: más rápido y el blur queda más suave al re-estirar.
  for (sf::RenderTexture &texture : textures_) {
    VERIFY(texture.resize(sf::Vector2u(gConfig.windowSize / 2.f)));
  }
}

void EffectBloom::Apply(const sf::Texture &input, sf::RenderTarget &output) {
  // 1. Promedia la entrada hacia A (mitad de tamaño), antes del blur.
  downsampleShader_.setUniform("sourceTexture", input);
  downsampleShader_.setUniform("texelSize", sf::Vector2f(1.f / input.getSize().x, 1.f / input.getSize().y));
  Render(downsampleShader_, textures_[0]);

  // 2. Blur VERTICAL sobre A, resultado en B (texelSize solo en Y).
  blurShader_.setUniform("sourceTexture", textures_[0].getTexture());
  blurShader_.setUniform("texelSize", sf::Vector2f(0, 1.f / textures_[0].getSize().y));
  Render(blurShader_, textures_[1]);

  // 3. Blur HORIZONTAL sobre B, resultado en A. Separable: 11 vs 81 muestras.
  blurShader_.setUniform("sourceTexture", textures_[1].getTexture());
  blurShader_.setUniform("texelSize", sf::Vector2f(1.f / textures_[1].getSize().x, 0));
  Render(blurShader_, textures_[0]);

  // 4. Dibuja la original en output sumándole el brillo difuminado de A.
  additiveShader_.setUniform("sourceTexture", sf::Shader::CurrentTexture);
  additiveShader_.setUniform("bloomTexture", textures_[0].getTexture());
  output.draw(sf::Sprite(input), &additiveShader_);
}

// Ejecuta el shader sobre cada píxel de una textura intermedia.
void EffectBloom::Render(const sf::Shader &shader, sf::RenderTexture &output) {
  const sf::Vector2f outputSize(output.getSize());

  // UV invertidas en Y: así lo espera SFML para no voltear las texturas.
  const sf::Vertex quad[] = {
      {{0, 0}, sf::Color::White, {0, 1}},
      {{outputSize.x, 0}, sf::Color::White, {1, 1}},
      {{0, outputSize.y}, sf::Color::White, {0, 0}},
      {outputSize, sf::Color::White, {1, 0}},
  };

  sf::RenderStates states(&shader);
  // Reemplaza el contenido previo en vez de mezclarlo.
  states.blendMode = sf::BlendNone;

  output.draw(quad, 4, sf::PrimitiveType::TriangleStrip, states);
  output.display();
}
