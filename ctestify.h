#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//Colors ANSI escape sequences
#define CRED     "\x1b[31m"
#define CGREEN   "\x1b[32m"
#define CYELLOW  "\x1b[33m"
#define CRESET   "\x1b[0m"

//All global fields required by engine
int total_functions = 0;
int firstphase = 1;
int successed = 0;
int failed = 0;
int ran = 0;
int assert_failed = 0;
char* current_test_suite = "";
jmp_buf sigsegv_buf = {0};
int stdout_backup = 0;
int dev_null_fd = 0;
FILE* ctestify_stdout;
struct ComparerRet comparerret;
char* messages[] = {
	"Expected values equality!",
	"Expected true value!",
	"Expected false value!",
	"Expected bigger value!",
	"Expected less value!",
	"Expected bigger or equal value!",
	"Expected less or equal value!",
	"Expected function success, but it caused a segfault!"
};

//Unions, enums and structs 
typedef enum Mode{
    SignedInt,
    UnsignedInt,
    Float,
    Double,
    CharPointer
} Mode;

typedef enum ComparerResult{
    EQ=0,
	LESS=1,
	BIGGER=2
} ComparerResult;

typedef enum Test{
    EX_EQ,
	EX_TRUE,
	EX_FALSE,
	EX_BIG,
	EX_LESS,
	EX_BIGEQ,
	EX_LESSEQ
} Test;

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

ComparerResult signed_int_comparer(int64_t arg1, int64_t arg2){
    comparerret.args.intargs[0] = arg1;
    comparerret.args.intargs[1] = arg2;
    comparerret.mode = SignedInt;

	if (arg1 == arg2) return EQ;
	else if (arg1 > arg2) return BIGGER;
	else return LESS;
}
ComparerResult unsigned_int_comparer(uint64_t arg1, uint64_t arg2){
    comparerret.args.uintargs[0] = arg1;
    comparerret.args.uintargs[1] = arg2;
    comparerret.mode = UnsignedInt;

	if (arg1 == arg2) return EQ;
	else if (arg1 > arg2) return BIGGER;
	else return LESS;
}

ComparerResult str_comparer(char* arg1, char* arg2){
    comparerret.args.charpargs[0] = arg1;
    comparerret.args.charpargs[1] = arg2;
    comparerret.mode = CharPointer;

    if (!strcmp(arg1, arg2)) return EQ;
	else return LESS;
}
ComparerResult float_comparer(float arg1, float arg2){
    comparerret.args.floatargs[0] = arg1;
    comparerret.args.floatargs[1] = arg2;
    comparerret.mode = Float;

    if (arg1 == arg2) return EQ;
	else if (arg1 > arg2) return BIGGER;
	else return LESS;
}
ComparerResult double_comparer(double arg1, double arg2){
    comparerret.args.floatargs[0] = arg1;
    comparerret.args.floatargs[1] = arg2;
    comparerret.mode = Double;

    if (arg1 == arg2) return EQ;
	else if (arg1 > arg2) return BIGGER;
	else return LESS;
}

//Function bodies
int RETISGOOD(Test test, ComparerResult result){
		switch(test){
				case EX_EQ:
						return result == EQ ? 1 : 0;
				case EX_TRUE: 
						return result == BIGGER ? 1 : 0;
				case EX_FALSE:
						return result == EQ ? 1 : 0;
				case EX_LESS:
						return result == LESS ? 1 : 0;
				case EX_BIG:
						return result == BIGGER ? 1 : 0;
				case EX_BIGEQ:
						return result == BIGGER || result == EQ ? 1 : 0;
				case EX_LESSEQ:
						return result == LESS || result == EQ ? 1 : 0;}}


#define DEFAULT_BODY CHECK_ASSERT_FAILURE \
    if (strlen(testname) <= 1 || strlen(current_test_suite) <= 1){ \
        fprintf(ctestify_stdout, "Test name or test suite name can't be null!\n");assert_failed++;return;} \
    ran++; \
    if (RETISGOOD(test, comparerresult)){ \
        fprintf(ctestify_stdout, "%s%s%s %s.%s\n", CGREEN, "[      OK ]", CRESET, current_test_suite, testname); \
        successed++;}else{ \
        fprintf(ctestify_stdout, "%s%s%s %s.%s\n", CRED, "[ FAILURE ]", CRESET, current_test_suite, testname); \
		failed++; \
        int msg_avail = 0; \
        if (strlen(errmsg) > 1) \
		    fprintf(ctestify_stdout, "\t%s\n", errmsg); \
		else \
		    fprintf(ctestify_stdout, "\t%s\n", messages[index]); \
		if (EXTENDEDMSG_NREQUIRED(test)) return; \
        switch(comparerret.mode){ \
            case SignedInt:{PRINT_EXPECTED_SIGNED break;} \
            case UnsignedInt:{PRINT_EXPECTED_UNSIGNED break;} \
            case Float:{PRINT_EXPECTED_FLOAT break;} \
            case Double:{PRINT_EXPECTED_DOUBLE break;} \
            case CharPointer:{PRINT_EXPECTED_CHARP break;}}}

