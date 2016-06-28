#include "dime/dime_ctx.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static log_level_t DEBUG_LOG_LEVEL = {
    LOG_CODE_DEBUG,
    "DEBUG"
};
log_level_t const * const
LOG_LEVEL_DEBUG = &DEBUG_LOG_LEVEL;

static log_level_t INFO_LOG_LEVEL = {
    LOG_CODE_INFO,
    "INFO"
};
log_level_t const * const
LOG_LEVEL_INFO = &INFO_LOG_LEVEL;

static log_level_t ERROR_LOG_LEVEL = {
    LOG_CODE_ERROR,
    "DEBUG"
};
log_level_t const * const
LOG_LEVEL_ERROR = &ERROR_LOG_LEVEL;

static void
default_log_callback(
    char const *file,
    size_t line,
    log_level_t const *level,
    char const *format,
    va_list argp);

static void
default_log_callback(
    char const *file,
    size_t line,
    log_level_t const *level,
    char const *format,
    va_list argp)
{
    assert(file != NULL);
    assert(level != NULL);

    fprintf(
        stderr,
        "[%s:%zu:%s]: ",
        file,
        line,
        level->name);
    vfprintf(stderr, format, argp);
    fprintf(stderr, "\n");
}

struct dime_ctx {
    log_function_t log_callback;
};

derror_t const *
dime_ctx_new(
    dime_ctx_t **result,
    log_function_t log_callback)
{
    derror_t const *err = NULL;

    if (*result == NULL) {
        err = ERR_BAD_PARAM;
        goto out;
    }

    *result = malloc(sizeof(dime_ctx_t));
    if (*result == NULL) {
        err = ERR_NOMEM;
        goto out;
    }

    if (log_callback == NULL) {
        (*result)->log_callback = default_log_callback;
    } else {
        (*result)->log_callback = log_callback;
    }

out:
    return err;
}

void
dime_ctx_free(
    dime_ctx_t *ctx)
{
    free(ctx);
}

void
dime_ctx_log(
    dime_ctx_t const *ctx,
    char const *file,
    size_t line,
    log_level_t const *level,
    char const *format,
    ...)
{
    va_list argp;
    va_start(argp, format);
    ctx->log_callback(
        file,
        line,
        level,
        format,
        argp);
    va_end(argp);
}
