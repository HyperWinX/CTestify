#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

//Colors ANSI escape sequences
#define CRED     "\x1b[31m"
#define CGREEN   "\x1b[32m"
#define CYELLOW  "\x1b[33m"
#define CRESET   "\x1b[0m"

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

//Function bodies
#define DEFAULT_BODY if(assert_failed) return; \
    if (strlen(testname) <= 1 || strlen(current_test_suite) <= 1){ \
        printf("Test name or test suite name can't be null!\n");assert_failed++;return;} \
	printf("%s%s%s %s.%s\n", CGREEN, "[ RUN     ]", CRESET, current_test_suite, testname); \
    if (comparerresult){ \
        printf("%s%s%s %s.%s\n", CGREEN, "[      OK ]", CRESET, current_test_suite, testname); \
        successed++;}else{ \
        printf("%s%s%s %s.%s\n", CRED, "[ FAILURE ]", CRESET, current_test_suite, testname); \
        char buf[256] = {0}; \
        int msg_avail = 0; \
        if (strlen(errmsg) > 1){ \
            snprintf(buf, sizeof(buf), "\t%s\n", errmsg); \
            msg_avail = 1;} \
        switch(comparerret.mode){ \
            case SignedInt:{PRINT_EXPECTED_SIGNED break;} \
            case UnsignedInt:{PRINT_EXPECTED_UNSIGNED break;} \
            case Float:{PRINT_EXPECTED_FLOAT break;} \
            case Double:{PRINT_EXPECTED_DOUBLE break;} \
            case CharPointer:{PRINT_EXPECTED_CHARP break;}}failed++;}ran++;

//Small macroses for more easy functions structure, automatizing routines
#define SPACECOUNT max_funcname_len - strlen(funcname)
#define OFFSET SPACECOUNT,""
#define CHECK_ASSERT_FAILURE if (assert_failed) return;
#define RESET_COMPARERRET comparerret.args.intargs[0] = 0; comparerret.args.intargs[1] = 0; comparerret.mode = 0;
#define PRINT_RUN printf("%s%s%s [%s] %s\n", CGREEN, "[ RUN     ]", CRESET, current_tests, funcname);
#define PRINT_EXPECTED_SIGNED printf("\t%sActual value of %s: %ld (0x%lx)\n\tExpected: %ld (0x%lx)\n", msg_avail ? buf : "", firstarg, comparerret.args.intargs[0], comparerret.args.intargs[0], comparerret.args.intargs[1], comparerret.args.intargs[1]);
#define PRINT_EXPECTED_UNSIGNED printf("\t%sActual value of %s: %lu (0x%lx)\n\tExpected: %lu (0x%lx)\n", msg_avail ? buf : "", firstarg, comparerret.args.uintargs[0], comparerret.args.uintargs[0], comparerret.args.uintargs[1], comparerret.args.uintargs[1]);
#define PRINT_EXPECTED_FLOAT printf("\t%sActual value of %s: %f (0x%a)\n\tExpected: %f (0x%a)\n", msg_avail ? buf : "", firstarg, comparerret.args.floatargs[0], comparerret.args.floatargs[0], comparerret.args.floatargs[1], comparerret.args.floatargs[1]);
#define PRINT_EXPECTED_DOUBLE printf("\t%sActual value of %s: %lf (0x%a)\n\tExpected: %lf (0x%a)\n", msg_avail ? buf : "", firstarg, comparerret.args.doubleargs[0], comparerret.args.doubleargs[0], comparerret.args.doubleargs[1], comparerret.args.doubleargs[1]);
#define PRINT_EXPECTED_CHARP printf("\t%sActual value of %s: %s (0x%p)\n\tExpected: %s (0x%p)\n", msg_avail ? buf : "", firstarg, comparerret.args.charpargs[0], comparerret.args.charpargs[0], comparerret.args.charpargs[1], comparerret.args.charpargs[1]);


