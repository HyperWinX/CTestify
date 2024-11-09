#include <stdint.h>

// Defining internal types
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

extern void __ctestify_register_ctest(struct _test* _test_ptr);
extern void __ctestify_run_all_tests();

extern ComparisonResult __ctestify_compare_uint8_t(uint8_t, uint8_t);
extern ComparisonResult __ctestify_compare_uint16_t(uint16_t, uint16_t);
extern ComparisonResult __ctestify_compare_uint32_t(uint32_t, uint32_t);
extern ComparisonResult __ctestify_compare_uint64_t(uint64_t, uint64_t);
extern ComparisonResult __ctestify_compare_int8_t(int8_t, int8_t);
extern ComparisonResult __ctestify_compare_int16_t(int16_t, int16_t);
extern ComparisonResult __ctestify_compare_int32_t(int32_t, int32_t);
extern ComparisonResult __ctestify_compare_int64_t(int64_t, int64_t);
extern ComparisonResult __ctestify_compare_string(char*, char*);

#define _TEST_FUNC(suite, test_name) __ctest_##suite_##test_name
#define _TEST_PROP(suite, test_name) __test_repr_##suite_##test_name
#define _TEST_CTOR(suite, test_name) __register_##suite_##test_name

#define CTEST(suite, test_name) \
  void _TEST_FUNC(suite, test_name)(void); \
  struct _test _TEST_PROP(suite, test_name) = {.test_ptr = &_TEST_FUNC(suite, test_name), .suite_name = #suite, .name = #test_name, .file = __FILE__}; \
  __attribute__((constructor)) void _TEST_CTOR(suite, test_name)() { \
    __ctestify_register_ctest(&_TEST_PROP(suite, test_name)); \
  } \
  void _TEST_FUNC(suite, test_name)(void)

#define RUN_ALL_TESTS() __ctestify_run_all_tests()

#define __CTESTIFY_COMPARE(_val, _correct_val) _Generic((_val), \
  uint8_t: )

#define EXPECT_EQ(val, correct)
#define EXPECT_EQ(val, correct, err_msg)