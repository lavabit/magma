#include "dime/common/misc.h"
#include "dime/dmessage/parse.h"

static void
prsr_envelope_destroy(
    dmime_envelope_object_t *obj);

static sds
prsr_envelope_format(
    sds user_id,
    sds org_id,
    const char *user_fp,
    const char *org_fp,
    dmime_chunk_type_t type);

static int
prsr_envelope_labels_get(
    dmime_chunk_type_t type,
    char const **label1,
    char const **label2,
    char const **label3,
    char const **label4);

static dmime_envelope_object_t *
prsr_envelope_parse(
    char const *in,
    size_t insize,
    dmime_chunk_type_t type);

static dmime_common_headers_t *
prsr_headers_create(void);

static void
prsr_headers_destroy(
    dmime_common_headers_t *obj);

static unsigned char *
prsr_headers_format(
    dmime_common_headers_t *obj,
    size_t *outsize);

static dmime_common_headers_t *
prsr_headers_parse(
    unsigned char *in,
    size_t insize);

static dmime_header_type_t
prsr_headers_type_get(
    unsigned char *in,
    size_t insize);

/* PRIVATE FUNCTIONS */

/**
 * @brief
 *  Allocates memory for an empty dmime_common_headers_t type.
 * @return
 *  dmime_common_headers_t structure.
 * @free_using{prsr_headers_destroy}
*/
static dmime_common_headers_t *
prsr_headers_create(void)
{
    dmime_common_headers_t *result;

    if (!(result = malloc(sizeof(dmime_common_headers_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for parsed common headers");
    }

    memset(result, 0, sizeof(dmime_common_headers_t));

    return result;
}


/**
 * @brief
 *  Formats the passed in envelope data into a string to be passed to an
 *  envelope chunk.
 * @param user_id
 *  Stringer containing the user email address.
 * @param org_id
 *  Stringer containing the organizational TLD.
 * @param user_fp
 *  Cstring containing cryptographic user signet.
 * @param org_fp
 *  Cstring containing organizational signet fingerprint.
 * @param type
 *  The type of envelope chunk.
 * @return
 *  Buffer with formated envelope data.
 * @free_using{st_free}
 */
static sds
prsr_envelope_format(
    sds const user_id,
    sds const org_id,
    char const * user_fp,
    char const * org_fp,
    dmime_chunk_type_t type)
{
    char const *label1 = NULL;
    char const *label2 = NULL;
    char const *label3 = NULL;
    char const *label4 = NULL;
    char const *end1 = ">\r\n", *end2 = "]\r\n";
    sds result;

    if (user_id == NULL
        || sdslen(user_id) == 0
        || org_id == NULL
        || sdslen(org_id) == 0
        || user_fp == NULL
        || org_fp == NULL)
    {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if (prsr_envelope_labels_get(
        type,
        &label1,
        &label2,
        &label3,
        &label4) < 0)
    {
        PUSH_ERROR(ERR_UNSPEC, "failed to get envelope chunk labels");
        goto error;
    }

    result = sdsempty();
    result = sdscat(result, label1);
    result = sdscat(result, user_id);
    result = sdscat(result, end1);
    result = sdscat(result, label2);
    result = sdscat(result, user_fp);
    result = sdscat(result, end2);
    result = sdscat(result, label3);
    result = sdscat(result, org_id);
    result = sdscat(result, end1);
    result = sdscat(result, label4);
    result = sdscat(result, org_fp);
    result = sdscat(result, end2);

    if (result == NULL) {
        PUSH_ERROR(
            ERR_UNSPEC,
            "failed to merge strings to form the envelope data");
        goto error;
    }

    return result;

error:
    return NULL;
}

/**
 * @brief
 *  Assigns the correct envelope labels based on envelope chunk types.
 * @param type
 *  Envelope chunk's chunk type.
 * @param label1
 *  First label (user id).
 * @param label2
 *  Second label (user cryptographic b64 encoded signet).
 * @param label3
 *  Third label (organizational id).
 * @param label4
 *  Fourth label (organizational cryptographic signet fingerprint).
 * @return
 *  0 on success, -1 on failure.
*/
static int
prsr_envelope_labels_get(
    dmime_chunk_type_t type,
    char const **label1,
    char const **label2,
    char const **label3,
    char const **label4)
{
    switch(type) {
    case CHUNK_TYPE_ORIGIN:
        *label1 = "Author: <";
        *label2 = "Author-Signet: [";
        *label3 = "Destination: <";
        *label4 = "Destination-Signet: [";
        break;
    case CHUNK_TYPE_DESTINATION:
        *label1 = "Recipient: <";
        *label2 = "Recipient-Signet: [";
        *label3 = "Origin: <";
        *label4 = "Origin-Signet: [";
        break;
    default:
        PUSH_ERROR(
            ERR_UNSPEC,
            "Specified chunk type does not have envelope "
            "labels associated with it.");
        goto error;
    }

    return 0;

error:
    return -1;
}

/**
 * @brief
 *  Destroys a dmime_common_headers_t structure.
 * @param obj
 *  Headers to be destroyed.
 */
static void
prsr_headers_destroy(
    dmime_common_headers_t *obj)
{
    if(!obj) {
        return;
    }

    for(unsigned int i = 0; i < DMIME_NUM_COMMON_HEADERS; ++i) {
        if(obj->headers[i]) {
            _secure_wipe(
                obj->headers[i],
                sdslen(obj->headers[i]));
            sdsfree(obj->headers[i]);
        }
    }

    free(obj);
}


/**
 * @brief
 *  Formats the dmime_common_headers_t into a single array for the common
 *  headers chunk.
 * @param obj
 *  The headers to be formatted.
 * @param outsize
 *  Stores the size of the output array.
 * @return
 *  Returns the array of ASCII characters (not terminated by '\0') as pointer
 *  to unsigned char.
 * @free_using{free}
 */
static unsigned char *
prsr_headers_format(
    dmime_common_headers_t *obj,
    size_t *outsize)
{
    size_t size = 0, at = 0;
    unsigned char *result;

    if (!obj || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    for (unsigned int i = 0; i < DMIME_NUM_COMMON_HEADERS; ++i) {
        if(!obj->headers[i] && dmime_header_keys[i].required) {
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "a required common header field is missing");
        }

        if(dmime_header_keys[i].label && obj->headers[i]) {
            size +=
              dmime_header_keys[i].label_length +
              sdslen(obj->headers[i]) +
              2;
        }
    }

    if (!(result = malloc(size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for common headers data");
    }

    memset(result, 0, size);

    for (unsigned int i = 0; i < DMIME_NUM_COMMON_HEADERS; ++i) {
        if(obj->headers[i] && dmime_header_keys[i].label) {
            memcpy(
                result + at,
                (unsigned char *)dmime_header_keys[i].label,
                dmime_header_keys[i].label_length);
            at += dmime_header_keys[i].label_length;
            memcpy(
                result + at,
                obj->headers[i],
                sdslen(obj->headers[i]));
            at += sdslen(obj->headers[i]);
            result[at++] = (unsigned char)'\r';
            result[at++] = (unsigned char)'\n';
        }
    }

    *outsize = size;

    return result;
}


/**
 * @brief
 *  Reads the first bytes of the input array and determines the next header
 *  type.
 * @param in
 *  Input buffer.
 * @param insize
 *  Size of input buffer.
 * @return
 *  Common header type.
 */
static dmime_header_type_t
prsr_headers_type_get(
    unsigned char *in,
    size_t insize)
{
    if (!in || !insize) {
        RET_ERROR_CUST(HEADER_TYPE_NONE, ERR_BAD_PARAM, NULL);
    }

    if (insize < dmime_header_keys[HEADER_TYPE_TO].label_length) {
        RET_ERROR_CUST(HEADER_TYPE_NONE, ERR_UNSPEC, "invalid header syntax");
    }

    if (!memcmp(
            in,
            (unsigned char *)dmime_header_keys[HEADER_TYPE_TO].label,
            dmime_header_keys[HEADER_TYPE_TO].label_length))
    {
        return HEADER_TYPE_TO;
    }

    if (!memcmp(
            in,
            (unsigned char *)dmime_header_keys[HEADER_TYPE_CC].label,
            dmime_header_keys[HEADER_TYPE_CC].label_length))
    {
        return HEADER_TYPE_CC;
    }

    if (insize < dmime_header_keys[HEADER_TYPE_FROM].label_length) {
        RET_ERROR_CUST(HEADER_TYPE_NONE, ERR_UNSPEC, "invalid header syntax");
    }

    if (!memcmp(
            in,
            (unsigned char *)dmime_header_keys[HEADER_TYPE_FROM].label,
            dmime_header_keys[HEADER_TYPE_FROM].label_length))
    {
        return HEADER_TYPE_FROM;
    }

    if (!memcmp(
            in,
            (unsigned char *)dmime_header_keys[HEADER_TYPE_DATE].label,
            dmime_header_keys[HEADER_TYPE_DATE].label_length))
    {
        return HEADER_TYPE_DATE;
    }

    if (insize < dmime_header_keys[HEADER_TYPE_SUBJECT].label_length) {
        RET_ERROR_CUST(HEADER_TYPE_NONE, ERR_UNSPEC, "invalid header syntax");
    }

    if (!memcmp(
            in,
            (unsigned char *)dmime_header_keys[HEADER_TYPE_SUBJECT].label,
            dmime_header_keys[HEADER_TYPE_SUBJECT].label_length))
    {
        return HEADER_TYPE_SUBJECT;
    }

    if (insize < dmime_header_keys[HEADER_TYPE_ORGANIZATION].label_length) {
        RET_ERROR_CUST(HEADER_TYPE_NONE, ERR_UNSPEC, "invalid header syntax");
    }

    if (!memcmp(
            in,
            (unsigned char *)dmime_header_keys[HEADER_TYPE_ORGANIZATION].label,
            dmime_header_keys[HEADER_TYPE_ORGANIZATION].label_length))
    {
        return HEADER_TYPE_ORGANIZATION;
    }

    return HEADER_TYPE_NONE;
}

/**
 * @brief
 *  Parses the passed array of bytes into dmime_common_headers_t.
 * @param in
 *  Input buffer.
 * @param insize
 *  Input buffer size.
 * @return
 *  A dmime_common_headers_t array of sds strings containing parsed header info.
 * @free_using{prsr_headers_destroy}
 */
static dmime_common_headers_t *
prsr_headers_parse(
    unsigned char *in,
    size_t insize)
{
    dmime_header_type_t type;
    size_t at = 0, head_size;
    dmime_common_headers_t *result;

    if (!in || !insize) {
        RET_ERROR_CUST(0, ERR_BAD_PARAM, NULL);
    }

    if (!(result = prsr_headers_create())) {
        RET_ERROR_CUST(
            0,
            ERR_UNSPEC,
            "error creating a new dmime_common_headers_t object");
    }

    while (at < insize) {

        if ((type = prsr_headers_type_get(in + at, insize - at))
            == HEADER_TYPE_NONE)
        {
            prsr_headers_destroy(result);
            RET_ERROR_CUST(
                0,
                ERR_UNSPEC,
                "headers buffer contained an invalid header type");
        }

        if (result->headers[type]) {
            prsr_headers_destroy(result);
            RET_ERROR_CUST(
                0,
                ERR_UNSPEC,
                "headers buffer contains duplicate fields");
        }

        at += dmime_header_keys[type].label_length;
        head_size = 0;

        while (
            (at + head_size + 1) < insize
            && in[at + head_size] != '\r'
            && in[at + head_size + 1] != '\n')
        {
            ++head_size;
        }

        if ((at + head_size + 1) > insize
            || (
                (at + head_size + 1) == insize
                && in[at + head_size + 1] != '\n'))
        {
            prsr_headers_destroy(result);
            RET_ERROR_CUST(0, ERR_UNSPEC, "invalid header syntax");
        }

        result->headers[type] = sdsnewlen(in + at, head_size);
        at += head_size + 2;
    }

    return result;
}


/**
 * @brief
 *  Destroys a dmime_envelop_object_t structure.
 * @param obj
 *  Pointer to the object to be destroyed.
 */
static void
prsr_envelope_destroy(
    dmime_envelope_object_t *obj)
{
    if (!obj) {
        return;
    }

    if (obj->auth_recp) {
        _secure_wipe(
            obj->auth_recp,
            sdslen(obj->auth_recp));
        sdsfree(obj->auth_recp);
    }

    if (obj->auth_recp_fp) {
        _secure_wipe(
            obj->auth_recp_fp,
            sdslen(obj->auth_recp_fp));
        sdsfree(obj->auth_recp_fp);
    }

    if (obj->dest_orig) {
        _secure_wipe(
            obj->dest_orig,
            sdslen(obj->dest_orig));
        sdsfree(obj->dest_orig);
    }

    if (obj->dest_orig_fp) {
        _secure_wipe(
            obj->dest_orig_fp,
            sdslen(obj->dest_orig_fp));
        sdsfree(obj->dest_orig_fp);
    }

    free(obj);

}

/**
 * @brief
 *  Parses a binary buffer from a dmime message into a dmime origin object.
 * @param in
 *  Binary origin array.
 * @param insize
 *  Size of input array.
 * @param type
 *  Type of the chunk.
 * @return
 *  Pointer to a parsed dmime object or NULL on error.
 * @free_using{prsr_envelope_destroy}
*/
static dmime_envelope_object_t *
prsr_envelope_parse(
    char const *in,
    size_t insize,
    dmime_chunk_type_t type)
{
    dmime_envelope_object_t *result;
    char const *authrecp = NULL;
    char const *authrecp_signet = NULL;
    char const *destorig = NULL;
    char const *destorig_fp = NULL;
    char const *end1 = ">\r\n", *end2 = "]\r\n";
    char const *start;
    size_t string_size = 0, at = 0;

    if (!in || !insize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (prsr_envelope_labels_get(
            type,
            &authrecp,
            &authrecp_signet,
            &destorig,
            &destorig_fp)
        < 0)
    {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "failed to get envelope chunk labels");
    }

    if (!(result = malloc(sizeof(dmime_envelope_object_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for origin object");
    }

    memset(result, 0, sizeof(dmime_envelope_object_t));

    if (insize <= strlen(authrecp)
        || in != strstr(in, authrecp))
    {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid input buffer passed to envelope parser");
    }

    at += strlen(authrecp);
    start = in + at;

    while (at < insize && in[at] != '>') {
        if(!isprint(in[at])) {
            prsr_envelope_destroy(result);
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "invalid input buffer passed to envelope parser");
        }

        ++at;
    }

    string_size = in + at - start;

    if (!(result->auth_recp = sdsnewlen(start, string_size))) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not import sds string");
    }

    if (in + at != strstr(in + at, end1)) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid input buffer passed to envelope parser");
    }

    at += strlen(end1);

    if (insize - at <= strlen(authrecp_signet)
        || in + at != strstr(in, authrecp_signet))
    {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid input buffer passed to envelope parser");
    }

    at += strlen(authrecp_signet);
    start = in + at;

    while (at < insize && in[at] != ']') {
        if (!isprint(in[at])) {
            prsr_envelope_destroy(result);
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "invalid envelope origin buffer passed to parser");
        }

        ++at;
    }

    string_size = in + at - start;

    if (!(result->auth_recp_fp = sdsnewlen(start, string_size))) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not import sds string");
    }

    if (in + at != strstr(in + at, end2)) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid input buffer passed to envelope parser");
    }

    at += strlen(end2);

    if (insize - at <= strlen(destorig)
        || in + at != strstr(in, destorig))
    {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "invalid input buffer passed to envelope parser");
    }

    at += strlen(destorig);
    start = in + at;

    while (at < insize && in[at] != '>') {
        if (!isprint(in[at])) {
            prsr_envelope_destroy(result);
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "invalid input buffer passed to envelope parser");
        }

        ++at;
    }

    string_size = in + at - start;

    if (!(result->dest_orig = sdsnewlen(start, string_size))) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not import sds string");
    }

    if (in + at != strstr(in + at, end1)) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid input buffer passed to envelope parser");
    }

    at += strlen(end1);

    if (insize - at <= strlen(destorig_fp)
        || in + at != strstr(in, destorig_fp))
    {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "invalid input buffer passed to envelope parser");
    }

    at += strlen(destorig_fp);
    start = in + at;

    while (at < insize && in[at] != ']') {
        if (!isprint(in[at])) {
            prsr_envelope_destroy(result);
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "invalid input buffer passed to envelope parser");
        }

        ++at;
    }

    string_size = in + at - start;

    if (!(result->dest_orig_fp = sdsnewlen(start, string_size))) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not import sds string");
    }

    if (in + at != strstr(in + at, end2)) {
        prsr_envelope_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid input buffer passed to envelope parser");
    }

    return result;
}


