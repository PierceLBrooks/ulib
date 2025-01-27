/**
 * IO streams.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTREAM_H
#define USTREAM_H

#include "ustd.h"
#include "ustrbuf.h"
#include "utime.h"

ULIB_BEGIN_DECLS

/// @cond
typedef struct UString UString;
typedef struct UVersion UVersion;
/// @endcond

/// Return codes for IO streams.
typedef enum ustream_ret {

    /// Success.
    USTREAM_OK = 0,

    /// Buffer bounds exceeded, usually when writing to a stream backed by a fixed memory buffer.
    USTREAM_ERR_BOUNDS,

    /// Memory error, usually caused by failed allocations.
    USTREAM_ERR_MEM,

    /**
     * Input/output error, usually returned when a file or stream operation fails.
     *
     * @note When this happens, *errno* is sometimes set to a more meaningful value.
     */
    USTREAM_ERR_IO,

    /// Generic error.
    USTREAM_ERR

} ustream_ret;

/// Models an input stream.
typedef struct UIStream {

    /// Stream state.
    ustream_ret state;

    /// Bytes read since the last `reset` call.
    size_t read_bytes;

    /// Stream context, can be anything.
    void *ctx;

    /**
     * Pointer to a function that reads `count` bytes from the stream and writes them into `buf`.
     *
     * @param ctx Stream context.
     * @param buf Buffer to write into.
     * @param count Number of bytes to read.
     * @param[out] read Number of bytes actually read.
     * @return Return code.
     */
    ustream_ret (*read)(void *ctx, void *buf, size_t count, size_t *read);

    /**
     * Pointer to a function that resets the stream.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be reset.
     */
    ustream_ret (*reset)(void *ctx);

    /**
     * Pointer to a function that releases any resource reserved by the stream.
     * The provided function is invoked when `uistream_deinit` is called.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream does not need to release resources.
     */
    ustream_ret (*free)(void *ctx);

} UIStream;

/**
 * Initializes an input stream.
 *
 * @param ctx Stream context.
 * @param read_func `read` function pointer.
 * @param reset_func `reset` function pointer.
 * @param free_func `free` function pointer.
 * @return Stream instance.
 *
 * @public @memberof UIStream
 */
ULIB_INLINE
UIStream uistream(void *ctx, ustream_ret (*read_func)(void *, void *, size_t, size_t *),
                  ustream_ret (*reset_func)(void *), ustream_ret (*free_func)(void *)) {
    UIStream s = { USTREAM_OK, 0, ctx, read_func, reset_func, free_func };
    return s;
}

/**
 * Deinitializes the stream, releasing any reserved resource.
 *
 * @param stream Input stream.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_deinit(UIStream *stream);

/**
 * Resets the stream.
 *
 * @param stream Input stream.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_reset(UIStream *stream);

/**
 * Reads `count` bytes from the stream and writes them into `buf`.
 *
 * @param stream Input stream.
 * @param buf Input buffer.
 * @param count Maximum number of bytes to read.
 * @param[out] read Number of bytes read.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_read(UIStream *stream, void *buf, size_t count, size_t *read);

/**
 * Returns a stream that reads from the standard input.
 *
 * @return Standard input stream.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
UIStream *uistream_std(void);

/**
 * Initializes a stream that reads from the file at the specified path.
 *
 * @param stream Input stream.
 * @param path Path to the file to read from.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_from_path(UIStream *stream, char const *path);

/**
 * Initializes a stream that reads from the specified file.
 *
 * @param stream Input stream.
 * @param file The input file.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_from_file(UIStream *stream, FILE *file);

/**
 * Initializes a stream that reads from the specified buffer.
 *
 * @param stream Input stream.
 * @param buf The input buffer.
 * @param size Size of the input buffer.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_from_buf(UIStream *stream, void const *buf, size_t size);

/**
 * Initializes a stream that reads from the specified string buffer.
 *
 * @param stream Input stream.
 * @param buf String buffer.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_from_strbuf(UIStream *stream, UStrBuf const *buf);

/**
 * Initializes a stream that reads from the specified null-terminated string.
 *
 * @param stream Input stream.
 * @param string String.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_from_string(UIStream *stream, char const *string);

/**
 * Initializes a stream that reads from the specified string.
 *
 * @param stream Input stream.
 * @param string String.
 * @return Return code.
 *
 * @public @memberof UIStream
 */
ULIB_PUBLIC
ustream_ret uistream_from_ustring(UIStream *stream, UString const *string);

/// Models an output stream.
typedef struct UOStream {

    /// Stream state.
    ustream_ret state;

    /// Bytes written since the last `flush` call.
    size_t written_bytes;

    /// Stream context, can be anything.
    void *ctx;

    /**
     * Pointer to a function that reads `count` bytes from `buf` and writes them into the stream.
     *
     * @param ctx Stream context.
     * @param buf Buffer to read from.
     * @param count Number of bytes to write.
     * @param[out] rcount Number of bytes actually written.
     * @return Return code.
     */
    ustream_ret (*write)(void *ctx, void const *buf, size_t count, size_t *written);

    /**
     * Pointer to a function that writes a formatted string into the stream.
     *
     * @param ctx Stream context.
     * @param[out] written Number of bytes written.
     * @param format Format string.
     * @param args Format arguments.
     * @return Return code.
     *
     * @note Can be NULL, in which case the stream will fallback to `write`.
     */
    ustream_ret (*writef)(void *ctx, size_t *written, char const *format, va_list args);

    /**
     * Pointer to a function that flushes the stream, writing any buffered data.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be flushed.
     */
    ustream_ret (*flush)(void *ctx);

    /**
     * Pointer to a function that releases any resource reserved by the stream.
     * The provided function is invoked when `uostream_deinit` is called.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream does not need to release resources.
     */
    ustream_ret (*free)(void *ctx);

} UOStream;

