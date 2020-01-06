# smallunit

A small unit testing framework for C/C++, kinda inspired by rusts tests.

## Usage

### Example

```cpp
#include "smallunit.h"

// Some interesting code

int check_something() {
  ...
}

int do_something(int a, int b) {
  ...
}

// Defines a testing module
su_module(my_test_module, {
  // Defines a test case
  su_test(test_check_something, {
    // Asserts an expression to be true
    su_assert(check_something());
  })
  su_test(test_something, {
    // Asserts two expression to be equal
    su_assert_eq(do_something(1, 2), 42);
  })
})

int main() {
  // Runs a module
  su_run_module(my_test_module);
}
```

### Stopping on failure

```cpp
#include "test.h"

su_module(example, {
  su_test(fails, {
    su_assert(0);
  })
  // This won't be executed
  su_test(wont_run, {
    ...
  })
})

int main() {
  // Set to 1 to stop a module on the first failed assertion
  su_stop_on_failure = 1;
  su_run_module(...);
}
```