dmime_header_key_t
dmime_header_keys[DMIME_NUM_COMMON_HEADERS] = {
    {1, "Date: ", 6},
    {1, "To: ", 4},
    {0, "CC: ", 4},
    {1, "From: ", 6},
    {0, "Organization: ", 14},
    {1, "Subject: ", 9},
    {0, NULL, 0}
};


/* PUBLIC FUNCTIONS */


/**
 * @brief    Destroys a dmime_envelop_object_t structure.
 * @param    obj        Pointer to the object to be destroyed.
 */
void
dime_prsr_envelope_destroy(
    dmime_envelope_object_t *obj)
{
    PUBLIC_FUNCTION_IMPLEMENT(prsr_envelope_destroy, obj);
}

/**
 * @brief
 *  Formats the passed in envelope data into a string to be passed to an
 *  envelope chunk.
 * @param user_id
 *  Stringer containing the user email address.
 * @param org_id
 *  Stringer containing the organizational TLD.
 * @param user_fp
 *  Cstring containing cryptographic user signet.
 * @param org_fp
 *  Cstring containing organizational signet fingerprint.
 * @param type
 *  The type of envelope chunk.
 * @return
 *  Buffer with formated envelope data.
 * @free_using{st_free}
 */
sds
dime_prsr_envelope_format(
    sds user_id,
    sds org_id,
    char const * user_fp,
    char const * org_fp,
    dmime_chunk_type_t type)
{
    PUBLIC_FUNCTION_IMPLEMENT(prsr_envelope_format, user_id, org_id, user_fp, org_fp, type);
}

