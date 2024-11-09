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
#include <stdlib.h>

//Colors ANSI escape sequences
#define CRED     "\x1b[31m"
#define CGREEN   "\x1b[32m"
#define CYELLOW  "\x1b[33m"
#define CRESET   "\x1b[0m"
#define PROMPT   "==> "

//All global fields required by engine
int ctestify_total_functions = 0;
int ctestify_firstphase = 1;
int ctestify_successed = 0;
int ctestify_failed = 0;
int ctestify_ran = 0;
int assert_ctestify_failed = 0;
char* current_test_suite = "";
jmp_buf ctestify_sigsegv_buf = {0};
FILE* ctestify_stdout;
struct ComparerRet comparerret;
struct timeval ctestify_tstart, ctestify_tend, ctestify_tresult = {0};
char* ctestify_messages[] = {
	"Expected values equality!",
	"Expected true value!",
	"Expected false value!",
	"Expected bigger value!",
	"Expected less value!",
	"Expected bigger or equal value!",
	"Expected less or equal value!",
	"Expected function success, but it caused a segfault!",
	"Expected valid pointer!",
    "Expected not equal values!"
};

char* signals[] = {
	"",
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE",
	"SIGKILL",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM"
};

//Unions, enums and structs 
typedef enum Mode{
    SignedInt,
    UnsignedInt,
    Float,
    Double,
    CharPointer,
	VoidPointer
} Mode;

typedef enum ComparerResult{
    EQ=0,
	LESS=1,
	BIGGER=2
} ComparerResult;

typedef enum Test{
    EX_EQ,
    EX_NEQ,
	EX_TRUE,
	EX_FALSE,
	EX_BIG,
	EX_LESS,
	EX_BIGEQ,
	EX_LESSEQ,
	EX_VALPTR
} Test;

union ComparerArgs{
    uint64_t uintargs[2];
    int64_t intargs[2];
    float floatargs[2];
    double doubleargs[2];
    char* charpargs[2];
	void* voidpargs[2];
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
		void*: ptr_comparer, \
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
ComparerResult ptr_comparer(void* arg1, void* arg2){
	comparerret.args.voidpargs[0] = arg1;
	comparerret.args.voidpargs[1] = arg2;
	comparerret.mode = VoidPointer;

	if (arg1 == arg2) return EQ;
	else return BIGGER;
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
			return result == EQ;
        case EX_NEQ:
            return result != EQ;
		case EX_TRUE:
			return result != EQ;
		case EX_FALSE:
			return result == EQ;
		case EX_LESS:
			return result == LESS;
		case EX_BIG:
			return result == BIGGER;
		case EX_BIGEQ:
			return result == BIGGER || result == EQ;
		case EX_LESSEQ:
			return result == LESS || result == EQ;
		case EX_VALPTR:
			return result == BIGGER;
        default:
            return 0;
	}
}


#define DEFAULT_BODY CHECK_ASSERT_FAILURE \
    if (strlen(testname) <= 1 || strlen(current_test_suite) <= 1){ \
        fprintf(ctestify_stdout, "Test name or test suite name can't be null!\n");assert_ctestify_failed++;return;} \
    ctestify_ran++; \
    if (RETISGOOD(test, comparerresult)){ \
		gettimeofday(&ctestify_tend, NULL); \
		timersub(&ctestify_tend, &ctestify_tstart, &ctestify_tresult); \
		if (ctestify_tresult.tv_sec) fprintf(ctestify_stdout, "%s%s%s %s.%s (%ld.%03lds)\n", CGREEN, "[      OK ]", CRESET, current_test_suite, testname, ctestify_tresult.tv_sec, ctestify_tresult.tv_usec / 1000); \
		else fprintf(ctestify_stdout, "%s%s%s %s.%s (%ldms)\n", CGREEN, "[      OK ]", CRESET, current_test_suite, testname, ctestify_tresult.tv_usec / 1000); \
        ctestify_successed++;}else{ \
		gettimeofday(&ctestify_tend, NULL); \
		timersub(&ctestify_tend, &ctestify_tstart, &ctestify_tresult); \
        if (ctestify_tresult.tv_sec) fprintf(ctestify_stdout, "%s%s%s %s.%s (%ld.%03lds)\n", CRED, "[ FAILURE ]", CRESET, current_test_suite, testname, ctestify_tresult.tv_sec, ctestify_tresult.tv_usec / 1000); \
		else fprintf(ctestify_stdout, "%s%s%s %s.%s (%ldms)\n", CRED, "[ FAILURE ]", CRESET, current_test_suite, testname, ctestify_tresult.tv_usec / 1000); \
		ctestify_failed++; \
		if (isassert) assert_ctestify_failed++; \
        if (strlen(errmsg) > 1) \
		    fprintf(ctestify_stdout, "\t%s\n", errmsg); \
		else \
		    fprintf(ctestify_stdout, "\t%s\n", ctestify_messages[index]); \
		if (EXTENDEDMSG_NREQUIRED(test)) return; \
        switch(comparerret.mode){ \
            case SignedInt:{PRINT_EXPECTED_SIGNED break;} \
            case UnsignedInt:{PRINT_EXPECTED_UNSIGNED break;} \
            case Float:{PRINT_EXPECTED_FLOAT break;} \
            case Double:{PRINT_EXPECTED_DOUBLE break;} \
            case CharPointer:{PRINT_EXPECTED_CHARP break;} \
			case VoidPointer:{if(test == EX_EQ){PRINT_EXPECTEDEQ_VOIDP}else if(test == EX_VALPTR){PRINT_EXPECTED_VOIDP}}}}

