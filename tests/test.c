#include "test.h"
#include <stddef.h>
#include <stdio.h>

#define MAX_TESTS 256

#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define GRAY "\033[90m"

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

  printf(BOLD "\nRunning %d test(s):\n\n" RESET, test_count);

  for (int i = 0; i < test_count; i++) {
    if (test_funcs[i]() != 0) {
      printf(RED "❌ %s\n" RESET, test_names[i]);
      failed++;
    } else {
      printf(GREEN "✅ %s\n" RESET, test_names[i]);
    }
  }

  return failed;
}
