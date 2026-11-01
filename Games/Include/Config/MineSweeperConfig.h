#pragma once

#include "Scene/Scene.h"

namespace MineSweeper {
  const int GRID_WIDTH = 8;
  const int GRID_HEIGHT = 8;

  const float CELL_SIZE = 48;

  const int STATS_MINE_COUNT = 10;

  const float RESTART_COOLDOWN_DURATION = 2.0f;

  const sf::Color CELL_HIDDEN_COLOR(100, 100, 100);
  const sf::Color CELL_REVEALED_COLOR(125, 125, 125);
  const sf::Color CELL_FLAG_COLOR(200, 50, 50);
  const sf::Color CELL_MINE_COLOR(25, 25, 25);
  const sf::Color CELL_TEXT_COLOR(sf::Color::Blue);
  const sf::Color CELL_OUTLINE_COLOR(25, 25, 25);

  const int CELL_MINE_CROSS_SIZE = (int)CELL_SIZE - 4;
  const sf::Color CELL_MINE_CROSS_COLOR(200, 50, 50);

  const int RESULT_TEXT_SIZE = 90;
  const int RESULT_TEXT_OUTLINE_THICKNESS = 7;
  const sf::Color RESULT_WIN_COLOR(sf::Color::Green);
  const sf::Color RESULT_LOSE_COLOR(200, 50, 50);
  const sf::Color RESULT_OUTLINE_COLOR(25, 25, 25);
} // namespace MineSweeper
