#pragma once

#include "Scene/Scene.h"
#include <string>

namespace Pong {
  constexpr int DEFAULT_LIVES = 5;
  constexpr int MIN_LIVES = 1;
  constexpr int MAX_LIVES = 9;

  const sf::Vector2f PLAYER_SIZE{24.f, 160.f};
  constexpr float PLAYER_SPEED = 800.f;
  constexpr float PLAYER_AI_SPEED_FACTOR = 7.f;

  constexpr float BALL_RADIUS = 10.f;
  constexpr float BALL_SPEED = 500.f;
  constexpr float BALL_SPEED_MAX = 1200.f;
  constexpr float BALL_SPEED_BONUS_FACTOR = 1.05f;
  constexpr auto BALL_ANGLE_MAX = sf::degrees(45.f);

  const sf::Vector2f MAP_LINE_SEGMENT_SIZE{5.f, 20.f};

  const sf::Color PLAYER_1_COLOR = sf::Color::Cyan;
  const sf::Color PLAYER_2_COLOR = sf::Color::Red;
  const sf::Color BALL_COLOR = sf::Color::White;
  const sf::Color MAP_LINE_SEGMENT_COLOR = sf::Color::White;

  const std::string MUSIC_FILENAME = "PongMusic.wav";
  const std::string BOUNCE_FILENAME = "PongBounce.wav";
  constexpr float MUSIC_VOLUME = 30.f;
  constexpr float BOUNCE_VOLUME = 20.f;

  // Menú y pantallas superpuestas
  const sf::Color SCREEN_BACKDROP_COLOR(6, 10, 18, 235);
  const sf::Color MENU_TITLE_COLOR = sf::Color::White;
  const sf::Color MENU_ACCENT_COLOR = PLAYER_1_COLOR;
  const sf::Color MENU_OPTION_COLOR(150, 160, 175);
  const sf::Color MENU_HINT_COLOR(95, 105, 120);
  constexpr unsigned MENU_TITLE_SIZE = 72;
  constexpr unsigned MENU_OPTION_SIZE = 26;
  constexpr unsigned MENU_LIVES_SIZE = 40;
  constexpr unsigned SCREEN_TITLE_SIZE = 48;
  constexpr unsigned SCREEN_MESSAGE_SIZE = 36;
  constexpr unsigned SCREEN_HINT_SIZE = 18;
  constexpr float MENU_BLINK_SPEED = 4.f;
} // namespace Pong
