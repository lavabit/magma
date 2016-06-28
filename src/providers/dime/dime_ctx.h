#ifndef DIME_CTX_H
#define DIME_CTX_H

#include "dime/error_codes.h"

#include <stddef.h>
#include <stdarg.h>

typedef enum {
  LOG_CODE_DEBUG,
  LOG_CODE_INFO,
  LOG_CODE_ERROR
} log_code_t;

typedef struct {
    log_code_t code;
    char const *name;
} log_level_t;

extern log_level_t const * const
LOG_LEVEL_DEBUG;

extern log_level_t const * const
LOG_LEVEL_INFO;

extern log_level_t const * const
LOG_LEVEL_ERROR;

typedef void (*log_function_t)(
    char const * file,
    size_t line,
    log_level_t const *level,
    char const *format,
    va_list argp);

struct dime_ctx;
typedef struct dime_ctx dime_ctx_t;

derror_t const *
dime_ctx_new(
    dime_ctx_t **result,
    log_function_t log_callback);

void
dime_ctx_free(
    dime_ctx_t *ctx);

void
dime_ctx_log(
    dime_ctx_t const *ctx,
    char const *file,
    size_t line,
    log_level_t const *level,
    char const *format,
    ...);

#define LOG_DEBUG(ctx, ...) \
    dime_ctx_log( \
        ctx, \
        __FILE__, \
        __LINE__, \
        LOG_LEVEL_DEBUG, \
        __VA_ARGS__)

#define LOG_INFO(ctx, ...) \
    dime_ctx_log( \
        ctx, \
        __FILE__, \
        __LINE__, \
        LOG_LEVEL_INFO, \
        __VA_ARGS__)

#define LOG_ERROR(ctx, ...) \
    dime_ctx_log( \
        ctx, \
        __FILE__, \
        __LINE__, \
        LOG_LEVEL_ERROR, \
        __VA_ARGS__)

#endif
