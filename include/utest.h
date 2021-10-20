/**
 * Essential test utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UTEST_H
#define UTEST_H

#include "ustd.h"
#include "umacros.h"

ULIB_BEGIN_DECLS

/**
 * Defines the main test function.
 *
 * @param CODE Code to execute, generally a sequence of utest_run statements.
 *
 * @public
 */
#define utest_main(CODE)                                                                            \
    int main(void) {                                                                                \
        if (!utest_leak_start()) return EXIT_FAILURE;                                               \
        int exit_code = EXIT_SUCCESS;                                                               \
        { CODE }                                                                                    \
        if (!utest_leak_end()) exit_code = EXIT_FAILURE;                                            \
        return exit_code;                                                                           \
    }

/**
 * Runs a test batch.
 *
 * @param NAME Name of the test batch (must be a string literal).
 * @param ... Comma separated list of [void] -> bool test functions.
 *
 * @public
 */
#define utest_run(NAME, ...) do {                                                                   \
    int run_exit_code = EXIT_SUCCESS;                                                               \
    printf("Starting \"" NAME "\" tests.\n");                                                       \
                                                                                                    \
    bool (*tests_to_run[])(void) = { __VA_ARGS__ };                                                 \
    for (size_t test_i = 0; test_i < ulib_array_count(tests_to_run); ++test_i) {                    \
        if (!tests_to_run[test_i]()) run_exit_code = EXIT_FAILURE;                                  \
    }                                                                                               \
                                                                                                    \
    if (run_exit_code == EXIT_SUCCESS) {                                                            \
        printf("All \"" NAME "\" tests passed.\n");                                                 \
    } else {                                                                                        \
        exit_code = EXIT_FAILURE;                                                                   \
        printf("Some \"" NAME "\" tests failed.\n");                                                \
    }                                                                                               \
} while (0)

/**
 * Assert that the specified expression must be true.
 *
 * @param EXP Boolean expression.
 *
 * @public
 */
#define utest_assert(EXP) utest_assert_wrap(EXP,, "\"" #EXP "\" must be true.")

/**
 * Assert that the specified expression must be false.
 *
 * @param EXP Boolean expression.
 *
 * @public
 */
#define utest_assert_false(EXP) utest_assert_wrap(!(EXP),, "\"" #EXP "\" must be false.")

/**
 * Assert that the specified expression must not be NULL.
 *
 * @param EXP Expression returning any pointer.
 *
 * @public
 */
#define utest_assert_not_null(EXP) \
    utest_assert_wrap(EXP,, "\"" #EXP "\" must not be NULL.")

/**
 * Assert that the specified expression must be true.
 * Abort the tests if it is false.
 *
 * @param EXP Boolean expression.
 *
 * @public
 */
#define utest_assert_critical(EXP)                                                                  \
    utest_assert_wrap(EXP, exit(EXIT_FAILURE),                                                      \
                      "\"" #EXP "\" must be true.\nThis is a critical error, aborting...")

/**
 * Utility macro for test assertions.
 *
 * @param EXP Expression.
 * @param CODE Any custom code to run after printing the failure reason.
 * @param ... Failure reason as printf arguments.
 *
 * @public
 */
#define utest_assert_wrap(EXP, CODE, ...) do {                                                      \
    if (!(EXP)) {                                                                                   \
        printf("Test failed: %s, %s, line %d\nReason: ", __FILE__, __func__, __LINE__);             \
        printf(__VA_ARGS__);                                                                        \
        CODE;                                                                                       \
        printf("\n");                                                                               \
        return false;                                                                               \
    }                                                                                               \
} while (0)

/**
 * Start detection of memory leaks.
 *
 * @return True if detection started successfully, false otherwise.
 *
 * @public
 */
ULIB_PUBLIC
bool utest_leak_start(void);

/**
 * Ends detection of memory leaks and prints detected leaks to the console.
 *
 * @return True if no leaks were detected, false otherwise.
 *
 * @public
 */
ULIB_PUBLIC
bool utest_leak_end(void);

// Private API

ULIB_PUBLIC
void* p_utest_leak_malloc_impl(size_t size, char const *file, char const *fn, int line);

ULIB_PUBLIC
void* p_utest_leak_calloc_impl(size_t num, size_t size, char const *file, char const *fn, int line);

ULIB_PUBLIC
void* p_utest_leak_realloc_impl(void *ptr, size_t size, char const *file, char const *fn, int line);

ULIB_PUBLIC
void p_utest_leak_free_impl(void *ptr);

#define p_utest_leak_malloc(size) \
    p_utest_leak_malloc_impl(size, __FILE__, __func__, __LINE__)

#define p_utest_leak_calloc(num, size) \
    p_utest_leak_calloc_impl(num, size, __FILE__, __func__, __LINE__)

#define p_utest_leak_realloc(ptr, size) \
    p_utest_leak_realloc_impl(ptr, size, __FILE__, __func__, __LINE__)

#define p_utest_leak_free(ptr) p_utest_leak_free_impl(ptr)

ULIB_END_DECLS

#endif // UTEST_H