#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <vector>

class RenderManager;

struct Particle {
  sf::Vector2f position;
  sf::Vector2f velocity;
  sf::Vector2f acceleration;
  float remainingLife;
  float lifetime;
  float size;
  sf::Color color;
};

class ParticleManager {
private:
  std::vector<Particle> particles_;
  std::size_t maximumParticles_;

public:
  explicit ParticleManager(std::size_t maximumParticles = 128);

  void Emit(Particle particle);
  void Update(float deltaTime, sf::Vector2f driftVelocity = {});
  void Render(RenderManager &renderer) const;
  void Clear();
  std::size_t Count() const;
};
