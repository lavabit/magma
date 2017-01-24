
/**
 * @file /magma/providers/deprecated/deprecated.h
 *
 * @brief	Functions used to perform cryptographic operations and provide truly random data.
 */

#ifndef MAGMA_PROVIDERS_DEPRECATED_H
#define MAGMA_PROVIDERS_DEPRECATED_H

/// cryptex.c
uint64_t  cryptex_body_length(cryptex_t *cryptex);
uint64_t  cryptex_envelope_length(cryptex_t *cryptex);
uint64_t  cryptex_hmac_length(cryptex_t *cryptex);
uint64_t  cryptex_original_length(cryptex_t *cryptex);
uint64_t  cryptex_total_length(cryptex_t *cryptex);
void *    cryptex_alloc(uint64_t envelope, uint64_t hmac, uint64_t original, uint64_t body);
void *    cryptex_body_data(cryptex_t *cryptex);
void *    cryptex_envelope_data(cryptex_t *cryptex);
void *    cryptex_hmac_data(cryptex_t *cryptex);
void      cryptex_free(cryptex_t *cryptex);

/// ecies.c
uchr_t *      ecies_decrypt(stringer_t *key, ECIES_KEY_TYPE key_type, cryptex_t *cryptex, size_t *length);
cryptex_t *   ecies_encrypt(stringer_t *key, ECIES_KEY_TYPE key_type, unsigned char *data, size_t length);
void *        ecies_envelope_derivation(const void *input, size_t ilen, void *output, size_t *olen);
EC_GROUP *    ecies_group(uint64_t curve, bool_t precompute);
EC_KEY *      ecies_key_alloc(void);
EC_KEY *      ecies_key_create(void);
void          ecies_key_free(EC_KEY *key);
EC_KEY *      ecies_key_private(uint64_t format, placer_t data);
uchr_t *      ecies_key_private_bin(EC_KEY *key, size_t *olen);
stringer_t *  ecies_key_private_hex(EC_KEY *key);
EC_KEY *      ecies_key_public(uint64_t format, placer_t data);
uchr_t *      ecies_key_public_bin(EC_KEY *key, size_t *olen);
stringer_t *  ecies_key_public_hex(EC_KEY *key);
bool_t        ecies_start(void);
void          ecies_stop(void);

/// hmac.c
stringer_t *  hmac_digest(digest_t *digest, stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_md4(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_md5(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_ripemd160(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_sha(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_sha1(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_sha224(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_sha256(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_sha384(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  hmac_sha512(stringer_t *s, stringer_t *key, stringer_t *output);

/// scramble.c
scramble_t *  scramble_alloc(size_t length);
void *        scramble_body_data(scramble_t *buffer);
uint64_t      scramble_body_hash(scramble_t *buffer);
uint64_t      scramble_body_length(scramble_t *buffer);
stringer_t *  scramble_decrypt(stringer_t *key, scramble_t *input);
scramble_t *  scramble_encrypt(stringer_t *key, stringer_t *input);
void          scramble_free(scramble_t *buffer);
void          scramble_cleanup(scramble_t *buffer);
scramble_t *  scramble_import(stringer_t *s);
//uint64_t      scramble_orig_hash(scramble_t *buffer);
uint64_t      scramble_orig_length(scramble_t *buffer);
uint64_t      scramble_total_length(scramble_t *buffer);
void *        scramble_vector_data(scramble_t *buffer);
uint64_t      scramble_vector_length(scramble_t *buffer);

/// symmetric.c
stringer_t *  symmetric_decrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input);
stringer_t *  symmetric_encrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input);
stringer_t *  symmetric_key(cipher_t *cipher, stringer_t *key, stringer_t *output);
stringer_t *  symmetric_vector(cipher_t *cipher, stringer_t *output);

#endif
