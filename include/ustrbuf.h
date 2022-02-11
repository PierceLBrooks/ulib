/**
 * A mutable string buffer.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTRBUF_H
#define USTRBUF_H

#include "ustring.h"
#include "uvec_builtin.h"

ULIB_BEGIN_DECLS

/**
 * A mutable string buffer.
 *
 * @struct UStrBuf
 * @extends UVec
 */
typedef struct UVec(char) UStrBuf;

/**
 * Initializes a new string buffer.
 *
 * @return Initialized string buffer.
 *
 * @public @memberof UStrBuf
 */
ULIB_PUBLIC
UStrBuf ustrbuf_init(void);

/**
 * Deinitializes a string buffer previously initialized with ustrbuf_init.
 *
 * @param buf String buffer.
 *
 * @public @memberof UStrBuf
 */
ULIB_PUBLIC
void ustrbuf_deinit(UStrBuf *buf);

/**
 * Returns the number of characters in the string buffer.
 *
 * @param buf [UStrBuf *] String buffer.
 * @return Number of characters.
 *
 * @public @related UStrBuf
 */
#define ustrbuf_count(buf) ((buf)->count)

/**
 * Returns the size of the string buffer.
 *
 * @param buf [UStrBuf *] String buffer.
 * @return Size.
 *
 * @public @related UStrBuf
 */
#define ustrbuf_size(buf) ((buf)->allocated)

/**
 * Returns a pointer to the first character of the string buffer.
 *
 * @param buf [UStrBuf *] String buffer.
 * @return Pointer to the first character.
 *
 * @public @related UStrBuf
 */
#define ustrbuf_data(buf) uvec_storage(char, buf)

/**
 * Appends the specified formatted string to the string buffer.
 *
 * @param buf String buffer.
 * @param format Format string.
 * @param ... Format arguments.
 * @return UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @memberof UStrBuf
 */
ULIB_PUBLIC
uvec_ret ustrbuf_append_format(UStrBuf *buf, char const *format, ...);

/**
 * Appends the specified formatted string to the string buffer.
 *
 * @param buf String buffer.
 * @param format Format string.
 * @param args Format arguments.
 * @return UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @memberof UStrBuf
 */
ULIB_PUBLIC
uvec_ret ustrbuf_append_format_list(UStrBuf *buf, char const *format, va_list args);

/**
 * Converts the string buffer into a UString and deinitializes the buffer.
 *
 * @param buf String buffer.
 * @return String.
 *
 * @note After calling this function, the string buffer must not be used anymore.
 *
 * @public @memberof UStrBuf
 */
ULIB_PUBLIC
UString ustrbuf_to_ustring(UStrBuf *buf);

/**
 * Appends the specified string literal to the string buffer.
 *
 * @param buf [UStrBuf *] String buffer.
 * @param literal [char const []] String literal to append.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UStrBuf
 */
#define ustrbuf_append_literal(buf, literal) \
    uvec_append_array(char, buf, literal, sizeof(literal) - 1)

/**
 * Appends the specified string to the string buffer.
 *
 * @param buf [UStrBuf *] String buffer.
 * @param string [char const *] String to append.
 * @param length [ulib_uint] Length of the string.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UStrBuf
 */
#define ustrbuf_append_cstring(buf, string, length) \
    uvec_append_array(char, buf, string, length)

/**
 * Appends the specified uString to the string buffer.
 *
 * @param buf [UStrBuf *] String buffer.
 * @param string [UString] String to append.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UStrBuf
 */
#define ustrbuf_append_ustring(buf, string) \
    uvec_append_array(char, buf, ustring_data(string), ustring_length(string))

ULIB_END_DECLS

#endif // USTRBUF_H
