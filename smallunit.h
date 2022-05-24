#ifndef SMALLUNIT_H
#define SMALLUNIT_H
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#ifdef __clang__
#  define typeof(x) __typeof__(x)
#endif

#ifndef SU_COMAPCT_DEFAULT
#define SU_COMPACT_DEFAULT false
#endif
#ifndef SU_STOP_ON_FAILURE_DEFAULT
#define SU_STOP_ON_FAILURE_DEFAULT false
#endif

#define su__str_impl(x) #x
#define su__str(x) su__str_impl (x)
#define su__cat(x, y) x##y

enum
{
  SU_FAIL,
  SU_PASS,
  SU_SKIP
};

typedef struct
{
  unsigned failed;
  unsigned passed;
  unsigned skipped;
  double milliseconds;
} SUResult;

/* Define a module */
#define su_module_d(name, display_name, body)                     \
  SUResult su__cat (su__module_, name) ()                         \
  {                                                               \
    bool su_stop_on_failure = SU_STOP_ON_FAILURE_DEFAULT;         \
    bool su_compact = SU_COMPACT_DEFAULT;                         \
    SUResult su__result = (SUResult) {                            \
      .passed = 0, .failed = 0, .skipped = 0, .milliseconds = 0.0 \
    };                                                            \
    clock_t su__start;                                            \
    double su__test_time;                                         \
    void *su__skip;                                               \
    int su__status = SU_PASS;                                     \
    unsigned su__passed = 0;                                      \
    puts ("  " display_name);                                     \
    body;                                                         \
    putchar ('\n');                                               \
    if (su_compact && su__passed)                                 \
      putchar ('\n');                                             \
    su__print_result (&su__result);                               \
    return su__result;                                            \
  }

/* Define a module with the name as display name */
#define su_module(name, body) su_module_d (name, su__str (name), body)

#define su__test(name, body, id)                                       \
  if (su__status == SU_FAIL && su_stop_on_failure)                     \
    {                                                                  \
      ++su__result.skipped;                                            \
    }                                                                  \
  else                                                                 \
    {                                                                  \
      su__skip = && su__cat (su__test_case_skip_, id);                 \
      su__status = SU_PASS;                                            \
      su__start = clock ();                                            \
      body;                                                            \
      su__cat (su__test_case_skip_, id):;                              \
      su__test_time = (double)(clock () - su__start) / CLOCKS_PER_SEC  \
                      * 1000.0;                                        \
      su__test_result (&su__result, su__status, su__test_time, name,   \
                       &su__passed, su_compact);                       \
    }

/* Define a test case */
#define su_test(name, body) su__test (name, body, __LINE__)

/* Run a module */
#define su_run_module(name) su__cat (su__module_, name) ()

/* Declare an external module */
#define su_extern(name) extern SUResult su__cat (su__module_, name) ()

/* Flow control */

#define su_pass() { su__status = SU_PASS; goto *su__skip; }
#define su_fail() { su__status = SU_FAIL; goto *su__skip; }
#define su_skip() { su__status = SU_SKIP; goto *su__skip; }

/* Assertions */

#define SU__HERE printf ("%s(%d): ", __FILE__, __LINE__);
#define SU__AF(msg) \
  { SU__HERE; puts ("Assertion failed:\n  " msg); \
    su_fail (); }
#define SU__AFf(msg, ...) \
  { SU__HERE; printf ("Assertion failed: \n  " msg "\n", __VA_ARGS__); \
    su_fail (); }

#define su_assert(expr)                \
  {                                    \
    if (!(expr))                       \
      SU__AF ("'" su__str (expr) "'"); \
  }

#define su_assert_eq(a, b)                             \
  {                                                    \
    if (!((a) == (b)))                                 \
      SU__AF ("'" su__str (a) " == " su__str (b) "'"); \
  }

#define su_assert_arrays_eq(a, b, size)                          \
  {                                                              \
    typeof (a) su__a = (a);                                      \
    typeof (b) su__b = (b);                                      \
    for (size_t su__i = 0; su__i < size; ++su__i)                \
      {                                                          \
        if (su__a[su__i] != su__b[su__i])                        \
          SU__AFf ("Buffers '" su__str (a) "' and '" su__str (b) \
                   "' differ at index %zu", su__i);              \
      }                                                          \
  }



static void
su__test_result (SUResult *result, int status, double time, const char *name,
                 unsigned *passed, bool compact)
{
  result->milliseconds += time;
  switch (status)
    {
    case SU_PASS:
      {
        ++result->passed;
        ++*passed;
        printf ("    \x1b[32m:) \x1b[90m%s\x1b[0m", name);
        if (compact)
          printf (" (%d)\r", *passed);
        else
          putchar ('\n');
      } break;
    case SU_FAIL:
      {
        ++result->failed;
        if (*passed && compact)
          putchar ('\n');
        printf ("    \x1b[31m:( \x1b[90m%s\x1b[0m\n", name);
        *passed = 0;
      } break;
    case SU_SKIP:
      {
        ++result->skipped;
        if (*passed && compact)
          putchar ('\n');
        printf ("    \x1b[33m:/ \x1b[90m%s\x1b[0m\n", name);
        *passed = 0;
      } break;
    }
}

static inline void
su__print_result (SUResult *result)
{
  putchar (' ');
  if (result->passed > 0)
    printf (" \x1b[32m%d passing", result->passed);
  if (result->failed > 0)
    printf (" \x1b[31m%d failing", result->failed);
  if (result->skipped > 0)
    printf (" \x1b[33m%d skipped", result->skipped);
  if (result->milliseconds < 1000.0)
    printf (" \x1b[90m(%dms)\x1b[0m\n\n", (int)(result->milliseconds + 0.5));
  else
    printf (" \x1b[90m(%ds)\x1b[0m\n\n", (int)(result->milliseconds / 1000.0 + 0.5));
}

#endif /* !SMALLUNIT_H */
