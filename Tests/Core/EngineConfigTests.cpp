#include "Core/EngineConfig.h"
#include "TestSupport.h"

#include <cstdio>
#include <cstdlib>
#include <string>

int main() {
  const char *raw = std::getenv("AE_CONFIG_CASE");
  const std::string mode = raw ? raw : "";

  EXPECT_TRUE(gConfig.windowTitle == "Fixture");
  EXPECT_NEAR(gConfig.windowSize.x, 800, 0.001);
  EXPECT_NEAR(gConfig.windowSize.y, 600, 0.001);
  EXPECT_TRUE(gConfig.disableSfmlLogs);
  EXPECT_NEAR(gConfig.maximumDeltaTime.asSeconds(), 0.25, 0.001);
  EXPECT_NEAR(gConfig.globalVolume, 0.5, 0.001);
  EXPECT_TRUE(gConfig.backgroundColor.r == 10);
  EXPECT_TRUE(gConfig.backgroundColor.g == 20);
  EXPECT_TRUE(gConfig.backgroundColor.b == 30);
  EXPECT_NEAR(gConfig.cursorSpeed, 500, 0.001);
  EXPECT_NEAR(gConfig.joystickDeadzone, 0.2, 0.001);

  if (mode == "valid") {
    EXPECT_TRUE(gConfig.frameRateLimit == 60);
    EXPECT_NEAR(gConfig.cursorSize, 32, 0.001);
    EXPECT_TRUE(gConfig.games.size() == 2);
    if (gConfig.games.size() == 2) {
      EXPECT_TRUE(gConfig.games[0].name == "A");
      EXPECT_TRUE(gConfig.games[0].title == "Game A");
      EXPECT_TRUE(gConfig.games[0].controls == "A controls");
      EXPECT_TRUE(gConfig.games[1].name == "B");
      EXPECT_TRUE(gConfig.games[1].title == "Game B");
      EXPECT_TRUE(gConfig.games[1].controls == "B controls");
    }
  } else if (mode == "minimal") {
    EXPECT_TRUE(gConfig.frameRateLimit == 120);
    EXPECT_NEAR(gConfig.cursorSize, 24, 0.001);
    EXPECT_TRUE(gConfig.games.empty());
  } else {
    std::fprintf(stderr, "AE_CONFIG_CASE no reconocido: %s\n", mode.c_str());
    ++TestSupport::failures;
  }

  return TestSupport::Result();
}
