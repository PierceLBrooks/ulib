/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustring.h"
#include "umacros.h"
#include "ustrbuf.h"
#include <stdarg.h>

UString const ustring_null = { ._s = { ._size = 0 } };
UString const ustring_empty = { ._s = { ._size = 1 } };

static inline UString ustring_small(char const *buf, size_t length) {
    UString ret = { ._s = { ._size = (ulib_uint)length + 1 } };
    memcpy(ret._s._data, buf, length);
    return ret;
}

static inline UString ustring_large(char const *buf, size_t length) {
    return (UString){ ._l = { ._size = (ulib_uint)length + 1, ._data = buf } };
}

UString ustring_assign(char const *buf, size_t length) {
    bool should_free = true;
    UString ret = ustring_null;

    if (!buf) goto end;

    if (p_ustring_length_is_small(length)) {
        ret = ustring_small(buf, length);
        goto end;
    }

    ret = ustring_large(buf, length);
    should_free = false;

end:
    if (should_free) ulib_free((void *)buf);
    return ret;
}

UString ustring_copy(char const *buf, size_t length) {
    if (!buf) return ustring_null;
    if (p_ustring_length_is_small(length)) return ustring_small(buf, length);
    return ustring_large(ulib_str_dup(buf, length), length);
}

UString ustring_wrap(char const *buf, size_t length) {
    if (!buf) return ustring_null;
    if (p_ustring_length_is_small(length)) return ustring_small(buf, length);
    return ustring_large(buf, length);
}

char *ustring(UString *string, size_t length) {
    char *buf;

    if (p_ustring_length_is_small(length)) {
        *string = (UString){ ._s = { ._size = (ulib_uint)length + 1 } };
        buf = string->_s._data;
    } else {
        buf = ulib_malloc(length + 1);
        if (buf) {
            *string = (UString){ ._l = { ._size = (ulib_uint)length + 1, ._data = buf } };
        } else {
            *string = ustring_null;
        }
    }

    if (buf) buf[length] = '\0';
    return buf;
}

void ustring_deinit(UString *string) {
    if (!p_ustring_is_small(*string)) ulib_free((void *)(string)->_l._data);
}

char *ustring_deinit_return_data(UString *string) {
    char *ret;

    if (p_ustring_is_small(*string)) {
        ret = ulib_malloc(string->_size);
        if (ret) memcpy(ret, string->_s._data, string->_size);
    } else {
        ret = (char *)string->_l._data;
        string->_l._data = NULL;
    }

    return ret;
}

UString ustring_dup(UString string) {
    return ustring_copy(ustring_data(string), ustring_length(string));
}

ulib_uint ustring_index_of(UString string, char needle) {
    char const *data = ustring_data(string);
    ulib_uint len = ustring_length(string);
    char const *chr = memchr(data, needle, len);
    return chr ? (ulib_uint)(chr - data) : len;
}

ulib_uint ustring_index_of_last(UString string, char needle) {
    char const *data = ustring_data(string);
    ulib_uint len = ustring_length(string);

    for (ulib_uint i = len; i-- != 0;) {
        if (data[i] == needle) return i;
    }

    return len;
}

ulib_uint ustring_find(UString string, UString needle) {
    char const *const str_data = ustring_data(string), *const n_data = ustring_data(needle);
    ulib_uint const str_len = ustring_length(string), n_len = ustring_length(needle);
    ulib_uint const max_i = n_len < str_len ? str_len - n_len : 0;
    for (ulib_uint i = 0; i < max_i; ++i) {
        if (memcmp(str_data + i, n_data, n_len) == 0) return i;
    }
    return str_len;
}

ulib_uint ustring_find_last(UString string, UString needle) {
    char const *const str_data = ustring_data(string), *const n_data = ustring_data(needle);
    ulib_uint const str_len = ustring_length(string), n_len = ustring_length(needle);
    for (ulib_uint i = n_len < str_len ? str_len - n_len : 0; i-- != 0;) {
        if (memcmp(str_data + i, n_data, n_len) == 0) return i;
    }
    return str_len;
}

bool ustring_starts_with(UString string, UString prefix) {
    ulib_uint p_len = ustring_length(prefix);
    return p_len <= ustring_length(string) &&
           memcmp(ustring_data(string), ustring_data(prefix), p_len) == 0;
}

bool ustring_ends_with(UString string, UString suffix) {
    ulib_uint str_len = ustring_length(string), s_len = ustring_length(suffix);
    return s_len <= str_len &&
           memcmp(ustring_data(string) + str_len - s_len, ustring_data(suffix), s_len) == 0;
}

