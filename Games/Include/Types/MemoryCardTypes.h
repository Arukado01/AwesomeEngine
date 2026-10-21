#pragma once

#include "Config/MemoryCardConfig.h"

namespace MemoryCard {
  struct Card {
    sf::RectangleShape shape;
    sf::Color color;
    bool flipped;
    bool found;
  };

  struct CardPair {
    Card *first;
    Card *second;
    Cooldown visibleCooldown;
  };

  struct Stats {
    int level;
  };
} // namespace MemoryCard
