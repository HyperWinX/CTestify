#include <stdio.h>
#include <stdlib.h>

// Define the Test structure
typedef struct Test {
    const char *name;
    void (*test_function)(void);
    struct Test *next; // Pointer to the next test in the list
} Test;

// Head of the linked list for tests
Test *test_suite_head = NULL;

// Function to register a test case
void register_test(const char *name, void (*test_function)(void)) {
    Test *new_test = (Test*)malloc(sizeof(Test));
    new_test->name = name;
    new_test->test_function = test_function;
    new_test->next = test_suite_head; // Add to the front of the list
    test_suite_head = new_test; // Update the head to point to the new test
}

// Macro for test definition and registration
#define TEST(test_name) \
void test_name(); \
__attribute__((constructor)) void register_##test_name() { \
    register_test(#test_name, test_name); \
} \
void test_name()

// Example test cases
TEST(test_case_1) {
    printf("Running test_case_1\n");
    // Add your test logic here
}

TEST(test_case_2) {
    printf("Running test_case_2\n");
    // Add your test logic here
}

// Function to run all tests
void run_tests() {
    Test *current = test_suite_head;
    while (current != NULL) {
        printf("Executing Test: %s\n", current->name);
        current->test_function(); // Execute the test function
        current = current->next;  // Move to the next test in the list
    }
}

// Main function to initiate the tests
int main() {
    run_tests(); // Run all registered tests
    return 0;
}
