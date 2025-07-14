#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define TEXT_GREEN "\033[32m"
#define TEXT_RED "\033[31m"
#define TEXT_RESET "\033[0m"
#define TEXT_BOLD "\033[1m"
#define TEXT_GRAY "\033[90m"

#define TEST(name)                                                             \
  static int test_##name();                                                    \
  void register_##name();                                                      \
  __attribute__((constructor)) void register_##name() {                        \
    register_test(#name, test_##name);                                         \
  }                                                                            \
  static int test_##name()

#define ASSERT_STR_EQ(expected, actual)                                        \
  do {                                                                         \
    if (strcmp(expected, actual) != 0) {                                       \
      fprintf(stderr,                                                          \
              "  " TEXT_RED "✘ Assertion failed" TEXT_RESET " at %s:%d:\n",    \
              __FILE__, __LINE__);                                             \
      fprintf(stderr, "    Expected: \"%s\"\n", (expected));                   \
      fprintf(stderr, "    Actual:   \"%s\"\n\n", (actual));                   \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_SIZE_EQUALS(expected, actual)                                   \
  do {                                                                         \
    if ((size_t)expected != (size_t)actual) {                                  \
      fprintf(stderr,                                                          \
              "  " TEXT_RED "✘ Assertion failed" TEXT_RESET " at %s:%d:\n",    \
              __FILE__, __LINE__);                                             \
      fprintf(stderr, "    Expected: \"%zu\"\n", (size_t)(expected));          \
      fprintf(stderr, "    Actual:   \"%zu\"\n\n", (size_t)(actual));          \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_INT_EQUALS(expected, actual)                                    \
  do {                                                                         \
    if (expected != actual) {                                                  \
      fprintf(stderr,                                                          \
              "  " TEXT_RED "✘ Assertion failed" TEXT_RESET " at %s:%d:\n",    \
              __FILE__, __LINE__);                                             \
      fprintf(stderr, "    Expected: \"%d\"\n", (expected));                   \
      fprintf(stderr, "    Actual:   \"%d\"\n\n", (actual));                   \
      return 1;                                                                \
    }                                                                          \
  } while (0)

typedef int (*TestFunc)();
void register_test(const char *name, TestFunc fn);
int run_all_tests();

#endif