/**
 * @brief
 *  Parses a binary buffer from a dmime message into a dmime origin object.
 * @param in
 *  Binary origin array.
 * @param insize
 *  Size of input array.
 * @param type
 *  Type of the chunk.
 * @return
 *  Pointer to a parsed dmime object or NULL on error.
 * @free_using{dime_prsr_envelope_destroy}
*/
dmime_envelope_object_t *
dime_prsr_envelope_parse(
    char const *in,
    size_t insize,
    dmime_chunk_type_t type)
{
    PUBLIC_FUNCTION_IMPLEMENT(prsr_envelope_parse, in, insize, type);
}

/**
 * @brief
 *  Allocates memory for an empty dmime_common_headers_t type.
 * @return
 *  dmime_common_headers_t structure.
 * @free_using{dime_prsr_headers_destroy}
*/
dmime_common_headers_t *
dime_prsr_headers_create(void)
{
    PUBLIC_FUNCTION_IMPLEMENT(prsr_headers_create);
}

/**
 * @brief
 *  Destroys a dmime_common_headers_t structure.
 * @param obj
 *  Headers to be destroyed.
*/
void
dime_prsr_headers_destroy(dmime_common_headers_t *obj) {
    PUBLIC_FUNCTION_IMPLEMENT_VOID(prsr_headers_destroy, obj);
}

/**
 * @brief
 *  Formats the dmime_common_headers_t into a single array for the common
 *  headers chunk.
 * @param obj
 *  The headers to be formatted.
 * @param outsize
 *  Stores the size of the output array.
 * @return
 *  Returns the array of ASCII characters (not terminated by '\0') as pointer
 *  to unsigned char.
 * @free_using{free}
*/
unsigned char *
dime_prsr_headers_format(
    dmime_common_headers_t *obj,
    size_t *outsize)
{
    PUBLIC_FUNCTION_IMPLEMENT(prsr_headers_format, obj, outsize);
}

/**
 * @brief
 *  Parses the passed array of bytes into dmime_common_headers_t.
 * @param in
 *  Input buffer.
 * @param insize
 *  Input buffer size.
 * @return
 *  A dmime_common_headers_t array of sds strings containing parsed header info.
 * @free_using{dime_prsr_headers_destroy}
*/
dmime_common_headers_t *
dime_prsr_headers_parse(unsigned char *in, size_t insize) {
    PUBLIC_FUNCTION_IMPLEMENT(prsr_headers_parse, in, insize);
}
