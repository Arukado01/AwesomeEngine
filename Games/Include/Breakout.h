#pragma once

#include "Types/BreakoutTypes.h"
#include <memory>

namespace Breakout {
  class Game : public Scene {
  private:
    Player player;
    std::vector<Ball> balls;
    std::vector<Brick> bricks;
    std::vector<Bonus> bonuses;
    Screen screen = Screen::Playing;
    sf::String resultTitle;
    sf::Color resultColor = sf::Color::White;

    AudioGroup audioGroup;
    std::shared_ptr<sf::Music> music;
    std::shared_ptr<sf::Sound> bounceSound;

  public:
    Game(EngineContext &);

    void Start() override;
    void OnEvent(const sf::Event &) override;
    void Update() override;
    void Render() const override;
    void OnPause(bool) override;
    void OnCleanup() override;

  private:
    void InitPlayer();
    void InitBounceSound();
    void InitMusic();
    void BindInputs();
    void UpdateScoreText();
    void UpdateHighScoreText();
    void UpdateLivesText();
    void PlayBounce();
    void EventGameEnd(Screen result, const sf::String &title, sf::Color titleColor);
    void RenderResultScreen() const;

    void StartPlayer();
    void StartBricks();
    void StartBrick(Brick &brick, sf::Vector2i tile);

    void UpdatePlayer();
    void UpdateBalls();
    void UpdateBall(Ball &ball);
    void UpdateBonuses();
    void UpdateBonusesCleanup();

    void EventBallSpawn();
    void EventBallReset(Ball &ball);
    void EventBonusEnable();
    void EventBonusDisable(const Bonus &bonus);

    void HandleCollisions();
    void HandleCollisionsBallsMap();
    void HandleCollisionsBallMap(Ball &ball);
    void ResolveCollisionBallPlayer(Ball &ball);
    void ResolveCollisionBallDown(Ball &ball);
    void HandleCollisionsBallBricks(Ball &ball);
    void ResolveCollisionBallBrick(Ball &ball, Brick &brick);
    void ResolveCollisionBallBrickBounce(Ball &ball, Brick &brick);

    std::vector<BonusType> GetBonusTypesAllowed() const;
    bool IsBonusCompatible(BonusType a, BonusType b) const;
    bool IsBallNearPlayer(const Ball &ball) const;
  };
} // namespace Breakout
