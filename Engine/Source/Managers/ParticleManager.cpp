#include "Managers/ParticleManager.h"

#include "Managers/RenderManager.h"

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
