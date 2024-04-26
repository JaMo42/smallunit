#ifndef SMALLUNIT_H
#define SMALLUNIT_H
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

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

#ifdef __cplusplus
#define su__typeof(x) decltype(x)
#else
#define su__typeof(x) typeof(x)
#endif

enum
{
  SU_FAIL,
  SU_PASS,
  SU_SKIP
};

typedef struct
{
  unsigned passed;
  unsigned failed;
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
    su_print_result (&su__result);                                \
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
  { SU__HERE; puts ("Assertion failed:\n  " msg); }

#define SU__AFf(msg, ...) \
  { SU__HERE; printf ("Assertion failed: \n  " msg "\n", __VA_ARGS__); }

#ifdef __cplusplus
static inline bool SU__EQ(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}
template<class A, class B>
static inline bool SU__EQ(A a, B b) {
    return a == b;
}
#else
#define SU__EQ(a, b)                                                         \
    _Generic((a),                                                            \
        /* Note: all _Generic paths must typecheck for any value, including
           float, so we need to do this casting for the strings, there is no
           type punning happening here. */                                   \
        const char *: _Generic((b),                                          \
            const char*: strcmp(*(const char**)&a, *(const char**)&b) == 0,  \
            char*: strcmp(*(const char**)&a, *(const char**)&b) == 0,        \
            default: (a) == (b)                                              \
        ),                                                                   \
        char *: _Generic((b),                                                \
            const char *: strcmp(*(const char**)&a, *(const char**)&b) == 0, \
            char *: strcmp(*(const char**)&a, *(const char**)&b) == 0,       \
            default: (a) == (b)                                              \
        ),                                                                   \
        default: (a) == (b)                                                  \
    )
#endif

#ifdef FMT_H
#define SU__PRINT_VALUE(prefix, v)                                \
    do {                                                          \
        if (fmt_can_print(v)) {                                   \
            fmt_println("{}: \x1b[36m{:?}\x1b[m", (prefix), (v)); \
        }                                                         \
    } while (0)
#elifdef IC_H
#define SU__PRINT_VALUE(prefix, v)                             \
    do {                                                       \
        /* IC_STREAM may be stderr (it's the default) so we
           need to flush stdout to get the output in order. */ \
        fflush(stdout);                                        \
        fputs(prefix, IC_STREAM);                              \
        fputs(": ", IC_STREAM);                                \
        ic__print_value(v);                                    \
        fputs("\x1b[m\n", IC_STREAM);                          \
    } while (0)
#else
#define SU__PRINT_VALUE(prefix, v)
#endif

#define su_assert(expr)                           \
  {                                               \
    su__typeof(expr) su__expr = (expr);           \
    if (!su__expr) {                              \
      SU__AF ("'" #expr "'");                     \
      SU__PRINT_VALUE("    with expr", su__expr); \
      su_fail();                                  \
    }                                             \
  }

#define su_assert_eq(a, b)                    \
  {                                           \
    su__typeof(a) su__a = (a);                \
    su__typeof(b) su__b = (b);                \
    if (!SU__EQ(su__a, su__b)) {              \
      SU__AF ("'" #a " == " #b "'");          \
      SU__PRINT_VALUE("    with lhs", su__a); \
      SU__PRINT_VALUE("    with rhs", su__b); \
      su_fail();                              \
    }                                         \
  }

#define su_assert_arrays_eq(a, b, size)                  \
  {                                                      \
    su__typeof (a) su__a = (a);                          \
    su__typeof (b) su__b = (b);                          \
    for (size_t su__i = 0; su__i < size; ++su__i)        \
      {                                                  \
        if (!SU__EQ(su__a[su__i], su__b[su__i])) {       \
          SU__AFf ("Buffers '" #a "' and '" #b           \
                   "' differ at index %zu", su__i);      \
          SU__PRINT_VALUE("    with lhs", su__a[su__i]); \
          SU__PRINT_VALUE("    with rhs", su__b[su__i]); \
          su_fail();                                     \
        }                                                \
      }                                                  \
  }

#define su_bad_value(x, why)                   \
    {                                          \
        su__typeof(x) su__x = (x);             \
        SU__HERE;                              \
        puts("Bad value:");                    \
        puts("  '" #x "' " why);               \
        SU__PRINT_VALUE("    but was", su__x); \
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
su_print_result (SUResult *result)
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

[[maybe_unused]] static inline SUResult
su_new_result()
{
  return (SUResult) { 0, 0, 0, 0.0 };
}

[[maybe_unused]] static inline void
su_add_result(SUResult *to, SUResult result)
{
  to->passed += result.passed;
  to->failed += result.failed;
  to->skipped += result.skipped;
  to->milliseconds += result.milliseconds;
}

#endif /* !SMALLUNIT_H */

// Copyright 2024 Jakob Mohrbacher
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
