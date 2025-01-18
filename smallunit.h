// https://github.com/JaMo42/smallunit
#ifndef SMALLUNIT_H
#define SMALLUNIT_H
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stb_ds.h"

#ifndef SU_FIXTURE_IDENTIFIER
#define SU_FIXTURE_IDENTIFIER self
#endif

#ifndef SU_STDERR_BUF_SIZE
#define SU_STDERR_BUF_SIZE 4096
#endif

#if __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>
#define SU_HAS_VALGRIND
#define SU_RUNNING_ON_VALGRIND RUNNING_ON_VALGRIND
#else
#define SU_RUNNING_ON_VALGRIND false
#endif

#ifndef SU_NO_SHORT_NAMES
#define TEST su_test
#define TEST_F su_test_f

#define SKIP su_skip

#define EXPECT su_expect
#define EXPECT_EQ su_expect_eq
#define EXPECT_NE su_expect_ne
#define EXPECT_STREQ su_expect_streq
#define EXPECT_STRNE su_expect_strne
#define EXPECT_FLOAT_EQ su_expect_float_eq
#define EXPECT_DOUBLE_EQ su_expect_double_eq
#define EXPECT_NEAR su_expect_near

#define ASSERT su_assert
#define ASSERT_EQ su_assert_eq
#define ASSERT_NE su_assert_ne
#define ASSERT_STREQ su_assert_streq
#define ASSERT_STRNE su_assert_strne
#define ASSERT_FLOAT_EQ su_assert_float_eq
#define ASSERT_DOUBLE_EQ su_assert_double_eq
#define ASSERT_NEAR su_assert_near

#define EXPECT_EXIT su_expect_exit
#define EXPECT_DEATH su_expect_death
#endif

#define su_str_inner(x) #x
#define su_str(x) su_str_inner(x)
#define su_cat_inner(x, y) x##y
#define su_cat(x, y) su_cat_inner(x, y)
#define su_cat3_inner(x, y, z) x##y##z
#define su_cat3(x, y, z) su_cat3_inner(x, y, z)

#define su_arrpush(arr)                              \
    ({                                               \
        typeof(arr) _ptr = stbds_arraddnptr(arr, 1); \
        memset(_ptr, 0, sizeof(*_ptr));              \
        _ptr;                                        \
    })

typedef enum {
    SU_PASS,
    SU_FAIL,
    SU_SKIP,
} su_status_t;

typedef uint16_t su_count_t;

typedef struct {
    // ms * 10 + one decimal place
    uint64_t value;
} su_time_t;

su_time_t su_time_from(struct timespec t);
su_time_t su_time_add(su_time_t a, su_time_t b);
su_time_t su_time_sub(su_time_t a, su_time_t b);
double su_time_ms(su_time_t t);

typedef struct {
    int pid;
    int rx;
    int tx;
} su_subproc_info_t;

su_subproc_info_t su_subproc_begin(void);

typedef struct {
    int status;  // encoded value of waitpid
    char *_stderr_buf;
    const char *standard_error;
} su_subproc_result_t;

su_subproc_result_t su_subproc_end(su_subproc_info_t info);

typedef struct {
    int code_or_signal;
    bool is_signal;
    bool any_abnormal;
    const char *output;
} su_subproc_predicate_t;

su_subproc_predicate_t su_exited_with_code(int code);
su_subproc_predicate_t su_killed_by_signal(int signal);
su_subproc_predicate_t su_exited_abnormally(void);
bool su_subproc_predicate_matches_status(su_subproc_predicate_t predicate, int status);

/// Returns `true` if the assertion should fail!
bool su_check_subproc_result(
    const su_subproc_result_t *result,
    su_subproc_predicate_t predicate,
    const char *test_name,
    int line
);

typedef struct su_test su_test_t;

typedef void (*su_stateless_test_fn_t)(su_test_t *);
typedef void (*su_fixture_test_fn_t)(su_test_t *, void *);
typedef void *su_test_fn_t;

struct su_test {
    const char *name;
    su_status_t status;
    su_time_t runtime;
    su_test_fn_t fn;
};

typedef struct {
    void (*init)(void *);
    void (*clean)(void *);
    void (*run)(void *, su_test_t *);
} su_module_vtable_t;

