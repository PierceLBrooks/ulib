/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustrbuf.h"
#include <stdarg.h>

uvec_ret ustrbuf_append_format(UStrBuf *buf, char const *format, ...) {
    va_list args;
    va_start(args, format);
    uvec_ret ret = ustrbuf_append_format_list(buf, format, args);
    va_end(args);
    return ret;
}

uvec_ret ustrbuf_append_format_list(UStrBuf *buf, char const *format, va_list args) {
    size_t length = ulib_str_flength_list(format, args);
    size_t size = length + 1;
    uvec_ret ret = uvec_expand(char, buf, (ulib_uint)size);

    if (ret == UVEC_OK) {
        vsnprintf(ustrbuf_data(buf) + buf->_count, size, format, args);
        buf->_count += (ulib_uint)length;
    }

    return ret;
}

UString ustrbuf_to_ustring(UStrBuf *buf) {
    ulib_uint length = ustrbuf_length(buf);

    if (length < P_USTRING_SMALL_SIZE) {
        UString ret;
        char *nbuf = ustring(&ret, length);
        memcpy(nbuf, ustrbuf_data(buf), length);
        ustrbuf_deinit(buf);
        return ret;
    }

    char *nbuf = ulib_realloc(buf->_data, length + 1);

    if (!nbuf) {
        ustrbuf_deinit(buf);
        return ustring_null;
    }

    nbuf[length] = '\0';
    return ustring_wrap(nbuf, length);
}
