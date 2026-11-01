#pragma once

#include "Config/MineSweeperConfig.h"

namespace MineSweeper {
  enum class CellState {
    Hidden,
    Revealed,
    Flagged
  };

  struct CellVisual {
    sf::RectangleShape background;
    sf::CircleShape mine;
    sf::CircleShape flag;
    sf::Text mineCross{GetDefaultFont()};
    sf::Text adjacentCountText{GetDefaultFont()};
  };

  struct Cell {
    CellState state;
    bool mined;
    int adjacentCount;
    CellVisual visual;
  };

  struct Grid {
    std::array<std::array<Cell, GRID_WIDTH>, GRID_HEIGHT> cells;
  };

  struct Stats {
    bool firstClick;
    int flagCount;
    bool gameOver;
    sf::Text mineCountText{GetDefaultFont()};
  };
} // namespace MineSweeper
