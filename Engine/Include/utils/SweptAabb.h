#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <optional>

namespace Collision {
  struct SweepHit {
    float time; /* Fracción del frame en la que ocurre el contacto, entre 0 y 1. */
    sf::Vector2f normal; /* Dirección de la superficie de B hacia A; es cero si ya se solapaban. */
    bool initialOverlap; /* Indica que las cajas ya compartían espacio al comenzar el frame. */
  };

  /* Busca el primer contacto entre dos cajas alineadas a los ejes y de tamaño constante durante el frame. */
  std::optional<SweepHit> SweepAabb(const sf::FloatRect &previousA, const sf::FloatRect &currentA,
                                    const sf::FloatRect &previousB, const sf::FloatRect &currentB);
}
