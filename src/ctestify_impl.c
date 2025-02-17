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
  int line;
} _test;

typedef enum ComparisonResult {
  BIGGER,
  LESS,
  EQ,
  NOT_EQ,
  UNDEFINED
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
HIDDEN long double __ctestify_calc_test_time() {
  return ((long double)(ct_tend - ct_tstart)) / CLOCKS_PER_SEC;
}

HIDDEN void __ctestify_obj_to_str(ComparedObject* restrict _Nonnull obj, ComparedType type, char* restrict _Nonnull dst) {
  switch (type) {
    case Int8:
    case Int16:
    case Int32:
    case Int64:
      sprintf(dst, "%ld", (int64_t)obj->s_int);
      break;
    case UInt8:
    case UInt16:
    case UInt32:
    case UInt64:
      sprintf(dst, "%lu", (uint64_t)obj->u_int);
      break;
    case Float:
      sprintf(dst, "%f", obj->f_val);
      break;
    case Double:
      sprintf(dst, "%lf", obj->d_val);
      break;
    case String:
      sprintf(dst, "%s", obj->str);
      break;
    default: assert(0);
  }
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

void __ctestify_verify_result(ComparisonInfo result, bool is_fatal, char* err_msg, char* expr1, char* expr2, TestType type, int line, char* file) {
  char s_obj1[64], s_obj2[64];
  char *str1 = s_obj1, *str2 = s_obj2;
  int _failed = 0;

  if (!running) return;

  if (result.result == UNDEFINED) {
    switch (result.type) {
      case Int8:
      case Int16:
      case Int32:
      case Int64:
        result.result = (result.obj1.s_int == result.obj2.s_int) ? EQ : (result.obj1.s_int > result.obj2.s_int ? BIGGER : LESS);
        break;
      case UInt8:
      case UInt16:
      case UInt32:
      case UInt64:
        result.result = (result.obj1.u_int == result.obj2.u_int) ? EQ : (result.obj1.u_int > result.obj2.u_int ? BIGGER : LESS);
        break;
      case Float:
        result.result = (result.obj1.f_val == result.obj2.f_val) ? EQ : (result.obj1.f_val > result.obj2.f_val ? BIGGER : LESS);
        break;
      case Double:
        result.result = (result.obj1.d_val == result.obj2.d_val) ? EQ : (result.obj1.d_val > result.obj2.d_val ? BIGGER : LESS);
        break;
      case String:
        result.result = !strcmp(result.obj1.str, result.obj2.str) ? EQ : NOT_EQ;
        break;
      default: __builtin_unreachable();
    }
  }

  switch (type) {
    case __EQ:
      if (result.result != EQ) {
        ++_failed; break;
      }
      break;
    case __NEQ:
      if (result.result != NOT_EQ) {
        ++_failed; break;
      }
      break;
    default: assert(0);
  }

  if (!_failed) return;

  ct_tend = clock();
  long double time = __ctestify_calc_test_time();
  fprintf(ct_stdout, "%s%s%s %s.%s (%.3Lf%s)\n", CRED, "[ FAILURE ]", CRESET, current->suite_name, current->name, time < 1000 ? time * 1000 : time, time < 1000 ? "ms" : "s");

  if (result.type == String) {
    str1 = result.obj1.str;
    str2 = result.obj2.str;
  } else {
    __ctestify_obj_to_str(&result.obj1, result.type, str1);
    __ctestify_obj_to_str(&result.obj2, result.type, str2);
  }
  fprintf(ct_stdout, "Failed test at %s:%d\n", file, line);
  switch (type) {
    case __EQ:
      if (err_msg) {
        fprintf(ct_stdout, err_msg, expr1, str1, expr2, str2);
      } else {
        fprintf(ct_stdout, "Expected equality of following values:\n\tExpr: %s, value: %s\n\tExpr: %s, value: %s\n", expr1, str1, expr2, str2);
      }
      break;
    case __NEQ:
      if (err_msg) {
        fprintf(ct_stdout, err_msg, expr1, str1, expr2, str2);
      } else {
        fprintf("Expected not equality of following values: \n\tExpr: %s, value: %s\n\tExpr: %s, value: %s\n", expr1, str1, expr2, str2);
      }
    default: assert(0);
  }
  
  running = !is_fatal;
  ++failed;
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

ComparisonInfo __ctestify_comparisoninfo_ctor(ComparedType type, ComparedObject obj1, ComparedObject obj2, ComparisonResult res) {
  ComparisonInfo info = {
    res,
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
    long double time = __ctestify_calc_test_time();
    if (!failed) {
      fprintf(ct_stdout, "%s%s%s %s.%s (%.3Lf%s)\n", CGREEN, "[      OK ]", CRESET, current->suite_name, current->name, time < 1000 ? time * 1000 : time, time < 1000 ? "ms" : "s");
    }
    current = current->next;
  }
}
