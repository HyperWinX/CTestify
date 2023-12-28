#include <stdio.h>
#include <string.h>
#include <time.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define SPACECOUNT max_funcname_len - strlen(funcname)
#define OFFSET SPACECOUNT,""

#define EXPECT_FUNC_INT_EQ(func,expected) expect_func_int_eq(func,#func,expected);
#define ASSERT_INT_EQ(func,expected) assert_int_eq(func,#func,expected);

int total_functions = 0;
int currentfunction = 1;
int first_phase = 1;
int max_funcname_len = 0;
int successed = 0;
int failed = 0;

void test_main();

void converttime(double value, char* buffer) {
    sprintf(buffer, "%f", value);
}

void expect_func_int_eq(int (*func)(), char* funcname, int expected){
    if (first_phase == 1){
        total_functions++;
        int len = strlen(funcname);
        if (len > max_funcname_len) max_funcname_len = len;
        return;
    }
    printf("%s%*s%s%s%s\n", funcname, max_funcname_len - strlen(funcname), "", ANSI_COLOR_GREEN, " [ RUN     ]", ANSI_COLOR_RESET);
    clock_t start, stop;
    start = clock();
    int retcode = (*func)();
    stop = clock();
    char buf[20] = {0};
    converttime(((double)(stop - start)) / CLOCKS_PER_SEC, buf);
    if (retcode == expected){
        printf("%s%*s%s%s%s %ss\n",
                funcname, 
                OFFSET,
                ANSI_COLOR_GREEN, 
                " [      OK ]", 
                ANSI_COLOR_RESET, 
                buf);
        successed++;
    }
    else{
        printf("%s%*s%s%s%s %ss\n", 
                funcname,
                OFFSET, 
                ANSI_COLOR_RED, 
                " [ FAILURE ]", 
                ANSI_COLOR_RESET, 
                buf);
        failed++;
    }
}

void assert_int_eq(int (*func)(), char* funcname, int expected){
    puts("Exited");
}

int main(){
    total_functions = 0;
    test_main();
    if (total_functions == 0) return -1;
    first_phase = 0;
    printf("Running %d tests\n", total_functions);
    test_main();
    if (failed == 0) printf(ANSI_COLOR_GREEN);
    else if (failed != 0 && successed != 0) printf(ANSI_COLOR_YELLOW);
    else if (successed == 0) printf(ANSI_COLOR_RED);
    printf("%d of %d tests ran, %d successed, %d failed%s\n", total_functions, total_functions, successed, failed, ANSI_COLOR_RESET);
}