bool ustring_equals(UString lhs, UString rhs) {
    ulib_uint len = ustring_length(lhs);
    return len == ustring_length(rhs) && memcmp(ustring_data(lhs), ustring_data(rhs), len) == 0;
}

bool ustring_precedes(UString lhs, UString rhs) {
    return ustring_compare(lhs, rhs) < 0;
}

int ustring_compare(UString lhs, UString rhs) {
    ulib_uint l_len = ustring_length(lhs), r_len = ustring_length(rhs);
    int const res = memcmp(ustring_data(lhs), ustring_data(rhs), ulib_min(l_len, r_len));
    return res == 0 ? (l_len > r_len) - (l_len < r_len) : res;
}

ulib_uint ustring_hash(UString string) {
#define ulib_cstring_hash_range(HASH, STR, START, END)                                             \
    do {                                                                                           \
        for (ulib_uint i = (START); i < (END); ++i) {                                              \
            (HASH) = ((HASH) << 5u) - (HASH) + (ulib_uint)(STR)[i];                                \
        }                                                                                          \
    } while (0)

    ulib_uint const length = ustring_length(string);
    char const *cstr = ustring_data(string);

    ulib_uint const part_size = 32;
    ulib_uint hash = length;

    if (length <= 3 * part_size) {
        ulib_cstring_hash_range(hash, cstr, 0, length);
    } else {
        ulib_uint const half_idx = length / 2;
        ulib_uint const half_part_size = part_size / 2;
        ulib_cstring_hash_range(hash, cstr, 0, part_size);
        ulib_cstring_hash_range(hash, cstr, half_idx - half_part_size, half_idx + half_part_size);
        ulib_cstring_hash_range(hash, cstr, length - part_size, length);
    }

    return hash;
}

ulib_ret ustring_to_int(UString string, ulib_int *out, unsigned base) {
    char *end;
    char const *start = ustring_data(string);
    ulib_int r = ulib_str_to_int(start, &end, base);
    if (end < start + ustring_length(string)) return ULIB_ERR;
    if (out) *out = r;
    return ULIB_OK;
}

ulib_ret ustring_to_uint(UString string, ulib_uint *out, unsigned base) {
    char *end;
    char const *start = ustring_data(string);
    ulib_uint r = ulib_str_to_uint(start, &end, base);
    if (end < start + ustring_length(string)) return ULIB_ERR;
    if (out) *out = r;
    return ULIB_OK;
}

ulib_ret ustring_to_float(UString string, ulib_float *out) {
    char *end;
    char const *start = ustring_data(string);
    ulib_float r = ulib_str_to_float(start, &end);
    if (end < start + ustring_length(string)) return ULIB_ERR;
    if (out) *out = r;
    return ULIB_OK;
}

UString ustring_with_format(char const *format, ...) {
    va_list args;
    va_start(args, format);
    UString string = ustring_with_format_list(format, args);
    va_end(args);
    return string;
}

UString ustring_with_format_list(char const *format, va_list args) {
    UStrBuf buf = ustrbuf();

    if (ustrbuf_append_format_list(&buf, format, args)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_ustring(&buf);
}

UString ustring_join(UString const *strings, ulib_uint count, UString sep) {
    if (count == 0) return ustring_empty;

    UStrBuf buf = ustrbuf();

    if (ustrbuf_append_ustring(&buf, strings[0])) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    for (ulib_uint i = 1; i < count; ++i) {
        if (ustrbuf_append_ustring(&buf, sep) || ustrbuf_append_ustring(&buf, strings[i])) {
            ustrbuf_deinit(&buf);
            return ustring_null;
        }
    }

    return ustrbuf_to_ustring(&buf);
}

UString ustring_concat(UString const *strings, ulib_uint count) {
    return ustring_join(strings, count, ustring_empty);
}

UString ustring_repeating(UString string, ulib_uint times) {
    ulib_uint len = ustring_length(string);
    char const *data = ustring_data(string);

    UString ret;
    char *buf = ustring(&ret, len * times);
    if (ustring_is_empty(ret)) return ret;

    for (ulib_uint i = 0; i < times; ++i, buf += len) {
        memcpy(buf, data, len);
    }

    return ret;
}

UString ustring_to_upper(UString string) {
    UString ret;
    ulib_uint const len = ustring_length(string);
    char *buf = ustring(&ret, len);
    if (!buf) return ustring_null;
    ulib_str_to_upper(buf, ustring_data(string), len);
    return ret;
}

UString ustring_to_lower(UString string) {
    UString ret;
    ulib_uint const len = ustring_length(string);
    char *buf = ustring(&ret, len);
    if (!buf) return ustring_null;
    ulib_str_to_lower(buf, ustring_data(string), len);
    return ret;
}