typedef struct {
    const su_module_vtable_t *vtable;
    su_test_t *tests;
    const char *name;
    su_time_t runtime;
    su_count_t counts[3];
} su_module_t;

void su_module_run_test(su_module_t *mod, su_test_t *test);
void su_module_run(su_module_t *mod);

typedef struct {
    su_module_t mod;
    void *fixture;
    size_t object_size;
    void (*setup)(void *);
    void (*tear_down)(void *);
} su_fixture_owner_t;

typedef struct {
    bool skip_death_tests;
} su_options_t;

void su_options_default(su_options_t *options);

typedef struct {
    su_count_t counts[3];
    su_time_t runtime;
} su_result_t;

typedef struct {
    su_module_t **modules;

    struct {
        const char *key;
        su_module_t *value;
    } *modules_by_name;

    struct {
        const char *key;
        su_fixture_owner_t *value;
    } *fixtures_by_name;

    struct {
        const char *key;
        const char *value;
    } *test_names;

    su_options_t options;
    // we want runtime defaults so we cannot statically initialize options
    // with their default values
    bool options_initialized;
} su_state_t;

/// Get or create a module.
su_module_t *su_state_get_module(su_state_t *state, const char *name);
/// Get or create a fixture owner.
su_fixture_owner_t *su_state_get_fixture(su_state_t *state, const char *name);
/// Set a pretty test function name.
void su_state_set_test_name(su_state_t *state, const char *function_name, const char *pretty_name);
/// Get a pretty test function name.
const char *
su_state_test_name(su_state_t *state, const char *function_name, const char *pretty_name);
/// Run all tests in the state.
su_result_t su_state_run(su_state_t *state);
/// Free all memory of the state.
void su_state_drop(su_state_t *state);

extern su_state_t su__state;

typedef struct {
    uint64_t bits;
    bool is_nan;
    bool is_inf;
    bool sign;
    unsigned char sign_bit_index;
} su_float_t;

su_float_t su_float_float(float a);
su_float_t su_float_double(double a);
bool su_float_eq(su_float_t a, su_float_t b);

/// Returns 0 if no tests failed.
int su_run_all_tests(void);

void su_release_state(void);

#define su_test_name(_mod, _test) su_test_##_mod##_##_test

