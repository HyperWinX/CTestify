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

extern void __ctestify_register_ctest(struct _test* _test_ptr);
extern void __ctestify_run_all_tests();

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
