/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustream.h"
#include "ustring.h"
#include "uversion.h"
#include <stdarg.h>

typedef struct UStreamBuf {
    size_t size;
    char *orig;
    char *cur;
} UStreamBuf;

static ustream_ret ustream_file_read(void *file, void *buf, size_t count, size_t *read) {
    size_t const read_size = fread(buf, 1, count, file);
    if (read) *read = read_size;
    return count != read_size && ferror(file) ? USTREAM_ERR_IO : USTREAM_OK;
}

static ustream_ret ustream_file_write(void *file, void const *buf, size_t count, size_t *written) {
    size_t const written_size = fwrite(buf, 1, count, file);
    if (written) *written = written_size;
    return count != written_size ? USTREAM_ERR_IO : USTREAM_OK;
}

static ustream_ret ustream_file_writef(void *file, size_t *written,
                                       char const *format, va_list args) {
    int pf_ret = vfprintf(file, format, args);
    ustream_ret ret;
    size_t written_size;

    if (pf_ret < 0) {
        written_size = 0;
        ret = USTREAM_ERR_IO;
    } else {
        written_size = (size_t)pf_ret;
        ret = USTREAM_OK;
    }

    if (written) *written = written_size;
    return ret;
}

static ustream_ret ustream_file_reset(void *file) {
    rewind(file);
    return USTREAM_OK;
}

static ustream_ret ustream_file_flush(void *file) {
    return fflush(file) == 0 ? USTREAM_OK : USTREAM_ERR_IO;
}

static ustream_ret ustream_file_close(void *file) {
    return fclose(file) == 0 ? USTREAM_OK : USTREAM_ERR_IO;
}

static ustream_ret ustream_buf_read(void *ctx, void *buf, size_t count, size_t *read) {
    UStreamBuf *ibuf = ctx;
    size_t const read_size = count < ibuf->size ? count : ibuf->size;
    memcpy(buf, ibuf->cur, read_size);
    ibuf->cur += read_size;
    ibuf->size -= read_size;
    if (read) *read = read_size;
    return USTREAM_OK;
}

static ustream_ret ustream_buf_write(void *ctx, void const *buf, size_t count, size_t *written) {
    UStreamBuf *ibuf = ctx;
    ustream_ret ret;
    size_t written_size;

    if (count > ibuf->size) {
        ret = USTREAM_ERR_BOUNDS;
        written_size = ibuf->size;
    } else {
        ret = USTREAM_OK;
        written_size = count;
    }

    memcpy(ibuf->cur, buf, written_size);
    ibuf->cur += written_size;
    ibuf->size -= written_size;
    if (written) *written = written_size;

    return ret;
}

static ustream_ret ustream_buf_writef(void *ctx, size_t *written,
                                      char const *format, va_list args) {
    UStreamBuf *ibuf = ctx;
    int pf_ret = vsnprintf(ibuf->cur, ibuf->size, format, args);
    ustream_ret ret;
    size_t written_size;

    if (pf_ret < 0) {
        written_size = 0;
        ret = USTREAM_ERR_IO;
    } else if ((size_t)pf_ret < ibuf->size) {
        written_size = (size_t)pf_ret;
        ret = USTREAM_OK;
    } else {
        written_size = ibuf->size;
        ret = USTREAM_ERR_BOUNDS;
    }

    ibuf->cur += written_size;
    ibuf->size -= written_size;
    if (written) *written = written_size;

    return ret;
}

static ustream_ret ustream_buf_reset(void *ctx) {
    UStreamBuf *ibuf = ctx;
    ibuf->size += ibuf->cur - ibuf->orig;
    ibuf->cur = ibuf->orig;
    return USTREAM_OK;
}

static ustream_ret ustream_buf_free(void *buf) {
    ulib_free(buf);
    return USTREAM_OK;
}

static ustream_ret ustream_strbuf_write(void *ctx, void const *buf, size_t count, size_t *written) {
    UStrBuf *str_buf = ctx;
    ulib_uint start_count = uvec_count(str_buf);
    uvec_ret ret = ustrbuf_append_string(str_buf, buf, (ulib_uint)count);
    if (written) *written = uvec_count(str_buf) - start_count;
    return ret == UVEC_OK ? USTREAM_OK : USTREAM_ERR_MEM;
}

