#pragma once

#include "Config/BreakoutConfig.h"

namespace Breakout {
  /* Acciones lógicas que se enlazan con teclado o mando. */
  enum Action {
    MoveLeft,
    MoveRight,
  };

  /* El estado decide si la partida avanza o se muestra un resultado. */
  enum class Screen { Playing,
                      Win,
                      GameOver };

  struct Stats {
    int score;
    int lives;
    int highScore;
    sf::Text scoreText{GetDefaultFont()};
    sf::Text highScoreText{GetDefaultFont()};
    sf::Text livesText{GetDefaultFont()};
  };

  struct Player {
    sf::RectangleShape shape;
    float speed;
    bool magnetic;
    Stats stats;
  };

  struct Ball {
    sf::CircleShape shape;
    sf::Vector2f direction;
    float speed;
    bool alive;
    bool fire;
    bool invincible;
  };

  struct Brick {
    sf::RectangleShape shape;
    int lives;
  };

  enum BonusType {
    ExpandPaddle,
    ShrinkPaddle,
    MagnetPaddle,
    AccelerateBall,
    DecelerateBall,
    MultiBall,
    FireBall,
    InvincibleBall,
    Count
  };

  struct Bonus {
    BonusType type;
    Cooldown endCooldown;
    sf::Text text{GetDefaultFont()};
  };
} // namespace Breakout
