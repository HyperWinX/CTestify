#include "ctestify.h"

int func(){
    return 1;
}

int fucking_function_test(){
    return 2;
}

void test_main(){
    EXPECT_FUNC_INT_EQ(func, 1);
    EXPECT_FUNC_INT_EQ(func, 2);
    EXPECT_FUNC_INT_EQ(fucking_function_test, 3);
}