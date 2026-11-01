#pragma once

#include "Config/TicTacToeConfig.h"

namespace TicTacToe {
  enum class GameMode { PvAI, PvP }; // PvAI: humano (X) contra la máquina (O) · PvP: dos humanos al mismo tablero

  struct ModeButton {
    sf::RectangleShape shape;
    sf::Text label{GetDefaultFont()};
  };

  struct Cell {
    sf::RectangleShape shape;
    char symbol;
    sf::Text text{GetDefaultFont()};
  };

  struct Grid {
    std::array<std::array<Cell, GRID_SIZE>, GRID_SIZE> cells;
    sf::Vector2f cellSize;
  };
} // namespace TicTacToe
