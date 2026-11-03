#include "Managers/ParticleManager.h"

#include "Managers/RenderManager.h"
#include "Managers/RandomManager.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>

ParticleManager::ParticleManager(std::size_t maximumParticles) : maximumParticles_(maximumParticles) {
  particles_.reserve(maximumParticles_);
}

/* Agrega una partícula con vida válida si todavía hay espacio disponible. */
void ParticleManager::Emit(Particle particle) {
  if (particle.lifetime > 0 && particle.remainingLife > 0 && particles_.size() < maximumParticles_) {
    particles_.push_back(particle);
  }
}

/* Genera varias partículas a partir de rangos reutilizables y devuelve cuántas se crearon. */
std::size_t ParticleManager::EmitBurst(sf::Vector2f position, std::size_t count, const ParticleEmitterConfig &config, RandomManager &random) {
  if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
      !std::isfinite(config.lifetimeMin) || !std::isfinite(config.lifetimeMax) ||
      !std::isfinite(config.positionMin.x) || !std::isfinite(config.positionMin.y) ||
      !std::isfinite(config.positionMax.x) || !std::isfinite(config.positionMax.y) ||
      !std::isfinite(config.velocityMin.x) || !std::isfinite(config.velocityMin.y) ||
      !std::isfinite(config.velocityMax.x) || !std::isfinite(config.velocityMax.y) ||
      !std::isfinite(config.acceleration.x) || !std::isfinite(config.acceleration.y) ||
      config.lifetimeMin <= 0 || config.lifetimeMin > config.lifetimeMax || config.sizeMin <= 0 || config.sizeMin > config.sizeMax ||
      config.positionMin.x > config.positionMax.x || config.positionMin.y > config.positionMax.y ||
      config.velocityMin.x > config.velocityMax.x || config.velocityMin.y > config.velocityMax.y) return 0;
  const auto available = maximumParticles_ - particles_.size();
  const auto created = std::min(count, available);
  const auto value = [&](float min, float max) { return min == max ? min : random.Float(min, max); };
  for (std::size_t i = 0; i < created; ++i) {
    const float life = value(config.lifetimeMin, config.lifetimeMax);
    const sf::Vector2f offset{value(config.positionMin.x, config.positionMax.x), value(config.positionMin.y, config.positionMax.y)};
    const sf::Vector2f velocity{value(config.velocityMin.x, config.velocityMax.x), value(config.velocityMin.y, config.velocityMax.y)};
    Emit({position + offset, velocity, config.acceleration, life, life, static_cast<float>(random.Int(config.sizeMin, config.sizeMax)), config.color});
  }
  return created;
}

/* Mueve las partículas, aplica su aceleración y retira las que ya terminaron. */
void ParticleManager::Update(float deltaTime, sf::Vector2f driftVelocity) {
  for (auto &particle : particles_) {
    particle.remainingLife -= deltaTime;
    particle.velocity += particle.acceleration * deltaTime;
    particle.position += (particle.velocity + driftVelocity) * deltaTime;
  }
  std::erase_if(particles_, [](const Particle &particle) { return particle.remainingLife <= 0; });
}

/* Dibuja cada partícula como un cuadrado que se desvanece con el tiempo. */
void ParticleManager::Render(RenderManager &renderer) const {
  sf::RectangleShape shape;
  for (const auto &particle : particles_) {
    const float lifeRatio = std::clamp(particle.remainingLife / particle.lifetime, 0.f, 1.f);
    auto color = particle.color;
    color.a = static_cast<std::uint8_t>(color.a * lifeRatio);
    shape.setFillColor(color);
    shape.setSize({particle.size, particle.size});
    shape.setPosition({std::round(particle.position.x), std::round(particle.position.y)});
    renderer.Draw(shape);
  }
}

/* Elimina las partículas, por ejemplo al reiniciar o salir de una escena. */
void ParticleManager::Clear() { particles_.clear(); }

/* Devuelve cuántas partículas siguen activas. */
std::size_t ParticleManager::Count() const { return particles_.size(); }
