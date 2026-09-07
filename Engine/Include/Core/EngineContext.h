#pragma once
#include "Managers/RandomManager.h"
#include "Managers/SaveManager.h"
#include "Managers/TimeManager.h"

struct EngineContext {
  RandomManager random;
  TimeManager time;
  SaveManager save;
};
