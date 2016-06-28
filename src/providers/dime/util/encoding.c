#include "dime/util/encoding.h"

#include <string.h>

static unsigned char const
base64_chars[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char const
base64_vals[128] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
};

/**
 * @brief
 *  Perform base64-decoding on a string and return the result.
 * @note
 *  SOLVED PROBLEM - why are we reinventing the wheel?
 * @param result
 *  double pointer to a newly allocated buffer containing the base64-decoded
 *  data, or NULL on failure.
 * @param buf
 *  a pointer to a buffer containing the data to be decoded.
 * @param len
 *  the length, in bytes, of the buffer to be decoded.
 * @param outlen
 *  a pointer to a variable to receive the length of the decoded data.
 * @free_using{free}
 */
derror_t const *
libdime_base64_decode(
    dime_ctx_t const *dime_ctx,
    unsigned char **result,
    size_t *result_length,
    char const *buf,
    size_t len)
{
    derror_t const *err;
    unsigned char *o;
    const char *p;
    size_t new_len;
    size_t written = 0;
    int loop = 0;
    int value = 0;

    if (dime_ctx == NULL
        || result == NULL
        || result_length == NULL
        || buf == NULL
        || len == 0
        || len % 4 != 0)
    {
        err = ERR_BAD_PARAM;
        goto error;
    }

    new_len = (len / 4) * 3;
    if (buf[len - 1] == '=') {
        new_len--;
    }
    if (buf[len - 2] == '=') {
        new_len--;
    }

    *result = malloc(new_len);
    if (*result == NULL) {
        err = ERR_NOMEM;
        goto error;
    }

    memset(*result, 0, new_len);

    o = *result;
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
                value = base64_vals[(int)*p++] << 18;
                loop++;
                break;
            case 1:
                value += base64_vals[(int)*p++] << 12;
                *o++ = (value & 0x00ff0000) >> 16;
                written++;
                loop++;
                break;
            case 2:
                value += (unsigned int)base64_vals[(int)*p++] << 6;
                *o++ = (value & 0x0000ff00) >> 8;
                written++;
                loop++;
                break;
            case 3:
                value += (unsigned int)base64_vals[(int)*p++];
                *o++ = value & 0x000000ff;
                written++;
                loop = 0;
                break;
            default:
                err = ERR_ENCODING;
                LOG_ERROR(dime_ctx,
                    "an unexpected error occurred during base64 decoding");
                goto cleanup_result;
            }

        } else if (*p == '=') {
            i = len;
        } else {
            p++;
        }

    }

    *result_length = written;

    return NULL;

cleanup_result:
    free(*result);
    *result = NULL;
error:
    return err;
}

/**
 * @brief
 *  Return the base64-encoded string of a data buffer.
 * @param result
 *  a pointer to a newly allocated sds string containing the base64-encoded
 *  data, or NULL on failure.
 * @param buf
 *  a pointer to the data buffer to be base-64 encoded.
 * @param len
 *  the length, in bytes, of the buffer to be encoded.
 * @free_using{sdsfree}
 */
