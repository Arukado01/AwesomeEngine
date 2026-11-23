#pragma once

#include "Types/PongTypes.h"
#include <memory>

namespace Pong {
  class Game : public Scene {
  public:
    explicit Game(EngineContext &context);

    void Start() override;
    void OnEvent(const sf::Event &event) override;
    void Update() override;
    void Render() const override;
    void OnPause(bool paused) override;
    void OnCleanup() override;

  private:
    Player player1;
    Player player2;
    Ball ball;
    Mode mode = Mode::VersusAI;
    Screen screen = Screen::Menu;
    int startingLives = DEFAULT_LIVES;
    int menuSelection = 0; // 0 = un jugador, 1 = dos jugadores
    sf::String resultMessage;
    sf::Color resultColor = PLAYER_1_COLOR;

    AudioGroup audioGroup;
    std::shared_ptr<sf::Music> music;
    std::shared_ptr<sf::Sound> bounceSound;

    void InitPlayer(Player &player, unsigned int joystickId, sf::Color color);
    void BindInputs();
    void StartMatch();
    void UpdateLivesText(Player &player);
    void EventBallReset();
    void EventLifeLost(Player &loser);

    void UpdatePlayer(Player &player, Action up, Action down);
    void UpdatePlayerAI(Player &player);
    void MovePlayer(Player &player, float velocityY);

    void HandlePaddleCollisions();
    void BounceFromPaddle(const Player &player, float directionX);
    void HandleCourtBoundaries();
    void PlayBounce();

    void RenderMap() const;
    void RenderMenu() const;
    void RenderGameOver() const;
  };
} // namespace Pong
