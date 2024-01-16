#include "ctestify.h"

int func(){
    int *p = 0;
    *p = 0xDEAD;
    return 0;
}

int fucking_function_test(){
    return 5;
}

void test_main(){
    SET_TEST_SUITE_NAME(TESTS);
    EXPECT_EQ(ACTUALTESTS, 5, 0);
    EXPECT_EQ(STREQTEST, "test", "test");
}
