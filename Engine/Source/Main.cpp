#include <SFML/GpuPreference.hpp>
#include <SFML/System/Sleep.hpp>

#include "Core/Engine.h"
#include "SFML/System/Time.hpp"

SFML_DEFINE_DISCRETE_GPU_PREFERENCE

int main() {
  Engine engine;

  while (engine.IsRunning()) {
    engine.ProcessEvent();

    if (!engine.HasFocus()) {
      sf::sleep(sf::milliseconds(10));
      continue;
    }

    engine.Update();
    engine.Render();
  }
}
