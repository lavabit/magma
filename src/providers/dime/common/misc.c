#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <openssl/x509.h>
#include <openssl/err.h>
#include "dime/common/misc.h"
#include "dime/common/error.h"

#include "providers/symbols.h"

int _verbose = 0;

static const unsigned char
_base64_chars[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char
_base64_vals[128] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63, 52, 53,
  54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0,
  0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
  42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
};

/**
 * @brief
 *  Return the base64-encoded string of a data buffer without padding.
 * @note
 *  This function ensures the smallest possible output length with no trailing
 *  '=' characters.
 * @param buf
 *  a pointer to the data buffer to be base-64 encoded.
 * @param len
 *  the length, in bytes, of the buffer to be encoded.
 * @return
 *  a pointer to a newly allocated null-terminated string containing the
 *  base64-encoded data, or NULL on failure.
 * @free_using{free}
 */
char *
_b64encode_nopad(
    unsigned char const *buf,
    size_t len)
{
    char *result, *ptr;

    if (!buf || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(result = _b64encode(buf, len))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base64 encode input");
    }

    ptr = result + strlen(result) - 1;

    while ((ptr >= result) && (*ptr == '=')) {
        *ptr-- = 0;
    }

    return result;
}


/**
 * @brief
 *  Decode a base64-encoded string without any padding characters and return
 *  the result.
 * @param buf
 *  a pointer to a buffer containing the data to be decoded.
 * @param len
 *  the length, in bytes, of the buffer to be decoded.
 * @param outlen
 *  a pointer to a variable to receive the length of the decoded data.
 * @return
 *  a pointer to a newly allocated buffer containing the base64-decoded, or
 *  NULL on failure.
 * @free_using{free}
 */
unsigned char *
_b64decode_nopad(
    char const *buf,
    size_t len,
    size_t *outlen)
{
    unsigned char *result;
    char *padded;
    size_t padlen;

    if (!buf || !len || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(len % 4)) {
        return (_b64decode(buf, len, outlen));
    }

    padlen = (len + 4) & ~(3);

    if (!(padded = malloc(padlen + 1))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for temporary string");
    }

    memset(padded, '=', padlen);
    padded[padlen] = 0;
    memcpy(padded, buf, len);

    result = _b64decode(padded, padlen, outlen);
    free(padded);

    return result;
}


/**
 * @brief
 *  Store a 4-byte value in a buffer in network byte order.
 * @param buf
 *  a pointer to the data buffer where the value will be inserted.
 * @param val
 *  the 4 byte value to be placed in the specified buffer, in network byte
 *  order.
 */
void
_int_no_put_4b(void *buf, uint32_t val)
{
    unsigned char *bptr = (unsigned char *)buf;

    if (!buf) {
        return;
    }

    bptr[0] = (val & 0xff000000) >> 24;
    bptr[1] = (val & 0x00ff0000) >> 16;
    bptr[2] = (val & 0x0000ff00) >> 8;
    bptr[3] = val & 0x000000ff;
}


/**
 * @brief
 *  Store a 3-byte value in a buffer in network byte order.
 * @param buf
 *  a pointer to the data buffer where the value will be inserted.
 * @param val
 *  the 3 byte value to be placed in the specified buffer, in network byte
 *  order.
 */
void
_int_no_put_3b(void *buf, uint32_t val)
{
    unsigned char *bptr = (unsigned char *)buf;

    if (!buf) {
        return;
    }

    bptr[0] = (val & 0x00ff0000) >> 16;
    bptr[1] = (val & 0x0000ff00) >> 8;
    bptr[2] = val & 0x000000ff;
}


/**
 * @brief
 *  Store a 2-byte value in a buffer in network byte order.
 * @param buf
 *  a pointer to the data buffer where the value will be inserted.
 * @param val
 *  the 2 byte value to be placed in the specified buffer, in network byte
 *  order.
 */
void
_int_no_put_2b(void *buf, uint16_t val)
{
    unsigned char *bptr = (unsigned char *)buf;

    if (!buf) {
        return;
    }

    bptr[0] = (val & 0x0000ff00) >> 8;
    bptr[1] = val & 0x000000ff;
}

/**
 * @brief
 *  Fetch a 4 byte value from a buffer and return it in host byte order.
 * @param buf
 *  a pointer to the data buffer from which the bytes will be read.
 * @return
 *  the value of the first 4 bytes in the buffer in host byte order.
 */
uint32_t
_int_no_get_4b(const void *buf)
{
    uint32_t result = 0;
    unsigned char *sptr = (unsigned char *)buf;

    if (!buf) {
        return 0;
    }

    result |= (sptr[0] << 24);
    result |= (sptr[1] << 16);
    result |= (sptr[2] << 8);
    result |= sptr[3];

    return result;
}

/**
 * @brief
 *  Fetch a 3 byte value from a buffer and return it in host byte order.
 * @param buf
 *  a pointer to the data buffer from which the bytes will be read.
 * @return
 *  the value of the first 3 bytes in the buffer in host byte order.
 */
