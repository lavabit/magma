
/**
 * @file /magma/providers/cryptography/cryptography.h
 *
 * @brief	Functions used to perform cryptographic operations and provide truly random data.
 */

#ifndef MAGMA_PROVIDERS_CRYPTOGRAPHY_H
#define MAGMA_PROVIDERS_CRYPTOGRAPHY_H

// These OpenSSL macros need to be redefined here to avoid compilation problems.
#define BN_num_bytes_d(a)		((BN_num_bits_d(a)+7)/8)
#define OPENSSL_free_d(addr)	CRYPTO_free_d(addr)

// The list of ciphers support depending on the SSL security level required.
#define MAGMA_CIPHERS_HIGH		"ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-CHACHA20-POLY1305"
#define MAGMA_CIPHERS_MEDIUM	"EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+SHA384:EECDH+ECDSA+SHA256:EECDH+aRSA+SHA384:EECDH+aRSA+SHA256:EECDH+aRSA+RC4:EECDH:EDH+aRSA:!RC4:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!SRP:!DSS:!SSLv2:!RC4-SHA:!SEED"
#define MAGMA_CIPHERS_LOW		"EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+SHA384:EECDH+ECDSA+SHA256:EECDH+aRSA+SHA384:EECDH+aRSA+SHA256:EECDH+aRSA+RC4:EECDH:EDH+aRSA:!RC4:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!SRP:!DSS:!SSLv2:!RC4-SHA:!SEED"
#define MAGMA_CIPHERS_GENERIC	"HIGH:MEDIUM"

// The default algorithms used by ECIES interface.
#define ECIES_HMAC NID_sha512
#define ECIES_CURVE NID_sect571k1
#define ECIES_CIPHER NID_aes_256_cbc
#define ECIES_ENVELOPE NID_sha512

typedef enum {
	ECIES_PRIVATE_HEX = 1,
	ECIES_PRIVATE_BINARY = 2,

	ECIES_PUBLIC_HEX = 4,
	ECIES_PUBLIC_BINARY = 8
} ECIES_KEY_TYPE;

typedef struct __attribute__ ((packed)) {
	struct {
		uint64_t envelope;
		uint64_t hmac;
		uint64_t original;
		uint64_t body;
	} length;
} cryptex_head_t;

typedef struct __attribute__ ((packed)) {

	uint32_t engine;

	struct {
		uint64_t vector;
		uint64_t original;
		uint64_t scrambled;
	} length;

	struct {
		// uint64_t original; using a cipher that supports creating an hmac.
		uint32_t scrambled;
	} hash;

} scramble_head_t;

typedef void * digest_t;
typedef void * cipher_t;
typedef char * cryptex_t;
typedef stringer_t scramble_t;

// Allows the inclusion of this PRIME header without having to include the OpenSSL headers.
#ifdef HEADER_OPENSSL_TYPES_H
typedef SSL TLS;
#else
typedef void TLS;
#endif

/// ciphers.c
cipher_t *  cipher_id(int_t id);
cipher_t *  cipher_name(stringer_t *name);
int_t       cipher_numeric_id(cipher_t *c);
int_t       cipher_block_length(cipher_t *c);
int_t       cipher_key_length(cipher_t *c);
int_t       cipher_vector_length(cipher_t *c);

/// cryptex.c
uint64_t  deprecated_cryptex_body_length(cryptex_t *cryptex);
uint64_t  deprecated_cryptex_envelope_length(cryptex_t *cryptex);
uint64_t  deprecated_cryptex_hmac_length(cryptex_t *cryptex);
uint64_t  deprecated_cryptex_original_length(cryptex_t *cryptex);
uint64_t  deprecated_cryptex_total_length(cryptex_t *cryptex);
void *    deprecated_cryptex_alloc(uint64_t envelope, uint64_t hmac, uint64_t original, uint64_t body);
void *    deprecated_cryptex_body_data(cryptex_t *cryptex);
void *    deprecated_cryptex_envelope_data(cryptex_t *cryptex);
void *    deprecated_cryptex_hmac_data(cryptex_t *cryptex);
void      deprecated_cryptex_free(cryptex_t *cryptex);

/// digest.c
digest_t *    digest_id(int_t id);
digest_t *    digest_name(stringer_t *name);

/// ecies.c
uchr_t *      deprecated_ecies_decrypt(stringer_t *key, ECIES_KEY_TYPE key_type, cryptex_t *cryptex, size_t *length);
cryptex_t *   deprecated_ecies_encrypt(stringer_t *key, ECIES_KEY_TYPE key_type, unsigned char *data, size_t length);
void *        deprecated_ecies_envelope_derivation(const void *input, size_t ilen, void *output, size_t *olen);
EC_GROUP *    deprecated_ecies_group(uint64_t curve, bool_t precompute);
EC_KEY *      deprecated_ecies_key_alloc(void);
EC_KEY *      deprecated_ecies_key_create(void);
void          deprecated_ecies_key_free(EC_KEY *key);
EC_KEY *      deprecated_ecies_key_private(uint64_t format, placer_t data);
uchr_t *      deprecated_ecies_key_private_bin(EC_KEY *key, size_t *olen);
stringer_t *  deprecated_ecies_key_private_hex(EC_KEY *key);
EC_KEY *      deprecated_ecies_key_public(uint64_t format, placer_t data);
uchr_t *      deprecated_ecies_key_public_bin(EC_KEY *key, size_t *olen);
stringer_t *  deprecated_ecies_key_public_hex(EC_KEY *key);
bool_t        deprecated_ecies_start(void);
void          deprecated_ecies_stop(void);

