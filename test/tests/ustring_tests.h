/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTRING_TESTS_H
#define USTRING_TESTS_H

#include "ustd.h"

ULIB_BEGIN_DECLS

bool ustring_utils_test(void);
bool ustrbuf_test(void);
bool ustring_test(void);

#define USTRING_TESTS                                                                               \
    ustring_utils_test,                                                                             \
    ustrbuf_test,                                                                                   \
    ustring_test

ULIB_END_DECLS

#endif // USTRING_TESTS_H