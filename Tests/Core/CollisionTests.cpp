#include "Managers/CollisionManager.h"
#include "TestSupport.h"

#include <cstdlib>

namespace {
  sf::FloatRect Box(float x, float y, float width = 10.f, float height = 10.f) {
    return sf::FloatRect({x, y}, {width, height});
  }
} // namespace

int main() {
  CollisionManager collisions;

  // 1. Dos cajas superpuestas comparten un área positiva.
  {
    const auto hit = collisions.Intersect(Box(0.f, 0.f), Box(5.f, 5.f));
    EXPECT_TRUE(hit.has_value());
    if (hit) {
      EXPECT_NEAR(hit->position.x, 5.f, 0.001f);
      EXPECT_NEAR(hit->position.y, 5.f, 0.001f);
      EXPECT_NEAR(hit->size.x, 5.f, 0.001f);
      EXPECT_NEAR(hit->size.y, 5.f, 0.001f);
    }
  }

  // 2. Dos cajas separadas no comparten área.
  {
    const auto hit = collisions.Intersect(Box(0.f, 0.f), Box(20.f, 0.f));
    EXPECT_FALSE(hit.has_value());
  }

  // 3. Si ya se solapaban, el sweep devuelve tiempo cero y normal vacía.
  {
    const auto hit = collisions.Sweep(Box(0.f, 0.f), Box(1.f, 0.f),
                                      Box(5.f, 5.f), Box(5.f, 5.f));
    EXPECT_TRUE(hit.has_value());
    if (hit) {
      EXPECT_NEAR(hit->time, 0.f, 0.001f);
      EXPECT_TRUE(hit->initialOverlap);
      EXPECT_NEAR(hit->normal.x, 0.f, 0.001f);
      EXPECT_NEAR(hit->normal.y, 0.f, 0.001f);
    }
  }

  // 4. A cruza B a mitad del frame: impacto en 0.4 con normal horizontal.
  {
    const auto hit = collisions.Sweep(Box(0.f, 0.f), Box(100.f, 0.f),
                                      Box(50.f, 0.f), Box(50.f, 0.f));
    EXPECT_TRUE(hit.has_value());
    if (hit) {
      EXPECT_FALSE(hit->initialOverlap);
      EXPECT_NEAR(hit->time, 0.4f, 0.001f);
      EXPECT_NEAR(hit->normal.x, -1.f, 0.001f);
      EXPECT_NEAR(hit->normal.y, 0.f, 0.001f);
    }
  }

  // 5. A no alcanza a B durante el frame.
  {
    const auto hit = collisions.Sweep(Box(0.f, 0.f), Box(20.f, 0.f),
                                      Box(50.f, 0.f), Box(50.f, 0.f));
    EXPECT_FALSE(hit.has_value());
  }

  // 6. Empate en esquina: el código prioriza el eje vertical.
  {
    const auto hit = collisions.Sweep(Box(0.f, 0.f), Box(10.f, 10.f),
                                      Box(10.f, 10.f), Box(10.f, 10.f));
    EXPECT_TRUE(hit.has_value());
    if (hit) {
      EXPECT_FALSE(hit->initialOverlap);
      EXPECT_NEAR(hit->time, 0.f, 0.001f);
      EXPECT_NEAR(hit->normal.x, 0.f, 0.001f);
      EXPECT_NEAR(hit->normal.y, -1.f, 0.001f);
    }
  }

  // La prueba negativa de CTest activa esta rama a propósito.
  if (std::getenv("AE_TEST_FORCE_FAIL") != nullptr) {
    EXPECT_TRUE(false);
  }

  return TestSupport::Result();
}
