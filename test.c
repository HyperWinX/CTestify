#include "include/ctestify.h"
#include <assert.h>

CTEST(FooSuite, Test1) {
  EXPECT_EQ(5, 5);
}

int main() {
  RUN_ALL_TESTS();
}
