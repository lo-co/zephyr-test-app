#include <zephyr/ztest.h>

ZTEST(my_test_suite, test_example) {
  zassert_true(1 == 1, "Numbers don't match");
}

ZTEST_SUITE(my_test_suite, NULL, NULL, NULL, NULL, NULL);
