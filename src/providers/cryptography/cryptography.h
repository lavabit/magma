
/**
 * @file /magma/providers/cryptography/cryptography.h
 *
 * @brief	Functions used to perform cryptographic operations and provide truly random data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PROVIDERS_CRYPTOGRAPHY_H
#define MAGMA_PROVIDERS_CRYPTOGRAPHY_H

// These OpenSSL macros need to be redefined here to avoid compilation problems.
#define BN_num_bytes_d(a)		((BN_num_bits_d(a)+7)/8)
#define OPENSSL_free_d(addr)	CRYPTO_free_d(addr)

// The list of ciphers support depending on the SSL security level required.
#define MAGMA_CIPHERS_HIGH		"ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-CHACHA20-POLY1305"
#define MAGMA_CIPHERS_MEDIUM	"EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+SHA384:EECDH+ECDSA+SHA256:EECDH+aRSA+SHA384:EECDH+aRSA+SHA256:EECDH+aRSA+RC4:EECDH:EDH+aRSA:RC4:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!SRP:!DSS:!SSLv2:!RC4-SHA:!SEED"
#define MAGMA_CIPHERS_LOW		"EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+SHA384:EECDH+ECDSA+SHA256:EECDH+aRSA+SHA384:EECDH+aRSA+SHA256:EECDH+aRSA+RC4:EECDH:EDH+aRSA:!RC4:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!SRP:!DSS:!SSLv2:!RC4-SHA:!SEED"
#define MAGMA_CIPHERS_GENERIC	"HIGH:MEDIUM"

// The default algorithms used by ECIES interface.
#define ECIES_HMAC NID_sha512
#define ECIES_CURVE NID_sect571k1
#define ECIES_CIPHER NID_aes_256_cbc
#define ECIES_ENVELOPE NID_sha512

// The STACIE number constants for clamping hash rounds between 8 and 16,777,216, which represents the number of possible
// values for an unsigned 24 bit integer, if you include 0. In other words 0 to 16,777,215 equals 16,777,216.
#define STACIE_KEY_ROUNDS_MIN		8
#define STACIE_KEY_ROUNDS_MAX		16777216

// The STACIE token derivation stage uses a fixed number of hash rounds, and that number is dictated by this parameter.
#define STACIE_TOKEN_ROUNDS			8

// This STACIE implementation will always use salt and nonce values which are 128 bytes in length.
#define STACIE_SALT_LENGTH		128
#define STACIE_NONCE_LENGTH		128

// This STACIE implementation uses SHA-2/512 resulting in key, token, and shard lengths of 64 bytes.
#define STACIE_KEY_LENGTH		64
#define STACIE_TOKEN_LENGTH		64
#define STACIE_SHARD_LENGTH		64

// This STACIE implementation only supports realm encryption of buffers up to 16,777,215 bytews in length.
#define STACIE_ENCRYPT_MIN		1
#define STACIE_ENCRYPT_MAX		16777215
#define STACIE_BLOCK_LENGTH		16
#define STACIE_ENVELOPE_LENGTH	34

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

/// ciphers.c
cipher_t *  cipher_id(int_t id);
cipher_t *  cipher_name(stringer_t *name);
int_t       cipher_numeric_id(cipher_t *c);
int_t       cipher_block_length(cipher_t *c);
int_t       cipher_key_length(cipher_t *c);
int_t       cipher_vector_length(cipher_t *c);

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

/// digest.c
digest_t *    digest_id(int_t id);
digest_t *    digest_name(stringer_t *name);

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
int      ssl_print(SSL *ssl, const char *format, va_list args);
int      ssl_read(SSL *ssl, void *buffer, int length, bool_t block);
bool_t   ssl_server_create(void *server, uint_t security_level);
void     ssl_server_destroy(void *server);
int      ssl_write(SSL *ssl, const void *buffer, int length);
void *   tls_client_alloc(int_t sockd);
void     tls_free(SSL *ssl);
SSL *    tls_server_alloc(void *server, int sockd, int flags);
int      tls_status(SSL *ssl);

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

/// stacie.c
stringer_t *  stacie_entropy_seed_derive(uint32_t rounds, stringer_t *password, stringer_t *salt);
stringer_t *  stacie_hashed_key_derive(stringer_t *base, uint32_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt);
stringer_t *  stacie_hashed_token_derive(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce);
stringer_t *  stacie_nonce_create(stringer_t *output);
stringer_t *  stacie_realm_cipher_key(stringer_t *realm_key);
stringer_t *  stacie_realm_decrypt(stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer);
stringer_t *  stacie_realm_encrypt(uint16_t serial, stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer);
stringer_t *  stacie_realm_key_derive(stringer_t *master_key, stringer_t *realm, stringer_t *shard);
stringer_t *  stacie_realm_tag_key(stringer_t *realm_key);
stringer_t *  stacie_realm_vector_key(stringer_t *realm_key);
uint32_t      stacie_rounds_calculate(stringer_t *password, uint32_t bonus);
stringer_t *  stacie_salt_create(stringer_t *output);
stringer_t *  stacie_shard_create(stringer_t *output);

/// symmetric.c
stringer_t *  symmetric_decrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input);
stringer_t *  symmetric_encrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input);
stringer_t *  symmetric_key(cipher_t *cipher, stringer_t *key, stringer_t *output);
stringer_t *  symmetric_vector(cipher_t *cipher, stringer_t *output);

/// parameters.c
DH *   dh_exchange_2048(SSL *ssl, int is_export, int keylength);
DH *   dh_exchange_4096(SSL *ssl, int is_export, int keylength);
DH *   dh_params_2048(void);
DH *   dh_params_4096(void);
void   dh_params_generate(void);
int    dh_params_generate_callback(int p, int n, BN_GENCB *cb);

#endif
