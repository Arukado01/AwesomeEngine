#pragma once

#include "Scene/Scene.h"
#include <string>
#include <string_view>

namespace Breakout {
  const int PLAYER_LIVES = 8;
  const int PLAYER_SPEED = 1000;

  // Clave del récord persistido en Content/Save.json
  const std::string_view STATS_HIGH_SCORE_KEY = "Breakout:High Score";

  const float BALL_RADIUS = 10;
  const float BALL_SPEED = 650;
  const sf::Angle BALL_ANGLE_MAX = sf::degrees(60);
  const float BALL_MAGNET_RANGE = 300;

  const sf::Vector2i BRICK_MAP_SIZE(10, 7);
  const int BRICK_LIVES_MAX = 3;

  const float BONUS_COOLDOWN_DURATION = 5;
  const int BONUS_TEXT_SIZE = 20;
  const float BONUS_TEXT_OFFSET_Y = 25;

  const sf::Color PLAYER_COLOR(sf::Color::White);
  const sf::Color BALL_COLOR(sf::Color::White);
  const sf::Color BALL_FIRE_COLOR(255, 92, 0);

  const std::vector<sf::Color> BRICK_COLORS = {
      {120, 220, 250},
      {0, 128, 128},
      {0, 71, 171},
      {18, 10, 143}};

  const std::string PLAYER_TEXTURE_FILENAME = "Paddle.png";

  // Margen entre el HUD y los bordes de la ventana
  constexpr float STATS_HUD_MARGIN = 24.f;

  const std::string MUSIC_FILENAME = "PongMusic.wav";
  const std::string BOUNCE_FILENAME = "PongBounce.wav";
  constexpr float MUSIC_VOLUME = 30.f;
  constexpr float BOUNCE_VOLUME = 20.f;

  // Pantallas de resultado (victoria y derrota)
  const sf::Color SCREEN_BACKDROP_COLOR(6, 10, 18, 235);
  const sf::Color WIN_TITLE_COLOR(120, 220, 250);
  const sf::Color GAME_OVER_TITLE_COLOR(220, 70, 90);
  const sf::Color SCREEN_MESSAGE_COLOR(210, 224, 235);
  const sf::Color SCREEN_HINT_COLOR(95, 105, 120);
  constexpr unsigned SCREEN_TITLE_SIZE = 48;
  constexpr unsigned SCREEN_MESSAGE_SIZE = 30;
  constexpr unsigned SCREEN_HINT_SIZE = 18;
} // namespace Breakout
