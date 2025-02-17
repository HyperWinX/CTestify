#pragma once

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

// Disable warnings on casting integers
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

extern void __ctestify_register_ctest(struct _test* _test_ptr);
extern ComparisonInfo __ctestify_comparisoninfo_ctor(ComparedType, ComparedObject, ComparedObject, ComparisonResult);
extern void __ctestify_run_all_tests();

extern void __ctestify_verify_result(ComparisonInfo, int, char*, char*, char*, TestType, int, char*);
extern ComparisonResult __ctestify_compare_string(char*, char*);
extern ComparisonResult __ctestify_compare_float(float, float);
extern ComparisonResult __ctestify_compare_double(double, double);

inline void __ctestify_assign_union_str(ComparedObject* obj, char* val) { obj->str = val; }
inline void __ctestify_assign_union_u64(ComparedObject* obj, int64_t val) { obj->s_int = val; }
inline void __ctestify_assign_union_i64(ComparedObject* obj, uint64_t val) { obj->u_int = val; }
inline void __ctestify_assign_union_f32(ComparedObject* obj, float val) { obj->f_val = val; }
inline void __ctestify_assign_union_f64(ComparedObject* obj, double val) { obj->d_val = val; }

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

#define __CT_CONSTRUCT_OBJ1(type, val) ({ \
    ComparedObject obj; \
    _Generic((val), \
      uint8_t: __ctestify_assign_union_u64, \
      uint16_t: __ctestify_assign_union_u64, \
      uint32_t: __ctestify_assign_union_u64, \
      uint64_t: __ctestify_assign_union_u64, \
      int8_t: __ctestify_assign_union_i64, \
      int16_t: __ctestify_assign_union_i64, \
      int32_t: __ctestify_assign_union_i64, \
      int64_t: __ctestify_assign_union_i64, \
      float: __ctestify_assign_union_f32, \
      double: __ctestify_assign_union_f64, \
      char*: __ctestify_assign_union_str, \
      default: __ctestify_nofunction("Unknown type, cannot assign to union!")(&obj, val); \
     obj; })

#define __CT_CONSTRUCT_OBJ(val) ({ \
    ComparedObject obj; \
    _Generic((val), \
      uint8_t: obj.u_int, \
      uint16_t: obj.u_int, \
      uint32_t: obj.u_int, \
      uint64_t: obj.u_int, \
      int8_t: obj.s_int, \
      int16_t: obj.s_int, \
      int32_t: obj.s_int, \
      int64_t: obj.s_int, \
      float: obj.f_val, \
      double: obj.d_val, \
      char*: obj.str) = val; obj; })


#define CTEST(suite, test_name) \
  void _TEST_FUNC(suite, test_name)(void); \
  struct _test _TEST_PROP(suite, test_name) = {.test_ptr = &_TEST_FUNC(suite, test_name), .suite_name = #suite, .name = #test_name, .file = __FILE__, .line = __LINE__}; \
  __attribute__((constructor)) void _TEST_CTOR(suite, test_name)() { \
    __ctestify_register_ctest(&_TEST_PROP(suite, test_name)); \
  } \
  void _TEST_FUNC(suite, test_name)(void)

#define RUN_ALL_TESTS() __ctestify_run_all_tests()

#define __CT_UNICOMPARE(val, expected) (val == expected)
#define __CT_COMPAREINFO_CTOR(val, expected) __ctestify_comparisoninfo_ctor(__CT_DETERMINE_TYPE(val), __CT_CONSTRUCT_OBJ(val), __CT_CONSTRUCT_OBJ(expected), UNDEFINED)

#define __CT_GENERIC_COMPARE(_val, _correct_val) _Generic((_val), \
  uint8_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  uint16_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  uint32_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  uint64_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  int8_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  int16_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  int32_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  int64_t: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  float: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  double: __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  char*:  __CT_COMPAREINFO_CTOR(_val, _correct_val), \
  default: __ctestify_comparisoninfo_ctor(Unknown, __CT_CONSTRUCT_OBJ(#_val), __CT_CONSTRUCT_OBJ(#_correct_val), _val == _correct_val ? EQ : NOT_EQ))

#define __CT_EXPECT_EQ(val, expected) \
    __ctestify_verify_result(__CT_GENERIC_COMPARE(val, expected), false, NULL, #val, #expected, __EQ, __LINE__, __FILE__)

#define __CT_EXPECT_EQM(val, expected, err_msg) \
    __ctestify_verify_result(__CT_GENERIC_COMPARE(val, expected), false, err_msg, #val, #expected, __EQ, __LINE__, __FILE__)

#define EXPECT_EQ(...) __CT_EXPECT_IMPL(__VA_ARGS__, __CT_EXPECT_EQM, __CT_EXPECT_EQ)(__VA_ARGS__)
#define __CT_EXPECT_IMPL(_1, _2, _3, NAME, ...) NAME
