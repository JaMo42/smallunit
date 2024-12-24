#define SU_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include "smallunit.h"

#define arr_for_each(_a, _it) \
    for (typeof(_a) _it = (_a), end = (_a) + arrlen(_a); _it != end; ++_it)

#define hm_for_each(_a, _it) for (typeof(_a) _it = (_a), end = (_a) + hmlen(_a); _it != end; ++_it)

#define sh_for_each(_a, _it) for (typeof(_a) _it = (_a), end = (_a) + shlen(_a); _it != end; ++_it)

int
factorial(int n) {
    if (n == 0) {
        return 1;
    }
    return n * factorial(n - 1);
}

typedef struct queue_node {
    int *item;
    struct queue_node *next;
} queue_node_t;

typedef struct queue {
    queue_node_t *head;
    queue_node_t *tail;
    size_t size;
} queue_t;

void
queue_new(queue_t *q) {
    *q = (queue_t){0};
}

void
queue_drop(queue_t *q) {
    queue_node_t *node = q->head;
    while (node) {
        queue_node_t *next = node->next;
        free(node->item);
        free(node);
        node = next;
    }
    *q = (queue_t){0};
}

size_t
queue_size(queue_t *q) {
    return q->size;
}

void
queue_push(queue_t *q, int item) {
    queue_node_t *node = malloc(sizeof(*node));
    *node = (queue_node_t){.item = malloc(sizeof(item))};
    *node->item = item;
    if (q->tail) {
        q->tail->next = node;
    } else {
        q->head = node;
    }
    q->tail = node;
    ++q->size;
}

int *
queue_pop(queue_t *q) {
    if (q->size == 0) {
        return NULL;
    }
    queue_node_t *node = q->head;
    int *item = node->item;
    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(node);
    --q->size;
    return item;
}

typedef struct queue_test {
    queue_t q0, q1, q2;
} queue_test;

void
queue_test_setup(queue_test *self) {
    // not actually needed since `self` is zeroed
    queue_new(&self->q0);
    queue_new(&self->q1);
    queue_new(&self->q2);

    queue_push(&self->q1, 1);
    queue_push(&self->q2, 2);
    queue_push(&self->q2, 3);
}

void
queue_test_tear_down(queue_test *self) {
    queue_drop(&self->q0);
    queue_drop(&self->q1);
    queue_drop(&self->q2);
}

su_test(factorial_test, handles_zero_input) {
    su_expect_eq(factorial(0), 1);
}

su_test(factorial_test, handles_positive_input) {
    su_expect_eq(factorial(1), 1);
    su_expect_eq(factorial(2), 2);
    su_expect_eq(factorial(3), 6);
    su_expect_eq(factorial(8), 40320);
}

su_test_f(queue_test, is_empty_initially) {
    su_expect_eq(queue_size(&self->q0), 0);
}

su_test_f(queue_test, pop_works) {
    int *n = queue_pop(&self->q0);
    su_expect_eq(n, NULL);

    n = queue_pop(&self->q1);
    su_assert_ne(n, NULL);
    su_expect_eq(*n, 1);
    su_expect_eq(queue_size(&self->q1), 0);
    free(n);

    su_expect_eq(queue_pop(&self->q1), NULL);

    n = queue_pop(&self->q2);
    su_assert_ne(n, NULL);
    su_expect_eq(*n, 2);
    su_expect_eq(queue_size(&self->q2), 1);
    free(n);
}

#define MACRO_VALUE false

su_test(mytests, bar) {
    su_expect(MACRO_VALUE);
    puts("still here");
}

su_test(mytests, baz) {
    su_assert(false);
    puts("not here");
}

su_test(mytests, qux) {
    su_skip();
}

su_test(mytests, assertions) {
    su_expect_eq(1, 1);
    su_expect_ne(1, 2);
    su_expect_streq("foo", "foo");
    su_expect_strne("foo", "bar");
    su_expect_float_eq(1.0f, 1.0f);
    su_expect_double_eq(1.0, 1.0);
    su_expect_near(3.1415926, 22.0 / 7.0, 0.002);
}

#define do_float(_T, _step, _expected_direct)                               \
    do {                                                                    \
        const _T step = _step;                                              \
        _T sum = 0.0;                                                       \
        for (int i = 0; i < 10; ++i) {                                      \
            sum += step;                                                    \
        }                                                                   \
        const _T product = step * 10;                                       \
        su_expect(su_float_eq(su_float_##_T(sum), su_float_##_T(product))); \
        su_expect_eq((sum == product), _expected_direct);                   \
    } while (0)

su_test(mytests, float_equality_within_4_ulp) {
    do_float(float, 0.1f, false);
    do_float(float, 10.0f, true);
    do_float(float, 1000000000000000000.0f, false);
    do_float(double, 0.1, false);
    do_float(double, 10.0, true);
    do_float(double, 1000000000000000000.0, true);
    do_float(double, 10000000000000000000000.0, false);
}

static void
my_error(void) {
    fputs("error message", stderr);
    exit(1);
}

su_test(death_tests, nullpointer_write_crashes) {
    su_expect_exit(*(volatile char *)0 = 'A', su_killed_by_signal(SIGSEGV), NULL);
    char *valid = malloc(1);
    su_expect_exit(*valid = 'A', su_killed_by_signal(SIGSEGV), NULL);
    free(valid);
}

su_test(death_tests, death_error) {
    su_expect_death(my_error(), "error message");
}

int
main() {
    return su_run_all_tests();
}
