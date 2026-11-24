#pragma once

#include "Types/ClickerTypes.h"
#include <memory>

namespace Clicker {
  class Game : public Scene {
  private:
    Target target;
    Stats stats;

    AudioGroup audioGroup;
    std::shared_ptr<sf::Music> music;

  public:
    Game(EngineContext &);

    void Start();
    void Update();
    void Render() const;
    void OnPause(bool paused);
    void OnCleanup();

  private:
    void InitTarget();
    void InitStats();
    void InitMusic();

    void StartInputs();

    void StartTarget();
    void StartStats();

    void UpdateStats();
    void UpdateTarget();
    void UpdateTargetColor();

    void EventTargetClicked();
    void EventTargetTeleport();
    void EventStatsScoreIncrease();

    bool IsTargetHovered() const;
  };
} // namespace Clicker
