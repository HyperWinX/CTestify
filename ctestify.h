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

//Function bodies
#define EXPECT_FUNC_INT_BODY CHECK_ASSERT_FAILURE \
    HANDLE_PHASE1 \
    PRINT_RUN \
    clock_t start, stop; \
    int sigsegv = 0, retcode = 0; \
    short executed = 0; \
    if (!setjmp(sigsegv_buf)){ \
        start = clock(); \
        retcode = (*func)(); \
        stop = clock(); \
        executed++; \
    } else{ stop = clock(); sigsegv++; } \
    long double time = ((double)(stop - start)) / CLOCKS_PER_SEC; \
    totaltime += time; \
    if (!executed && sigsegv){ PRINT_SIGSEGV failed++;} \
    else if (retcode == expected){ \
        PRINT_OK successed++; \
    } else { \
        PRINT_EXPECTED failed++;} ran++;
#define ASSERT_FUNC_INT_BODY CHECK_ASSERT_FAILURE \
    HANDLE_PHASE1 \
    PRINT_RUN \
    clock_t start, stop; \
    int sigsegv = 0, retcode = 0; \
    short executed = 0; \
    if (!setjmp(sigsegv_buf)){ \
        start = clock(); \
        retcode = (*func)(); \
        stop = clock(); \
        executed++; \
    } else{ stop = clock(); sigsegv++;} \
    long double time = ((double)(stop - start)) / CLOCKS_PER_SEC; \
    totaltime += time; \
    if (!executed && sigsegv){ PRINT_SIGSEGV failed++; assert_failed++;} \
    else if (retcode == expected){ \
        PRINT_OK successed++; \
    } else { \
        PRINT_EXPECTED failed++; assert_failed++;} ran++;

//Small macroses for more easy functions structure, automatizing routines
#define SPACECOUNT max_funcname_len - strlen(funcname)
#define OFFSET SPACECOUNT,""
#define CHECK_ASSERT_FAILURE if (assert_failed) return;
#define PRINT_RUN printf("%s%s%s [%s] %s\n", ANSI_COLOR_GREEN, "[ RUN     ]", ANSI_COLOR_RESET, current_tests, funcname);
#define PRINT_SIGSEGV printf("%s%s%s [%s] %s:%d (%Lfs)\n", ANSI_COLOR_RED, sigsegv ? "[ SIGSEGV ]" : "[ FAILURE ]", ANSI_COLOR_RESET, current_tests, funcname, line, time);
#define PRINT_OK printf("%s%s%s [%s] %s (%Lfs)\n", ANSI_COLOR_GREEN, "[      OK ]", ANSI_COLOR_RESET, current_tests, funcname, time);
#define PRINT_EXPECTED printf("%s  Expected: %lld\n  Got: %d%s\n", ANSI_COLOR_RED, (int64_t)expected, retcode, ANSI_COLOR_RESET);
#define HANDLE_PHASE1 if (first_phase == 1){ \
        total_functions++; \
        int len = strlen(funcname); \
        if (len > max_funcname_len) max_funcname_len = len; \
        return;}

