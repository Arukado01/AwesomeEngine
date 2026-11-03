#include "Managers/CollisionManager.h"

/* Devuelve el área que comparten dos cajas, si existe. */
std::optional<sf::FloatRect> CollisionManager::Intersect(const sf::FloatRect &a, const sf::FloatRect &b) const {
  return a.findIntersection(b); // SFML devuelve vacío si no hay un área compartida.
}

/* Busca el primer contacto durante el movimiento de ambas cajas en el frame. */
std::optional<Collision::SweepHit> CollisionManager::Sweep(const sf::FloatRect &previousA, const sf::FloatRect &currentA,
                                                           const sf::FloatRect &previousB, const sf::FloatRect &currentB) const {
  return Collision::SweepAabb(previousA, currentA, previousB, currentB); // Las posiciones anteriores y actuales definen el recorrido.
}
