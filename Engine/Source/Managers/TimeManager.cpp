#include "Managers/TimeManager.h"
#include "Core/EngineConfig.h"
#include <algorithm>

void TimeManager::Update() {
  const sf::Time currentTime = clock_.getElapsedTime();
  deltaTime_ = std::min(currentTime - previousTime_, gConfig.maximumDeltaTime);
  previousTime_ = currentTime;
}

float TimeManager::GetDeltaTime() const { return deltaTime_.asSeconds(); }

float TimeManager::GetElapsedTime() const {
  return clock_.getElapsedTime().asSeconds();
}
