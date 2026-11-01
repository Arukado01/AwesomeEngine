#pragma once

#include "Config/RunnerConfig.h"

#include <deque>
#include <vector>

namespace Runner {
  class Game : public Scene {
  private:
    Player player;
    std::deque<Obstacle> obstacles;
    Stats stats;
    GameOverState gameOver;
    sf::RectangleShape ground;
    std::array<BackgroundLayer, BACKGROUND_LAYER_COUNT> backgroundLayers;
    BackgroundLayer foregroundCity;
    sf::RectangleShape platformFront;
    float platformOffset = 0;
    sf::Texture *obstacleTexture = nullptr;
    std::vector<DustParticle> dustParticles;
    std::vector<FootstepPulse> footstepPulses;

  public:
    explicit Game(EngineContext &);

    void Start() override;
    void Update() override;
    void Render() const override;
    void OnPause(bool paused) override;

  private:
    void InitEnvironment();
    void InitGameOver();

    void BindInputs();

    void StartPlayer();
    void StartStats();

    void UpdatePlayer();
    void UpdatePlayerSize();
    bool CanPlayerStand() const;
    void SetSlidePhase(SlidePhase phase);
    void UpdatePlayerGrounded();
    void UpdatePlayerAnimation();
    void UpdatePlayerEffects();
    void UpdateEffects();
    void UpdateEnvironment(float deltaTime);
    void UpdateObstacles();
    void UpdateStats();
    int GetDistance() const;
    void UpdateGameOver();

    void EventPlayerFootstep();
    void EmitDust(sf::Vector2f position, int count, bool sliding);
    void EventObstacleSpawn();
    void EventGameOver();

    void HandleCollisionsPlayerObstacles();

    static sf::Texture &LoadTexture(EngineContext &context, const std::string &filename, bool repeated = false);
    static std::optional<ObstacleHit> SweepObstacle(const sf::FloatRect &previousPlayer, const sf::FloatRect &currentPlayer,
                                                    const sf::FloatRect &previousObstacle, const sf::FloatRect &currentObstacle);
    static bool OverlapsHorizontally(const sf::FloatRect &playerBounds, const sf::FloatRect &obstacleBounds);
    void RenderEffects() const;
    void RenderGameOver() const;
    void RenderEnvironment() const;
    void RenderObstacle(const Obstacle &obstacle) const;
  };
} // namespace Runner
