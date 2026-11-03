#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <vector>

class RenderManager;
class RandomManager;

struct Particle {
  sf::Vector2f position;
  sf::Vector2f velocity;
  sf::Vector2f acceleration;
  float remainingLife;
  float lifetime;
  float size;
  sf::Color color;
};

struct ParticleEmitterConfig {
  sf::Vector2f positionMin;
  sf::Vector2f positionMax;
  sf::Vector2f velocityMin;
  sf::Vector2f velocityMax;
  sf::Vector2f acceleration;
  float lifetimeMin;
  float lifetimeMax;
  int sizeMin;
  int sizeMax;
  sf::Color color;
};

class ParticleManager {
private:
  std::vector<Particle> particles_;
  std::size_t maximumParticles_;

public:
  explicit ParticleManager(std::size_t maximumParticles = 128);

  void Emit(Particle particle);
  std::size_t EmitBurst(sf::Vector2f position, std::size_t count, const ParticleEmitterConfig &config, RandomManager &random);
  void Update(float deltaTime, sf::Vector2f driftVelocity = {});
  void Render(RenderManager &renderer) const;
  void Clear();
  std::size_t Count() const;
};
