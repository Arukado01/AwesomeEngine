#pragma once

#include "Scene/Scene.h"

namespace Tetris {
  const int GRID_WIDTH = 10;
  const int GRID_HEIGHT = 20;

  const float BLOCK_SIZE = 32;

  const int STATS_SCORE_PER_LINE = 100;
  const std::string_view STATS_HIGH_SCORE_KEY = "Tetris:High Score";

  const float ACTION_COOLDOWN_DURATION = 0.25f;
  const float PIECE_FALL_COOLDOWN_DURATION = 0.5f;
  const float SOFT_DROP_COOLDOWN_DURATION = 0.05f;

  const int PIECE_TYPE_COUNT = 7;
  const int PIECE_BLOCK_COUNT = 4;

  const sf::Color BACKGROUND_COLOR(10, 13, 24, 190);
  const sf::Color BOARD_COLOR(18, 23, 37);
  const sf::Color GRID_COLOR(42, 48, 65);
  const sf::Color SCORE_COLOR(171, 180, 201);

  const std::array<sf::Color, PIECE_TYPE_COUNT> PIECE_COLORS{{
      {29, 123, 137},
      {153, 124, 38},
      {112, 65, 146},
      {43, 119, 84},
      {155, 56, 73},
      {47, 85, 150},
      {162, 91, 38},
  }};

  const std::string MUSIC_FILENAME = "TetrisMusic.ogg";
  const std::string LINE_CLEAR_SOUND_FILENAME = "TetrisLine.wav";
  const float MUSIC_VOLUME = 35;
  const float LINE_CLEAR_SOUND_VOLUME = 45;

  const float GAME_OVER_DELAY = 2.5f;
  const sf::Color GAME_OVER_BACKDROP_COLOR(6, 8, 16, 225);
  const sf::Color GAME_OVER_TITLE_COLOR(208, 100, 128);

  const std::vector<std::vector<sf::Vector2i>> PIECE_SHAPES =
      {
          {{0, 1}, {1, 1}, {2, 1}, {3, 1}}, // I
          {{1, 0}, {2, 0}, {1, 1}, {2, 1}}, // O
          {{1, 0}, {0, 1}, {1, 1}, {2, 1}}, // T
          {{1, 0}, {2, 0}, {0, 1}, {1, 1}}, // S
          {{0, 0}, {1, 0}, {1, 1}, {2, 1}}, // Z
          {{0, 0}, {0, 1}, {1, 1}, {2, 1}}, // J
          {{2, 0}, {0, 1}, {1, 1}, {2, 1}}, // L
  };

  const std::vector<sf::Vector2f> PIECE_PIVOTS =
      {
          {1.5f, 1.5f}, // I
          {1.5f, 0.5f}, // O
          {1.0f, 1.0f}, // T
          {1.0f, 1.0f}, // S
          {1.0f, 1.0f}, // Z
          {1.0f, 1.0f}, // J
          {1.0f, 1.0f}, // L
  };
} // namespace Tetris
