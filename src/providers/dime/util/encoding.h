#ifndef ENCODING_H
#define ENCODING_H

#include "dime/dime_ctx.h"
#include "dime/error_codes.h"

#include "dime/sds/sds.h"

#include <stddef.h>
#include <stdlib.h>

derror_t const *
libdime_base64_decode(
    dime_ctx_t const *dime_ctx,
    unsigned char **result,
    size_t *result_length,
    char const *buf,
    size_t len);

derror_t const *
libdime_base64_encode(
    dime_ctx_t const *dime_ctx,
    sds *result,
    unsigned char const *buf,
    size_t len);

#endif
