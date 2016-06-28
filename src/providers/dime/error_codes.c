#include "dime/error_codes.h"

static derror_t const CRYPTO_ERROR = {
  ERRCODE_CRYPTO,
  "cryptographic error"
};
derror_t const * const
ERR_CRYPTO = &CRYPTO_ERROR;

static derror_t const NOMEM_ERROR = {
  ERRCODE_NOMEM,
  "out of memory error"
};
derror_t const * const
ERR_NOMEM = &NOMEM_ERROR;

static derror_t const BAD_PARAM_ERROR = {
  ERRCODE_BAD_PARAM,
  "bad parameter error"
};
derror_t const * const
ERR_BAD_PARAM = &BAD_PARAM_ERROR;

static derror_t const FILE_IO_ERROR = {
  ERRCODE_FILE_IO,
  "file I/O error"
};
derror_t const * const
ERR_FILE_IO = &FILE_IO_ERROR;

static derror_t const ENCODING_ERROR = {
  ERRCODE_ENCODING,
  "file I/O error"
};
derror_t const * const
ERR_ENCODING = &ENCODING_ERROR;
