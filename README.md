<h1 align="center">
  <br>
  <a href="https://github.com/HyperWinX/CTestify/archive/master.zip">CTestify</a>
</h1>

<h4 align="center">Fast and portable C testing framework.</h4>

<p align="center">
    <a href="https://github.com/HyperWinX/CTestify/issues">
    <img src="https://img.shields.io/github/issues/HyperWinX/CTestify?color=lime"
         alt="GitHub opened issues">
    <img src="https://img.shields.io/badge/status-stable-lime"
         alt="Status">
    <img src="https://img.shields.io/github/license/HyperWinX/CTestify?color=lime"
         alt="License">
    <img src="https://img.shields.io/github/stars/HyperWinX/CTestify?color=lime"
         alt="Stars">
</p>

---
<table>
<tr>
<td>

**CTestify** is a testing framework for C programs, designed to be as fast and easy to use as possible.

**CTestify** runs in two iterations: the first counts the tests, while the second actually executes them. It measures the test execution time and displays it upon completion. Additionally, it includes built-in protection against unexpected segmentation faults. If a test encounters a segfault, it gets a special mark **[ SIGSEGV ]** instead of **[ FAILURE ]**.

If you need a fast and simple testing framework, choose **CTestify**!

</td>
</tr>
</table>

# Table of contents
- [Installation](#installation)
- [Usage](#usage)
    - [Constructor and destructor](#custom-env-constructor-and-destructor)
    - [Supported data types](#supported-data-types)
    - [M type of tests](#m-type-of-tests)
    - [ASSERT tests](#assert-tests)
    - [EXPECT_EQ](#expect_eq)
    - [EXPECT_NEQ](#expect_neq)
    - [EXPECT_TRUE](#expect_true)
    - [EXPECT_FALSE](#expect_false)
    - [EXPECT_BIGGER](#expect_bigger)
    - [EXPECT_LESS](#expect_less)
    - [EXPECT_BIGGEROREQ](#expect_biggeroreq)
    - [EXPECT_LESSOREQ](#expect_lessoreq)
    - [EXPECT_VALIDPTR](#expect_validptr)
    - [PRINT](#print)

## Installation
1. Download main header [ctestify.h](ctestify.h)
2. Include it to your source file.
3. Create entry point like this:
```c
void test_main(){
    // Write tests here
}
```
4. To set test suite name use
```c
SET_TEST_SUITE_NAME(TESTS);
```
without it tests wont work!
5. Now you can write some tests!

## Usage

### Custom env constructor and destructor
Maybe, you need to do something before or after tests. If so, you need to define TESTINGENVCTL before including framework.
```c
#define TESTINGENVCTL
#include "ctestify.h"
```
Now you can write them. You should create two functions.
```c
int TestingEnvironmentSetUp(){}
int TestingEnvironmentDestroy(){}
```
Functions return zero if successed, and nonzero if failed.

### Supported data types
Currently tests support int (from int8_t to uint64_t), float, double, char* and void*. Char* means string, so it uses strcmp();

### M type of tests
At the end of any test you can add M, and then pass string as additional argument. It will be your custom error message. For example:
```c
EXPECT_EQM(EQTEST1, 5, 5, "Custom error message!");
```

### ASSERT tests
Every EXPECT test has it's own ASSERT variant.

### EXPECT_EQ
The most basic test, expects equality of two values.  
Signature: EXPECT_EQ(test_name, value, expected)
```c
EXPECT_EQ(EQTEST1, 5, 7); // Will fail
EXPECT_EQ(EQTEST2, 5, 5); // Good test
```

### EXPECT_NEQ
Expects value, not equal to second.  
Signature: EXPECT_NEQ(test_name, value, expected)
```c
EXPECT_NEQ(NEQTEST1, 5, 5); // Will fail
EXPECT_NEQ(NEQTEST2, 5, 7); // Good test
```

### EXPECT_TRUE
Expects value bigger than zero.  
Signature: EXPECT_TRUE(test_name, value);
```c
EXPECT_TRUE(TRUETEST1, 0); // Will fail
EXPECT_TRUE(TRUETEST2, 65536); // Good test
```

### EXPECT_FALSE
Expects zero value.  
Signature: EXPECT_FALSE(test_name, value);
```c
EXPECT_FALSE(FALSETEST1, 65536); // Will fail
EXPECT_FALSE(FALSETEST2, 0); // Good test
```

### EXPECT_BIGGER
Expects value bigger than passed to "expected" argument.  
Signature: EXPECT_BIGGER(test_name, value, expected);
```c
EXPECT_BIGGER(EXBIG1, 4, 7); // Will fail
EXPECT_BIGGER(EXBIG2, 10, 7); // Good test
```

### EXPECT_LESS
Expects value less than passed to "expected" argument.  
Signature: EXPECT_LESS(test_name, value, expected);
```c
EXPECT_LESS(EXBIG1, 10, 7); // Will fail
EXPECT_LESS(EXBIG2, 4, 7); // Good test
```

### EXPECT_BIGGEROREQ
Expects value bigger or equal than passed to "expected" argument.  
Signature: EXPECT_BIGGER(test_name, value, expected);
```c
EXPECT_BIGGEROREQ(EXBIGOREQ1, 4, 7); // Will fail
EXPECT_BIGGER(EXBIGOREQ2, 10, 7); // Good test
```

### EXPECT_LESSOREQ
Expects value less or equal than passed to "expected" argument.  
Signature: EXPECT_LESS(test_name, value, expected);
```c
EXPECT_LESSOREQ(EXLESSOREQ1, 10, 7); // Will fail
EXPECT_LESS(EXLESSOREQ2, 4, 7); // Good test
```

### EXPECT_VALIDPTR
Expects nonnull pointer.  
Signature: EXPECT_VALIDPTR(test_name, ptr);
```c
EXPECT_VALIDPTR(VALIDPTR1, (void*)0); // Will fail
EXPECT_VALIDPTR(VALIDPTR2, (void*)0xFF0154297AC8); // Good test
```

### PRINT
Prints user messages during test run. Formatting supported! Also flushes buffer, so can be used in loops.
```c
PRINT("INTERNAL MESSAGE %s %d", "TEST", 2);
```

## List of ASSERT variants
- ASSERT_EQ
- ASSERT_TRUE
- ASSERT_FALSE
- ASSERT_BIGGER
- ASSERT_LESS
- ASSERT_BIGGEROREQ
- ASSERT_LESSOREQ
- ASSERT_VALIDPTR