#define su_test(_mod, _test)                                                                     \
    void su_test_name(_mod, _test)(su_test_t *);                                                 \
    static void __attribute__((constructor)) su_cat3(su__register_, _mod, _test)() {             \
        su_module_t *mod = su_state_get_module(&su__state, su_str(_mod));                        \
        su_test_t *test = su_arrpush(mod->tests);                                                \
        test->name = su_str(_test);                                                              \
        test->fn = (su_test_fn_t)su_test_name(_mod, _test);                                      \
        su_state_set_test_name(&su__state, su_str(su_test_name(_mod, _test)), #_mod "." #_test); \
    }                                                                                            \
    void su_test_name(_mod, _test)(su_test_t * su_self)

#define su_test_f(_fixture, _test)                                                        \
    void su_test_name(_fixture, _test)(su_test_t *, _fixture *);                          \
    static void __attribute__((constructor)) su_cat3(su__register_, _fixture, _test)() {  \
        su_fixture_owner_t *fixture = su_state_get_fixture(&su__state, su_str(_fixture)); \
        su_test_t *test = su_arrpush(fixture->mod.tests);                                 \
        test->name = su_str(_test);                                                       \
        test->fn = (su_test_fn_t)su_test_name(_fixture, _test);                           \
        fixture->object_size = sizeof(_fixture);                                          \
        fixture->setup = (void (*)(void *))_fixture##_setup;                              \
        fixture->tear_down = (void (*)(void *))_fixture##_tear_down;                      \
        su_state_set_test_name(                                                           \
            &su__state, su_str(su_test_name(_fixture, _test)), #_fixture "." #_test       \
        );                                                                                \
    }                                                                                     \
    void su_test_name(_fixture, _test)(su_test_t * su_self, _fixture * SU_FIXTURE_IDENTIFIER)

#define su_pretty_function() su_state_test_name(&su__state, __func__, __PRETTY_FUNCTION__)

#define su_skip()                         \
    do {                                  \
        if (su_self->status == SU_PASS) { \
            su_self->status = SU_SKIP;    \
        }                                 \
        return;                           \
    } while (0)

#define su_assert_impl(_expr, _msg, _fatal)                                                    \
    do {                                                                                       \
        if (!(_expr)) {                                                                        \
            fprintf(                                                                           \
                stderr, "%s(%d): Assertion failed: %s\n", su_pretty_function(), __LINE__, _msg \
            );                                                                                 \
            su_self->status = SU_FAIL;                                                         \
            if (_fatal) {                                                                      \
                return;                                                                        \
            }                                                                                  \
        }                                                                                      \
    } while (0)

#define su_expect(_expr) su_assert_impl(_expr, #_expr, false)
#define su_expect_eq(_a, _b) su_assert_impl((_a) == (_b), #_a " == " #_b, false)
#define su_expect_ne(_a, _b) su_assert_impl((_a) != (_b), #_a " != " #_b, false)
#define su_expect_streq(_a, _b) su_assert_impl(strcmp(_a, _b) == 0, #_a " == " #_b, false)
#define su_expect_strne(_a, _b) su_assert_impl(strcmp(_a, _b) != 0, #_a " != " #_b, false)
/// The two float values are almost equal (within 4 ULP's from each other)
#define su_expect_float_eq(_a, _b) \
    su_assert_impl(su_float_eq(su_float_float(_a), su_float_float(_b)), #_a " == " #_b, false)
#define su_expect_double_eq(_a, _b) \
    su_assert_impl(su_float_eq(su_float_double(_a), su_float_double(_b)), #_a " == " #_b, false)
#define su_expect_near(_a, _b, _tolerance) \
    su_assert_impl(fabs((_a) - (_b)) <= (_tolerance), #_a " == " #_b, false)

#define su_assert(_expr) su_assert_impl(_expr, #_expr, true)
#define su_assert_eq(_a, _b) su_assert_impl((_a) == (_b), #_a " == " #_b, true)
#define su_assert_ne(_a, _b) su_assert_impl((_a) != (_b), #_a " != " #_b, true)
#define su_assert_streq(_a, _b) su_assert_impl(strcmp(_a, _b) == 0, #_a " == " #_b, true)
#define su_assert_strne(_a, _b) su_assert_impl(strcmp(_a, _b) != 0, #_a " != " #_b, true)
#define su_assert_float_eq(_a, _b) \
    su_assert_impl(su_float_eq(su_float_float(_a), su_float_float(_b)), #_a " == " #_b, true)
#define su_assert_double_eq(_a, _b) \
    su_assert_impl(su_float_eq(su_float_double(_a), su_float_double(_b)), #_a " == " #_b, true)
#define su_assert_near(_a, _b, _tolerance) \
    su_assert_impl(fabs((_a) - (_b)) <= (_tolerance), #_a " == " #_b, true)

#define su_expect_exit(_stmt, _pred, _output)                                                 \
    do {                                                                                      \
        if (su__state.options.skip_death_tests) {                                             \
            su_skip();                                                                        \
        } else {                                                                              \
            su_subproc_info_t su_info = su_subproc_begin();                                   \
            if (su_info.pid == 0) {                                                           \
                _stmt;                                                                        \
            }                                                                                 \
            su_subproc_result_t su_result = su_subproc_end(su_info);                          \
            if (su_check_subproc_result(&su_result, _pred, su_pretty_function(), __LINE__)) { \
                su_self->status = SU_FAIL;                                                    \
                return;                                                                       \
            }                                                                                 \
        }                                                                                     \
    } while (0)

#define su_expect_death(_stmt, _output) su_expect_exit(_stmt, su_exited_abnormally(), _output)

#endif  // SMALLUNIT_H

// MARK: - Implementation

#ifdef SU_IMPLEMENTATION
#include <ctype.h>

#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *SU_STATUS_LABELS[] = {
    "\x1b[32m:)\x1b[m",
    "\x1b[31m:(\x1b[m",
    "\x1b[33m:/\x1b[m",
};

su_state_t su__state;

// MARK: - Time

su_time_t
su_time_from(struct timespec t) {
    return (su_time_t){.value = t.tv_sec * 10000 + t.tv_nsec / 100000};
}

su_time_t
su_time_add(su_time_t a, su_time_t b) {
    return (su_time_t){.value = a.value + b.value};
}

su_time_t
su_time_sub(su_time_t a, su_time_t b) {
    return (su_time_t){.value = a.value - b.value};
}

double
su_time_ms(su_time_t t) {
    return (double)t.value / 10.0;
}

// MARK: - Subprocesses

static void
su_disable_core_dumps(void) {
    struct rlimit rlim;
    rlim.rlim_cur = 0;
    rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
        perror("setrlimit");
    }
}

su_subproc_info_t
su_subproc_begin(void) {
    int p[2];
    if (pipe(p) == -1) {
        perror("pipe");
        exit(1);
    }
    const int rx = p[0];
    const int tx = p[1];
    const int pid = fork();
    switch (pid) {
    case -1: perror("fork"); exit(1);

    case 0:
        su_disable_core_dumps();
        dup2(tx, STDERR_FILENO);
        close(rx);
        break;

    default: close(tx); break;
    }
    return (su_subproc_info_t){.pid = pid, .rx = rx, .tx = tx};
}

su_subproc_result_t
su_subproc_end(su_subproc_info_t info) {
    if (info.pid == 0) {
        close(info.tx);
        exit(EXIT_SUCCESS);
    }
    int status;
    waitpid(info.pid, &status, 0);
    char *const output = malloc(SU_STDERR_BUF_SIZE);
    ssize_t n = read(info.rx, output, SU_STDERR_BUF_SIZE);
    if (n == -1) {
        perror("read");
        exit(1);
    }
    close(info.rx);
    output[n] = '\0';
    while (n > 1 && isspace(output[n - 1])) {
        output[--n] = '\0';
    }
    const char *trimmed = output;
    while (isspace(*trimmed)) {
        ++trimmed;
    }
    return (su_subproc_result_t){
        .status = status,
        ._stderr_buf = output,
        .standard_error = trimmed,
    };
}

su_subproc_predicate_t
su_exited_with_code(int code) {
    return (su_subproc_predicate_t){.code_or_signal = code, .is_signal = false};
}

su_subproc_predicate_t
su_killed_by_signal(int signal) {
    return (su_subproc_predicate_t){.code_or_signal = signal, .is_signal = true};
}

su_subproc_predicate_t
su_exited_abnormally(void) {
    return (su_subproc_predicate_t){.any_abnormal = true};
}

bool
su_subproc_predicate_matches_status(su_subproc_predicate_t predicate, int status) {
    if (predicate.any_abnormal) {
        return WIFEXITED(status) && WEXITSTATUS(status) != 0;
    } else if (predicate.is_signal) {
        return WIFSIGNALED(status) && WTERMSIG(status) == predicate.code_or_signal;
    } else {
        return WIFEXITED(status) && WEXITSTATUS(status) == predicate.code_or_signal;
    }
}

static char *
su_describe_status(int status) {
    char *result;
    if (WIFEXITED(status)) {
        const int code = WEXITSTATUS(status);
        if (code == 0) {
            result = strdup("exited normally");
        } else {
            asprintf(&result, "code(%d)", code);
        }
    } else if (WIFSIGNALED(status)) {
        const int signal = WTERMSIG(status);
        asprintf(&result, "signal(%d)", signal);
    } else {
        result = strdup("unknown");
    }
    return result;
}

bool
su_check_subproc_result(
    const su_subproc_result_t *result,
    su_subproc_predicate_t predicate,
    const char *test_name,
    int line
) {
    const bool status_matches = su_subproc_predicate_matches_status(predicate, result->status);
    const bool output_matches
        = !predicate.output || strcmp(predicate.output, result->standard_error) == 0;
    if (status_matches && output_matches) {
        return false;
    }
    printf("%s(%d): expected ", test_name, line);
    if (!status_matches) {
        if (predicate.any_abnormal) {
            printf("abnormal exit, got %s\n", su_describe_status(result->status));
        } else if (predicate.is_signal) {
            printf(
                "killed by signal %d, got %s\n",
                predicate.code_or_signal,
                su_describe_status(result->status)
            );
        } else {
            printf(
                "exited with code %d, got %s\n",
                predicate.code_or_signal,
                su_describe_status(result->status)
            );
        }
    } else {
        printf("output \"%s\", got \"%s\"\n", predicate.output, result->standard_error);
    }
    return true;
}

// MARK: - Module

static void
su_print_results(su_count_t *counts, su_time_t runtime) {
    const char *sep = "";
    if (counts[SU_PASS]) {
        printf("\x1b[32m%d passing\x1b[m", counts[SU_PASS]);
        sep = " ";
    }
    if (counts[SU_FAIL]) {
        printf("%s\x1b[31m%d failing\x1b[m", sep, counts[SU_FAIL]);
        sep = " ";
    }
    if (counts[SU_SKIP]) {
        printf("%s\x1b[33m%d skipped\x1b[m", sep, counts[SU_SKIP]);
    }
    const double ms = su_time_ms(runtime);
    if (ms >= 1000.0) {
        printf(" \x1b[2m(%.2fs)\x1b[m\n", ms / 1000.0);
    } else {
        printf(" \x1b[2m(%lums)\x1b[m\n", (unsigned long)(ms + 0.5));
    }
}

void
su_module_run_test(su_module_t *mod, su_test_t *test) {
    struct timespec start, end;
    test->status = SU_PASS;
    clock_gettime(CLOCK_MONOTONIC, &start);
    mod->vtable->run(mod, test);
    clock_gettime(CLOCK_MONOTONIC, &end);
    test->runtime = su_time_sub(su_time_from(end), su_time_from(start));
}

void
su_module_run(su_module_t *mod) {
    printf("  %s\n", mod->name);
    memset(mod->counts, 0, sizeof(mod->counts));
    mod->runtime = (su_time_t){0};
    mod->vtable->init(mod);
    for (int i = 0; i < arrlen(mod->tests); ++i) {
        su_test_t *test = &mod->tests[i];
        su_module_run_test(mod, test);
        ++mod->counts[test->status];
        mod->runtime = su_time_add(mod->runtime, test->runtime);
        printf("    %s \x1b[2m%s\x1b[m\n", SU_STATUS_LABELS[test->status], test->name);
    }
    mod->vtable->clean(mod);
    fputs("\n  ", stdout);
    su_print_results(mod->counts, mod->runtime);
    fputc('\n', stdout);
}

static void
su_noop(void *_) {
    (void)_;
}

static void
su_run_stateless_test(void *_, su_test_t *test) {
    (void)_;
    ((su_stateless_test_fn_t)test->fn)(test);
}

static const su_module_vtable_t SU_MODULE_VTABLE = {
    .init = su_noop,
    .clean = su_noop,
    .run = su_run_stateless_test,
};

static void
su_fixture_owner_init(void *p_self) {
    su_fixture_owner_t *self = p_self;
    self->fixture = calloc(1, self->object_size);
    self->setup(self->fixture);
}

static void
su_fixture_owner_clean(void *p_self) {
    su_fixture_owner_t *self = p_self;
    self->tear_down(self->fixture);
    free(self->fixture);
}

static void
su_run_fixture_test(void *p_self, su_test_t *test) {
    su_fixture_owner_t *self = p_self;
    ((su_fixture_test_fn_t)test->fn)(test, self->fixture);
}

static const su_module_vtable_t SU_FIXTURE_OWNER_VTABLE = {
    .init = su_fixture_owner_init,
    .clean = su_fixture_owner_clean,
    .run = su_run_fixture_test,
};

// MARK: - State

void
su_options_default(su_options_t *options) {
    options->skip_death_tests = SU_RUNNING_ON_VALGRIND;
}

su_module_t *
su_state_get_module(su_state_t *state, const char *name) {
    const ptrdiff_t index = shgeti(state->modules_by_name, name);
    if (index < 0) {
        su_module_t *mod = calloc(1, sizeof(*mod));
        mod->name = name;
        mod->vtable = &SU_MODULE_VTABLE;
        // NOLINTNEXTLINE
        arrput(state->modules, mod);
        shput(state->modules_by_name, name, mod);
        return mod;
    } else {
        return state->modules_by_name[index].value;
    }
}

su_fixture_owner_t *
su_state_get_fixture(su_state_t *state, const char *name) {
    const ptrdiff_t index = shgeti(state->fixtures_by_name, name);
    if (index < 0) {
        su_fixture_owner_t *fixture = calloc(1, sizeof(*fixture));
        fixture->mod.name = name;
        fixture->mod.vtable = &SU_FIXTURE_OWNER_VTABLE;
        // NOLINTNEXTLINE
        arrput(state->modules, (su_module_t *)fixture);
        shput(state->fixtures_by_name, name, fixture);
        return fixture;
    } else {
        return state->fixtures_by_name[index].value;
    }
}

void
su_state_set_test_name(su_state_t *state, const char *function_name, const char *pretty_name) {
    shput(state->test_names, function_name, pretty_name);
}

const char *
su_state_test_name(su_state_t *state, const char *function_name, const char *pretty_name) {
    const ptrdiff_t index = shgeti(state->test_names, function_name);
    if (index < 0) {
        return pretty_name;
    } else {
        return state->test_names[index].value;
    }
}

su_result_t su_state_run(su_state_t *state) {
    su_result_t result = {0};
    if (!state->options_initialized) {
        su_options_default(&state->options);
        state->options_initialized = true;
    }
    for (int i = 0; i < arrlen(state->modules); ++i) {
        su_module_t *mod = state->modules[i];
        su_module_run(mod);
        result.counts[SU_PASS] += mod->counts[SU_PASS];
        result.counts[SU_FAIL] += mod->counts[SU_FAIL];
        result.counts[SU_SKIP] += mod->counts[SU_SKIP];
        result.runtime = su_time_add(result.runtime, mod->runtime);
    }
    fputs("Total:\n  ", stdout);
    su_print_results(result.counts, result.runtime);
    return result;
}

void
su_state_drop(su_state_t *state) {
    for (int i = 0; i < arrlen(state->modules); ++i) {
        arrfree(state->modules[i]->tests);
        free(state->modules[i]);
    }
    arrfree(state->modules);
    shfree(state->modules_by_name);
    shfree(state->fixtures_by_name);
    shfree(state->test_names);
}

// MARK: - Float

su_float_t
su_float_float(float a) {
    uint64_t bits = 0;
    memcpy(&bits, &a, sizeof(a));
    return (su_float_t){
        .bits = bits,
        .is_nan = isnan(a),
        .is_inf = isinf(a),
        .sign = signbit(a),
        .sign_bit_index = 31,
    };
}

su_float_t
su_float_double(double a) {
    uint64_t bits = 0;
    memcpy(&bits, &a, sizeof(bits));
    return (su_float_t){
        .bits = bits,
        .is_nan = isnan(a),
        .is_inf = isinf(a),
        .sign = signbit(a),
        .sign_bit_index = 63,
    };
}

static uint64_t
su_float_sign_magnitude_to_biased(su_float_t f) {
    const uint64_t sign_mask = 1ull << f.sign_bit_index;
    if (f.bits & sign_mask) {
        return ~f.bits + 1;
    } else {
        return f.bits | sign_mask;
    }
}

// https://gist.github.com/2b-t/02daa85ea5d83fc2cb96bfcf0570ab71
bool
su_float_eq(su_float_t a, su_float_t b) {
    if (a.is_nan || b.is_nan) {
        return false;
    }
    if (a.is_inf != b.is_inf || a.sign != b.sign) {
        return false;
    }
    const uint64_t a_biased = su_float_sign_magnitude_to_biased(a);
    const uint64_t b_biased = su_float_sign_magnitude_to_biased(b);
    const uint64_t distance = a_biased > b_biased ? a_biased - b_biased : b_biased - a_biased;
    return distance <= 4;
}

// MARK: - Global

bool su_streq(const char *a, const char *b) {
    if (a == b) {
        return true;
    } else if (!a || !b) {
        return false;
    } else {
        return strcmp(a, b) == 0;
    }
}

int
su_run_all_tests() {
    su_result_t result = su_state_run(&su__state);
    su_release_state();
    return result.counts[SU_FAIL] ? 1 : 0;
}

void
su_release_state() {
    su_state_drop(&su__state);
}

#endif  // SU_IMPLEMENTATION

// Copyright 2024, 2025 Jakob Mohrbacher
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
