#pragma once

#include "Scene/Scene.h"

namespace Runner {
  enum Action {
    Jump,
    Crouch,
    Fall
  };

  enum class PlayerAnimation {
    Run,
    Jump,
    Slide,
    Fall
  };

  enum class SlidePhase {
    Prepare,
    Loop,
    Recover
  };

  enum class PlayerSound {
    FootstepLeft,
    FootstepRight,
    Slide,
    Jump,
    Count
  };

  struct SoundEffectConfig {
    const char *filename;
    float volume;
    bool looping;
  };

  struct PlayerAnimationClip {
    int firstFrame;
    int frameCount;
    float framesPerSecond;
  };

  struct ObstacleHit {
    float time;
    bool landing;
  };

  struct Player {
    sf::RectangleShape shape;
    sf::Sprite sprite;
    sf::FloatRect previousBounds;
    sf::Vector2f velocity{};
    bool grounded = true;
    bool sliding = false;
    PlayerAnimation animation = PlayerAnimation::Run;
    float animationTime = 0;
    int animationFrame = -1;
    float stepImpact = 0;
    float slideDustTime = 0;
    SlidePhase slidePhase = SlidePhase::Prepare;
    float slidePhaseTime = 0;

    explicit Player(const sf::Texture &texture) : sprite(texture) {}
  };

  struct Obstacle {
    sf::FloatRect bounds;
    sf::FloatRect previousBounds;
    int spriteIndex = 0;
  };

  struct BackgroundLayer {
    sf::RectangleShape shape;
    float offset = 0;
  };

  struct DustParticle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float remainingLife;
    float lifetime;
    float size;
  };

  struct FootstepPulse {
    sf::Vector2f position;
    float remainingLife;
  };

  struct Stats {
    float speed = 0;
    float obstacleNextX = 0;
    float distance = 0;
    sf::Text distanceText{GetDefaultFont()};
  };

  struct GameOverState {
    bool active = false;
    bool restartRequested = false;
    Cooldown restartCooldown;
    sf::RectangleShape backdrop;
    sf::Text title{GetDefaultFont()};
    sf::Text distanceText{GetDefaultFont()};
    sf::Text restartText{GetDefaultFont()};
  };
} // namespace Runner