//Small macroses for more easy functions structure, automatizing routines
#define EXTENDEDMSG_NREQUIRED(test) (test == EX_TRUE || test == EX_FALSE)
#define CHECK_ASSERT_FAILURE if (assert_failed) return;
#define RESET_COMPARERRET memset(&comparerret, 0x00, sizeof(comparerret));
#define PRINT_EXPECTED_SIGNED fprintf(ctestify_stdout, "\tExpected: %ld (0x%lx)\n\tActual value of %s: %ld (0x%lx)\n", comparerret.args.intargs[1], comparerret.args.intargs[1], firstarg, comparerret.args.intargs[0], comparerret.args.intargs[0]);
#define PRINT_EXPECTED_UNSIGNED fprintf(ctestify_stdout, "\tExpected: %lu (0x%lx)\n\tActual value of %s: %lu (0x%lx)\n", comparerret.args.uintargs[1], comparerret.args.uintargs[1], firstarg, comparerret.args.uintargs[0], comparerret.args.uintargs[0]);
#define PRINT_EXPECTED_FLOAT fprintf(ctestify_stdout, "\tExpected: %f (0x%a)\n\tActual value of %s: %f (0x%a)\n", comparerret.args.floatargs[1], comparerret.args.floatargs[1], firstarg, comparerret.args.floatargs[0], comparerret.args.floatargs[0]);
#define PRINT_EXPECTED_DOUBLE fprintf(ctestify_stdout, "\tExpected: %lf (0x%a)\n\tActual value of %s: %lf (0x%a)\n", comparerret.args.doubleargs[1], comparerret.args.doubleargs[1], firstarg, comparerret.args.doubleargs[0], comparerret.args.doubleargs[0]);
#define PRINT_EXPECTED_CHARP fprintf(ctestify_stdout, "\tExpected: %s (0x%p)\n\tActual value of %s: %s (0x%p)\n", comparerret.args.charpargs[1], comparerret.args.charpargs[1], firstarg, comparerret.args.charpargs[0], comparerret.args.charpargs[0]);

