#include "test.h"
#include <stddef.h>
#include <stdio.h>

#define MAX_TESTS 256

static TestFunc test_funcs[MAX_TESTS];
static const char *test_names[MAX_TESTS];
static int test_count = 0;

void register_test(const char *name, TestFunc fn) {
  test_names[test_count] = name;
  test_funcs[test_count] = fn;
  test_count++;
}

int run_all_tests() {
  int failed = 0;

  printf(TEXT_BOLD "\nRunning %d test(s):\n\n" TEXT_RESET, test_count);

  for (int i = 0; i < test_count; i++) {
    if (test_funcs[i]() != 0) {
      printf(TEXT_RED "❌ %s\n" TEXT_RESET, test_names[i]);
      failed++;
    } else {
      printf(TEXT_GREEN "✅ %s\n" TEXT_RESET, test_names[i]);
    }
  }

  return failed;
}
