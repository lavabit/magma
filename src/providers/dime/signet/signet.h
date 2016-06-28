#ifndef DIME_SGNT_SIGNET_H
#define DIME_SGNT_SIGNET_H

#include <stdint.h>
#include "dime/signet/common.h"

typedef enum {
    SS_UNKNOWN = 0,                 /**< Invalid signet, state unknown/currently unclassified */
    SS_MALFORMED,                   /**< Invalid signet, it either doesn't fit the field format or has multiple unique fields */
    SS_OVERFLOW,                    /**< Invalid signet due to it being too large. */
    SS_INCOMPLETE,                  /**< Invalid signet, it is missing fields required to fit one of the valid categories, likely unsigned */
    SS_BROKEN_COC,                  /**< Invalid signet due to chain of custody signature being invalid*/
    SS_INVALID,                     /**< Invalid signet, one or more signatures can not be verified */
    SS_SSR,                         /**< Valid unsigned SSR */
    SS_CRYPTO,                      /**< Valid cryptographic signet */
    SS_FULL,                        /**< Valid full signet */
    SS_ID,                          /**< Valid full signet with ID and organizational-identifiable-signature */
} signet_state_t;

typedef enum {
    SIGNET_TYPE_ERROR,
    SIGNET_TYPE_ORG = 1,
    SIGNET_TYPE_USER,
    SIGNET_TYPE_SSR
} signet_type_t;

typedef struct {
    signet_type_t type; uint32_t fields[256];           /**< Each index corresponds to a different field type identifier. The value of fields[index] is the byte directly after the first occurence of the corresponding field type identifier. */
                                    /**< If fields[index] is 0 it means that the corresponding field type identifier occurred 0 times.*/
    uint32_t size;                  /**< Combined length of all the fields */
    unsigned char *data;
} signet_t;

EC_KEY *                dime_sgnt_enckey_fetch(const signet_t *signet);
int                     dime_sgnt_enckey_set(signet_t *signet, EC_KEY *key, unsigned char format);
int                     dime_sgnt_fid_count_get(const signet_t *signet, unsigned char fid);
int                     dime_sgnt_fid_exists(const signet_t *signet, unsigned char fid);
unsigned char *         dime_sgnt_fid_num_fetch(const signet_t *signet, unsigned char fid, unsigned int num, size_t *data_size);
int                     dime_sgnt_fid_num_remove(signet_t *signet, unsigned char fid, int num);
int                     dime_sgnt_field_defined_create(signet_t *signet, unsigned char fid, size_t data_size, const unsigned char *data);
int                     dime_sgnt_field_defined_set(signet_t *signet, unsigned char fid, size_t data_size, const unsigned char *data);
int                     dime_sgnt_field_undefined_create(signet_t *signet, size_t name_size, const unsigned char *name, size_t data_size, const unsigned char *data);
unsigned char *         dime_sgnt_field_undefined_fetch(const signet_t *signet, size_t name_size, const unsigned char *name, size_t *data_size);
int                     dime_sgnt_field_undefined_remove(signet_t *signet, size_t name_size, const unsigned char *name);
int                     dime_sgnt_file_create(signet_t *signet, const char *filename);
char *                  dime_sgnt_fingerprint_crypto(const signet_t *signet);
char *                  dime_sgnt_fingerprint_full(const signet_t *signet);
char *                  dime_sgnt_fingerprint_id(const signet_t *signet);
char *                  dime_sgnt_fingerprint_ssr(const signet_t *signet);
char *                  dime_sgnt_id_fetch(signet_t *signet);
int                     dime_sgnt_id_set(signet_t *signet, size_t id_size, const unsigned char *id);
int                     dime_sgnt_msg_sig_verify(const signet_t *signet, ed25519_signature sig, const unsigned char *buf, size_t buf_len);
int                     dime_sgnt_sig_coc_sign(signet_t *signet, ED25519_KEY *key);
int                     dime_sgnt_sig_crypto_sign(signet_t *signet, ED25519_KEY *key);
int                     dime_sgnt_sig_full_sign(signet_t *signet, ED25519_KEY *key);
int                     dime_sgnt_sig_id_sign(signet_t *signet, ED25519_KEY *key);
int                     dime_sgnt_sig_ssr_sign(signet_t *signet, ED25519_KEY *key);
signet_t *              dime_sgnt_signet_binary_deserialize(const unsigned char *in, size_t in_len);
unsigned char *         dime_sgnt_signet_binary_serialize(signet_t *signet, uint32_t *serial_size);
signet_t *              dime_sgnt_signet_b64_deserialize(const char *b64_in);
char *                  dime_sgnt_signet_b64_serialize(signet_t *signet);
signet_t *              dime_sgnt_signet_create(signet_type_t type);
signet_t *              dime_sgnt_signet_create_w_keys(signet_type_t type, const char *keysfile);
signet_t *              dime_sgnt_signet_crypto_split(const signet_t *signet);
void                    dime_sgnt_signet_destroy(signet_t *signet);
void                    dime_sgnt_signet_dump(FILE *fp, signet_t *signet);
signet_t *              dime_sgnt_signet_dupe(signet_t *signet);
signet_t *              dime_sgnt_signet_full_split(const signet_t *signet);
signet_t *              dime_sgnt_signet_load(const char *filename);
ED25519_KEY *           dime_sgnt_signkey_fetch(const signet_t *signet);
ED25519_KEY **          dime_sgnt_signkeys_msg_fetch(const signet_t *signet);
ED25519_KEY **          dime_sgnt_signkeys_signet_fetch(const signet_t *signet);
ED25519_KEY **          dime_sgnt_signkeys_software_fetch(const signet_t *signet);
ED25519_KEY **          dime_sgnt_signkeys_tls_fetch(const signet_t *signet);
int                     dime_sgnt_signkey_set(signet_t *signet, ED25519_KEY *key, unsigned char format);
int                     dime_sgnt_sok_create(signet_t *signet, ED25519_KEY *key, unsigned char format, uint8_t perm);
ED25519_KEY *           dime_sgnt_sok_num_fetch(const signet_t *signet, unsigned int num);
const char *            dime_sgnt_state_to_str(signet_state_t state);
signet_type_t           dime_sgnt_type_get(const signet_t *signet);
int                     dime_sgnt_type_set(signet_t *signet, signet_type_t type);
signet_state_t          dime_sgnt_validate_all(const signet_t *signet, const signet_t *previous, const signet_t *orgsig, const unsigned char **dime_pok);


#endif
