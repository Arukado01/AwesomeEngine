#pragma once

#include "Config/PongConfig.h"

namespace Pong {
  enum Action {
    Player1MoveUp,
    Player1MoveDown,
    Player2MoveUp,
    Player2MoveDown,
  };

  /* El modo decide quién controla la paleta derecha. */
  enum class Mode { VersusAI,
                    VersusPlayer };

  /* La pantalla decide qué acciones acepta el juego y qué dibuja. */
  enum class Screen { Menu,
                      Playing,
                      GameOver };

  struct Stats {
    int lives = 0;
    sf::Text livesText{GetDefaultFont()};
  };

  struct Player {
    unsigned int joystickId = 0;
    sf::RectangleShape shape;
    Stats stats;
  };

  struct Ball {
    sf::CircleShape shape;
    sf::Vector2f direction;
    float speed = BALL_SPEED;
  };
} // namespace Pong
