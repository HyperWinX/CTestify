#include <stdlib.h>

typedef void(*test)(void);
typedef struct _test {
  struct _test* next; // We are using linked list

  test test_ptr; // Test function itself
  char* name; // Test name
} _test;

[[gnu::visibility("hidden")]]
_test* head = NULL;
[[gnu::visibility("hidden")]]
_test* current = NULL;

void __ctestify_register_ctest(_test* _test_ptr) {
  if (!head) 
    head = current = _test_ptr;
  else {
    current->next = _test_ptr;
    current = current->next;
  }
}

void __ctestify_run_all_tests() {
  _test* current_test = head;
  while (current_test) {
    current_test->test_ptr();
    current_test = current_test->next;
  }
}