/// hash.c
stringer_t *  hash_digest(digest_t *digest, stringer_t *s, stringer_t *output);
stringer_t *  hash_md4(stringer_t *s, stringer_t *output);
stringer_t *  hash_md5(stringer_t *s, stringer_t *output);
stringer_t *  hash_ripemd160(stringer_t *s, stringer_t *output);
stringer_t *  hash_sha(stringer_t *s, stringer_t *output);
stringer_t *  hash_sha1(stringer_t *s, stringer_t *output);
stringer_t *  hash_sha224(stringer_t *s, stringer_t *output);
stringer_t *  hash_sha256(stringer_t *s, stringer_t *output);
stringer_t *  hash_sha384(stringer_t *s, stringer_t *output);
stringer_t *  hash_sha512(stringer_t *s, stringer_t *output);

/// hmac.c
stringer_t *  deprecated_hmac_digest(digest_t *digest, stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_md4(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_md5(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_ripemd160(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_sha(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_sha1(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_sha224(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_sha256(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_sha384(stringer_t *s, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_hmac_sha512(stringer_t *s, stringer_t *key, stringer_t *output);

/// openssl.c
bool_t           lib_load_openssl(void);
const char *     lib_version_openssl(void);
DH *             ssl_dh2048_exchange_callback(SSL *ssl, int is_export, int keylength);
DH *             ssl_dh4096_exchange_callback(SSL *ssl, int is_export, int keylength);
int              ssl_dh_generate_callback(int p, int n, BN_GENCB *cb);
EC_KEY *         ssl_ecdh_exchange_callback(SSL *ssl, int is_export, int keylength);
char *           ssl_error_string(chr_t *buffer, int_t length);
void             ssl_locking_callback(int mode, int n, const char *file, int line);
bool_t           ssl_start(void);
void             ssl_stop(void);
unsigned long    ssl_thread_id_callback(void);
void             ssl_thread_stop(void);
bool_t           ssl_verify_privkey(const char *keyfile);

/// tls.c
bool_t        ssl_server_create(void *server, uint_t security_level);
void          ssl_server_destroy(void *server);
int_t         tls_bits(TLS *tls);
stringer_t *  tls_cipher(TLS *tls, stringer_t *output);
void *        tls_client_alloc(int_t sockd);
stringer_t *  tls_error(TLS *tls, int_t code, stringer_t *output);
void          tls_free(TLS *tls);
int           tls_print(TLS *tls, const char *format, va_list args);
int           tls_read(TLS *tls, void *buffer, int length, bool_t block);
TLS *         tls_server_alloc(void *server, int sockd, int flags);
int           tls_status(TLS *tls);
chr_t *       tls_suite(TLS *tls);
chr_t *       tls_version(TLS *tls);
int           tls_write(TLS *tls, const void *buffer, int length, bool_t block);

/// random.c
bool_t        rand_start(void);
bool_t        rand_thread_start(void);
int16_t       rand_get_int16(void);
int32_t       rand_get_int32(void);
int64_t       rand_get_int64(void);
int8_t        rand_get_int8(void);
size_t        rand_write(stringer_t *s);
stringer_t *  rand_choices(chr_t *choices, size_t len, stringer_t *output);
uint16_t      rand_get_uint16(void);
uint32_t      rand_get_uint32(void);
uint64_t      rand_get_uint64(void);
uint8_t       rand_get_uint8(void);
void          rand_stop(void);

/// scramble.c
scramble_t *  deprecated_scramble_alloc(size_t length);
void *        deprecated_scramble_body_data(scramble_t *buffer);
uint64_t      deprecated_scramble_body_hash(scramble_t *buffer);
uint64_t      deprecated_scramble_body_length(scramble_t *buffer);
stringer_t *  deprecated_scramble_decrypt(stringer_t *key, scramble_t *input);
scramble_t *  deprecated_scramble_encrypt(stringer_t *key, stringer_t *input);
void          deprecated_scramble_free(scramble_t *buffer);
void          deprecated_scramble_cleanup(scramble_t *buffer);
scramble_t *  deprecated_scramble_import(stringer_t *s);
uint64_t      deprecated_scramble_orig_length(scramble_t *buffer);
uint64_t      deprecated_scramble_total_length(scramble_t *buffer);
void *        deprecated_scramble_vector_data(scramble_t *buffer);
uint64_t      deprecated_scramble_vector_length(scramble_t *buffer);

/// symmetric.c
stringer_t *  deprecated_symmetric_decrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input);
stringer_t *  deprecated_symmetric_encrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input);
stringer_t *  deprecated_symmetric_key(cipher_t *cipher, stringer_t *key, stringer_t *output);
stringer_t *  deprecated_symmetric_vector(cipher_t *cipher, stringer_t *output);

/// parameters.c
DH *   dh_exchange_2048(SSL *ssl, int is_export, int keylength);
DH *   dh_exchange_4096(SSL *ssl, int is_export, int keylength);
DH *   dh_params_2048(void);
DH *   dh_params_4096(void);
void   dh_params_generate(void);
int    dh_params_generate_callback(int p, int n, BN_GENCB *cb);
DH *   dh_static_2048(SSL *ssl, int is_export, int keylength);
DH *   dh_static_4096(SSL *ssl, int is_export, int keylength);

#endif
