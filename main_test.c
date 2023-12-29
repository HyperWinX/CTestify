#include "ctestify.h"

int func(){
    int *p = 0;
    *p = 0xDEAD;
}

int fucking_function_test(){
    return 5;
}

void test_main(){
    EXPECT_FUNC_INT_EQ(fucking_function_test, 2);
    ASSERT_FUNC_INT_EQ(fucking_function_test, 2);
    EXPECT_FUNC_INT_EQ(fucking_function_test, 2);
}