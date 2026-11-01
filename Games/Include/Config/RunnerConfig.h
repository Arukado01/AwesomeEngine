#pragma once

#include "Types/RunnerTypes.h"

#include <array>
#include <string>

namespace Runner {
  const sf::Vector2f PLAYER_SIZE(28, 48);
  const sf::Vector2f PLAYER_SLIDE_SIZE(54, 25);
  const float PLAYER_JUMP_VELOCITY = -600;
  const float PLAYER_FALL_FACTOR = 2.5f;
  const float PLAYER_GRAVITY = 1800;
  const std::string PLAYER_TEXTURE_FILENAME = "Runner.png";
  const sf::Vector2i PLAYER_FRAME_SIZE(64, 64);
  const sf::Vector2f PLAYER_SPRITE_SCALE(1, 1);
  const int PLAYER_SHEET_COLUMNS = 4;
  const float PLAYER_ANIMATION_MAX_FPS = 18;
  const float PLAYER_COLLISION_TOLERANCE = 0.001f;

  const std::array<PlayerAnimationClip, 4> PLAYER_ANIMATIONS{{
      {0, 8, 12}, // Carrera
      {13, 2, 8}, // Salto
      {8, 5, 12}, // Deslizamiento
      {15, 2, 8}, // Caída
  }};

  const float SLIDE_PREPARE_DURATION = 0.10f;
  const float SLIDE_LOOP_FPS = 6;
  const int SLIDE_LOOP_FIRST_FRAME = 1;
  const int SLIDE_LOOP_FRAME_COUNT = 2;
  const int SLIDE_RECOVER_FIRST_FRAME = 3;
  const int SLIDE_RECOVER_FRAME_COUNT = 2;

  const std::array<int, 2> PLAYER_FOOTSTEP_FRAMES{{0, 4}};
  const float PLAYER_FOOTSTEP_OFFSET = 10;
  const float FOOTSTEP_DURATION = 0.12f;
  const float FOOTSTEP_COMPRESSION = 0.04f;
  const float FOOTSTEP_PULSE_WIDTH = 18;
  const float FOOTSTEP_PULSE_MIN_WIDTH = 4;
  const float SLIDE_DUST_INTERVAL = 0.045f;
  const int FOOTSTEP_DUST_COUNT = 3;
  const int SLIDE_DUST_COUNT = 3;
  const int SLIDE_DUST_BURST_COUNT = 10;
  const std::size_t MAX_DUST_PARTICLES = 128;
  const std::size_t MAX_FOOTSTEP_PULSES = 8;
  const float DUST_MIN_LIFETIME = 0.22f;
  const float DUST_MAX_LIFETIME = 0.48f;
  const float DUST_GRAVITY = 45;
  const float DUST_SCROLL_FACTOR = 0.65f;
  const float FOOTSTEP_DUST_SPEED = 45;
  const float SLIDE_DUST_SPEED = 100;
  const float FOOTSTEP_DUST_LIFT = 25;
  const float SLIDE_DUST_LIFT = 48;
  const float DUST_MIN_LIFT = 8;
  const float DUST_SPAWN_RADIUS = 3;
  const float DUST_SPAWN_HEIGHT = 2;
  const int FOOTSTEP_DUST_MAX_SIZE = 2;
  const int SLIDE_DUST_MAX_SIZE = 4;
  const float DUST_OPACITY = 180;
  const float FOOTSTEP_OPACITY = 180;
  const sf::Color DUST_COLOR(180, 164, 140);
  const sf::Color FOOTSTEP_COLOR(66, 221, 245);

  const std::size_t BACKGROUND_LAYER_COUNT = 2;
  const std::array<std::string, BACKGROUND_LAYER_COUNT> BACKGROUND_TEXTURE_FILENAMES{{"RunnerSkyline.png", "RunnerCity.png"}};
  const std::array<float, BACKGROUND_LAYER_COUNT> BACKGROUND_SPEED_FACTORS{{0.08f, 0.24f}};
  const sf::Color BACKGROUND_TINT(155, 165, 185);
  const std::string FOREGROUND_TEXTURE_FILENAME = "RunnerForeground.png";
  const float FOREGROUND_SPEED_FACTOR = 1.2f;
  const sf::Color FOREGROUND_TINT(190, 195, 210);
  const float ENVIRONMENT_PIXEL_SCALE = 2;
  const std::string PLATFORM_TEXTURE_FILENAME = "RunnerPlatform.png";
  const std::string OBSTACLE_TEXTURE_FILENAME = "RunnerObstacles.png";
  const sf::Vector2i OBSTACLE_SPRITE_FRAME_SIZE(32, 80);
  const int OBSTACLE_SPRITE_CAP_HEIGHT = 8;
  const int OBSTACLE_GROUND_SPRITE_COUNT = 3;
  const int OBSTACLE_AIR_SPRITE_INDEX = 3;

  const float OBSTACLE_WIDTH = 30;
  const float OBSTACLE_MIN_DISTANCE = 250;
  const float OBSTACLE_MAX_DISTANCE = 450;
  const float OBSTACLE_MIN_HEIGHT = 45;
  const float OBSTACLE_MAX_HEIGHT = 70;
  const float OBSTACLE_AIR_PROBABILITY = 0.2f;

  const float STATS_SPEED_START = 200;
  const float STATS_SPEED_INCREASE = 10;
  const float STATS_SPEED_MAX = 800;
  const int STATS_DISTANCE_SCALE = 100;

  const std::string MUSIC_FILENAME = "RunnerCyberpunk.ogg";
  const float MUSIC_VOLUME = 32;
  const float MUSIC_GAME_OVER_VOLUME = 18;

  const std::array<SoundEffectConfig, static_cast<std::size_t>(PlayerSound::Count)> PLAYER_SOUNDS{{
      {"RunnerFootstepLeft.wav", 9, false},
      {"RunnerFootstepRight.wav", 9, false},
      {"RunnerSlide.wav", 20, true},
      {"RunnerJump.wav", 20, false},
  }};

  const float GAME_OVER_RESTART_DELAY = 2.5f;
  const std::string GAME_OVER_TITLE = "GAME OVER";
  const unsigned int GAME_OVER_TITLE_SIZE = 52;
  const unsigned int GAME_OVER_DISTANCE_TEXT_SIZE = 28;
  const unsigned int GAME_OVER_RESTART_TEXT_SIZE = 22;
  const sf::Vector2f GAME_OVER_TITLE_POSITION(0.5f, 0.42f);
  const sf::Vector2f GAME_OVER_DISTANCE_TEXT_POSITION(0.5f, 0.50f);
  const sf::Vector2f GAME_OVER_RESTART_TEXT_POSITION(0.5f, 0.57f);
  const sf::Color GAME_OVER_BACKDROP_COLOR(8, 12, 20, 180);
  const sf::Color GAME_OVER_TITLE_COLOR(255, 105, 115);
  const sf::Color GAME_OVER_DISTANCE_TEXT_COLOR(245, 245, 250);
  const sf::Color GAME_OVER_RESTART_TEXT_COLOR(220, 230, 240);

  const float GROUND_HEIGHT = 10;

  const sf::Color STATS_DISTANCE_TEXT_COLOR(sf::Color::White);
} // namespace Runner