static ustream_ret ustream_strbuf_writef(void *ctx, size_t *written,
                                         char const *format, va_list args) {
    UStrBuf *str_buf = ctx;
    ulib_uint start_count = uvec_count(str_buf);
    uvec_ret ret = ustrbuf_append_format_list(str_buf, format, args);
    if (written) *written = uvec_count(str_buf) - start_count;
    return ret == UVEC_OK ? USTREAM_OK : USTREAM_ERR_MEM;
}

static ustream_ret ustream_strbuf_free(void *ctx) {
    ustrbuf_deinit(ctx);
    ulib_free(ctx);
    return USTREAM_OK;
}

static ustream_ret ustream_null_write(ulib_unused void *ctx, ulib_unused void const *buf,
                                      ulib_unused size_t count, size_t *written) {
    if (written) *written = 0;
    return USTREAM_OK;
}

ustream_ret uistream_deinit(UIStream *stream) {
    return stream->state = stream->free ? stream->free(stream->ctx) : USTREAM_OK;
}

ustream_ret uistream_reset(UIStream *stream) {
    return stream->state = stream->reset ? stream->reset(stream->ctx) : USTREAM_OK;
}

ustream_ret uistream_read(UIStream *stream, void *buf, size_t count, size_t *read) {
    if (!stream->state) stream->state = stream->read(stream->ctx, buf, count, read);
    return stream->state;
}

ustream_ret uistream_from_path(UIStream *stream, char const *path) {
    FILE *in_file = fopen(path, "rb");
    ustream_ret ret = uistream_from_file(stream, in_file);
    if (ret == USTREAM_OK) stream->free = ustream_file_close;
    return ret;
}

ustream_ret uistream_from_file(UIStream *stream, FILE *file) {
    ustream_ret state = file ? USTREAM_OK : USTREAM_ERR_IO;
    *stream = (UIStream) { .state = state };
    if (state == USTREAM_OK) {
        stream->ctx = file;
        stream->read = ustream_file_read;
        stream->reset = ustream_file_reset;
    }
    return state;
}

ustream_ret uistream_from_buf(UIStream *stream, void const *buf, size_t size) {
    UStreamBuf *raw_buf = ulib_alloc(raw_buf);
    ustream_ret state = raw_buf ? USTREAM_OK : USTREAM_ERR_MEM;
    *stream = (UIStream) { .state = state };
    if (state == USTREAM_OK) {
        raw_buf->orig = raw_buf->cur = (void *)buf;
        raw_buf->size = size;
        stream->ctx = raw_buf;
        stream->read = ustream_buf_read;
        stream->reset = ustream_buf_reset;
        stream->free = ustream_buf_free;
    }
    return state;
}

ustream_ret uistream_from_strbuf(UIStream *stream, UStrBuf const *buf) {
    return uistream_from_buf(stream, ustrbuf_data(buf), ustrbuf_size(buf));
}

ustream_ret uistream_from_string(UIStream *stream, char const *string) {
    return uistream_from_buf(stream, string, strlen(string));
}

ustream_ret uistream_from_ustring(UIStream *stream, UString const *string) {
    return uistream_from_buf(stream, ustring_data(*string), ustring_length(*string));
}

ustream_ret uostream_deinit(UOStream *stream) {
    return stream->state = stream->free ? stream->free(stream->ctx) : USTREAM_OK;
}

ustream_ret uostream_flush(UOStream *stream) {
    return stream->state = stream->flush ? stream->flush(stream->ctx) : USTREAM_OK;
}

ustream_ret uostream_write(UOStream *stream, void const *buf, size_t count, size_t *written) {
    if (!stream->state) stream->state = stream->write(stream->ctx, buf, count, written);
    return stream->state;
}

ustream_ret uostream_writef(UOStream *stream, size_t *written, char const *format, ...) {
    va_list args;
    va_start(args, format);
    ustream_ret ret = uostream_writef_list(stream, written, format, args);
    va_end(args);
    return ret;
}

