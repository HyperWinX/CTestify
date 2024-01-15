#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <stdbool.h>

//Colors ANSI escape sequences
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//All global fields required by engine
int total_functions = 0;
int currentfunction = 1;
int firstphase = 1;
int max_funcname_len = 0;
int successed = 0;
int failed = 0;
int ran = 0;
int assert_failed = 0;
long double totaltime = 0;
char* current_test_suite = "";
jmp_buf sigsegv_buf = {0};
struct ComparerRet comparerret;

//Unions, enums and structs
typedef enum Mode{
    SignedInt,
    UnsignedInt,
    Float,
    Double,
    CharPointer
} Mode;

union ComparerArgs{
    uint64_t uintargs[2];
    int64_t intargs[2];
    float floatargs[2];
    double doubleargs[2];
    char* charpargs[2];
};

struct ComparerRet{
    union ComparerArgs args;
    Mode mode;
};

//Comparers
#define COMPARER(arg1, arg2) _Generic((arg1), \
		int8_t: signed_int_comparer, \
		int16_t: signed_int_comparer, \
		int32_t: signed_int_comparer, \
		int64_t: signed_int_comparer, \
		uint8_t: unsigned_int_comparer, \
		uint16_t: unsigned_int_comparer, \
		uint32_t: unsigned_int_comparer, \
		uint64_t: unsigned_int_comparer, \
		char*: str_comparer, \
		float: float_comparer, \
		double: double_comparer)(arg1, arg2)

int signed_int_comparer(int64_t arg1, int64_t arg2){
    comparerret.args.intargs[0] = arg1;
    comparerret.args.intargs[1] = arg2;

    return arg1 == arg2;
}
int unsigned_int_comparer(uint64_t arg1, uint64_t arg2){
    comparerret.args.uintargs[0] = arg1;
    comparerret.args.uintargs[1] = arg2;

    return arg1 == arg2;
}
int str_comparer(char* arg1, char* arg2){
    comparerret.args.charpargs[0] = arg1;
    comparerret.args.charpargs[1] = arg2;

    return !strcmp(arg1, arg2);
}
int float_comparer(float arg1, float arg2){
    comparerret.args.floatargs[0] = arg1;
    comparerret.args.floatargs[1] = arg2;

    return arg1 == arg2;
}
int double_comparer(double arg1, double arg2){
    comparerret.args.floatargs[0] = arg1;
    comparerret.args.floatargs[1] = arg2;

    return arg1 == arg2;
}

//Small macroses for more easy functions structure, automatizing routines
#define SPACECOUNT max_funcname_len - strlen(funcname)
#define OFFSET SPACECOUNT,""
#define CHECK_ASSERT_FAILURE if (assert_failed) return;
#define RESET_COMPARERRET comparerret.args.intargs[0] = 0; comparerret.args.intargs[1] = 0; comparerret.mode = 0;
#define PRINT_RUN printf("%s%s%s [%s] %s\n", ANSI_COLOR_GREEN, "[ RUN     ]", ANSI_COLOR_RESET, current_tests, funcname);
#define PRINT_EXPECTED_SIGNED printf("\t%s: %ld (0x%lx)\n\tExpected: %ld (0x%lx)\n", firstarg, comparerret.args.intargs[0], comparerret.args.intargs[0], comparerret.args.intargs[1], comparerret.args.intargs[1]);
#define PRINT_EXPECTED_UNSIGNED printf("\t%s: %lu (0x%lx)\n\tExpected: %lu (0x%lx)\n", firstarg, comparerret.args.uintargs[0], comparerret.args.uintargs[0], comparerret.args.uintargs[1], comparerret.args.uintargs[1]);
#define PRINT_EXPECTED_FLOAT printf("\t%s: %f (0x%lx)\n\tExpected: %f (0x%x)\n", firstarg, comparerret.args.floatargs[0], comparerret.args.floatargs[0], comparerret.args.floatargs[1], comparerret.args.floatargs[1]);
#define PRINT_EXPECTED_DOUBLE printf("\t%s: %lf (0x%lx)\n\tExpected: %lf (0x%lx)\n", firstarg, comparerret.args.doubleargs[0], comparerret.args.doubleargs[0], comparerret.args.doubleargs[1], comparerret.args.doubleargs[1]);
#define PRINT_EXPECTED_CHARP printf("\t%s: %s (0x%lx)\n\tExpected: %s (0x%lx)\n", firstarg, comparerret.args.charpargs[0], comparerret.args.charpargs[0], comparerret.args.charpargs[1], comparerret.args.charpargs[1]);


//General EXPECT and ASSERT declaration
#define EXPECT_EQ(test_name, value, expected) if (firstphase) total_functions++; else expect_equals("", __LINE__, COMPARER(value, expected), #value, #expected, test_name)
#define ASSERT_EQ(test_name, value, expected) if (firstphase) total_functions++; else assert_equals("", __LINE__, COMPARER(value, expected), #value, #expected, test_name)
#define EXPECT_EQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, COMPARER(value, expected), #value, #expected, test_name)
#define ASSERT_EQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else assert_equals(errmsg, __LINE__, COMPARER(value, expected), #value, #expected, test_name)

