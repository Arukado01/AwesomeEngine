#pragma once

#include "Graphics/Effect.h"

#include <SFML/Graphics/Shader.hpp>

class EffectInverted : public Effect {
private:
  sf::Shader shader_;

public:
  EffectInverted();

  void Apply(const sf::Texture &input, sf::RenderTarget &output) override;
};
