#define TESTINGENVCTL
#include "ctestify.h"

int TestingEnvironmentSetUp(){
    puts("Setting up\n");
    return 0;
}

int TestingEnvironmentDestroy(){
    puts("Destroying...");
    return 0;
}

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

    EXPECT_EQ(EQTEST1, 5, 5);
    EXPECT_EQM(EQTEST2, 5, 5, "Custom error message for expect_eq()!");
    EXPECT_EQ(EQTEST3, 5, 4);
    EXPECT_EQM(EQTEST4, 5, 4, "Custom error message for expect_eq()!");

    EXPECT_TRUE(TRUETEST1, 5);
    EXPECT_TRUEM(TRUETEST2, 5, "Custom error message for expect_true()!");
    EXPECT_TRUE(TRUETEST3, 0);
    EXPECT_TRUEM(TRUETEST4, 0, "Custom error message for expect_true()!");

    EXPECT_FALSE(FALSETEST1, 0);
    EXPECT_FALSEM(FALSETEST2, 0, "Custom error message for expect_false()!");
    EXPECT_FALSE(FALSETEST3, 5);
    EXPECT_FALSEM(FALSETEST4, 5, "Custom error message for expect_false()!");

    EXPECT_BIGGER(BIGGERTEST1, 5, 1);
    EXPECT_BIGGERM(BIGGERTEST2, 5, 1, "Custom error message for expect_bigger()!");
    EXPECT_BIGGER(BIGGERTEST3, 5, 10);
    EXPECT_BIGGERM(BIGGERTEST4, 5, 10, "Custom error message for expect_bigger()!");

    EXPECT_LESS(LESSTEST1, 1, 5);
    EXPECT_LESSM(LESSTEST2, 1, 5, "Custom error message for expect_less()!");
    EXPECT_LESS(LESSTEST3, 10, 5);
    EXPECT_LESSM(LESSTEST4, 10, 5, "Custom error message for expect_less()!");

    EXPECT_BIGGEROREQ(BIGGEROREQTEST1, 5, 1);
    EXPECT_BIGGEROREQM(BIGGEROREQTEST2, 5, 1, "Custom error message for expect_biggeroreq()!");
    EXPECT_BIGGEROREQ(BIGGEROREQTEST3, 5, 10);
    EXPECT_BIGGEROREQM(BIGGEROREQTEST4, 5, 10, "Custom error message for expect_biggeroreq()!");

    EXPECT_LESSOREQ(LESSOREQTEST1, 1, 5);
    EXPECT_LESSOREQM(LESSOREQTEST2, 1, 5, "Custom error message for expect_less()!");
    EXPECT_LESSOREQ(LESSOREQTEST3, 10, 5);
    EXPECT_LESSOREQM(LESSOREQTEST4, 10, 5, "Custom error message for expect_less()!");

	EXPECT_EQ(VOIDPTEST1, (void*)1, (void*)1);
	EXPECT_EQM(VOIDPTEST2, (void*)1, (void*)1, "Custom error message!");
	EXPECT_EQ(VOIDPTEST3, (void*)2, (void*)1)
	EXPECT_EQM(VOIDPTEST4, (void*)2, (void*)1, "Custom error message!");

	EXPECT_VALIDPTR(VOIDPTEST5, (void*)5);
	EXPECT_VALIDPTRM(VOIDPTEST6, (void*)5, "Custom error message for expect_validptr()!");
	EXPECT_VALIDPTR(VOIDPTEST7, (void*)0);
	EXPECT_VALIDPTRM(VOIDPTEST8, (void*)0, "Custom error message for expect_validptr()!");
    EXPECT_EQ(SAFEFUNCTEST1, func(), 5);
	EXPECT_EQ(TEST, 9, 0);
}