static ustream_ret uostream_writef_list_fallback(UOStream *stream, size_t *written,
                                                 char const *format, va_list args) {
    size_t len = ulib_str_flength_list(format, args);
    size_t size = len + 1;
    char *buf = ulib_malloc(size);

    if (buf) {
        vsnprintf(buf, size, format, args);
        stream->state = uostream_write(stream, buf, len, written);
        ulib_free(buf);
    } else {
        if (written) *written = 0;
        stream->state = USTREAM_ERR_MEM;
    }

    return stream->state;
}

ustream_ret uostream_writef_list(UOStream *stream, size_t *written,
                                 char const *format, va_list args) {
    if (!stream->state) {
        if (stream->writef) {
            stream->state = stream->writef(stream->ctx, written, format, args);
        } else {
            stream->state = uostream_writef_list_fallback(stream, written, format, args);
        }
    }
    return stream->state;
}

ustream_ret uostream_write_string(UOStream *stream, UString const *string, size_t *written) {
    return uostream_write(stream, ustring_data(*string), ustring_length(*string), written);
}

ustream_ret uostream_write_time(UOStream *stream, UTime const *time, size_t *written) {
    return uostream_writef(stream, written, "%lld/%02u/%02u-%02u:%02u:%02u",
                           time->year, time->month, time->day,
                           time->hour, time->minute, time->second);
}

ustream_ret uostream_write_time_interval(UOStream *stream, utime_ns interval, utime_unit unit,
                                         unsigned decimal_digits, size_t *written) {
    static char const* str[] = { "ns", "us", "ms", "s", "m", "h", "d" };
    unit = ulib_clamp(unit, UTIME_NANOSECONDS, UTIME_DAYS);
    double c_interval = utime_interval_convert(interval, unit);
    return uostream_writef(stream, written, "%.*f %s", decimal_digits, c_interval, str[unit]);
}

ustream_ret uostream_write_version(UOStream *stream, UVersion const *version, size_t *written) {
    return uostream_writef(stream, written, "%u.%u.%u",
                           version->major, version->minor, version->patch);
}

ustream_ret uostream_to_path(UOStream *stream, char const *path) {
    FILE *out_file = fopen(path, "wb");
    ustream_ret ret = uostream_to_file(stream, out_file);
    if (ret == USTREAM_OK) stream->free = ustream_file_close;
    return ret;
}

ustream_ret uostream_to_file(UOStream *stream, FILE *file) {
    ustream_ret state = file ? USTREAM_OK : USTREAM_ERR_IO;
    *stream = (UOStream) { .state = state };
    if (state == USTREAM_OK) {
        stream->ctx = file;
        stream->write = ustream_file_write;
        stream->writef = ustream_file_writef;
        stream->flush = ustream_file_flush;
    }
    return state;
}

ustream_ret uostream_to_buf(UOStream *stream, void *buf, size_t size) {
    UStreamBuf *raw_buf = ulib_alloc(raw_buf);
    ustream_ret state = raw_buf ? USTREAM_OK : USTREAM_ERR_MEM;
    *stream = (UOStream) { .state = state };
    if (state == USTREAM_OK) {
        raw_buf->orig = raw_buf->cur = buf;
        raw_buf->size = size;
        stream->ctx = raw_buf;
        stream->write = ustream_buf_write;
        stream->writef = ustream_buf_writef;
        stream->free = ustream_buf_free;
    }
    return state;
}

ustream_ret uostream_to_strbuf(UOStream *stream, UStrBuf *buf) {
    *stream = (UOStream) { .state = USTREAM_OK };

    if (!buf) {
        if ((buf = ulib_alloc(buf))) {
            *buf = ustrbuf_init();
            stream->free = ustream_strbuf_free;
        } else {
            stream->state = USTREAM_ERR_MEM;
        }
    }

    if (stream->state == USTREAM_OK) {
        stream->ctx = buf;
        stream->write = ustream_strbuf_write;
        stream->writef = ustream_strbuf_writef;
    }

    return stream->state;
}

ustream_ret uostream_to_null(UOStream *stream) {
    *stream = (UOStream) {
        .state = USTREAM_OK,
        .write = ustream_null_write
    };
    return USTREAM_OK;
}
