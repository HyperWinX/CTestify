<h1 align="center">CTestify</h1>
<h3 align="center">Small and fast C programs testing framework</h3>
<hr />
<h4 align="center">Usage guide</h4>
<p>You can compare any integer, float, double or char pointer</p>
<pre><code class="language-c">EXPECT_EQ(test_name, value, expected_value) // checks if value equals to expected value
EXPECT_EQM(test_name, value, expected_value, errmsg) // same as EXPECT_EQ, but you can pass custom error message
ASSERT_EQ(test_name, value, expected_value)
ASSERT_EQM(test_name, value, expected_value, errmsg) // same as expect, but they tests will exit if any assert will fail

EXPECT_TRUE(test_name, value) // if value if bigger than one, than its true
EXPECT_TRUEM(test_name, value, errmsg) // same, but with custom error message
ASSERT_TRUE(test_name, value)
ASSERT_TRUEM(test_name, value, errmsg) // all the same, but assert

EXPECT_FALSE(test_name, value) // if value if bigger than one, than its true
EXPECT_FALSEM(test_name, value, errmsg) // same, but with custom error message
ASSERT_FALSE(test_name, value)
ASSERT_FALSEM(test_name, value, errmsg) // all the same, but assert
</code></pre>

<h3>If you want to create your own TestingEnvironmentSetUp() and TestingEnvironmentDestroy(), you can do following:</h3>

<pre><code class="language-c">
  // Find, where your code starts
  // And, before include directive, write this define
  // With this you can write your own implementation. Dont forget that's they have int return type, 0 is no errors, 1 and more is error

  #define TESTINGENVCTL
  #include "ctestify.h"

  int TestingEnvironmentSetUp(){
    // Do something
    return 0;
  }
  int TestingEnvironmentDestroy(){
    // Do something
    return 0;
  }
  void test_main(){
    // Dont't forget about custom entry point! It's test_main, not main
  }
</code></pre>
