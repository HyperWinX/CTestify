#include <asm-generic/fcntl.h>
#include <stdlib.h>
#include <unistd.h>

//Colors ANSI escape sequences
#define CRED     "\x1b[31m"
#define CGREEN   "\x1b[32m"
#define CYELLOW  "\x1b[33m"
#define CRESET   "\x1b[0m"
#define PROMPT   "==> "
#define HIDDEN [[gnu::visibility("hidden")]]

typedef void(*test)(void);
typedef struct _test {
  struct _test* next;
                                                    test test_ptr;                                    char* suite_name;                                 char* name;
  char* file;
  char* line;
} _test;

//Internal data
HIDDEN _test* head = NULL;
HIDDEN _test* last = NULL;
HIDDEN _test* current = NULL;
HIDDEN FILE* ct_stdout = NULL;
HIDDEN long double ct_tstart;
HIDDEN long double ct_tend;

// Internal functions
HIDDEN long double __calc_test_time() {
  return ((long double)(ct_tend - ct_tstart)) / CLOCKS_PER_SEC;
}

void __ctestify_verify_result(int result) {
  if (result) {
    fprintf(ct_stdout, "%s%s%s %s.%s (%.3Lf%s)\n", CGREEN, "[      OK ]", CRESET, current->suite_name, current->name, time < 1000 ? time * 1000 : time, time < 1000 ? "ms" : "s"); \
  } else {

  }
}

void __ctestify_register_ctest(_test* _test_ptr) {
  if (!head) 
    head = last = _test_ptr;
  else {
    last->next = _test_ptr;
    last = last->next;
  }
}

void __ctestify_run_all_tests() {
  // Setup stdout
  ct_stdout = fopen("/dev/tty", "a");
  // Disable general stdout
  dup2(open("/dev/null", O_WRONLY), fileno(stdout));

  // Traverse linked list, executing tests
  current = head;
  while (current) {
    fprintf(ct_stdout, "%s%s%s %s.%s\n", CGREEN, "[ RUN     ]", CRESET, current->suite_name, current->name); \
    ct_tstart = clock();
    current->test_ptr();
    ct_tend = clock();
    current = current->next;
  }
}
