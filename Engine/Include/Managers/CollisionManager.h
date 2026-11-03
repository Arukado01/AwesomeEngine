#pragma once

#include "Utils/SweptAabb.h"

class CollisionManager {
public:
  std::optional<sf::FloatRect> Intersect(const sf::FloatRect &a, const sf::FloatRect &b) const;
  std::optional<Collision::SweepHit> Sweep(const sf::FloatRect &previousA, const sf::FloatRect &currentA,
                                           const sf::FloatRect &previousB, const sf::FloatRect &currentB) const;
};
