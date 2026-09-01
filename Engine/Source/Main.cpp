#include "Core/Engine.h"
#include <SFML/GpuPreference.hpp>

SFML_DEFINE_DISCRETE_GPU_PREFERENCE

int main() {
  Engine engine;

  while (engine.IsRunning()) {
    engine.ProcessEvent();
    engine.Update();
    engine.Render();
  }
}
