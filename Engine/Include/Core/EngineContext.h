#pragma once
#include "Managers/AudioManager.h"
#include "Managers/ClipboardManager.h"
#include "Managers/InputManager.h"
#include "Managers/RandomManager.h"
#include "Managers/ResourceManager.h"
#include "Managers/SaveManager.h"
#include "Managers/TimeManager.h"

struct EngineContext {
  RandomManager random;
  TimeManager time;
  SaveManager save;
  ClipboardManager clipboard;
  ResourceManager resources;
  AudioManager audio;
  InputManager input;
};