#define EXPECT_TRUE(test_name, val) if (firstphase) total_functions++; else expect_equals("", __LINE__, val > 0, #val, "", test_name)
#define ASSERT_TRUE(test_name, val) if (firstphase) total_functions++; else assert_equals("", __LINE__, val > 0, #val, "", test_name)
#define EXPECT_TRUEM(test_name, val, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, val > 0, #val, "", test_name)
#define ASSERT_TRUEM(test_name, val, errmsg) if (firstphase) total_functions++; else assert_equals(errmsg, __LINE__, val > 0, #val, "", test_name)

#define EXPECT_FALSE(test_name, val) if (firstphase) total_functions++; else expect_equals("", __LINE__, val == 0, #val, "", test_name)
#define ASSERT_FALSE(test_name, val) if (firstphase) total_functions++; else expect_equals("", __LINE__, val == 0, #val, "", test_name)
#define EXPECT_FALSEM(test_name, val, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, val == 0, #val, "", test_name)
#define ASSERT_FALSEM(test_name, val, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, val == 0, #val, "", test_name)

//Additional functions
#define PRINT_START(func) print_start(#func);
#define SET_TEST_SUITE_NAME(name) current_test_suite = name;

//Handler of segmentation fault, required for tests engine stability
void sigsegv_handler(int s){
    switch (s){
        case SIGSEGV:
            longjmp(sigsegv_buf, 1);
            break;
    }
}

void test_main();
int TestingEnvironmentSetUp();
int TestingEnvironmentDestroy();

//All testing functions

void expect_equals(char* errmsg, int32_t line, int compareresult, char* firstarg, char* secondarg, char* testname){
    if(assert_failed) return;
    if (strlen(testname) <= 1 || strlen(current_test_suite) <= 1){
        printf("Test name or test suite name can't be null!\n");
        assert_failed++;
        return;
    }
	printf("%s%s%s %s.%s\n", ANSI_COLOR_GREEN, "[ RUN     ]", ANSI_COLOR_RESET, current_test_suite, testname);
    if (compareresult){
        printf("%s%s%s %s.%s\n", ANSI_COLOR_GREEN, "[      OK ]", ANSI_COLOR_RESET, current_test_suite, testname);
        successed++;
    }
    else{
        printf("%s%s%s %s.%s\n", ANSI_COLOR_RED, "[ FAILURE ]", ANSI_COLOR_RESET, current_test_suite, testname);
        switch(comparerret.mode){
            case SignedInt:{
                printf("\t%s: %ld (0x%lx)\n\tExpected: %ld (0x%lx)\n", firstarg, comparerret.args.intargs[0], comparerret.args.intargs[0],
                       comparerret.args.intargs[1],
                       comparerret.args.intargs[1]);
                break;
            }
            case UnsignedInt:{
                printf("\t%s: %lu (0x%lx)\n\tExpected: %lu (0x%lx)\n", firstarg, comparerret.args.uintargs[0], comparerret.args.uintargs[0],
                       comparerret.args.uintargs[1],
                       comparerret.args.uintargs[1]);
                break;
            }
            case Float:{
                printf("\t%s: %f (0x%lx)\n\tExpected: %f (0x%lx)\n", firstarg, comparerret.args.floatargs[0], comparerret.args.floatargs[0],
                       comparerret.args.floatargs[1],
                       comparerret.args.floatargs[1]);
                break;
            }
            case Double:{
                printf("\t%s: %f (0x%lx)\n\tExpected: %f (0x%lx)\n", firstarg, comparerret.args.floatargs[0], comparerret.args.floatargs[0],
                       comparerret.args.floatargs[1],
                       comparerret.args.floatargs[1]);
                break;
            }
            case CharPointer:{

            }
            default:{
                printf("bruh wtf\n");
                break;
            }
        }
        failed++;
    }
    ran++;
    RESET_COMPARERRET
}

void assert_equals(char* errmsg, int32_t line, int compareresult, char* firstarg, char* secondarg, char* testname){
    
}

//Actual entry point, instead of the fake one
int main(){
    // Testing environment setup
    printf("%s[=========]%s Setting up testing environment...\n\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
    signal(SIGSEGV, sigsegv_handler);
    total_functions = 0;
    time_t start, end;
    test_main();
    if (total_functions == 0){
        printf(ANSI_COLOR_RED "No tests detected!\n" ANSI_COLOR_RESET);
        return 0;
    }
    firstphase = 0;
    TestingEnvironmentSetUp();
    // Running tests
    printf("%s[=========]%s Running %d tests\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, total_functions);
    start = clock();
    test_main();
    end = clock();
    // Tests finalization, print results and destroy testing environment
    long double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    if (successed == 0) printf(ANSI_COLOR_RED);
    else if (failed == 0) printf(ANSI_COLOR_GREEN);
    else printf(ANSI_COLOR_YELLOW);
    printf("[=========]%s %d tests finished (%Lfs total)\n\n", ANSI_COLOR_RESET, ran, total_time);
    printf("%s[=========]%s Destroying testing environment...\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
    TestingEnvironmentDestroy();
    if (successed) printf("%s[ SUCCESS ]%s %d tests.\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, successed);
    if (failed) printf("%s[ FAILURE ]%s %d tests.\n", ANSI_COLOR_RED, ANSI_COLOR_RESET, failed);
}
