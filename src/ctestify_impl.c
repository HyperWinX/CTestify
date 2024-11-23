#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

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
typedef enum TestType {
  __EQ,
  __NEQ,
  __LESS,
  __GR,
  __LESSE,
  __GRE
} TestType;
typedef enum ComparedType {
  Int8,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  Float,
  Double,
  String,
  Unknown
} ComparedType;
typedef union ComparedObject {
  char* str;
  int64_t s_int;
  uint64_t u_int;
  float f_val;
  double d_val;
} ComparedObject;
typedef struct ComparisonInfo {
  ComparisonResult result;
  ComparedType type;
  ComparedObject obj1, obj2;
} ComparisonInfo;

//Internal data
HIDDEN _test* head = NULL;
HIDDEN _test* last = NULL;
HIDDEN _test* current = NULL;
HIDDEN FILE* ct_stdout = NULL;
HIDDEN long double ct_tstart;
HIDDEN long double ct_tend;
HIDDEN int running = 1;

HIDDEN int total_tests = 0;
HIDDEN int successful = 0;
HIDDEN int failed = 0;

// Internal functions
long double __ctestify_calc_test_time() {
  return ((long double)(ct_tend - ct_tstart)) / CLOCKS_PER_SEC;
}

// Comparison functions
ComparisonResult __ctestify_compare_string(char* a, char* b) {
  return (strcmp(a, b) ? NOT_EQ : EQ);
}
ComparisonResult __ctestify_compare_float(float a, float b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}
ComparisonResult __ctestify_compare_double(double a, double b) {
  if (a < b) return LESS;
  else if (a > b) return BIGGER;
  else return EQ;
}

void __ctestify_verify_result(ComparisonInfo result, bool is_fatal, char* err_msg, char* expr1, char* expr2, TestType type) {
  int _failed = 0;

  switch (type) {
    case __EQ:
      if (result.result != EQ) {
        ++_failed; break;
      }
    case __NEQ:
      if (result.result != NOT_EQ) {
        ++_failed; break;
      }
    default: assert(1);
  }

  if (!_failed) return;

  ct_tend = clock();
  long double time = __ctestify_calc_test_time();
  fprintf(ct_stdout, "%s%s%s %s.%s (%.3Lf%s)\n", CRED, "[ FAILURE ]", CRESET, current->suite_name, current->name, time < 1000 ? time * 1000 : time, time < 1000 ? "ms" : "s");
}

void __ctestify_register_ctest(_test* _test_ptr) {
  if (!head) 
    head = last = _test_ptr;
  else {
    last->next = _test_ptr;
    last = last->next;
  }
  ++total_tests;
}

ComparisonInfo __ctestify_comparisoninfo_ctor(ComparisonResult result, ComparedType type, ComparedObject obj1, ComparedObject obj2) {
  ComparisonInfo info = {
    result,
    type,
    obj1,
    obj2
  };
  return info;
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