/**
 * Initializes an output stream.
 *
 * @param ctx Stream context.
 * @param write_func `write` function pointer.
 * @param writef_func `writef` function pointer.
 * @param flush_func `flush` function pointer.
 * @param free_func `free` function pointer.
 * @return Stream instance.
 *
 * @public @memberof UOStream
 */
ULIB_INLINE
UOStream uostream(void *ctx, ustream_ret (*write_func)(void *, void const *, size_t, size_t *),
                  ustream_ret (*writef_func)(void *, size_t *, char const *, va_list),
                  ustream_ret (*flush_func)(void *), ustream_ret (*free_func)(void *)) {
    UOStream s = { USTREAM_OK, 0, ctx, write_func, writef_func, flush_func, free_func };
    return s;
}

/**
 * Writes the specified string literal into the stream.
 *
 * @param stream [UOStream *] Output stream.
 * @param literal [char const []] String literal.
 * @param[out] written [size_t *] Number of bytes written.
 * @return Return code.
 *
 * @public @related UOStream
 */
#define uostream_write_literal(stream, literal, written)                                           \
    uostream_write(stream, literal, sizeof(literal) - 1, written)

/**
 * Deinitializes the stream, releasing any reserved resource.
 *
 * @param stream Output stream.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_deinit(UOStream *stream);

/**
 * Flushes the stream, writing any buffered data.
 *
 * @param stream Output stream.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_flush(UOStream *stream);

/**
 * Writes `count` bytes from `buf` into the specified output stream.
 *
 * @param stream Output stream.
 * @param buf Buffer.
 * @param count Number of bytes to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_write(UOStream *stream, void const *buf, size_t count, size_t *written);

/**
 * Writes a formatted string into the stream.
 *
 * @param stream Output stream.
 * @param[out] written Number of bytes written.
 * @param format Format string.
 * @param ... Format arguments.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_writef(UOStream *stream, size_t *written, char const *format, ...);

/**
 * Writes a formatted string into the stream.
 *
 * @param stream Output stream.
 * @param[out] written Number of bytes written.
 * @param format Format string.
 * @param args Format arguments.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret
uostream_writef_list(UOStream *stream, size_t *written, char const *format, va_list args);

/**
 * Writes a string into the stream.
 *
 * @param stream Output stream.
 * @param string String.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_write_string(UOStream *stream, UString const *string, size_t *written);

/**
 * Writes the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_write_time(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the specified time interval into the stream.
 *
 * @param stream Output stream.
 * @param interval Time interval.
 * @param unit Time unit.
 * @param decimal_digits Number of decimal digits to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_write_time_interval(UOStream *stream, utime_ns interval, utime_unit unit,
                                         unsigned decimal_digits, size_t *written);

/**
 * Writes the specified version into the stream.
 *
 * @param stream Output stream.
 * @param version Version.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_write_version(UOStream *stream, UVersion const *version, size_t *written);

/**
 * Returns a stream that writes to the standard output.
 *
 * @return Standard output stream.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
UOStream *uostream_std(void);

/**
 * Returns a stream that writes to the standard error.
 *
 * @return Standard error stream.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
UOStream *uostream_stderr(void);

/**
 * Returns a stream that discards its output.
 *
 * @return Null output stream.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
UOStream *uostream_null(void);

/**
 * Initializes a stream that writes to the file at the specified path.
 *
 * @param stream Output stream.
 * @param path Path to the file to write to.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_to_path(UOStream *stream, char const *path);

/**
 * Initializes a stream that writes to the specified file.
 *
 * @param stream Output stream.
 * @param file The output file.
 * @return Return code.
 *
 * @note You are responsible for closing the file.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_to_file(UOStream *stream, FILE *file);

/**
 * Initializes a stream that writes to the specified buffer.
 *
 * @param stream Output stream.
 * @param buf The output buffer.
 * @param size Size of the output buffer.
 * @return Return code.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_to_buf(UOStream *stream, void *buf, size_t size);

/**
 * Initializes a stream that writes to the specified string buffer.
 *
 * @param stream Output stream.
 * @param buf The output buffer.
 * @return Return code.
 *
 * @note If `buf` is NULL, the stream will allocate a new string buffer and set it as its context.
 *       In this case, the string buffer will be deinitialized when calling `uostream_deinit`.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_to_strbuf(UOStream *stream, UStrBuf *buf);

/**
 * Initializes a stream that writes to multiple substreams.
 *
 * @param stream Output stream.
 * @return Return code.
 *
 * @note Multi-streams behave as follows:
 *       - In case of error of any of the substreams, only the first detected error code
 *         is returned. It is your responsibility to check the state of each individual
 *         substream if that is important for your use case.
 *       - The reported written bytes are the maximum bytes written by any of the underlying
 *         substreams.
 *       - Calling `uostream_deinit` deinitializes all substreams.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_to_multi(UOStream *stream);

/**
 * Adds a new output stream to the specified multi-stream.
 *
 * @param stream Output stream.
 * @param other Stream to add.
 * @return Return code.
 *
 * @note Both streams must have been initialized beforehand, and `stream`
 *       must have been initialized via `uostream_to_multi`.
 *
 * @public @memberof UOStream
 */
ULIB_PUBLIC
ustream_ret uostream_add_substream(UOStream *stream, UOStream const *other);

ULIB_END_DECLS

#endif // USTREAM_H
