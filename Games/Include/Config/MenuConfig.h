#pragma once

#include "Scene/Scene.h"

namespace Menu {
  /* Entrada de controles: nombre del juego y sus acciones reales. */
  struct ControlInfo {
    std::string_view title;
    std::string_view body;
  };

  const std::array BUTTONS_NAMES =
      {
          "Clicker", "MemoryCard", "TicTacToe", "MineSweeper",
          "Runner", "Tetris", "Pong", "Breakout"};

  const int BUTTONS_PER_ROW = 2;
  const float BUTTONS_PADDING_RATIO = 0.5f;

  const int BUTTON_TEXT_SIZE = 24;

  const sf::Color BUTTON_COLOR(150, 150, 225);
  const sf::Color BUTTON_HOVERED_COLOR(sf::Color::Cyan);
  const sf::Color BUTTON_TEXT_COLOR(sf::Color::Black);

  /* Controles reales de cada juego, en el mismo orden que BUTTONS_NAMES. */
  const std::array CONTROLS_TEXTS =
      {
          ControlInfo{"Clicker",
                      "Hit: Left Click / South"},

          ControlInfo{"Memory Card",
                      "Flip: Any Click / Any Button"},

          ControlInfo{"Tic-Tac-Toe",
                      "Mode: 1 = vs AI / 2 = PvP\n"
                      "Play: Any Click / Any Button"},

          ControlInfo{"Minesweeper",
                      "Reveal: Left Click / South\n"
                      "Flag: Right Click / West"},

          ControlInfo{"Runner",
                      "Jump: W / South\n"
                      "Crouch: S / West\n"
                      "Fast Fall: S / Stick Y"},

          ControlInfo{"Tetris",
                      "Move: Arrows / A / D / Stick X\n"
                      "Rotate: Up / W / North\n"
                      "Soft Drop: Down / S / Stick Y\n"
                      "Hard Drop: Space / South"},

          ControlInfo{"Pong",
                      "Player 1: W / S / Left Stick Y\n"
                      "Player 2: Arrows / Right Stick Y\n"
                      "Menu: Up / Down + Enter"},

          ControlInfo{"Breakout",
                      "Move: A / D / Stick X\n"
                      "Restart: Enter"},
  };

  // Pantalla de controles: panel central con borde sobre un fondo oscurecido
  const sf::Color CONTROLS_BACKDROP_COLOR(0, 0, 0, 200);
  const sf::Color CONTROLS_PANEL_COLOR(18, 23, 37);
  const sf::Color CONTROLS_PANEL_BORDER_COLOR(150, 150, 225);
  const sf::Color CONTROLS_TITLE_COLOR(sf::Color::Cyan);
  const sf::Color CONTROLS_TEXT_COLOR(175, 175, 175);
  constexpr unsigned CONTROLS_TITLE_TEXT_SIZE = 34;
  constexpr unsigned CONTROLS_BODY_TEXT_SIZE = 24;
  constexpr unsigned CONTROLS_HINT_TEXT_SIZE = 16;
  const sf::Vector2f CONTROLS_PANEL_SIZE_RATIO(0.55f, 0.55f);
  constexpr float CONTROLS_PANEL_OUTLINE_THICKNESS = 2.f;

  // Encabezado y pie del menú principal
  const std::string MENU_LOGO_FILENAME = "Icon.png";
  constexpr float MENU_LOGO_SIZE = 48.f;
  constexpr float MENU_LOGO_GAP = -8.f; // Acerca las formas visibles de la A y la W.
  const std::string MENU_TITLE_TEXT = "WESOME ENGINE";
  const std::string MENU_HINT_TEXT = "Right Click / West: controles de cada juego";
  const sf::Color MENU_TITLE_COLOR = sf::Color::White;
  const sf::Color MENU_HINT_COLOR(120, 130, 145);
  constexpr unsigned MENU_TITLE_TEXT_SIZE = 44;
  constexpr unsigned MENU_HINT_TEXT_SIZE = 16;
  constexpr float MENU_HEADER_HEIGHT_RATIO = 0.14f;
  constexpr float MENU_FOOTER_HEIGHT_RATIO = 0.06f;

  const float MUSIC_VOLUME = 5;
  const float MUSIC_PITCH = 1;

  const std::string BUTTON_TEXTURE_FILENAME = "Button.png";
  const std::string MUSIC_FILENAME = "Music.mp3";
} // namespace Menu
