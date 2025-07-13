#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define ASSERT(msg, cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "❌ %s:%d: %s\n", __FILE__, __LINE__, msg);              \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define TEST(name)                                                             \
  static int test_##name();                                                    \
  void register_##name();                                                      \
  __attribute__((constructor)) void register_##name() {                        \
    register_test(#name, test_##name);                                         \
  }                                                                            \
  static int test_##name()

typedef int (*TestFunc)();
void register_test(const char *name, TestFunc fn);
int run_all_tests();

#endif
