/*
Copyright (c) 2020 Jakob Mohrbacher

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef SMALLUNIT_H
#define SMALLUNIT_H

/* Whether a modules tests should stop if an assertion failed. */
extern int su_stop_on_failure;

#define su_concat(x, y) x##y
#define su_stringify(s) #s

/* Defines a testing module */
#define su_module(module_name, body)\
  void su_concat(su_module_, module_name)() {                 \
    int _return = 1;                                          \
    int _tests_run = 0;                                       \
    int _errors = 0;                                          \
    puts("Running module: " su_stringify(module_name));       \
    body                                                      \
    if (_tests_run == 1) {                                    \
      fputs("Finished: Ran 1 test, ", stdout);                \
    } else {                                                  \
      printf("Finished: Ran %d tests, ", _tests_run);         \
    }                                                         \
    if (!_errors) {                                           \
      puts("all tests passed");                               \
    } else if (_errors == _tests_run) {                       \
      puts("all tests failed");                               \
    } else {                                                  \
      printf("%d %s\n",                                       \
             _errors,                                         \
             _errors == 1 ? "test failed " : "tests failed"); \
    }                                                         \
  }                                                           \

/* Defines a test case */
#define su_test(name, body)                     \
  if (_return || !su_stop_on_failure) {         \
    puts(" Running test: " su_stringify(name)); \
    ++_tests_run;                               \
    body;                                       \
    if (_return) {                              \
      puts("  Success");                        \
    } else {                                    \
      puts("  Failure");                        \
      ++_errors;                                \
    }                                           \
  }

/* Calls a testing module */
#define su_run_module(name)\
  su_concat(su_module_, name)();

/* Assertions */

#define assert(expr)\
  {                                                    \
    _return = !!expr;                                  \
    if (!_return) {                                    \
      puts("  Assertion failed: " su_stringify(expr)); \
    }                                                  \
  };

#define assert_eq(a, b)\
  {                                                                        \
    _return = (a) == (b);                                                  \
    if (!_return) {                                                        \
      puts("  Assertion failed: " su_stringify(a) " == " su_stringify(b)); \
    }                                                                      \
  }

#endif /* !SMALLUNIT_H */