derror_t const *
libdime_base64_encode(
    dime_ctx_t const *dime_ctx,
    sds *result,
    unsigned char const *buf,
    size_t len)
{
    derror_t const *err = NULL;
    unsigned char *o;
    const unsigned char *p;
    size_t new_len;
    unsigned char c1;
    unsigned char c2;
    unsigned char c3;

    if (dime_ctx == NULL
        || result == NULL
        || buf == NULL
        || len == 0)
    {
        err = ERR_BAD_PARAM;
        goto error;
    }

    new_len = (len / 3) * 4;
    // It seems the base64 padding scheme was designed to simplify this
    // computation -- cool!
    if (len % 3 != 0) {
        new_len += 4;
    }

    *result = sdsnewlen(NULL, new_len);
    if (*result == NULL) {
        err = ERR_NOMEM;
        goto error;
    }

    o = (unsigned char *)*result;
    p = buf;

    // This will process three bytes at a time.
    for(size_t i = 0; i < len / 3; ++i) {
        c1 = (*p++) & 0xFF;
        c2 = (*p++) & 0xFF;
        c3 = (*p++) & 0xFF;

        *o++ = base64_chars[c1 >> 2];
        *o++ = base64_chars[((c1 << 4) | (c2 >> 4)) & 0x3F];
        *o++ = base64_chars[((c2 << 2) | (c3 >> 6)) & 0x3F];
        *o++ = base64_chars[c3 & 0x3F];
    }

    // Encode the remaining one or two characters in the input buffer
    switch (len % 3) {
    case 0:
        break;
    case 1:
        c1 = (*p++) & 0xFF;
        *o++ = base64_chars[(c1 & 0xFC) >> 2];
        *o++ = base64_chars[((c1 & 0x03) << 4)];
        *o++ = '=';
        *o++ = '=';
        break;
    case 2:
        c1 = (*p++) & 0xFF;
        c2 = (*p++) & 0xFF;
        *o++ = base64_chars[(c1 & 0xFC) >> 2];
        *o++ = base64_chars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
        *o++ = base64_chars[((c2 & 0x0F) << 2)];
        *o++ = '=';
        break;
    default:
        LOG_ERROR(dime_ctx,
            "an unexpected error occurred during base64 encoding");
        goto cleanup_result;
    }

    return NULL;

cleanup_result:
    sdsfree(*result);
    *result = NULL;
error:
    return err;
}
//
///**
// * @brief
// *  Return the base64-encoded string of a data buffer without padding.
// * @note
// *  This function ensures the smallest possible output length with no trailing
// *  '=' characters.
// * @param buf
// *  a pointer to the data buffer to be base-64 encoded.
// * @param len
// *  the length, in bytes, of the buffer to be encoded.
// * @return
// *  a pointer to a newly allocated null-terminated string containing the
// *  base64-encoded data, or NULL on failure.
// * @free_using{free}
// */
//char *
//b64encode_nopad(
//    unsigned char const *buf,
//    size_t len)
//{
//    char *result;
//    char *ptr;
//
//    if (buf == NULL || len == NULL) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    if (!(result = _b64encode(buf, len))) {
//        RET_ERROR_PTR(ERR_UNSPEC, "could not base64 encode input");
//    }
//
//    ptr = result + strlen(result) - 1;
//
//    while ((ptr >= result) && (*ptr == '=')) {
//        *ptr-- = 0;
//    }
//
//    return result;
//}
//
///**
// * @brief
// *  Decode a base64-encoded string without any padding characters and return
// *  the result.
// * @param buf
// *  a pointer to a buffer containing the data to be decoded.
// * @param len
// *  the length, in bytes, of the buffer to be decoded.
// * @param outlen
// *  a pointer to a variable to receive the length of the decoded data.
// * @return
// *  a pointer to a newly allocated buffer containing the base64-decoded, or
// *  NULL on failure.
// * @free_using{free}
// */
//unsigned char *
//_b64decode_nopad(
//    char const *buf,
//    size_t len,
//    size_t *outlen)
//{
//    unsigned char *result;
//    char *padded;
//    size_t padlen;
//
//    if (!buf || !len || !outlen) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    if (!(len % 4)) {
//        return (_b64decode(buf, len, outlen));
//    }
//
//    padlen = (len + 4) & ~(3);
//
//    if (!(padded = malloc(padlen + 1))) {
//        PUSH_ERROR_SYSCALL("malloc");
//        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for temporary string");
//    }
//
//    memset(padded, '=', padlen);
//    padded[padlen] = 0;
//    memcpy(padded, buf, len);
//
//    result = _b64decode(padded, padlen, outlen);
//    free(padded);
//
//    return result;
//}
//
///**
// * @brief
// *  Return a simple hex string representing a data buffer.
// * @param buf
// *  a pointer to the data buffer to be encoded as a hex string.
// * @param len
// *  the size, in bytes, of the buffer to be processed.
// * @return
// *  NULL on failure, or a newly allocated null-terminated string containing the
// *  hex encoded input on success.
// * @free_using{free}
// */
//char *
//_hex_encode(unsigned char const *buf, size_t len)
//{
//    char *result;
//    size_t newlen;
//
//    if (!buf || !len) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    newlen = (len * 2) + 1;
//
//    if (!(result = malloc(newlen))) {
//        PUSH_ERROR_SYSCALL("malloc");
//        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for hex string");
//    }
//
//    memset(result, 0, newlen);
//
//    for (size_t i = 0; i < len; i++) {
//        snprintf(&(result[i * 2]), 3, "%.2x", (unsigned char)buf[i]);
//    }
//
//    return result;
//}
