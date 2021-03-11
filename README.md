# smallunit

A small unit testing framework for C/C++.

## Usage

### Defining a module

With custom name:
```c
su_module_d (module_name, "Module Name", { ... })
```

`module_name` must be valid C identifier but may also start with a number.

With default name:
```c
su_module (module_name, { ... })
```
This uses the module name as display name.

### Declaring an external module

```
su_extern (module_name);
```

### Module options

```c
su_module (my_module, {
  su_stop_on_failure = false;
  su_compact = false;
})
```

- `su_stop_on_failure` stop module execution once a test fails (defaults to `false`)

- `su_compact` squish subsequent passing tests into one line (see in examples) (defaults to `false`)

The default values can be changed by defining `SU_COMPACT_DEFAULT` or `SU_STOP_ON_FAILURE_DEFAULT` as respectively.

### Defining a test case

```c
su_module (my_module, {
  su_test ("My test case", {
    ...
  })
})
```

### Running a module

```c
SUResult result = su_run_module (module_name);
```

The `SUResult` structure is declared as

```c
typedef struct
{
  unsigned failed;
  unsigned passed;
  unsigned skipped;
  double milliseconds;
} SUResult;
```

- `failed`, `passed`, `skipped` number of tests failed/passed/skipped

- `milliseconds` time taken in milliseconds

### Flow control

```c
su_pass ();
su_fail ();
su_skip ();
```

Use inside a test case to abort and pass/fail/skip it.

### Assertions

```c
su_assert (expr);
```

Asserts that `expr` is `true`.

```c
su_assert_eq (a, b);
```

Asserts that `a == b`.

```c
su_assert_arrays_eq (a, b, count);
```

For each index `i` from `0` to `count`, asserts that `a[i] == b[i]`.

## Examples

### Assertions

```c
su_module (my_module, {
  su_test ("Assert expression", {
    su_assert (1);
    su_assert (0);
  })

  su_test ("Assert equality", {
    su_assert_eq (1, 1);
    su_assert_eq (1, 2);
  })

  su_test ("Assert equality of arrays", {
    const char *s1 = "Hello World";
    const char *s2 = "Hello Sailor";
    su_assert_arrays_eq (s1, s2, 11);
  })
})
```

Output:

```
  my_module
a.c(18): Assertion failed:
  '0'
    :( Assert expression
a.c(23): Assertion failed:
  '1 == 2'
    :( Assert equality
a.c(30): Assertion failed:
  Buffers 's1' and 's2' differ at index 6
    :( Assert equality of arrays

  3 failing (0ms)
```

### Stopping on failure

```c
su_module (example, {
  su_stop_on_failure = true;
  su_test ("Passing Test", { su_pass (); })
  su_test ("Failing Test", { su_fail (); })
  su_test ("Passing Test", { su_pass (); })
})
```

Output:

```
  example
    :) Passing Test
    :( Failing Test

  1 passing 1 failing 1 skipped (0ms)
```

### Compact output

```c
su_module (a_lot_of_passing_tests, {
  su_compact = true;
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Failing Test", { su_fail (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Failing Test", { su_fail (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
  su_test ("Passing Test", { su_pass (); })
})
```

Output:

```
  a_lot_of_passing_tests
    :) Passing Test (3)
    :( Failing Test
    :) Passing Test (7)
    :( Failing Test
    :) Passing Test (4)

  14 passing 2 failing (0ms)
```

### Limitations

- Requires GCC or clang

- Due to some problem with macros, designated initializers need some extra parentheses:

```c
struct Pair { int x; int y; };

su_module (blabla, {
  struct Pair my_pair = ((struct Pair) { .x = 1, .y = 2 });
})
```

