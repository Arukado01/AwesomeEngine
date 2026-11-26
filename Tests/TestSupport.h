#pragma once

#include <cmath>
#include <cstdio>

namespace TestSupport {
  inline int failures = 0;

  inline void Fail(const char *file, int line, const char *expression) {
    std::fprintf(stderr, "%s:%d: fallo: %s\n", file, line, expression);
    ++failures;
  }

  inline void FailNear(const char *file, int line, const char *expression,
                       double actual, double expected, double epsilon) {
    std::fprintf(stderr,
                 "%s:%d: fallo: %s (actual=%g, esperado=%g, epsilon=%g)\n",
                 file, line, expression, actual, expected, epsilon);
    ++failures;
  }

  inline int Result() {
    if (failures == 0) {
      std::printf("OK: todas las comprobaciones pasaron\n");
      return 0;
    }

    std::fprintf(stderr, "FALLARON %d comprobaciones\n", failures);
    return 1;
  }
} // namespace TestSupport

#define EXPECT_TRUE(expr)                             \
  do {                                                \
    if (!(expr)) {                                    \
      ::TestSupport::Fail(__FILE__, __LINE__, #expr); \
    }                                                 \
  } while (0)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define EXPECT_NEAR(actual, expected, epsilon)                     \
  do {                                                             \
    const double ae_actual = static_cast<double>(actual);          \
    const double ae_expected = static_cast<double>(expected);      \
    const double ae_epsilon = static_cast<double>(epsilon);        \
    if (std::fabs(ae_actual - ae_expected) > ae_epsilon) {         \
      ::TestSupport::FailNear(__FILE__, __LINE__,                  \
                              #actual " ~= " #expected,            \
                              ae_actual, ae_expected, ae_epsilon); \
    }                                                              \
  } while (0)