//Small macroses for more easy functions structure, automatizing routines
#define EXTENDEDMSG_NREQUIRED(test) (test == EX_TRUE || test == EX_FALSE)
#define CHECK_ASSERT_FAILURE if (assert_ctestify_failed) return;
#define RESET_COMPARERRET memset(&comparerret, 0x00, sizeof(comparerret));
#define PRINT_EXPECTED_SIGNED fprintf(ctestify_stdout, "\tExpected: %ld (0x%lx)\n\tActual value of %s: %ld (0x%lx)\n", comparerret.args.intargs[1], comparerret.args.intargs[1], firstarg, comparerret.args.intargs[0], comparerret.args.intargs[0]);
#define PRINT_EXPECTED_UNSIGNED fprintf(ctestify_stdout, "\tExpected: %lu (0x%lx)\n\tActual value of %s: %lu (0x%lx)\n", comparerret.args.uintargs[1], comparerret.args.uintargs[1], firstarg, comparerret.args.uintargs[0], comparerret.args.uintargs[0]);
#define PRINT_EXPECTED_FLOAT fprintf(ctestify_stdout, "\tExpected: %f (0x%a)\n\tActual value of %s: %f (0x%a)\n", comparerret.args.floatargs[1], comparerret.args.floatargs[1], firstarg, comparerret.args.floatargs[0], comparerret.args.floatargs[0]);
#define PRINT_EXPECTED_DOUBLE fprintf(ctestify_stdout, "\tExpected: %lf (0x%a)\n\tActual value of %s: %lf (0x%a)\n", comparerret.args.doubleargs[1], comparerret.args.doubleargs[1], firstarg, comparerret.args.doubleargs[0], comparerret.args.doubleargs[0]);
#define PRINT_EXPECTED_CHARP fprintf(ctestify_stdout, "\tExpected: '%s' (0x%p)\n\tActual value of %s: '%s' (0x%p)\n", comparerret.args.charpargs[1], comparerret.args.charpargs[1], firstarg, comparerret.args.charpargs[0], comparerret.args.charpargs[0]);
#define PRINT_EXPECTEDEQ_VOIDP fprintf(ctestify_stdout, "\tExpected: %p\n\tActual pointer value: %p\n", comparerret.args.voidpargs[1], comparerret.args.voidpargs[0]);
#define PRINT_EXPECTED_VOIDP fprintf(ctestify_stdout, "\tActual pointer value: %p\n", comparerret.args.voidpargs[0]);

