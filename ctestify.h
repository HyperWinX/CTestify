#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define SECONDS 1
#define MILLISECONDS 2
#define MICROSECONDS 3

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
    if (!executed && sigsegv) printf("%s%s%s %s:%d (%Lfs)\n", ANSI_COLOR_RED, sigsegv ? "[ SIGSEGV ]" : "[ FAILURE ]", ANSI_COLOR_RESET, funcname, line, time); \
    else if (retcode == expected){ \
        printf("%s%s%s %s (%Lfs)\n", ANSI_COLOR_GREEN, "[      OK ]", ANSI_COLOR_RESET, funcname, time); successed++; \
    } else { \
        printf("%s  Expected: %d\n  Got: %d%s\n", ANSI_COLOR_RED, expected, retcode, ANSI_COLOR_RESET); failed++;} ran++;

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
    if (!executed && sigsegv){ printf("%s%s%s %s:%d (%Lfs)\n", ANSI_COLOR_RED, sigsegv ? "[ SIGSEGV ]" : "[ FAILURE ]", ANSI_COLOR_RESET, funcname, line, time); failed++; assert_failed++;} \
    else if (retcode == expected){ \
        printf("%s%s%s %s (%Lfs)\n", ANSI_COLOR_GREEN, "[      OK ]", ANSI_COLOR_RESET, funcname, time); successed++; \
    } else { \
        printf("%s  Expected: %d\n  Got: %d%s\n", ANSI_COLOR_RED, expected, retcode, ANSI_COLOR_RESET); failed++; assert_failed++;} ran++;

#define SPACECOUNT max_funcname_len - strlen(funcname)
#define OFFSET SPACECOUNT,""
#define CHECK_ASSERT_FAILURE if (assert_failed) return;
#define PRINT_RUN printf("%s%s%s %s\n", ANSI_COLOR_GREEN, "[ RUN     ]", ANSI_COLOR_RESET, funcname);
#define HANDLE_PHASE1 if (first_phase == 1){ \
        total_functions++; \
        int len = strlen(funcname); \
        if (len > max_funcname_len) max_funcname_len = len; \
        return;}

#define EXPECT_FUNC_INT_EQ(func,expected) _Generic((expected), \
                                        int8_t: expect_func_int8_eq, \
                                        uint8_t: expect_func_uint8_eq, \
                                        int16_t: expect_func_int16_eq, \
                                        uint16_t: expect_func_uint16_eq, \
                                        int32_t: expect_func_int32_eq, \
                                        uint32_t: expect_func_uint32_eq, \
                                        int64_t: expect_func_int64_eq, \
                                        uint64_t: expect_func_uint64_eq)(func, #func, expected, __LINE__);
#define ASSERT_FUNC_INT_EQ(func,expected) _Generic((expected), \
                                        int8_t: assert_func_int8_eq, \
                                        uint8_t: assert_func_uint8_eq, \
                                        int16_t: assert_func_int16_eq, \
                                        uint16_t: assert_func_uint16_eq, \
                                        int32_t: assert_func_int32_eq, \
                                        uint32_t: assert_func_uint32_eq, \
                                        int64_t: assert_func_int64_eq, \
                                        uint64_t: assert_func_uint64_eq)(func, #func, expected, __LINE__);
int total_functions = 0;
int currentfunction = 1;
int first_phase = 1;
int max_funcname_len = 0;
int successed = 0;
int failed = 0;
int ran = 0;
int assert_failed = 0;
long double totaltime = 0;
jmp_buf sigsegv_buf = {0};

void sigsegv_handler(int s){
    switch (s){
        case SIGSEGV:
            longjmp(sigsegv_buf, 1);
            break;
    }
}

void test_main();

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

void expect_int8_eq(int8_t result, int8_t expected, char* funcname, int line){

}

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
    printf("%s[ SUCCESS ]%s %d tests.\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET, successed);
    printf("%s[ FAILURE ]%s %d tests.\n", ANSI_COLOR_RED, ANSI_COLOR_RESET, failed);
}