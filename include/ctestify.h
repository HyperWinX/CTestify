#pragma once

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"

// Defining internal types
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

extern void __ctestify_register_ctest(struct _test* _test_ptr);
extern ComparisonInfo __ctestify_comparisoninfo_ctor(ComparisonResult, ComparedType, ComparedObject, ComparedObject);
extern void __ctestify_run_all_tests();

extern void __ctestify_verify_result(ComparisonInfo, int, char*, char*, char*, TestType);
extern ComparisonResult __ctestify_compare_string(char*, char*);
extern ComparisonResult __ctestify_compare_float(float, float);
extern ComparisonResult __ctestify_compare_double(double, double);

#define _TEST_FUNC(suite, test_name) __ctest_##suite_##test_name
#define _TEST_PROP(suite, test_name) __test_repr_##suite_##test_name
#define _TEST_CTOR(suite, test_name) __register_##suite_##test_name

#define __CT_DETERMINE_TYPE(val) (_Generic((val), \
  uint8_t: UInt8, \
  uint16_t: UInt16, \
  uint32_t: UInt32, \
  uint64_t: UInt64, \
  int8_t: Int8, \
  int16_t: Int16, \
  int32_t: Int32, \
  int64_t: Int64, \
  float: Float, \
  double: Double, \
  char*: String, \
  default: Unknown))
#define __CT_CONSTRUCT_OBJ(type, val) ({ \
    ComparedObject obj; \
    switch(type) { \
      case UInt8: \
      case UInt16: \
      case UInt32: \
      case UInt64: \
      obj.u_int = val; break; \
      case Int8: \
      case Int16: \
      case Int32: \
      case Int64: \
      obj.s_int = val; break; \
      case String: \
      obj.str = (char*)val; break; \
      case Float: \
      obj.f_val = (float)val; break; \
      case Double: \
      obj.d_val = (double)val; break; \
      default: \
      assert(0 && "Unknown compared object type, provide custom comparer"); \
    }; obj; })

#define CTEST(suite, test_name) \
  void _TEST_FUNC(suite, test_name)(void); \
  struct _test _TEST_PROP(suite, test_name) = {.test_ptr = &_TEST_FUNC(suite, test_name), .suite_name = #suite, .name = #test_name, .file = __FILE__, .line = __LINE__}; \
  __attribute__((constructor)) void _TEST_CTOR(suite, test_name)() { \
    __ctestify_register_ctest(&_TEST_PROP(suite, test_name)); \
  } \
  void _TEST_FUNC(suite, test_name)(void)

#define RUN_ALL_TESTS() __ctestify_run_all_tests()

#define __CT_UNICOMPARE(val, expected) (val == expected)
#define __CT_INTCOMPARE(val, expected) __ctestify_comparisoninfo_ctor((val == expected ? EQ : (val > expected ? BIGGER : LESS)), __CT_DETERMINE_TYPE(val), __CT_CONSTRUCT_OBJ(__CT_DETERMINE_TYPE(val), val), __CT_CONSTRUCT_OBJ(__CT_DETERMINE_TYPE(expected), expected))
#define __CT_GENERIC_COMPARE(_val, _correct_val) _Generic((_val), \
  uint8_t: __CT_INTCOMPARE(_val, _correct_val), \
  uint16_t: __CT_INTCOMPARE(_val, _correct_val), \
  uint32_t: __CT_INTCOMPARE(_val, _correct_val), \
  uint64_t: __CT_INTCOMPARE(_val, _correct_val), \
  int8_t: __CT_INTCOMPARE(_val, _correct_val), \
  int16_t: __CT_INTCOMPARE(_val, _correct_val), \
  int32_t: __CT_INTCOMPARE(_val, _correct_val), \
  int64_t: __CT_INTCOMPARE(_val, _correct_val), \
  float: __ctestify_compare_float((float)_val, (float)_correct_val), \
  double: __ctestify_compare_double((double)_val, (double)_correct_val), \
  char*: __ctestify_compare_string((char*)_val, (char*)_correct_val), \
  default: __CT_UNICOMPARE(_val, _correct_val))

#define EXPECT_EQ(...) __CT_EXPECT_IMPL(__VA_ARGS__, __CT_EXPECT_EQM, __CT_EXPECT_EQ)(__VA_ARGS__)
#define __CT_EXPECT_IMPL(_1, _2, _3, NAME, ...) __CT_EXPECT_EQ
#define __CT_EXPECT_EQ(val, expected) __ctestify_verify_result(__CT_GENERIC_COMPARE(val, expected), false, NULL, #val, #expected, __EQ)
#define __CT_EXPECT_EQM(val, expected, err_msg) __ctestify_verify_result(__CT_GENERIC_COMPARE(val, expected), false, err_msg, #val, #expected, __EQ)

