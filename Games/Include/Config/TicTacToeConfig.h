#pragma once

#include "Scene/Scene.h"

namespace TicTacToe {
  const int GRID_SIZE = 3;

  const int CELL_TEXT_SIZE = 115;

  const float RESTART_COOLDOWN_DURATION = 2.0f;
  const float AI_TURN_COOLDOWN_DURATION = 0.5f;

  const sf::Color CELL_COLOR(10, 10, 25);
  const sf::Color CELL_OUTLINE_COLOR(100, 150, 225);
  const sf::Color CELL_SYMBOL_X_COLOR(90, 90, 200);
  const sf::Color CELL_SYMBOL_O_COLOR(200, 90, 90);

  const float CELL_WINNING_LINE_THICKNESS = 12.f;
  const sf::Color CELL_WINNING_LINE_COLOR(sf::Color::White);
  const float CELL_WINNING_LINE_OVERHANG = 40.f;

  const int RESULT_TEXT_SIZE = 90;
  const int RESULT_TEXT_OUTLINE_THICKNESS = 7;

  const int MENU_TITLE_TEXT_SIZE = 64;
  const int MENU_BUTTON_TEXT_SIZE = 72;
  const float MENU_TITLE_HEIGHT_RATIO = 0.15f;   // título centrado en la franja libre superior del menú
  const float MENU_BUTTON_HEIGHT_RATIO = 0.34f;  // altura de cada banda de botón: dos apiladas ocupan dos tercios
  const float MENU_LABEL_WIDTH_RATIO = 0.9f;     // margen del texto dentro de su banda (porcentaje del ancho)
  const sf::Color MENU_TITLE_COLOR(sf::Color::White);
  const sf::Color MENU_BUTTON_COLOR(10, 10, 25);
  const sf::Color MENU_BUTTON_HOVER_COLOR(28, 34, 66);
  const sf::Color MENU_BUTTON_TEXT_COLOR(100, 150, 225);

  const int TURN_TEXT_SIZE = 45;
  const float TURN_TEXT_BOTTOM_OFFSET = 30.f;
  const sf::Color TURN_TEXT_COLOR(225, 225, 235);
} // namespace TicTacToe