uint32_t
_int_no_get_3b(const void *buf)
{
    uint32_t result = 0;
    unsigned char *sptr = (unsigned char *)buf;

    if (!buf) {
        return 0;
    }

    result |= (sptr[0] << 16);
    result |= (sptr[1] << 8);
    result |= sptr[2];

    return result;
}


/**
 * @brief
 *  Fetch a 2 byte value from a buffer and return it in host byte order.
 * @param buf
 *  a pointer to the data buffer from which the bytes will be read.
 * @return
 *  the value of the first 2 bytes in the buffer in host byte order.
 */
uint16_t
_int_no_get_2b(const void *buf)
{
    uint16_t result = 0;
    unsigned char *sptr = (unsigned char *)buf;

    if (!buf) {
        return 0;
    }

    result |= (sptr[0] << 8);
    result |= sptr[1];

    return result;
}


/**
 * @brief
 *  Return a simple hex string representing a data buffer.
 * @param buf
 *  a pointer to the data buffer to be encoded as a hex string.
 * @param len
 *  the size, in bytes, of the buffer to be processed.
 * @return
 *  NULL on failure, or a newly allocated null-terminated string containing the
 *  hex encoded input on success.
 * @free_using{free}
 */
char *
_hex_encode(unsigned char const *buf, size_t len)
{
    char *result;
    size_t newlen;

    if (!buf || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    newlen = (len * 2) + 1;

    if (!(result = malloc(newlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for hex string");
    }

    memset(result, 0, newlen);

    for (size_t i = 0; i < len; i++) {
        snprintf(&(result[i * 2]), 3, "%.2x", (unsigned char)buf[i]);
    }

    return result;
}

/**
 * @brief
 *  Set the debugging level of the current process.
 * @note
 *  The debugging level determines which debug strings are displayed to the
 *  console.  If a debug string is printed with a level equal to or higher than
 *  the debugging level, it will be displayed to the user; otherwise it will be
 *  suppressed.
 * @param level
 *  the value of the new debugging level to be set.
 */
void
_set_dbg_level(unsigned int level)
{
    _verbose = level;
}


/**
 * @brief
 *  Get the debugging level of the current process.
 * @return
 *  the current value of the debugging level.
 */
unsigned int
_get_dbg_level(void)
{
    return _verbose;
}

/**
 * @brief
 *  Append variable-argument formatted data to a dynamically allocated
 *  null-terminated string.
 */
int
_str_printf(char **sbuf, const char *fmt, ...)
{
    va_list ap;
    int result;

    va_start(ap, fmt);
    result = __str_printf(sbuf, fmt, ap);
    va_end(ap);

    return result;
}

/**
 * @brief
 *  Append variable-argument formatted data to a dynamically allocated
 *  null-terminated string.
 */
int
__str_printf(
    char **sbuf,
    const char *fmt,
    va_list ap)
{
    va_list copy;
    char *result, *endptr, tmp;
    size_t total, more;
    int retval;

    if (!sbuf || !fmt) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    va_copy(copy, ap);
    total = more = vsnprintf(&tmp, 1, fmt, copy) + 1;
    va_end(copy);

    // We already have an allocated string so we need to append to it.
    if (*sbuf) {
        total += strlen(*sbuf);
    }

    if (!(result = realloc(*sbuf, total))) {
        PUSH_ERROR_SYSCALL("realloc");

        if (*sbuf) {
            free(*sbuf);
        }

        RET_ERROR_INT(ERR_NOMEM, "unable to reallocate more space for string");
    }

    if (!*sbuf) {
        memset(result, 0, total);
    }

    endptr = result + strlen(result);

    va_copy(copy, ap);
    retval = vsnprintf(endptr, more + 1, fmt, copy);
    va_end(copy);

    *sbuf = result;
    return retval;
}

/**
 * @brief
 *  Append data to a dynamically allocated buffer.
 * @note
 *  This function should be called for the first time with *buf set to NULL.
 * @param buf
 *  a pointer to the address of the output buffer that will be resized to hold
 *  the result, or NULL to allocate one.
 * @param blen
 *  a pointer to a variable that holds the buffer's current size and will
 *  receive the size of the newly resized output buffer.
 * @param data
 *  a pointer to a buffer holding the data that will be appended to the end of
 *  the output buffer.
 * @param dlen
 *  the size, in bytes, of the data buffer to be appended to the output buffer.
 * @return
 *  the new size of the (re)allocated output buffer, or 0 on failure.
 */
size_t
_mem_append(
    unsigned char **buf,
    size_t *blen,
    unsigned char const *data,
    size_t dlen)
{
    void *obuf;

    if (!buf || !blen || !data) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    if (_verbose >= 7) {
        _dbgprint(7, "_mem_append() called: ");
        _dump_buf(data, dlen, 1);
    }

    if (!*buf) {

        if (!(*buf = malloc(dlen))) {
            PUSH_ERROR_SYSCALL("malloc");
            RET_ERROR_UINT(
                ERR_NOMEM,
                "unable to allocate memory for buffer expansion");
        }

        memcpy(*buf, data, dlen);
        *blen = dlen;
    } else {

        if (!(*buf = realloc((obuf = *buf), *blen + dlen))) {
            PUSH_ERROR_SYSCALL("realloc");
            free(obuf);
            RET_ERROR_UINT(
                ERR_NOMEM,
                "unable to reallocate memory for buffer expansion");
        }

        memcpy(*buf + *blen, data, dlen);
        *blen = *blen + dlen;
    }

    return *blen;
}

/**
 * @brief
 *  Free a pointer chain and all its member elements.
 * @note
 *  All pointers in the chain will be free()'ed. To avoid this behavior, call
 *  free() on the pointer chain instead.
 * @param
 *  buf the address of the pointer chain to be freed.
 */
void
_ptr_chain_free(void *buf)
{
    unsigned char **memptr = (unsigned char **)buf;

    if (!buf) {
        return;
    }

    while (*memptr) {
        free(*memptr);
        memptr++;
    }

    free(buf);
}

/**
 * @brief
 *  Append an address to the end of a pointer chain.
 * @param buf
 *  the address of the target pointer chain. If NULL, allocate a new one for
 *  the caller.
 * @param addr
 *  the address to be appended to the pointer chain.
 */
void *
_ptr_chain_add(void *buf, const void *addr)
{
    unsigned char **newbuf, **ptr = (unsigned char **)buf, **obuf;
    size_t bufsize = sizeof(unsigned char *);

    // Get the entire size of the buffer, if it's not NULL.
    if (buf) {

        while (*ptr) {
            ptr++;
        }

        ptr++;
        bufsize = (unsigned long)ptr - (unsigned long)buf;
    }

    // Reallocate with space for one more pointer at the end of it.
    if (!(newbuf = realloc((obuf = buf), bufsize + sizeof(unsigned char *)))) {
        PUSH_ERROR_SYSCALL("realloc");

        if (obuf) {
            free(obuf);
        }

        RET_ERROR_PTR(ERR_NOMEM, "unable to lengthen pointer chain");
    }

    if (!buf) {
        ptr = (unsigned char **)newbuf;
    } else {
        ptr =
            (unsigned char **)((unsigned char *)newbuf
            + bufsize
            - sizeof(unsigned char *));
    }

    // Replace the next-to-last entry in the new pointer chain with our
    // address, then follow it with a terminating NULL pointer.
    *ptr++ = (unsigned char *)addr;
    *ptr = NULL;

    return newbuf;
}

/**
 * @brief
 *  Count the number of elements in a pointer chain.
 * @param buf
 *  the address of the pointer chain to be counted.
 * @return
 *  the number of elements in the pointer chain, or -1 on general error.
 */
int
_count_ptr_chain(void *buf)
{
    unsigned char **memptr = (unsigned char **)buf;
    unsigned int result = 0;

    if (!buf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    while (*memptr) {
        memptr++;
        result++;
    }

    return result;
}

/**
 * @brief
 *  Clone a pointer chain.
 * @param buf
 *  a pointer to the pointer chain to be cloned.
 * @return
 *  a pointer to the cloned pointer chain on success, or NULL on failure.
 * @free_usingref{ptr_chain_free}
 */
void *
_ptr_chain_clone(void *buf)
{
    unsigned char **result, **inptr = (unsigned char **)buf;
    size_t totsize;
    int nitems;

    if (!buf) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if ((nitems = _count_ptr_chain(buf)) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to count items in pointer chain");
    } else if (!nitems) {
        RET_ERROR_PTR(ERR_UNSPEC, "cannot clone empty pointer chain");
    }

    totsize = (nitems + 1) * sizeof(unsigned char *);

    if (!(result = malloc(totsize))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(result, 0, totsize);

    for (int i = 0; i < nitems; i++) {
        result[i] = inptr[i];
    }

    return result;
}

/**
 * @brief
 *  Get the human-readable, US-specific representation of a specified UNIX
 *  time.
 * @param time
 *  the time to be converted to a human-readable string.
 * @param local
 *  if set, return the time string as a local time; if not, as UTC.
 * @return
 *  a string containing the formatted date-time; or NULL on errors.
 * @free_using{free}
 */
char *
_get_chr_date(time_t time, int local)
{
    struct tm tmr;
    char tbuf[64];
    char *result;
    const char *local_fmt = "%H:%M:%S %a %b %d %Y";
    char const *utc_fmt = "%H:%M:%S %a %b %d %Y (UTC)";

    if (local && (!localtime_r(&time, &tmr))) {
        RET_ERROR_PTR(ERR_UNSPEC, "error occurred formatting local date-time");
    } else if (!local && (!gmtime_r(&time, &tmr))) {
        RET_ERROR_PTR(ERR_UNSPEC, "error occurred formatting UTC date-time");
    }

    memset(tbuf, 0, sizeof(tbuf));

    if (!strftime(tbuf, sizeof(tbuf), (local ? local_fmt : utc_fmt), &tmr)) {
        RET_ERROR_PTR(ERR_UNSPEC, "error occurred packaging date-time");
    }

    if (!(result = strdup(tbuf))) {
        PUSH_ERROR_SYSCALL("strdup");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate space for date-time string");
    }

    return result;
}

/**
 * @brief
 *  Perform base64-decoding on a string and return the result.
 * @param buf
 *  a pointer to a buffer containing the data to be decoded.
 * @param len
 *  the length, in bytes, of the buffer to be decoded.
 * @param outlen
 *  a pointer to a variable to receive the length of the decoded data.
 * @return
 *  a pointer to a newly allocated buffer containing the base64-decoded, or
 *  NULL on failure.
 * @free_using{free}
 */
unsigned char *
_b64decode(
    char const *buf,
    size_t len,
    size_t *outlen)
{
    unsigned char *o, *result;
    const char *p;
    size_t new_len, written = 0;
    int loop = 0, value = 0;

    if (!buf || !len || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    new_len = B64_DECODED_LEN(len);

    if (!(result = malloc(new_len))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "unable to allocate buffer for base64 decoded data");
    }

    memset(result, 0, new_len);

    o = result;
    p = buf;

    // Get four characters at a time from the input buffer and decode them.
    for (size_t i = 0; i < len; i++) {

        // Only process legit base64 characters.
        if ((*p >= 'A' && *p <= 'Z')
            || (*p >= 'a' && *p <= 'z')
            || (*p >= '0' && *p <= '9')
            || *p == '+'
            || *p == '/')
        {
            // Do the appropriate operation.
            switch (loop) {

            case 0:
                value = _base64_vals[(int)*p++] << 18;
                loop++;
                break;
            case 1:
                value += _base64_vals[(int)*p++] << 12;
                *o++ = (value & 0x00ff0000) >> 16;
                written++;
                loop++;
                break;
            case 2:
                value += (unsigned int)_base64_vals[(int)*p++] << 6;
                *o++ = (value & 0x0000ff00) >> 8;
                written++;
                loop++;
                break;
            case 3:
                value += (unsigned int)_base64_vals[(int)*p++];
                *o++ = value & 0x000000ff;
                written++;
                loop = 0;
                break;
            default:
                free(result);
                RET_ERROR_PTR(
                    ERR_UNSPEC,
                    "an unexpected error occurred during base64 decoding");
                break;
            }

        } else if (*p == '=') {
            i = len;
        } else {
            p++;
        }

    }

    *outlen = written;

    return result;
}


/**
 * @brief
 *  Return the base64-encoded string of a data buffer.
 * @param buf
 *  a pointer to the data buffer to be base-64 encoded.
 * @param len
 *  the length, in bytes, of the buffer to be encoded.
 * @return
 *  a pointer to a newly allocated null-terminated string containing the
 *  base64-encoded data, or NULL on failure.
 * @free_using{free}
 */
char *
_b64encode(unsigned char const *buf, size_t len)
{
    unsigned char *o;
    const unsigned char *p;
    char *result;
    size_t new_len;
    unsigned char c1, c2, c3;

    if (!buf || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    new_len = B64_ENCODED_LEN(len) + 4;

    if (!(result = malloc(new_len))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate space for base64-encoded string");
    }

    memset(result, 0, new_len);

    o = (unsigned char *)result;
    p = buf;

    // This will process three bytes at a time.
    for(size_t i = 0; i < len / 3; ++i) {
        c1 = (*p++) & 0xFF;
        c2 = (*p++) & 0xFF;
        c3 = (*p++) & 0xFF;

        *o++ = _base64_chars[c1 >> 2];
        *o++ = _base64_chars[((c1 << 4) | (c2 >> 4)) & 0x3F];
        *o++ = _base64_chars[((c2 << 2) | (c3 >> 6)) & 0x3F];
        *o++ = _base64_chars[c3 & 0x3F];
    }

    // Encode the remaining one or two characters in the input buffer
    switch (len % 3) {
    case 0:
        break;
    case 1:
        c1 = (*p++) & 0xFF;
        *o++ = _base64_chars[(c1 & 0xFC) >> 2];
        *o++ = _base64_chars[((c1 & 0x03) << 4)];
        *o++ = '=';
        *o++ = '=';
        break;
    case 2:
        c1 = (*p++) & 0xFF;
        c2 = (*p++) & 0xFF;
        *o++ = _base64_chars[(c1 & 0xFC) >> 2];
        *o++ = _base64_chars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
        *o++ = _base64_chars[((c2 & 0x0F) << 2)];
        *o++ = '=';
        break;
    default:
        free(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "an unexpected error occurred during base64 encoding");
        break;
    }

    return result;
}


/**
 * @brief
 *  Dump a data buffer to the console for debugging purposes.
 * @param buf
 *  a pointer to the data buffer to have its contents dumped.
 * @param len
 *  the size, in bytes, of the data buffer to be dumped.
 * @param all_hex
 *  if set, print all characters as hex codes - even printable ones.
 */
void
_dump_buf(
    unsigned char const *buf,
    size_t len,
    int all_hex)
{
    if (!buf) {
        return;
    }

    fprintf(
        stderr,
        "Dumping buf of size %u bytes ...: |",
        (unsigned int)len);

    for(size_t i = 0; i < len; i++) {

        if (!all_hex && (isgraph(buf[i]) || buf[i] == ' ')) {
            fprintf(stderr, "%c", buf[i]);
        } else {
            fprintf(stderr, "%.2x ", (unsigned char)buf[i]);
        }

        if (!((i + 1) % 64)) {
            fprintf(stderr, "\n");
        } else if (!((i + 1) % 16)) {
            fprintf(stderr, "    ");
        }

    }

    fprintf(stderr, "|\n");
}


/**
 * @brief
 *  Dump a data buffer to the console in abbreviated format for debugging
 *  purposes.
 * @note
 *  At most, only the first nouter leading and trailing bytes will be dumped.
 * @param buf
 *  a pointer to the data buffer to have its contents dumped.
 * @param len
 *  the size, in bytes, of the data buffer to be dumped.
 * @param nouter
 *  the number of outer (leading and trailing) bytes to be dumped.
 * @param all_hex
 *  if set, print all characters as hex codes - even printable ones.
 */
void
_dump_buf_outer(
    unsigned char const *buf,
    size_t len,
    size_t nouter,
    int all_hex)
{
    if (!buf) {
        return;
    }

    // If too small, just dump the entire thing.
    if (len <= nouter * 2) {
        _dump_buf(buf, len, all_hex);
        return;
    }

    fprintf(
        stderr,
        "Dumping buf of size %u bytes ...: |",
        (unsigned int)len);

    for (size_t i = 0; i < nouter; i++) {

        if ((!all_hex && isgraph(buf[i])) || buf[i] == ' ') {
            fprintf(stderr, "%c", buf[i]);
        } else {
            fprintf(stderr, "\\x%.2x", (unsigned char)buf[i]);
        }

    }

    fprintf(stderr, "  ...  ");

    for(size_t i = len - nouter; i < len; i++) {

        if ((!all_hex && isgraph(buf[i])) || buf[i] == ' ') {
            fprintf(stderr, "%c", buf[i]);
        } else {
            fprintf(stderr, "\\x%.2x", (unsigned char)buf[i]);
        }

    }

    fprintf(stderr, "|\n");
}


/**
 * @brief
 *  Print a debug string to the console.
 * @param dbglevel
 *  the minimum necessary debugging level to be active in order for the data to
 *  be displayed.
 * @param fmt
 *  the format string specifier to be used to display the user data.
 * @param ...
 *  the variable argument list specified by the format string.
 */
void
_dbgprint(
    unsigned int dbglevel,
    const char *fmt,
    ...)
{
    va_list ap;

    va_start(ap, fmt);
    __dbgprint(dbglevel, fmt, ap);
    va_end(ap);
}


/**
 * @brief
 *  Print a debug string to the console.
 */
void
__dbgprint(
    unsigned int dbglevel,
    char const *fmt,
    va_list ap)
{
    if ((unsigned int)_verbose < dbglevel) {
        return;
    }

    if (dbglevel) {

        for (unsigned int i = 0; i < dbglevel; i++) {
            fprintf(stderr, "-");
        }

        fprintf(stderr, " ");
    }

    vfprintf(stderr, fmt, ap);

}

/**
 * @brief
 *  Compute an SHA1, SHA256, or SHA512 hash of a block of data.
 * @param nbits
 *  specifies the number of bits for the hash operation (160, 256, and 512 are
 *  accepted values).
 * @param buf
 *  a pointer to a data buffer containing the data to be hashed.
 * @param blen
 *  the size, in bytes, of the buffer to be hashed.
 * @param outbuf
 *  a buffer that will contain the output of the hash operation (the caller is
 *  responsible for allocating the correct amount of space).
 * @return
 *  0 on success or < 0 if an error was encountered.
 */
int
_compute_sha_hash(
    size_t nbits,
    unsigned char const *buf,
    size_t blen,
    unsigned char *outbuf)
{
    SHA512_CTX ctx512;
    SHA256_CTX ctx256;
    SHA_CTX ctx160;

    if (!buf || !outbuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (nbits == 160) {

        if (!SHA1_Init_d(&ctx160)
            || !SHA1_Update_d(&ctx160, buf, blen)
            || !SHA1_Final_d(outbuf, &ctx160))
        {
            PUSH_ERROR_OPENSSL();
            RET_ERROR_INT(
                ERR_UNSPEC,
                "error occurred in SHA1 hash operation");
        }

    } else if (nbits == 256) {

        if (!SHA256_Init_d(&ctx256)
            || !SHA256_Update_d(&ctx256, buf, blen)
            || !SHA256_Final_d(outbuf, &ctx256))
        {
            PUSH_ERROR_OPENSSL();
            RET_ERROR_INT(
                ERR_UNSPEC,
                "error occurred in SHA-256 hash operation");
        }

    } else if (nbits == 512) {

        if (!SHA512_Init_d(&ctx512)
            || !SHA512_Update_d(&ctx512, buf, blen)
            || !SHA512_Final_d(outbuf, &ctx512))
        {
            PUSH_ERROR_OPENSSL();
            RET_ERROR_INT(
                ERR_UNSPEC,
                "error occurred in SHA-512 hash operation");
        }

    } else {
        RET_ERROR_INT(
            ERR_BAD_PARAM,
            "SHA hash requested against unsupported bit size");
    }

    return 0;
}


/**
 * @brief
 *  Compute an SHA1, SHA256, or SHA512 hash of a block of data.
 * @param nbits
 *  specifies the number of bits for the hash operation (160, 256, and 512 are
 *  accepted values).
 * @param bufs
 *  a pointer to a data buffer containing the data to be hashed.
 * @param outbuf
 *  a buffer that will contain the output of the hash operation (the caller is
 *  responsible for allocating the correct amount of space).
 * @return
 *  0 on success or < 0 if an error was encountered.
 */
int
_compute_sha_hash_multibuf(
    size_t nbits,
    sha_databuf_t *bufs,
    unsigned char *outbuf)
{
    SHA512_CTX ctx512;
    SHA256_CTX ctx256;
    SHA_CTX ctx160;
    sha_databuf_t *bufptr = bufs;
    int res;

    if (!bufs || !outbuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if ((nbits != 160) && (nbits != 256) && (nbits != 512)) {
        RET_ERROR_INT(
            ERR_BAD_PARAM,
            "SHA multi-buffer hash requested against unsupported bit size");
    }

    switch(nbits) {
    case 160:
        res = SHA1_Init_d(&ctx160);
        break;
    case 256:
        res = SHA256_Init_d(&ctx256);
        break;
    case 512:
        res = SHA512_Init_d(&ctx512);
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid number of bits for SHA hash");
        break;
    }

    if (!res) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "error initializing SHA context");
    }

    while (bufptr->data) {

        switch(nbits) {
        case 160:
            res = SHA1_Update_d(&ctx160, bufptr->data, bufptr->len);
            break;
        case 256:
            res = SHA256_Update_d(&ctx256, bufptr->data, bufptr->len);
            break;
        case 512:
            res = SHA512_Update_d(&ctx512, bufptr->data, bufptr->len);
            break;
        }

        if (!res) {
            PUSH_ERROR_OPENSSL();
            RET_ERROR_INT(ERR_UNSPEC, "error updating multi-buffer SHA context");
        }

        bufptr++;
    }

    switch(nbits) {
    case 160:
        res = SHA1_Final_d(outbuf, &ctx160);
        break;
    case 256:
        res = SHA256_Final_d(outbuf, &ctx256);
        break;
    case 512:
        res = SHA512_Final_d(outbuf, &ctx512);
        break;
    }

    if (!res) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error finalizing multi-buffer SHA operation");
    }

    return 0;
}


/**
 * @brief
 *  Decode an RSA public key from a buffer.
 * @note
 *  This format is identical to the wire format of RSA public keys used for
 *  DNSSEC validation.
 * @param data
 *  a pointer to the data buffer that stores the encoded RSA public key in
 *  binary form.
 * @param dlen
 *  the length, in bytes, of the data buffer.
 * @return
 *  a pointer to an RSA public key structure on success, or NULL on failure.
 * @free_using{RSA_free}
 */
RSA *
_decode_rsa_pubkey(unsigned char *data, size_t dlen)
{
    RSA *result;
    BIGNUM *mod, *exp;
    unsigned char *ptr = data + 1;
    uint16_t explen;
    size_t left = dlen - 1;

    if (!data || !dlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    // If this first byte is > 0, then it's the entire length as a one byte
    // field.  Otherwise, if it's zero, the following 2 bytes store the
    // exponent length.
    if (!*data) {
        if (left < sizeof(explen)) {
            RET_ERROR_PTR(ERR_UNSPEC, "RSA key buffer underflow");
        }

        explen = ntohs(*((uint16_t *)ptr));
        ptr += sizeof(explen);
        left -= sizeof(explen);
    } else {
        explen = *data;
    }

    if (left < explen) {
        RET_ERROR_PTR(ERR_UNSPEC, "reached premature end of exponent");
    }

    if (!(exp = BN_bin2bn_d(ptr, explen, NULL))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not read exponent");
    }

    ptr += explen;
    left -= explen;

    // The remainder is the modulus.
    if (!(mod = BN_bin2bn_d(ptr, left, NULL))) {
        PUSH_ERROR_OPENSSL();
        BN_free_d(exp);
        RET_ERROR_PTR(ERR_UNSPEC, "could not read modulus");
    }

    if (!(result = RSA_new_d())) {
        PUSH_ERROR_OPENSSL();
        BN_free_d(exp);
        BN_free_d(mod);
        RET_ERROR_PTR(ERR_UNSPEC, "could not create new RSA public key holder");
    }

    result->n = mod;
    result->e = exp;
    result->d = NULL;
    result->p = NULL;
    result->q = NULL;

    return result;
}


/**
 * @brief
 *  Encode and store an RSA public key into a buffer.
 * @param pubkey
 *  a pointer to the RSA public key to be encoded.
 * @param enclen
 *  a pointer to a size_t that will receive the total size of the encoded
 *  public key on completion.
 * @return
 *  a pointer to a buffer containing the encoded RSA public key as binary data
 *  on success, or NULL on failure.
 * @free_using{free}
 */
unsigned char *
_encode_rsa_pubkey(RSA *pubkey, size_t *enclen)
{
    unsigned char *result = NULL, *rptr;
    unsigned short explen;
    unsigned char byte = 0;

    if (!pubkey || !enclen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    // If the exponent length can be fit in one byte, it will. Otherwise, it will be stored as a null byte followed by a 2-byte length.
    if ((explen = BN_num_bytes(pubkey->e)) < 0xff) {
        byte = explen;
        *enclen = explen + 1 + BN_num_bytes(pubkey->n);
    } else {
        *enclen = explen + 3 + BN_num_bytes(pubkey->n);
        explen = htons(explen);
    }

    if (!(result = malloc(*enclen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate space for encoded RSA public key");
    }

    memset(result, 0, *enclen);
    rptr = result;

    // If the first byte is null it means the length field follows
    if (!(*rptr++ = byte)) {
        memcpy(rptr, &explen, sizeof(explen));
        rptr += sizeof(explen);
    }

    // Finally, store the exponent followed by the modulus.
    if (!BN_bn2bin_d(pubkey->e, rptr)) {
        PUSH_ERROR_OPENSSL();
        free(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not encode RSA public key exponent");
    }

    rptr += BN_num_bytes(pubkey->e);

    if (!BN_bn2bin_d(pubkey->n, rptr)) {
        PUSH_ERROR_OPENSSL();
        free(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not encode RSA public key modulus");
    }

    return result;
}


/**
 * @brief
 *  Get the SHA signature of an x509 certificate in DER format.
 * @param cert
 *  a pointer to the x509 certificate to be hashed.
 * @param nbits
 *  the number of bits in the SHA operation (160, 256, and 512 are valid
 *  values).
 * @param out
 *  a pointer to a data buffer to receive the output (which the caller is
 *  responsible for allocating properly depending on the hash size).
 * @return
 *  0 if the hash operation was completed successfully, or < 0 on error.
 */
int
_get_x509_cert_sha_hash(
    X509 *cert,
    size_t nbits,
    unsigned char *out)
{
    unsigned char *buf = NULL;
    int blen;

    if (!cert || !out) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }
    if ((blen = i2d_X509(cert, &buf)) < 0) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize X509 certificate");
    }

    if (_compute_sha_hash(nbits, buf, blen, out) < 0) {
        CRYPTO_free_d(buf);
        RET_ERROR_INT(ERR_UNSPEC, "SHA hash on X509 certificate data failed");
    }

    // TODO: Is this really the proper way to free this buffer?
    CRYPTO_free_d(buf);

    return 0;
}

/**
 * @brief
 *  Check to see if a buffer is filled with null bytes.
 * @param buf
 *  a pointer to the data buffer to be scanned.
 * @param len
 *  the size, in bytes, of the data buffer to be scanned.
 * @return
 *  1 if the buffer was filled exclusively with null bytes, 0 if it was not, or
 *  -1 on general error.
 */
int
_is_buf_zeroed(
    void *buf,
    size_t len)
{
    unsigned char *ptr = (unsigned char *)buf;

    if (!buf || !len) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    for(size_t i = 0; i < len; i++) {

        if (ptr[i]) {
            return 0;
        }

    }

    return 1;
}


/**
 * @brief
 *  Create a pem file with specified tags and filename.
 * @param b64_data
 *  Null terminated base64 encoded data.
 * @param tag
 *  Null terminated ASCII string containing the desired PEM tag.
 * @param filename
 *  Null terminated string containing the desired filename.
 * @return
 *  0 on success, -1 on failure.
 */
int
_write_pem_data(
    char const *b64_data,
    char const *tag,
    char const *filename)
{
    FILE *fp;
    char fbuf[BUFSIZ];
    size_t data_size;

    if(!b64_data || !tag || !filename) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    } else if(!strlen(filename) || !strlen(tag) || !(data_size = strlen(b64_data))) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(!(fp = fopen(filename, "w"))) {
        PUSH_ERROR_SYSCALL("fopen");
        RET_ERROR_INT_FMT(ERR_UNSPEC, "could not open file for writing: %s", filename);
    }
    setbuf(fp, fbuf);

    fprintf(fp, "-----BEGIN %s-----\n", tag);

    for(size_t i = 0; i < data_size; ++i) {

        if(i % 64 == 0 && i) {
            fprintf(fp, "\n");
        }

        fprintf(fp, "%c", b64_data[i]);
    }

    fprintf(fp, "\n-----END %s-----\n", tag);
    fclose(fp);
    _secure_wipe(fbuf, sizeof(fbuf));

    return 0;
}



/**
 * @brief
 *  Read the raw contents of a file into a buffer.
 * @param filename
 *  a null-terminated string with the name of the file to have its contents
 *  read into memory.
 * @param fsize
 *  a pointer to a variable that will receive the length, in bytes, of the read
 *  data.
 * @return
 *  a pointer to a buffer containing the raw contents of the specified file, or
 *  NULL on failure.
 * @free_using{free}
 */
unsigned char *
_read_file_data(
    char const *filename,
    size_t *fsize)
{
    unsigned char *result;
    struct stat sb;
    int fd, nread;

    if (!filename || !fsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if ((fd = open(filename, O_RDONLY)) < 0) {
        PUSH_ERROR_SYSCALL("read");
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "could not open file for reading: %s", filename);
    }

    if (fstat(fd, &sb) < 0) {
        PUSH_ERROR_SYSCALL("stat");
        close(fd);
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "could not determine file size: %s", filename);
    }

    if (!(result = malloc(sb.st_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        close(fd);
        RET_ERROR_PTR_FMT(
            ERR_UNSPEC,
            "could not allocate space for file contents: %s",
            filename);
    }

    if ((nread = read(fd, result, sb.st_size)) < 0) {
        PUSH_ERROR_SYSCALL("read");
        close(fd);
        free(result);
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "error reading data from file: %s", filename);
    }

    close(fd);

    if (nread != sb.st_size) {
        free(result);
        RET_ERROR_PTR_FMT(
            ERR_UNSPEC,
            "could not read entire contents of file: %s",
            filename);
    }

    *fsize = sb.st_size;

    return result;
}


/**
 * @brief
 *  Read the contents of a specified tag inside a PEM file into a buffer.
 * @note
 *  This function will remove all newline characters inside the tag being read.
 * @param pemfile
 *  the name of the PEM file to be parsed.
 * @param tag
 *  the name of a tag in the file to have its contents read into memory.
 * @param nospace
 *  if set, strip all whitespace from the PEM file content.
 * @return
 *  a pointer to a buffer containing the contents of the specified PEM file
 *  tag, or NULL on failure.
 * @free_using{free}
 */
char *
_read_pem_data(
    char const *pemfile,
    char const *tag,
    int nospace)
{
    FILE *fp;
    const char *hyphens = "-----", *begin = "BEGIN ", *end = "END ";
    char line[4096], *result = NULL, *ptr;
    unsigned int in_tag = 0, in_end = 0, slen;

    (void)nospace; /* TODO: use this parameter */

    if (!pemfile || !tag || !strlen(tag)) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(fp = fopen(pemfile, "r"))) {
        PUSH_ERROR_SYSCALL("fopen");
        RET_ERROR_PTR_FMT(
            ERR_UNSPEC,
            "could not open PEM file for reading: %s",
            pemfile);
    }

    while (!feof(fp)) {

        // If we're in the tag and just read a line of data, it needs to be
        // appended to the result.
        if ((in_tag > 1) && strlen(line)) {
            ptr = line;

            while (*ptr) {

                if (isspace(*(unsigned char *)ptr)) {
                    slen = strlen(ptr + 1);
                    memmove(ptr, (ptr + 1), slen);
                    ptr[slen] = 0;
                    continue;
                }

                ptr++;
            }

            if (!_str_printf(&result, line)) {
                fclose(fp);
                RET_ERROR_PTR(
                    ERR_NOMEM,
                    "unable to allocate space for PEM file contents");
            }

        }

        if (in_tag) {
            in_tag++;
        }

        memset(line, 0, sizeof(line));

        if (!fgets(line, sizeof(line), fp)) {
            break;
        }

        if ((   slen = strlen(line))
                && ((line[slen - 1] == '\n')
            || (line[slen - 1] == '\r')))
        {
            line[slen - 1] = 0;
        }

        ptr = line;

        while (*ptr && isspace(*(unsigned char *)ptr)) {
            ptr++;
        }

        // Parse the leading hyphens and either the BEGIN or END tag.
        if (strncmp(ptr, hyphens, strlen(hyphens))) {
            continue;
        }

        ptr += strlen(hyphens);

        if (!strncmp(ptr, end, strlen(end))) {
            in_end = 1;
            ptr += strlen(end);
        } else if (!strncmp(ptr, begin, strlen(begin))) {
            in_end = 0;
            ptr += strlen(begin);
        } else {
            continue;
        }

        // Is this the tag we actually want?
        if (strncmp(ptr, tag, strlen(tag))) {
            continue;
        }

        ptr += strlen(tag);

        if (strncmp(ptr, hyphens, strlen(hyphens))) {
            continue;
        }

        ptr += strlen(hyphens);

        // The trailing hyphens shouldn't be followed by anything other than
        // whitespace.
        while (*ptr && isspace(*(unsigned char *)ptr)) {
            ptr++;
        }

        if (*ptr) {
            continue;
        }

        // If we have successfully parsed an end tag then it's time to return;
        if (in_end) {
            fclose(fp);
            return result;
        }

        // Otherwise, the next line(s) of data will correspond to our tag.
        in_tag = 1;
    }

    // If we got this far, we never correctly parsed (all) the data we expected
    // to read.
    if (result) {
        free(result);
    }

    fclose(fp);

    if (in_tag) {
        RET_ERROR_PTR(ERR_UNSPEC, "did not find tag end in PEM file");
    }

    RET_ERROR_PTR(ERR_UNSPEC, "could not find PEM tag");
}


/**
 * @brief
 *  Securely wipe a memory buffer, in preparation for deallocation.
 * @param buf
 *  a pointer to the data buffer to be wiped.
 * @param len
 *  the size, in bytes, of the data buffer to be wiped.
 */
void
_secure_wipe(void *buf, size_t len)
{
    if (!buf || !len) {
        return;
    }

    memset(buf, 255, len);
    memset(buf, 0xaa, len);
    memset(buf, 0x55, len);
    memset(buf, 0, len);
}