//General EXPECT and ASSERT declaration
#define EXPECT_EQ(func,expected) _Generic((expected), \
                                        int8_t: expect_func_int8_eq, \
                                        uint8_t: expect_func_uint8_eq, \
                                        int16_t: expect_func_int16_eq, \
                                        uint16_t: expect_func_uint16_eq, \
                                        int32_t: expect_func_int32_eq, \
                                        uint32_t: expect_func_uint32_eq, \
                                        int64_t: expect_func_int64_eq, \
                                        uint64_t: expect_func_uint64_eq)(func, #func, expected, __LINE__);
#define EXPECT_TRUE(func) _Generic((func), \
                                           int8_t: expect_true_func_int8, \
                                           uint8_t: expect_true_func_uint8, \
                                           int16_t: expect_true_func_int16, \
                                            uint16_t: expect_true_func_uint16, \
                                            int32_t: expect_true_func_int32, \
                                            uint32_t: expect_true_func_uint32, \
                                            int64_t: expect_true_func_int64, \
                                            uint64_t: expect_true_func_uint64)(func, #func, __LINE__);
#define ASSERT_EQ(func,expected) _Generic((expected), \
                                        int8_t: assert_func_int8_eq, \
                                        uint8_t: assert_func_uint8_eq, \
                                        int16_t: assert_func_int16_eq, \
                                        uint16_t: assert_func_uint16_eq, \
                                        int32_t: assert_func_int32_eq, \
                                        uint32_t: assert_func_uint32_eq, \
                                        int64_t: assert_func_int64_eq, \
                                        uint64_t: assert_func_uint64_eq)(func, #func, expected, __LINE__);
//Additional functions
#define INIT() char* funcname = ""; int line = 0; int sigsegv = 0; time_t start = {0}; time_t stop = {0}; long double time = 0; 
#define SETRETURN setjmp(sigsegv_buf)
#define START_CRITICAL_FUNCTION_TEST(func) funcname = #func; line = __LINE__; sigsegv = 0; if (SETRETURN){PROCESS_SEGV failed++; FATAL_TEST} \
                                          else { if (first_phase){ total_functions++; return;} else { PRINT_START(funcname) start = clock()
#define END_CRITICAL_FUNCTION_TEST } } stop = clock(); time = ((double)(stop - start)) / CLOCKS_PER_SEC; \
                                    totaltime += time; HANDLE_FUNCTION_TEST ran++;
#define START_FUNCTION_TEST(func) funcname = #func; line = __LINE__; sigsegv = 0; if (SETRETURN){PROCESS_SEGV failed++;} \
                                          else { if (first_phase){ total_functions++; return;} else { PRINT_START(funcname) start = clock()
#define END_FUNCTION_TEST } } stop = clock(); time = ((double)(stop - start)) / CLOCKS_PER_SEC; \
                                    totaltime += time; HANDLE_FUNCTION_TEST ran++;
#define PROCESS_SEGV failed++; sigsegv++;
#define FATAL_TEST assert_failed++;
#define PRINT_START(func) print_start(#func);
#define HANDLE_FUNCTION_TEST if (!sigsegv) {PRINT_OK successed++;} else {PRINT_SIGSEGV return;}
#define SET_TITLE(title) current_tests = title;
#define SET_SECTION(name) printf(ANSI_COLOR_GREEN "\n[%s]%s\n\n", name, ANSI_COLOR_RESET);

//All global fields required by engine
int total_functions = 0;
int currentfunction = 1;
int first_phase = 1;
int max_funcname_len = 0;
int successed = 0;
int failed = 0;
int ran = 0;
int assert_failed = 0;
long double totaltime = 0;
char* current_tests = "";
jmp_buf sigsegv_buf = {0};

//Handler of segmentation fault, required for tests engine stability
void sigsegv_handler(int s){
    switch (s){
        case SIGSEGV:
            longjmp(sigsegv_buf, 1);
            break;
    }
}

void test_main();

//All testing functions

void print_start(char* funcname){
    PRINT_RUN
}

void handle_function_test(char* funcname, int32_t line){

}

void expect_func_int8_eq(int8_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_uint8_eq(uint8_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_int16_eq(int16_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_uint16_eq(uint16_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_int32_eq(int32_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_uint32_eq(uint32_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_int64_eq(int64_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void expect_func_uint64_eq(uint64_t (*func)(), char* funcname, int8_t expected, int line){
    EXPECT_FUNC_INT_BODY
}

void assert_func_int8_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_uint8_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_int16_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_uint16_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_int32_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_uint32_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_int64_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

void assert_func_uint64_eq(int (*func)(), char* funcname, int expected, int line){
    ASSERT_FUNC_INT_BODY
}

//Actual entry point, instead of the fake one
int main(){
    signal(SIGSEGV, sigsegv_handler);
    total_functions = 0;
    test_main();
    if (total_functions == 0){
        printf(ANSI_COLOR_RED "No tests detected!\n" ANSI_COLOR_RESET);
        return 0;
    }
    first_phase = 0;
    printf("%s[=========]%s Running %d tests\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, total_functions);
    test_main();
    if (successed == 0) printf(ANSI_COLOR_RED);
    else if (failed == 0) printf(ANSI_COLOR_GREEN);
    else printf(ANSI_COLOR_YELLOW);
    printf("[=========]%s %d tests finished (%Lfs total)\n\n", ANSI_COLOR_RESET, ran, totaltime);
    if (successed) printf("%s[ SUCCESS ]%s %d tests.\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, successed);
    if (failed) printf("%s[ FAILURE ]%s %d tests.\n", ANSI_COLOR_RED, ANSI_COLOR_RESET, failed);
}