#define SAFE_WRAPPER(func, line, errmsg, test_name, test, value, expected, index) \
	if (ctestify_firstphase){ctestify_total_functions++;} else if (!assert_ctestify_failed) { \
    fprintf(ctestify_stdout, "%s%s%s %s.%s\n", CGREEN, "[ RUN     ]", CRESET, current_test_suite, test_name); \
    gettimeofday(&ctestify_tstart, NULL); \
	if (!setjmp(ctestify_sigsegv_buf)){ \
		func(errmsg, line, COMPARER(value, expected), test, #value, #expected, test_name, index); \
	} else { \
		gettimeofday(&ctestify_tend, NULL); \
		timersub(&ctestify_tend, &ctestify_tstart, &ctestify_tresult); \
		if (ctestify_tresult.tv_sec) fprintf(ctestify_stdout, "%s[ SIGSEGV ]%s %s.%s (%ld.%03lds)\n\t%s\n", CRED, CRESET, current_test_suite, test_name, ctestify_tresult.tv_sec, ctestify_tresult.tv_usec / 1000, ctestify_messages[7]); \
		else fprintf(ctestify_stdout, "%s[ SIGSEGV ]%s %s.%s (%ldms)\n\t%s\n", CRED, CRESET, current_test_suite, test_name, ctestify_tresult.tv_usec / 1000, ctestify_messages[7]); \
		assert_ctestify_failed++;ctestify_failed++;ctestify_ran++;}}

//General EXPECT and ASSERT declarations  
#define EXPECT_EQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_EQ, value, expected, 0)
#define ASSERT_EQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_EQ, value, expected, 0)
#define EXPECT_EQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_EQ, value, expected, 0)
#define ASSERT_EQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_EQ, value, expected, 0)

#define EXPECT_NEQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_NEQ, value, expected, 9)
#define ASSERT_NEQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_NEQ, value, expected, 9)
#define EXPECT_NEQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_NEQ, value, expected, 9)
#define ASSERT_NEQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_NEQ, value, expected, 9)

#define EXPECT_TRUE(test_name, value) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_TRUE, value, 0, 1)
#define ASSERT_TRUE(test_name, value) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_TRUE, value, 0, 1)
#define EXPECT_TRUEM(test_name, value, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_TRUE, value, 0, 1)
#define ASSERT_TRUEM(test_name, value, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_TRUE, value, 0, 1)

#define EXPECT_FALSE(test_name, value) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_FALSE, value, 0, 2)
#define ASSERT_FALSE(test_name, value) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_FALSE, value, 0, 2)
#define EXPECT_FALSEM(test_name, value, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_FALSE, value, 0, 2)
#define ASSERT_FALSEM(test_name, value, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_FALSE, value, 0, 2)

#define EXPECT_BIGGER(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_BIG, value, expected, 3)
#define ASSERT_BIGGER(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_BIG, value, expected, 3)
#define EXPECT_BIGGERM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_BIG, value, expected, 3)
#define ASSERT_BIGGERM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_BIG, value, expected, 3)

#define EXPECT_LESS(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_LESS, value, expected, 4)
#define ASSERT_LESS(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_LESS, value, expected, 4)
#define EXPECT_LESSM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_LESS, value, expected, 4)
#define ASSERT_LESSM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_LESS, value, expected, 4)

#define EXPECT_BIGGEROREQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_BIGEQ, value, expected, 5)
#define ASSERT_BIGGEROREQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_BIGEQ, value, expected, 5)
#define EXPECT_BIGGEROREQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_BIGEQ, value, expected, 5)
#define ASSERT_BIGGEROREQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_BIGEQ, value, expected, 5)

#define EXPECT_LESSOREQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_LESSEQ, value, expected, 6)
#define ASSERT_LESSOREQ(test_name, value, expected) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_LESSEQ, value, expected, 6)
#define EXPECT_LESSOREQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_LESSEQ, value, expected, 6)
#define ASSERT_LESSOREQM(test_name, value, expected, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_LESSEQ, value, expected, 6)

#define EXPECT_VALIDPTR(test_name, ptr) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, "", #test_name, EX_VALPTR, ptr, 0, 8)
#define ASSERT_VALIDPTR(test_name, ptr) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, "", #test_name, EX_VALPTR, ptr, 0, 8)
#define EXPECT_VALIDPTRM(test_name, ptr, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(expect_equals, __LINE__, errmsg, #test_name, EX_VALPTR, ptr, 0, 8)
#define ASSERT_VALIDPTRM(test_name, ptr, errmsg) if (ctestify_firstphase) ctestify_total_functions++; else SAFE_WRAPPER(assert_equals, __LINE__, errmsg, #test_name, EX_VALPTR, ptr, 0, 8)

//Additional functions
#define PRINT_START(func) print_start(#func);
#define SET_TEST_SUITE_NAME(name) current_test_suite = #name

#define PRINT(...) \
		fprintf(ctestify_stdout, "%s%s", CGREEN, PROMPT); \
		fprintf(ctestify_stdout, __VA_ARGS__); \
		fprintf(ctestify_stdout, "%s\n", CRESET); \
		fflush(ctestify_stdout);

//Handler of segmentation fault, required for tests engine stability 
void sigsegv_handler(int s){
	if (!ctestify_firstphase) longjmp(ctestify_sigsegv_buf, 1);
    if (s <= 15)
		fprintf(ctestify_stdout, "%sGot signal: %s%s\n", CRED, signals[s], CRESET);
	else
		fprintf(ctestify_stdout, "%sGot unknown signal!%s\n", CRED, CRESET);
	fprintf(ctestify_stdout, "%sCTestify: exiting now.%s\n", CRED, CRESET);
	exit(1);
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
	int isassert = 0;
    DEFAULT_BODY
    RESET_COMPARERRET
}

void assert_equals(char* errmsg, int32_t line, ComparerResult comparerresult, Test test, char* firstarg, char* secondarg, char* testname, int index){
	int isassert = 1;
    DEFAULT_BODY
    if (!RETISGOOD(test, comparerresult)) assert_ctestify_failed++;
    RESET_COMPARERRET
}

//Actual entry point, instead of the fake one
int main(){
    // Setting up local stdout
    ctestify_stdout = fopen("/dev/tty", "a");
    if (!ctestify_stdout){ printf(CRED "ctestify_failed to acquire local stdout copy!\n" CRESET); return 0;}
    // Testing environment setup
    fprintf(ctestify_stdout, "%s[=========]%s Setting up testing environment...\n\n", CGREEN, CRESET);
    signal(SIGSEGV, sigsegv_handler);
    ctestify_total_functions = 0;
    struct timeval start, end, result;
    // Disable stdout 
    int stdout_backup = dup(fileno(stdout));
    int dev_null_fd = open("/dev/null", O_WRONLY);
    dup2(dev_null_fd, fileno(stdout));
    test_main();
    // Return stdout back 
    dup2(stdout_backup, fileno(stdout));
	close(dev_null_fd);
	close(stdout_backup);
    if (ctestify_total_functions == 0){
        fprintf(ctestify_stdout, CRED "No tests detected!\n" CRESET);
        return 0;
    }
    ctestify_firstphase = 0;
    if (TestingEnvironmentSetUp()){
        fprintf(ctestify_stdout, CRED "Environment setup failure!\n" CRESET);
        return 1;
    }
    // Running tests
    fprintf(ctestify_stdout, "%s[=========]%s Running %d tests\n", CGREEN, CRESET, ctestify_total_functions);
    gettimeofday(&start, NULL);
    test_main();
    gettimeofday(&end, NULL);
    // Tests finalization, print results and destroy testing environment
    timersub(&end, &start, &result);
    if (ctestify_successed == 0) fprintf(ctestify_stdout, CRED);
    else if (ctestify_failed == 0) fprintf(ctestify_stdout, CGREEN);
    else fprintf(ctestify_stdout, CYELLOW);
    fprintf(ctestify_stdout, "[=========]%s %d out of %d tests finished ", CRESET, ctestify_ran, ctestify_total_functions);
    if (result.tv_sec) fprintf(ctestify_stdout, "(%ld.%03lds total)\n", result.tv_sec, result.tv_usec / 1000);
	else fprintf(ctestify_stdout, "(%ldms total)\n", result.tv_usec / 1000);
    fprintf(ctestify_stdout, "%s[=========]%s Destroying testing environment...\n\n", CGREEN, CRESET);
    if (TestingEnvironmentDestroy())
        fprintf(ctestify_stdout, CRED "\nTesting environment destroy failure!\n" CRESET);
	else fputc('\n', ctestify_stdout);
    if (ctestify_successed) fprintf(ctestify_stdout, "%s[ SUCCESS ]%s %d tests.\n", CGREEN, CRESET, ctestify_successed);
    if (ctestify_failed) fprintf(ctestify_stdout, "%s[ FAILURE ]%s %d tests.\n", CRED, CRESET, ctestify_failed);
}
