#pragma once
#include "Managers/RandomManager.h"
#include "Managers/TimeManager.h"

struct EngineContext {
  RandomManager random;
  TimeManager time;
};
