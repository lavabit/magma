#ifndef ERROR_CODES_H
#define ERROR_CODES_H

typedef enum {
  ERRCODE_NOMEM,
  ERRCODE_CRYPTO,
  ERRCODE_BAD_PARAM,
  ERRCODE_FILE_IO,
  ERRCODE_ENCODING
} dime_errcode_t;

typedef struct {
  dime_errcode_t code;
  char const * message;
} derror_t;

extern derror_t const * const
ERR_CRYPTO;
extern derror_t const * const
ERR_NOMEM;
extern derror_t const * const
ERR_BAD_PARAM;
extern derror_t const * const
ERR_FILE_IO;
extern derror_t const * const
ERR_ENCODING;

#endif
