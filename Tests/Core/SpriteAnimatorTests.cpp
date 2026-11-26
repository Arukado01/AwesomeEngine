#include "Graphics/SpriteAnimator/SpriteAnimator.h"
#include "TestSupport.h"

#include <cstdio>
#include <vector>

namespace {
  std::vector<int> frames;

  void Record(int frame) { frames.push_back(frame); }

  void CheckFrames(const std::vector<int> &expected, const char *file, int line) {
    if (frames != expected) {
      std::fprintf(stderr, "%s:%d: cuadros inesperados. actual=", file, line);
      for (const int frame : frames) {
        std::fprintf(stderr, "%d ", frame);
      }
      std::fprintf(stderr, " esperado=");
      for (const int frame : expected) {
        std::fprintf(stderr, "%d ", frame);
      }
      std::fprintf(stderr, "\n");
      ++TestSupport::failures;
    }
    frames.clear();
  }
} // namespace

#define EXPECT_FRAMES(...) CheckFrames((__VA_ARGS__), __FILE__, __LINE__)

int main() {
  // 1. Estado inicial: Loop, cuadro 0, reproduciendo.
  {
    SpriteAnimator animator;
    EXPECT_TRUE(animator.GetMode() == SpritePlaybackMode::Loop);
    EXPECT_TRUE(animator.IsPlaying());
    EXPECT_FALSE(animator.IsFinished());
    EXPECT_NEAR(animator.CurrentFrame(4), 0, 0.001);
  }

  // 2. Avance parcial: 0,25 s a 8 FPS cruza los cuadros 1 y 2.
  {
    SpriteAnimator animator;
    frames.clear();
    const int frame = animator.Advance(0.25f, 4, 8.f, Record);
    EXPECT_NEAR(frame, 2, 0.001);
    EXPECT_FRAMES(std::vector<int>{1, 2});
  }

  // 3. Loop completo: 1 s a 4 FPS cruza 1, 2, 3 y vuelve a 0.
  {
    SpriteAnimator animator;
    frames.clear();
    const int frame = animator.Advance(1.f, 4, 4.f, Record);
    EXPECT_NEAR(frame, 0, 0.001);
    EXPECT_FRAMES(std::vector<int>{1, 2, 3, 0});
    EXPECT_TRUE(animator.IsPlaying());
    EXPECT_FALSE(animator.IsFinished());
  }

  // 4. Once: conserva el último cuadro y no repite eventos.
  {
    SpriteAnimator animator;
    animator.SetMode(SpritePlaybackMode::Once);
    EXPECT_TRUE(animator.GetMode() == SpritePlaybackMode::Once);

    frames.clear();
    EXPECT_NEAR(animator.Advance(0.5f, 4, 1.f, Record), 0, 0.001);
    EXPECT_FRAMES(std::vector<int>{});
    EXPECT_FALSE(animator.IsFinished());

    EXPECT_NEAR(animator.Advance(0.5f, 4, 1.f, Record), 1, 0.001);
    EXPECT_FRAMES(std::vector<int>{1});

    EXPECT_NEAR(animator.Advance(3.f, 4, 1.f, Record), 3, 0.001);
    EXPECT_FRAMES(std::vector<int>{2, 3});
    EXPECT_TRUE(animator.IsFinished());
    EXPECT_FALSE(animator.IsPlaying());

    frames.clear();
    EXPECT_NEAR(animator.Advance(1.f, 4, 1.f, Record), 3, 0.001);
    EXPECT_FRAMES(std::vector<int>{});
    EXPECT_TRUE(animator.IsFinished());
  }

  // 5. Resume no revive un Once terminado; Reset sí reinicia.
  {
    SpriteAnimator animator;
    animator.SetMode(SpritePlaybackMode::Once);
    animator.Advance(5.f, 4, 1.f);
    EXPECT_TRUE(animator.IsFinished());

    animator.Resume();
    EXPECT_TRUE(animator.IsFinished());
    EXPECT_FALSE(animator.IsPlaying());

    animator.Reset();
    EXPECT_FALSE(animator.IsFinished());
    EXPECT_TRUE(animator.IsPlaying());
    EXPECT_NEAR(animator.CurrentFrame(4), 0, 0.001);
  }

  // 6. Pausa conserva la fracción y no emite eventos.
  {
    SpriteAnimator animator;
    frames.clear();
    EXPECT_NEAR(animator.Advance(0.0625f, 4, 8.f, Record), 0, 0.001);
    EXPECT_FRAMES(std::vector<int>{});

    animator.Pause();
    EXPECT_FALSE(animator.IsPlaying());
    EXPECT_NEAR(animator.Advance(1.f, 4, 8.f, Record), 0, 0.001);
    EXPECT_FRAMES(std::vector<int>{});

    animator.Resume();
    EXPECT_NEAR(animator.Advance(0.0625f, 4, 8.f, Record), 1, 0.001);
    EXPECT_FRAMES(std::vector<int>{1});
  }

  // 7. Un callback puede pausar y cancela el resto del avance.
  {
    SpriteAnimator animator;
    frames.clear();
    int calls = 0;
    const int frame = animator.Advance(1.f, 4, 4.f, [&](int value) {
      ++calls;
      Record(value);
      animator.Pause();
    });

    EXPECT_NEAR(frame, 1, 0.001);
    EXPECT_FRAMES(std::vector<int>{1});
    EXPECT_TRUE(calls == 1);
    EXPECT_FALSE(animator.IsPlaying());
  }

  // 8. FrameRect: primera fila, salto de fila y cuadro inválido.
  {
    const SpriteAnimationClip clip{2, 4, 8.f};

    const sf::IntRect first = SpriteAnimator::FrameRect(clip, 0, 4, {16, 24});
    EXPECT_NEAR(first.position.x, 32, 0.001);
    EXPECT_NEAR(first.position.y, 0, 0.001);
    EXPECT_NEAR(first.size.x, 16, 0.001);
    EXPECT_NEAR(first.size.y, 24, 0.001);

    const sf::IntRect wrapped = SpriteAnimator::FrameRect(clip, 3, 4, {16, 24});
    EXPECT_NEAR(wrapped.position.x, 16, 0.001);
    EXPECT_NEAR(wrapped.position.y, 24, 0.001);
    EXPECT_NEAR(wrapped.size.x, 16, 0.001);
    EXPECT_NEAR(wrapped.size.y, 24, 0.001);

    const sf::IntRect invalid = SpriteAnimator::FrameRect(clip, 4, 4, {16, 24});
    EXPECT_NEAR(invalid.size.x, 0, 0.001);
    EXPECT_NEAR(invalid.size.y, 0, 0.001);
  }

  // 9. Parámetros inválidos no alteran el cuadro ni emiten eventos.
  {
    SpriteAnimator animator;
    frames.clear();
    EXPECT_NEAR(animator.Advance(0.f, 4, 8.f, Record), 0, 0.001);
    EXPECT_NEAR(animator.Advance(-1.f, 4, 8.f, Record), 0, 0.001);
    EXPECT_NEAR(animator.Advance(1.f, 0, 8.f, Record), 0, 0.001);
    EXPECT_NEAR(animator.Advance(1.f, 4, 0.f, Record), 0, 0.001);
    EXPECT_FRAMES(std::vector<int>{});
  }

  // 10. Cambiar de modo reinicia el ciclo.
  {
    SpriteAnimator animator;
    animator.Advance(0.5f, 4, 4.f);
    EXPECT_NEAR(animator.CurrentFrame(4), 2, 0.001);

    animator.SetMode(SpritePlaybackMode::Once);
    EXPECT_NEAR(animator.CurrentFrame(4), 0, 0.001);
    EXPECT_FALSE(animator.IsFinished());
  }

  return TestSupport::Result();
}