#define SAFE_WRAPPER(func, line, errmsg, test_name, test, value, expected, index) \
	if (firstphase){total_functions++;} else if (!assert_failed) { \
    fprintf(ctestify_stdout, "%s%s%s %s.%s\n", CGREEN, "[ RUN     ]", CRESET, current_test_suite, test_name); \
	void* result = NULL; signal(SIGSEGV, sigsegv_handler); \
	if (!setjmp(sigsegv_buf)){ \
		func(errmsg, line, COMPARER(value, expected), test, #value, #expected, test_name, index); \
	} else { \
		fprintf(ctestify_stdout, "%s[ SIGSEGV ]%s %s.%s\n\t%s\n", CRED, CRESET, current_test_suite, test_name, messages[7]); \
		assert_failed++;failed++;ran++;}}

//General EXPECT and ASSERT declarations  
#define EXPECT_EQ(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_EQ, value, expected, 0)
#define ASSERT_EQ(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EQ_EQ, value, expected, 0)
#define EXPECT_EQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_EQ, value, expected, 0)
#define ASSERT_EQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_EQ, value, expected, 0)

#define EXPECT_TRUE(test_name, value) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_TRUE, value, 0, 1)
#define ASSERT_TRUE(test_name, value) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_TRUE, value, 0, 1)
#define EXPECT_TRUEM(test_name, value, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_TRUE, value, 0, 1)
#define ASSERT_TRUEM(test_name, value, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_TRUE, value, 0, 1)

#define EXPECT_FALSE(test_name, value) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_FALSE, value, 0, 2)
#define ASSERT_FALSE(test_name, value) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_FALSE, value, 0, 2)
#define EXPECT_FALSEM(test_name, value, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_FALSE, value, 0, 2)
#define ASSERT_FALSEM(test_name, value, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_FALSE, value, 0, 2)

#define EXPECT_BIGGER(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_BIG, value, expected, 3)
#define ASSERT_BIGGER(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_BIG, value, expected, 3)
#define EXPECT_BIGGERM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_BIG, value, expected, 3)
#define ASSERT_BIGGERM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_BIG, value, expected, 3)

#define EXPECT_LESS(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_LESS, value, expected, 4)
#define ASSERT_LESS(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_LESS, value, expected, 4)
#define EXPECT_LESSM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_LESS, value, expected, 4)
#define ASSERT_LESSM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_LESS, value, expected, 4)

#define EXPECT_BIGGEROREQ(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_BIGEQ, value, expected, 5)
#define ASSERT_BIGGEROREQ(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_BIGEQ, value, expected, 5)
#define EXPECT_BIGGEROREQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_BIGEQ, value, expected, 5)
#define ASSERT_BIGGEROREQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_BIGEQ, value, expected, 5)

#define EXPECT_LESSOREQ(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_LESSEQ, value, expected, 6)
#define ASSERT_LESSOREQ(test_name, value, expected) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_LESSEQ, value, expected, 6)
#define EXPECT_LESSOREQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_LESSEQ, value, expected, 6)
#define ASSERT_LESSOREQM(test_name, value, expected, errmsg) if (firstphase) total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_LESSEQ, value, expected, 6)

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

void expect_equals(char* errmsg, int32_t line, ComparerResult comparerresult, Test test, char* firstarg, char* secondarg, char* testname, int index){
    DEFAULT_BODY
    RESET_COMPARERRET
}

void assert_equals(char* errmsg, int32_t line, ComparerResult comparerresult, Test test, char* firstarg, char* secondarg, char* testname, int index){
    DEFAULT_BODY
    if (!comparerresult) assert_failed++;
    RESET_COMPARERRET
}

//Actual entry point, instead of the fake one
int main(){
    // Setting up local stdout
    ctestify_stdout = fopen("/dev/tty", "a");
    if (!ctestify_stdout){ printf(CRED "Failed to acquire local stdout copy!\n" CRESET); return 0;}
    // Testing environment setup
    fprintf(ctestify_stdout, "%s[=========]%s Setting up testing environment...\n\n", CGREEN, CRESET);
    signal(SIGSEGV, sigsegv_handler);
    total_functions = 0;
    time_t start, end;
    // Disable stdout 
    stdout_backup = dup(fileno(stdout));
    dev_null_fd = open("/dev/null", O_WRONLY);
    dup2(dev_null_fd, fileno(stdout));
    test_main();
    // Return stdout back 
    dup2(stdout_backup, fileno(stdout));
    if (total_functions == 0){
        fprintf(ctestify_stdout, CRED "No tests detected!\n" CRESET);
        return 0;
    }
    firstphase = 0;
    if (TestingEnvironmentSetUp()){
        fprintf(ctestify_stdout, CRED "Environment setup failure!\n" CRESET);
        return 1;
    }
    // Running tests
    fprintf(ctestify_stdout, "%s[=========]%s Running %d tests\n", CGREEN, CRESET, total_functions);
    start = clock();
    test_main();
    end = clock();
    // Tests finalization, print results and destroy testing environment
    long double total_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    if (successed == 0) fprintf(ctestify_stdout, CRED);
    else if (failed == 0) fprintf(ctestify_stdout, CGREEN);
    else fprintf(ctestify_stdout, CYELLOW);
    fprintf(ctestify_stdout, "[=========]%s %d tests finished ", CRESET, ran);
    fprintf(ctestify_stdout, "(%.3Lfms total)\n\n", total_time);
    fprintf(ctestify_stdout, "%s[=========]%s Destroying testing environment...\n", CGREEN, CRESET);
    if (TestingEnvironmentDestroy())
        fprintf(ctestify_stdout, CRED "Testing environment destroy failure!\n" CRESET);
    close(stdout_backup);
    close(dev_null_fd);
    if (successed) fprintf(ctestify_stdout, "%s[ SUCCESS ]%s %d tests.\n", CGREEN, CRESET, successed);
    if (failed) fprintf(ctestify_stdout, "%s[ FAILURE ]%s %d tests.\n", CRED, CRESET, failed);
}
