# smallunit

A small unit testing framework for C.

v2 (legacy): https://raw.githubusercontent.com/JaMo42/smallunit/b70cd334e36bf320b38db8037773fcfa02bf9aa9/smallunit.h

## Requirements

- `std_ds.h` (https://github.com/nothings/stb/blob/master/stb_ds.h)

- C only, Linux, and GNU extensions

## Usage

See `test.c`.

### Defining the symbols

```c
#define SU_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include "smallunit.h"
```

### Declaring tests

```c
su_test(module_name, test_name) {
}
```

- Defines a test case in a stateless module.

- Modules are just groups of tests, either stateless or fixtures, their naming is adopted from previous incarnations of this library

### Fixtures

```c
typedef struct {
    bool value;
} thing_test;

void thing_test_setup(thing_test *self) {
    self->value = true;
}

void thing_test_tear_down(thing_test *self) {
}

su_test_f(thing_test, test_name) {
    su_expect(self->value);
}
```

- The naming matters here, the first parameter of `su_test_f` must be the type name of the fixture, which must be typedef'd, and the `<typename>_setup` and `<typename>_tear_down` functions must exist.

- Prior to calling `setup` the value is zeroed.

- The identifier for the fixture object inside the test cases can be changed by defining `SU_FIXTURE_IDENTIFIER`, and defaults to `self`.

### Running

```c
int main(void) {
    return su_run_all_tests();
}
```

All modules, fixtures, and tests are added automatically, `su_run_all_tests` runs them and provides a return value for the main function, being `1` if any test failed and `0` otherwise (including if tests were skipped).

Modules (both stateless and fixtures), and the tests inside them are run in declaration order.

### Short names

If `SU_NO_SHORT_NAMES` is not defined, the `su_name` macros will have `NAME` defined as an alias (`su_test_f` => `TEST_F`, `su_expect_eq` => `EXPECT_EQ`, etc.), generally matching macro names from GoogleTest.

## Assertions

Almost all assertions come in two flavors: `su_expect...` and `su_assert...`.
These behave like in GoogleTest, where `expect` will fail the test case but keep running it, and `assert` will immediately return from the current function on failure.
The only exceptions are the death tests which only have `expect`.

### Basic

Fatal Assertion | Nonfatal Assertion | Verifies
---|---|---
su_assert(*condition*) | su_expect(condition) | *condition* is true

### Binary

Fatal Assertion | Nonfatal Assertion | Verifies
---|---|---
su_assert_eq(*a*, *b*) | su_expect_eq(*a*, *b*) | *a* == *b*
su_assert_ne(*a*, *b*) | su_expect_ne(*a*, *b*) | *a* != *b*

### C strings

Fatal Assertion | Nonfatal Assertion | Verifies
---|---|---
su_assert_streq(*a*, *b*) | su_expect_streq(*a*, *b*) | the two C strings have the same content
su_assert_strne(*a*, *b*) | su_expect_strne(*a*, *b*) | the two C strings have different content

Passing `NULL` to these is valid.

### Floats

Fatal Assertion | Nonfatal Assertion | Verifies
---|---|---
su_assert_float_eq(*a*, *b*) | su_expect_float_eq(*a*, *b*) | The two float values are almost equal
su_assert_double_eq(*a*, *b*) | su_expect_double_eq(*a*, *b*) | The two double values are almost equal
su_assert_near(*a*, *b*, *abs_err*) | su_expect_near(*a*, *b*, *abs_err*) | The difference between the two values doesn't exceed *abs_err*.

"almost equal" means the two values are within 4 ULP's from each other.

### Death tests

Nonfatal Assertion | Verifies
---|---
su_expect_exit(*statement*, *predicate*, *output*) | that *statement* causes the process to terminate in away that matches *predicate*, and produces the specified `stderr` output.
su_expect_death(*statement*, *output*) | that *statement* causes the process to terminate with a non-zero exit code and produces the specified `stderr` output.

If *output* is `NULL` the output is not checked.
By default only `4096` bytes of output are captured, this can be increased by defining `SU_STDERR_BUF_SIZE`.

Predicate | Requires
---|---
su_exited_with_code(*code*) | The program exited normally with `code`
su_killed_by_signal(*signal_number*) | The program was killed by the given signal
su_exited_abnormally() | The program exited with a non-zero exit code or was killed by any signal

### Explicit control flow

Function | Description
---|---
su_skip() | Marks the test as skipped and stops execution of the test

## License

2-Clause BSD License (it's at the bottom of the header).
