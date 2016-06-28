#ifndef DMSG_PARSER_H
#define DMSG_PARSER_H

#include "dime/dmessage/common.h"

typedef struct {
    int required;
    const char *label;
    size_t label_length;
} dmime_header_key_t;

typedef struct {
    sds auth_recp;
    sds auth_recp_fp;
    sds dest_orig;
    sds dest_orig_fp;
} dmime_envelope_object_t;

extern dmime_header_key_t dmime_header_keys[DMIME_NUM_COMMON_HEADERS];

typedef enum {
    HEADER_TYPE_DATE = 0,
    HEADER_TYPE_TO,
    HEADER_TYPE_CC,
    HEADER_TYPE_FROM,
    HEADER_TYPE_ORGANIZATION,
    HEADER_TYPE_SUBJECT,
    HEADER_TYPE_NONE
} dmime_header_type_t;

void
dime_prsr_envelope_destroy(
    dmime_envelope_object_t *obj);

sds
dime_prsr_envelope_format(
    sds user_id,
    sds org_id,
    char const *user_fp,
    char const *org_fp,
    dmime_chunk_type_t type);

dmime_envelope_object_t *
dime_prsr_envelope_parse(
    char const *in,
    size_t insize,
    dmime_chunk_type_t type);

dmime_common_headers_t *
dime_prsr_headers_create(void);

void
dime_prsr_headers_destroy(
    dmime_common_headers_t *obj);

unsigned char *
dime_prsr_headers_format(
    dmime_common_headers_t *obj,
    size_t *outsize);

dmime_common_headers_t *
dime_prsr_headers_parse(
    unsigned char *in,
    size_t insize);

#endif