//General EXPECT and ASSERT declaration
#define EXPECT_EQ(test_name, value, expected) if (firstphase) total_functions++; else expect_equals("", __LINE__, COMPARER(value, expected), #value, #expected, #test_name)
#define ASSERT_EQ(test_name, value, expected) if (firstphase) total_functions++; else assert_equals("", __LINE__, COMPARER(value, expected), #value, #expected, #test_name)
#define EXPECT_EQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, COMPARER(value, expected), #value, #expected, #test_name)
#define ASSERT_EQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else assert_equals(errmsg, __LINE__, COMPARER(value, expected), #value, #expected, #test_name)

#define EXPECT_TRUE(test_name, val) if (firstphase) total_functions++; else expect_equals("", __LINE__, val > 0, #val, "", #test_name)
#define ASSERT_TRUE(test_name, val) if (firstphase) total_functions++; else assert_equals("", __LINE__, val > 0, #val, "", #test_name)
#define EXPECT_TRUEM(test_name, val, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, val > 0, #val, "", #test_name)
#define ASSERT_TRUEM(test_name, val, errmsg) if (firstphase) total_functions++; else assert_equals(errmsg, __LINE__, val > 0, #val, "", #test_name)

#define EXPECT_FALSE(test_name, val) if (firstphase) total_functions++; else expect_equals("", __LINE__, val == 0, #val, "", #test_name)
#define ASSERT_FALSE(test_name, val) if (firstphase) total_functions++; else expect_equals("", __LINE__, val == 0, #val, "", #test_name)
#define EXPECT_FALSEM(test_name, val, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, val == 0, #val, "", #test_name)
#define ASSERT_FALSEM(test_name, val, errmsg) if (firstphase) total_functions++; else expect_equals(errmsg, __LINE__, val == 0, #val, "", #test_name)

//Additional functions
#define PRINT_START(func) print_start(#func);
#define SET_TEST_SUITE_NAME(name) current_test_suite = #name;

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

#ifndef TESTINGENVCTL
#define TESTINGENVCTL
int TestingEnvironmentSetUp(){return 0;}
int TestingEnvironmentDestroy(){return 0;}
#endif

//All testing functions

void expect_equals(char* errmsg, int32_t line, int comparerresult, char* firstarg, char* secondarg, char* testname){
    DEFAULT_BODY
    RESET_COMPARERRET
}

void assert_equals(char* errmsg, int32_t line, int comparerresult, char* firstarg, char* secondarg, char* testname){
    DEFAULT_BODY
    if (!comparerresult) assert_failed++;
    RESET_COMPARERRET
}

//Actual entry point, instead of the fake one
int main(){
    // Testing environment setup
    printf("%s[=========]%s Setting up testing environment...\n\n", CGREEN, CRESET);
    signal(SIGSEGV, sigsegv_handler);
    total_functions = 0;
    time_t start, end;
    // Disable stdout 
    int stdout_backup = dup(fileno(stdout));
    int dev_null_fd = open("/dev/null", O_WRONLY);
    dup2(dev_null_fd, fileno(stdout));
    close(dev_null_fd);
    test_main();
    // Return stdout back 
    dup2(stdout_backup, fileno(stdout));
    close(stdout_backup);
    if (total_functions == 0){
        printf(CRED "No tests detected!\n" CRESET);
        return 0;
    }
    firstphase = 0;
    if (TestingEnvironmentSetUp()){
        printf(CRED "Environment setup failure!\n" CRESET);
        return 1;
    }
    // Running tests
    printf("%s[=========]%s Running %d tests\n", CGREEN, CRESET, total_functions);
    start = clock();
    test_main();
    end = clock();
    // Tests finalization, print results and destroy testing environment
    long double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    if (successed == 0) printf(CRED);
    else if (failed == 0) printf(CGREEN);
    else printf(CYELLOW);
    printf("[=========]%s %d tests finished ", CRESET, ran);
    printf(total_time < 1 ? "(%.3Lfms total)\n\n" : "(%.3Lfs total)\n\n", total_time);
    printf("%s[=========]%s Destroying testing environment...\n", CGREEN, CRESET);
    if (TestingEnvironmentDestroy())
        printf(CRED "Testing environment destroy failure!\n" CRESET);
    if (successed) printf("%s[ SUCCESS ]%s %d tests.\n", CGREEN, CRESET, successed);
    if (failed) printf("%s[ FAILURE ]%s %d tests.\n", CRED, CRESET, failed);
}
