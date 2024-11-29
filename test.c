#include "include/ctestify.h"
#include <assert.h>
#include <stdint.h>

CTEST(CTestifyTesting, TestExpectEq) {
  uint8_t a1 = 5, b1 = 5;
  uint16_t a2 = 5, b2 = 5;
  uint32_t a3 = 5, b3 = 5;
  uint64_t a4 = 5, b4 = 5;
  int8_t a5 = 5, b5 = 5;
  int16_t a6 = 5, b6 = 5;
  int32_t a7 = 5, b7 = 5;
  int64_t a8 = 5, b8 = 5;
  float a9 = 5.95, b9 = 5.95;
  double a10 = 3.14179837, b10 = 3.14179837;

  EXPECT_EQ(a1, b1);
  EXPECT_EQ(a2, b2);
  EXPECT_EQ(a3, b3);
  EXPECT_EQ(a4, b4);
  EXPECT_EQ(a5, b5);
  EXPECT_EQ(a6, b6);
  EXPECT_EQ(a7, b7);
  EXPECT_EQ(a8, b8);
  EXPECT_EQ(a9, b9);
  EXPECT_EQ(a10, b10);
}

int main() {
  RUN_ALL_TESTS();
}
