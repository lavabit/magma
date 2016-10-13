#ifndef DIME_SGNT_KEYS_H
#define DIME_SGNT_KEYS_H

#include "dime/signet/common.h"

typedef enum {
    KEYS_TYPE_ERROR = 0,
    KEYS_TYPE_ORG,
    KEYS_TYPE_USER
} keys_type_t;

/* PUBLIC FUNCTIONS */

int           dime_keys_file_create(keys_type_t type, ED25519_KEY *sign_key, EC_KEY *enc_key, const char *filename);
EC_KEY *      dime_keys_enckey_fetch(const char *filename);
ED25519_KEY * dime_keys_signkey_fetch(const char *filename);
char *        dime_keys_generate(keys_type_t type);

#if 0
 not implemented yet TODO

int           dime_keys_file_sok_add(ED25519_KEY *sok, sok_permissions_t perm, const char *filename);
ED25519_KEY * dime_keys_sok_fetch_by_num(char const *filename, unsigned int num);

#endif

#endif
