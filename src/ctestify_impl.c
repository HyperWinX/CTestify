#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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
  test test_ptr;
  char* suite_name;
  char* name;
  char* file;
  char* line;
} _test;

typedef enum ComparisonResult {
  BIGGER,
  LESS,
  EQ,
  NOT_EQ
} ComparisonResult;

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

// Comparison functions
ComparisonResult __ctestify_compare_uint8_t(uint8_t a, uint8_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_uint16_t(uint16_t a, uint16_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_uint32_t(uint32_t a, uint32_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_uint64_t(uint64_t a, uint64_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_int8_t(int8_t a, int8_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_int16_t(int16_t a, int16_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_int32_t(int32_t a, int32_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_int64_t(int64_t a, int64_t b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_string(char* a, char* b) {
  if (strcmp(a, b)) return NOT_EQ;
  else return EQ;
}


void __ctestify_verify_result(int result) {
  long double time = __calc_test_time();
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
