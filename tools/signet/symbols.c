
/**
 * @file /magma/providers/symbols.c
 *
 * @brief Functions used to load the external library symbols.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <stdbool.h>

#include "symbols.h"

//! OPENSSL
DH * (*DH_new_d)(void) = NULL;
char **SSL_version_str_d = NULL;
int (*SSL_connect_d)(SSL *ssl) = NULL;
const SSL_METHOD * (*SSLv23_client_method_d)(void) = NULL;
const SSL_METHOD * (*TLSv1_server_method_d)(void) = NULL;
void (*DH_free_d)(DH *dh) = NULL;
int (*RAND_status_d)(void) = NULL;
void (*EVP_cleanup_d)(void) = NULL;
void (*OBJ_cleanup_d)(void) = NULL;
void (*BN_free_d)(BIGNUM *a) = NULL;
void (*RAND_cleanup_d)(void) = NULL;
void (*SSL_free_d)(SSL *ssl) = NULL;
int (*SSL_accept_d)(SSL *ssl) = NULL;
EC_KEY * (*EC_KEY_new_d)(void) = NULL;
void (*CRYPTO_free_d) (void *) = NULL;
void (*ENGINE_cleanup_d)(void) = NULL;
int (*CRYPTO_num_locks_d)(void) = NULL;
int (*SSL_library_init_d)(void) = NULL;
int (*SSL_want_d)(const SSL *s) = NULL;
int (*SSL_shutdown_d)(SSL *ssl) = NULL;
void (*BIO_sock_cleanup_d)(void) = NULL;
void (*ERR_free_strings_d)(void) = NULL;
SSL * (*SSL_new_d)(SSL_CTX * ctx) = NULL;
const EVP_MD * (*EVP_md4_d)(void) = NULL;
const EVP_MD * (*EVP_md5_d)(void) = NULL;
const EVP_MD * (*EVP_sha_d)(void) = NULL;
void (*COMP_zlib_cleanup_d)(void) = NULL;
int (*SSL_get_rfd_d)(const SSL *s) = NULL;
const EVP_MD * (*EVP_sha1_d)(void) = NULL;
void (*EC_KEY_free_d)(EC_KEY *key) = NULL;
const char * (*OBJ_nid2sn_d)(int n) = NULL;
const EVP_MD * (*EVP_sha224_d)(void) = NULL;
const EVP_MD * (*EVP_sha256_d)(void) = NULL;
const EVP_MD * (*EVP_sha384_d)(void) = NULL;
const EVP_MD * (*EVP_sha512_d)(void) = NULL;
void (*OBJ_NAME_cleanup_d)(int type) = NULL;
void (*SSL_CTX_free_d)(SSL_CTX *ctx) = NULL;
int (*SSL_pending_d)(const SSL *ssl) = NULL;
int	(*BN_num_bits_d)(const BIGNUM *) = NULL;
int (*X509_get_ext_count_d) (X509 *x) = NULL;
char * (*BN_bn2hex_d)(const BIGNUM *a) = NULL;
int (*EVP_MD_size_d)(const EVP_MD *md) = NULL;
unsigned long (*ERR_get_error_d)(void) = NULL;
void (*CONF_modules_unload_d)(int all) = NULL;
void (*HMAC_CTX_init_d)(HMAC_CTX *ctx) = NULL;
void (*SSL_load_error_strings_d)(void) = NULL;
int (*EVP_MD_type_d)(const EVP_MD *md) = NULL;
const EVP_MD * (*EVP_ripemd160_d)(void) = NULL;
const char * (*SSLeay_version_d)(int t) = NULL;
BIO * (*SSL_get_wbio_d)(const SSL * ssl) = NULL;
void (*EC_GROUP_free_d)(EC_GROUP *group) = NULL;
void (*EC_POINT_free_d)(EC_POINT *point) = NULL;
int (*EC_KEY_generate_key_d)(EC_KEY *key) = NULL;
void (*ASN1_STRING_TABLE_cleanup_d)(void) = NULL;
void (*HMAC_CTX_cleanup_d)(HMAC_CTX *ctx) = NULL;
int (*SSL_get_shutdown_d)(const SSL *ssl) = NULL;
void (*CRYPTO_cleanup_all_ex_data_d)(void) = NULL;
void (*EVP_MD_CTX_init_d)(EVP_MD_CTX *ctx) = NULL;
int (*EC_KEY_check_key_d)(const EC_KEY *key) = NULL;
int (*EVP_MD_CTX_cleanup_d)(EVP_MD_CTX *ctx) = NULL;
int (*SSL_peek_d)(SSL *ssl,void *buf,int num) = NULL;
X509_NAME *	(*X509_get_subject_name_d)(X509 *a) = NULL;
EC_KEY * (*EC_KEY_new_by_curve_name_d)(int nid) = NULL;
int (*BN_hex2bn_d)(BIGNUM **a, const char *str) = NULL;
int (*SSL_read_d)(SSL *ssl, void *buf, int num) = NULL;
int (*RAND_bytes_d)(unsigned char *buf, int num) = NULL;
void (*EVP_CIPHER_CTX_init_d)(EVP_CIPHER_CTX *a) = NULL;
int (*EVP_CIPHER_nid_d)(const EVP_CIPHER *cipher) = NULL;
void (*OPENSSL_add_all_algorithms_noconf_d)(void) = NULL;
int	(*SSL_get_error_d)(const SSL *s,int ret_code) = NULL;
const SSL_METHOD * (*SSLv23_server_method_d)(void) = NULL;
X509 * (*SSL_get_peer_certificate_d)(const SSL *s) = NULL;
int (*EVP_CIPHER_CTX_cleanup_d)(EVP_CIPHER_CTX *a) = NULL;
BIO * (*BIO_new_socket_d)(int sock, int close_flag) = NULL;
EC_GROUP * (*EC_GROUP_new_by_curve_name_d)(int nid) = NULL;
EC_POINT * (*EC_POINT_new_d)(const EC_GROUP *group) = NULL;
int	(*BN_bn2bin_d)(const BIGNUM *, unsigned char *) = NULL;
X509_EXTENSION * (*X509_get_ext_d) (X509 *x, int loc) = NULL;
SSL_CTX * (*SSL_CTX_new_d)(const SSL_METHOD * method) = NULL;
void (*SSL_set_bio_d)(SSL *ssl, BIO *rbio, BIO *wbio) = NULL;
int (*SSL_CTX_check_private_key_d)(const SSL_CTX *ctx) = NULL;
int (*SSL_write_d)(SSL *ssl, const void *buf, int num) = NULL;
void (*sk_pop_free_d)(_STACK *st, void(*func)(void *)) = NULL;
int (*EVP_CIPHER_iv_length_d)(const EVP_CIPHER *cipher) = NULL;
char * (*ERR_error_string_d)(unsigned long e, char *buf) = NULL;
int (*EVP_CIPHER_block_size_d)(const EVP_CIPHER *cipher) = NULL;
int (*EVP_CIPHER_key_length_d)(const EVP_CIPHER *cipher) = NULL;
const EC_GROUP * (*EC_KEY_get0_group_d)(const EC_KEY *key) = NULL;
const EVP_MD * (*EVP_get_digestbyname_d)(const char *name) = NULL;
int	(*SSL_CTX_set_cipher_list_d)(SSL_CTX *,const char *str) = NULL;
int (*EVP_CIPHER_CTX_iv_length_d)(const EVP_CIPHER_CTX *ctx) = NULL;
int (*EVP_DigestInit_d)(EVP_MD_CTX *ctx, const EVP_MD *type) = NULL;
int (*EC_KEY_set_group_d)(EC_KEY *key, const EC_GROUP *group) = NULL;
int (*EVP_CIPHER_CTX_block_size_d)(const EVP_CIPHER_CTX *ctx) = NULL;
int (*EVP_CIPHER_CTX_key_length_d)(const EVP_CIPHER_CTX *ctx) = NULL;
int (*RAND_load_file_d)(const char *filename, long max_bytes) = NULL;
void (*ERR_remove_thread_state_d)(const CRYPTO_THREADID *tid) = NULL;
unsigned long (*EVP_CIPHER_flags_d)(const EVP_CIPHER *cipher) = NULL;
const BIGNUM * (*EC_KEY_get0_private_key_d)(const EC_KEY *key) = NULL;
const EVP_CIPHER * (*EVP_get_cipherbyname_d)(const char *name) = NULL;
const EC_POINT * (*EC_KEY_get0_public_key_d)(const EC_KEY *key) = NULL;
int (*EC_GROUP_precompute_mult_d)(EC_GROUP *group, BN_CTX *ctx) = NULL;
int (*EC_KEY_set_private_key_d)(EC_KEY *key, const BIGNUM *prv) = NULL;
int (*EVP_CIPHER_CTX_set_padding_d)(EVP_CIPHER_CTX *c, int pad) = NULL;
STACK_OF(SSL_COMP) * (*SSL_COMP_get_compression_methods_d)(void) = NULL;
int (*BIO_vprintf_d)(BIO *bio, const char *format, va_list args) = NULL;
int (*EC_KEY_set_public_key_d)(EC_KEY *key, const EC_POINT *pub) = NULL;
unsigned long (*EVP_CIPHER_CTX_flags_d)(const EVP_CIPHER_CTX *ctx) = NULL;
void (*CRYPTO_set_id_callback_d)(unsigned long(*id_function)(void)) = NULL;
long (*SSL_CTX_ctrl_d)(SSL_CTX *ctx, int cmd, long larg, void *parg) = NULL;
void (*ERR_error_string_n_d)(unsigned long e, char *buf, size_t len) = NULL;
BIGNUM * (*BN_bin2bn_d)(const unsigned char *s, int len, BIGNUM *ret) = NULL;
int (*EVP_DigestUpdate_d)(EVP_MD_CTX *ctx, const void *d, size_t cnt) = NULL;
int (*HMAC_Final_d)(HMAC_CTX *ctx, unsigned char *md, unsigned int *len) = NULL;
int (*HMAC_Update_d)(HMAC_CTX *ctx, const unsigned char *data, size_t len) = NULL;
int (*SSL_CTX_use_certificate_chain_file_d)(SSL_CTX *ctx, const char *file) = NULL;
int (*EVP_DigestFinal_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) = NULL;
int (*EVP_DigestInit_ex_d)(EVP_MD_CTX *ctx, const EVP_MD *type, ENGINE *impl) = NULL;
int (*SSL_CTX_use_PrivateKey_file_d)(SSL_CTX *ctx, const char *file, int type) = NULL;
int (*EVP_CIPHER_CTX_ctrl_d)(EVP_CIPHER_CTX *ctx, int type, int arg, void *ptr) = NULL;
int (*X509_NAME_get_text_by_NID_d)(X509_NAME *name, int nid, char *buf,int len) = NULL;
int (*EVP_DigestFinal_ex_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) = NULL;
int (*EVP_EncryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl) = NULL;
int (*EVP_DecryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl) = NULL;
void (*EC_GROUP_set_point_conversion_form_d)(EC_GROUP *, point_conversion_form_t) = NULL;
int	(*DH_generate_parameters_ex_d)(DH *dh, int prime_len,int generator, BN_GENCB *cb) = NULL;
EC_POINT * (*EC_POINT_hex2point_d)(const EC_GROUP *, const char *, EC_POINT *, BN_CTX *) = NULL;
int (*SSL_CTX_load_verify_locations_d)(SSL_CTX *ctx, const char *CAfile, const char *CApath) = NULL;
int (*HMAC_Init_ex_d)(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md, ENGINE *impl) = NULL;
void (*SSL_CTX_set_tmp_dh_callback_d)(SSL_CTX *ctx, DH *(*dh)(SSL *ssl,int is_export, int keylength))  = NULL;
char * (*EC_POINT_point2hex_d)(const EC_GROUP *, const EC_POINT *, point_conversion_form_t form, BN_CTX *) = NULL;
void (*CRYPTO_set_locking_callback_d)(void(*locking_function)(int mode, int n, const char *file, int line)) = NULL;
void (*SSL_CTX_set_tmp_ecdh_callback_d)(SSL_CTX *ctx, EC_KEY *(*ecdh)(SSL *ssl,int is_export, int keylength)) = NULL;
int (*EVP_DecryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl) = NULL;
int (*EVP_EncryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl) = NULL;
int (*EC_POINT_oct2point_d)(const EC_GROUP *group, EC_POINT *p, const unsigned char *buf, size_t len, BN_CTX *ctx) = NULL;
int (*EVP_Digest_d)(const void *data, size_t count, unsigned char *md, unsigned int *size, const EVP_MD *type, ENGINE *impl) = NULL;
int (*EVP_DecryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv) = NULL;
int (*EVP_EncryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv) = NULL;
size_t (*EC_POINT_point2oct_d)(const EC_GROUP *group, const EC_POINT *p, point_conversion_form_t form, unsigned char *buf, size_t len, BN_CTX *ctx) = NULL;
int (*ECDH_compute_key_d)(void *out, size_t outlen, const EC_POINT *pub_key, EC_KEY *ecdh, void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen)) = NULL;
void *(*sk_pop_d)(_STACK *st) = NULL;
int (*i2d_OCSP_RESPONSE_d)(OCSP_RESPONSE *a, unsigned char **out) = NULL;
ECDSA_SIG * (*ECDSA_do_sign_d)(const unsigned char *dgst, int dgst_len, EC_KEY *eckey) = NULL;
void (*ECDSA_SIG_free_d)(ECDSA_SIG *a) = NULL;
int (*i2d_OCSP_CERTID_d)(OCSP_CERTID *a, unsigned char **out) = NULL;
OCSP_RESPONSE * (*d2i_OCSP_RESPONSE_d)(OCSP_RESPONSE **a, const unsigned char **in, long len) = NULL;
OCSP_REQUEST * (*OCSP_REQUEST_new_d)(void) = NULL;
void (*OCSP_BASICRESP_free_d)(OCSP_BASICRESP *a) = NULL;
int (*i2d_X509_d)(X509 *a, unsigned char **out) = NULL;
long (*SSL_CTX_callback_ctrl_d)(SSL_CTX *, int, void (*)(void)) = NULL;
long (*SSL_ctrl_d)(SSL *s, int cmd, long larg, void *parg) = NULL;
ASN1_STRING * (*X509_NAME_ENTRY_get_data_d)(X509_NAME_ENTRY *ne) = NULL;
BIGNUM * (*ASN1_INTEGER_to_BN_d)(const ASN1_INTEGER *ai, BIGNUM *bn) = NULL;
BIO * (*BIO_new_fp_d)(FILE *stream, int close_flag) = NULL;
char * (*X509_NAME_oneline_d)(X509_NAME *a, char *buf, int len) = NULL;
const char * (*OCSP_response_status_str_d)(long s) = NULL;
const char * (*X509_verify_cert_error_string_d)(long n) = NULL;
const EVP_CIPHER * (*EVP_aes_256_cbc_d)(void) = NULL;
EC_KEY * (*d2i_ECPrivateKey_d)(EC_KEY **key, const unsigned char **in, long len) = NULL;
EC_KEY * (*o2i_ECPublicKey_d)(EC_KEY **key, const unsigned char **in, long len) = NULL;
ECDSA_SIG * (*d2i_ECDSA_SIG_d)(ECDSA_SIG **sig, const unsigned char **pp, long len) = NULL;
EVP_CIPHER_CTX * (*EVP_CIPHER_CTX_new_d)(void) = NULL;
EVP_PKEY * (*EVP_PKEY_new_d)(void) = NULL;
int (*ASN1_GENERALIZEDTIME_print_d)(BIO *fp, const ASN1_GENERALIZEDTIME *a) = NULL;
int (*BIO_free_d)(BIO *a) = NULL;
int (*ECDSA_do_verify_d)(const unsigned char *dgst, int dgst_len, const ECDSA_SIG *sig, EC_KEY *eckey) = NULL;
int (*EVP_PKEY_set1_RSA_d)(EVP_PKEY *pkey, struct rsa_st *key) = NULL;
int (*EVP_VerifyFinal_d)(EVP_MD_CTX *ctx, const unsigned char *sigbuf, unsigned int siglen, EVP_PKEY *pkey) = NULL;
int (*i2d_ECDSA_SIG_d)(const ECDSA_SIG *sig, unsigned char **pp) = NULL;
int (*i2d_ECPrivateKey_d)(EC_KEY *key, unsigned char **out) = NULL;
int (*i2o_ECPublicKey_d)(EC_KEY *key, unsigned char **out) = NULL;
int (*OCSP_basic_verify_d)(void *bs, struct stack_st_X509 *certs, struct x509_store_st *st, unsigned long flags) = NULL;
int (*OCSP_check_nonce_d)(void *req, void *bs) = NULL;
int (*OCSP_check_validity_d)(ASN1_GENERALIZEDTIME *thisupd, ASN1_GENERALIZEDTIME *nextupd, long sec, long maxsec) = NULL;
int (*OCSP_parse_url_d)(const char *url, char **phost, char **pport, char **ppath, int *pssl) = NULL;
int (*OCSP_REQ_CTX_add1_header_d)(OCSP_REQ_CTX *rctx, const char *name, const char *value) = NULL;
int (*OCSP_REQ_CTX_set1_req_d)(OCSP_REQ_CTX *rctx, void *req) = NULL;
int (*OCSP_request_add1_nonce_d)(void *req, unsigned char *val, int len) = NULL;
int (*OCSP_REQUEST_print_d)(BIO *bp, void *a, unsigned long flags) = NULL;
int (*OCSP_resp_find_status_d)(void *bs, void *id, int *status, int *reason, ASN1_GENERALIZEDTIME **revtime, ASN1_GENERALIZEDTIME **thisupd, ASN1_GENERALIZEDTIME **nextupd) = NULL;
int (*OCSP_RESPONSE_print_d)(BIO *bp, OCSP_RESPONSE *o, unsigned long flags) = NULL;
int (*OCSP_response_status_d)(OCSP_RESPONSE *resp) = NULL;
int (*OCSP_sendreq_nbio_d)(OCSP_RESPONSE **presp, OCSP_REQ_CTX *rctx) = NULL;
int (*SHA1_Final_d)(unsigned char *md, SHA_CTX *c) = NULL;
int (*SHA1_Init_d)(SHA_CTX *c) = NULL;
int (*SHA1_Update_d)(SHA_CTX *c, const void *data, size_t len) = NULL;
int (*SHA256_Final_d)(unsigned char *md, SHA256_CTX *c) = NULL;
int (*SHA256_Init_d)(SHA256_CTX *c) = NULL;
int (*SHA256_Update_d)(SHA256_CTX *c, const void *data, size_t len) = NULL;
int (*SHA512_Final_d)(unsigned char *md, SHA512_CTX *c) = NULL;
int (*SHA512_Init_d)(SHA512_CTX *c) = NULL;
int (*SHA512_Update_d)(SHA512_CTX *c, const void *data, size_t len) = NULL;
int (*sk_num_d)(const _STACK *) = NULL;
int (*SSL_get_fd_d)(const SSL *s) = NULL;
int (*SSL_set_fd_d)(SSL *s, int fd) = NULL;
int (*X509_check_host_d)(X509 *x, const char *chk, size_t chklen, unsigned int flags, char **peername) = NULL;
int (*X509_check_issued_d)(X509 *issuer, X509 *subject) = NULL;
int (*X509_NAME_get_index_by_NID_d)(X509_NAME *name, int nid, int lastpos) = NULL;
int (*X509_STORE_CTX_get_error_d)(X509_STORE_CTX *ctx) = NULL;
int (*X509_STORE_CTX_get_error_depth_d)(X509_STORE_CTX *ctx) = NULL;
int (*X509_STORE_CTX_init_d)(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509, STACK_OF(X509) *chain) = NULL;
int (*X509_STORE_load_locations_d)(X509_STORE *ctx, const char *file, const char *path) = NULL;
int (*X509_STORE_set_flags_d)(X509_STORE *ctx, unsigned long flags) = NULL;
int (*X509_verify_cert_d)(X509_STORE_CTX *ctx) = NULL;
OCSP_REQ_CTX * (*OCSP_sendreq_new_d)(BIO *io, const char *path, void *req, int maxline) = NULL;
RSA * (*RSA_new_d)(void) = NULL;
RSA * (*RSAPublicKey_dup_d)(RSA *rsa) = NULL;
size_t (*BUF_strlcat_d)(char *dst, const char *src, size_t siz) = NULL;
struct stack_st_OPENSSL_STRING * (*X509_get1_ocsp_d)(X509 *x) = NULL;
struct stack_st_X509 * (*SSL_get_peer_cert_chain_d)(const SSL *s) = NULL;
unsigned char * (*ASN1_STRING_data_d)(ASN1_STRING *x) = NULL;
unsigned char * (*SHA512_d)(const unsigned char *d, size_t n, unsigned char *md) = NULL;
unsigned long (*ERR_peek_error_line_data_d)(const char **file, int *line, const char **data, int *flags) = NULL;
void (*BIO_free_all_d)(BIO *a) = NULL;
void (*EC_GROUP_clear_free_d)(EC_GROUP *group) = NULL;
void (*ERR_load_crypto_strings_d)(void) = NULL;
void (*ERR_print_errors_fp_d)(FILE *fp) = NULL;
void (*EVP_CIPHER_CTX_free_d)(EVP_CIPHER_CTX *a) = NULL;
void (*OCSP_REQUEST_free_d)(OCSP_REQUEST *a) = NULL;
void (*OCSP_RESPONSE_free_d)(OCSP_RESPONSE *a) = NULL;
void (*RSA_free_d)(RSA *r) = NULL;
void (*SSL_CTX_set_verify_d)(SSL_CTX *ctx, int mode, int (*cb) (int, X509_STORE_CTX *)) = NULL;
void (*X509_email_free_d)(struct stack_st_OPENSSL_STRING *sk) = NULL;
void (*X509_STORE_CTX_free_d)(X509_STORE_CTX *ctx) = NULL;
void (*X509_STORE_CTX_set_chain_d)(struct x509_store_ctx_st *ctx, struct stack_st_X509 *sk) = NULL;
void (*X509_STORE_free_d)(X509_STORE *v) = NULL;
void * (*OCSP_cert_to_id_d)(const EVP_MD *dgst, X509 *subject, X509 *issuer) = NULL;
void * (*OCSP_request_add0_id_d)(void *req, void *cid) = NULL;
void * (*OCSP_response_get1_basic_d)(OCSP_RESPONSE *resp) = NULL;
void * (*sk_value_d)(const _STACK *, int) = NULL;
X509 * (*X509_STORE_CTX_get_current_cert_d)(X509_STORE_CTX *ctx) = NULL;
X509_LOOKUP * (*X509_STORE_add_lookup_d)(X509_STORE *v, X509_LOOKUP_METHOD *m) = NULL;
X509_LOOKUP_METHOD * (*X509_LOOKUP_file_d)(void) = NULL;
X509_NAME_ENTRY * (*X509_NAME_get_entry_d)(X509_NAME *name, int loc) = NULL;
X509_STORE * (*X509_STORE_new_d)(void) = NULL;
X509_STORE_CTX * (*X509_STORE_CTX_new_d)(void) = NULL;
void (*ERR_clear_error_d)(void) = NULL;
void (*ERR_put_error_d)(int lib, int func, int reason, const char *file, int line) = NULL;

typedef struct {
	char * name;
	void **pointer;
} symbol_t;

typedef bool bool_t;

void *dynamic_lib_handle = NULL;

#define log_critical(...) printf (__VA_ARGS__)

/**
 * @brief	Initialize and bind an import symbol table.
 * @see		dlsym()
 * @param	count	the number of symbols in the table.
 * @param	symbols	the symbol table to be patched.
 * @return	true on success or false on failure.
 */
bool_t lib_symbols(size_t count, symbol_t symbols[]) {

	if (!count || !dynamic_lib_handle) {
		log_critical("An invalid request was made.\n");
		return false;
	}

	// Scans the symbols array to ensure none of the symbols/pointers have been referenced twice.
	for (size_t i = 0; i < count; i++) {
		for (size_t j = i + 1; j < count; j++) {
			if (symbols[i].pointer == symbols[j].pointer) {
				log_critical("A dynamic function pointer has been referenced twice. {name = %s / pointer = %p}\n", symbols[i].name, symbols[i].pointer);
				return false;
			}
			else if (!strcmp(symbols[i].name, symbols[j].name)) {
				log_critical("A dynamically loaded symbol has been referenced twice. {name = %s / pointer = %p}\n", symbols[i].name, symbols[i].pointer);
				return false;
			}
		}
	}

	// Loop through and setup the function pointers.
	for (size_t i = 0; i < count; i++) {
		if ((*(symbols[i].pointer) = dlsym(dynamic_lib_handle, symbols[i].name)) == NULL) {
			log_critical("Unable to establish a pointer to the function %s.\n", symbols[i].name);
			return false;
		}
	}

	return true;
}

/**
 * @brief	Initialize the OpenSSL library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_openssl(void) {

	symbol_t openssl[] = {
		M_BIND(ASN1_STRING_TABLE_cleanup), M_BIND(BIO_new_socket), M_BIND(BIO_sock_cleanup), M_BIND(BIO_vprintf), M_BIND(BN_bin2bn),
		M_BIND(BN_bn2bin), M_BIND(BN_bn2hex), M_BIND(BN_free), M_BIND(BN_hex2bn), M_BIND(BN_num_bits), M_BIND(COMP_zlib_cleanup),
		M_BIND(CONF_modules_unload), M_BIND(CRYPTO_cleanup_all_ex_data), M_BIND(CRYPTO_free), M_BIND(CRYPTO_num_locks),
		M_BIND(CRYPTO_set_id_callback), M_BIND(CRYPTO_set_locking_callback), M_BIND(DH_free), M_BIND(DH_generate_parameters_ex), M_BIND(DH_new),
		M_BIND(ECDH_compute_key), M_BIND(EC_GROUP_free), M_BIND(EC_GROUP_new_by_curve_name), M_BIND(EC_GROUP_precompute_mult),
		M_BIND(EC_GROUP_set_point_conversion_form), M_BIND(EC_KEY_check_key), M_BIND(EC_KEY_free), M_BIND(EC_KEY_generate_key),
		M_BIND(EC_KEY_get0_group),M_BIND(EC_KEY_get0_private_key), M_BIND(EC_KEY_get0_public_key), M_BIND(EC_KEY_new),
		M_BIND(EC_KEY_new_by_curve_name), M_BIND(EC_KEY_set_group), M_BIND(EC_KEY_set_private_key), M_BIND(EC_KEY_set_public_key),
		M_BIND(EC_POINT_free), M_BIND(EC_POINT_hex2point), M_BIND(EC_POINT_new), M_BIND(EC_POINT_oct2point), M_BIND(EC_POINT_point2hex),
		M_BIND(EC_POINT_point2oct),	M_BIND(ENGINE_cleanup),	M_BIND(ERR_error_string), M_BIND(ERR_error_string_n), M_BIND(ERR_free_strings),
		M_BIND(ERR_get_error), M_BIND(ERR_remove_thread_state), M_BIND(EVP_CIPHER_block_size),	M_BIND(EVP_CIPHER_CTX_block_size),
		M_BIND(EVP_CIPHER_CTX_cleanup),	M_BIND(EVP_CIPHER_CTX_init), M_BIND(EVP_CIPHER_CTX_iv_length), M_BIND(EVP_CIPHER_CTX_key_length),
		M_BIND(EVP_CIPHER_CTX_set_padding),	M_BIND(EVP_CIPHER_iv_length), M_BIND(EVP_CIPHER_key_length), M_BIND(EVP_CIPHER_nid),
		M_BIND(EVP_cleanup), M_BIND(EVP_DecryptFinal_ex), M_BIND(EVP_DecryptInit_ex), M_BIND(EVP_DecryptUpdate), M_BIND(EVP_Digest),
		M_BIND(EVP_DigestFinal), M_BIND(EVP_DigestFinal_ex), M_BIND(EVP_DigestInit), M_BIND(EVP_DigestInit_ex), M_BIND(EVP_DigestUpdate),
		M_BIND(EVP_EncryptFinal_ex), M_BIND(EVP_EncryptInit_ex), M_BIND(EVP_EncryptUpdate),	M_BIND(EVP_get_cipherbyname),
		M_BIND(EVP_get_digestbyname), M_BIND(EVP_md4), M_BIND(EVP_md5),	M_BIND(EVP_MD_CTX_cleanup),	M_BIND(EVP_MD_CTX_init),
		M_BIND(EVP_MD_size), M_BIND(EVP_ripemd160),	M_BIND(EVP_sha), M_BIND(EVP_sha1),	M_BIND(EVP_sha224),	M_BIND(EVP_sha256),
		M_BIND(EVP_sha384),	M_BIND(EVP_sha512),	M_BIND(HMAC_CTX_cleanup), M_BIND(HMAC_CTX_init), M_BIND(HMAC_Final), M_BIND(HMAC_Init_ex),
		M_BIND(HMAC_Update), M_BIND(OBJ_cleanup), M_BIND(OBJ_NAME_cleanup),	M_BIND(OBJ_nid2sn),	M_BIND(OPENSSL_add_all_algorithms_noconf),
		M_BIND(RAND_bytes),	M_BIND(RAND_cleanup), M_BIND(RAND_load_file), M_BIND(RAND_status), M_BIND(sk_pop_free),	M_BIND(SSL_accept),
		M_BIND(SSL_COMP_get_compression_methods), M_BIND(SSL_connect), M_BIND(SSL_CTX_check_private_key), M_BIND(SSL_CTX_ctrl),
		M_BIND(SSL_CTX_free), M_BIND(SSL_CTX_load_verify_locations), M_BIND(SSL_CTX_new), M_BIND(SSL_CTX_set_cipher_list),
		M_BIND(SSL_CTX_set_tmp_dh_callback), M_BIND(SSL_CTX_set_tmp_ecdh_callback), M_BIND(SSL_CTX_use_certificate_chain_file),
		M_BIND(SSL_CTX_use_PrivateKey_file), M_BIND(SSLeay_version), M_BIND(SSL_free), M_BIND(SSL_get_error), M_BIND(SSL_get_peer_certificate),
		M_BIND(SSL_get_shutdown), M_BIND(SSL_get_wbio), M_BIND(SSL_library_init), M_BIND(SSL_load_error_strings), M_BIND(SSL_new),
		M_BIND(SSL_read), M_BIND(SSL_set_bio), M_BIND(SSL_shutdown), M_BIND(SSLv23_client_method), M_BIND(SSLv23_server_method),
		M_BIND(SSL_version_str), M_BIND(SSL_write), M_BIND(TLSv1_server_method), M_BIND(X509_get_ext), M_BIND(X509_get_ext_count),
		M_BIND(X509_get_subject_name), M_BIND(X509_NAME_get_text_by_NID), M_BIND(EVP_MD_type), M_BIND(SSL_pending), M_BIND(SSL_want),
		M_BIND(SSL_get_rfd), M_BIND(EVP_CIPHER_CTX_ctrl), M_BIND(EVP_CIPHER_CTX_flags), M_BIND(EVP_CIPHER_flags), M_BIND(X509_STORE_CTX_new),
		M_BIND(sk_pop), M_BIND(i2d_OCSP_RESPONSE), M_BIND(ECDSA_do_sign), M_BIND(ECDSA_SIG_free), M_BIND(i2d_OCSP_CERTID), M_BIND(d2i_OCSP_RESPONSE),
		M_BIND(OCSP_REQUEST_new), M_BIND(OCSP_BASICRESP_free), M_BIND(i2d_X509), M_BIND(SSL_CTX_callback_ctrl), M_BIND(SSL_ctrl),
		M_BIND(X509_NAME_ENTRY_get_data), M_BIND(ASN1_INTEGER_to_BN), M_BIND(BIO_new_fp), M_BIND(X509_NAME_oneline), M_BIND(OCSP_response_status_str),
		M_BIND(X509_verify_cert_error_string), M_BIND(EVP_aes_256_cbc), M_BIND(d2i_ECPrivateKey), M_BIND(o2i_ECPublicKey), M_BIND(d2i_ECDSA_SIG),
		M_BIND(EVP_CIPHER_CTX_new), M_BIND(EVP_PKEY_new), M_BIND(ASN1_GENERALIZEDTIME_print), M_BIND(BIO_free), M_BIND(ECDSA_do_verify),
		M_BIND(EVP_PKEY_set1_RSA), M_BIND(EVP_VerifyFinal), M_BIND(i2d_ECDSA_SIG), M_BIND(i2d_ECPrivateKey), M_BIND(i2o_ECPublicKey),
		M_BIND(OCSP_basic_verify), M_BIND(OCSP_check_nonce), M_BIND(OCSP_check_validity), M_BIND(OCSP_parse_url), M_BIND(OCSP_REQ_CTX_add1_header),
		M_BIND(OCSP_REQ_CTX_set1_req), M_BIND(OCSP_request_add1_nonce), M_BIND(OCSP_REQUEST_print), M_BIND(OCSP_resp_find_status),
		M_BIND(OCSP_RESPONSE_print), M_BIND(OCSP_response_status), M_BIND(OCSP_sendreq_nbio), M_BIND(SHA1_Final), M_BIND(SHA1_Init),
		M_BIND(SHA1_Update), M_BIND(SHA256_Final), M_BIND(SHA256_Init), M_BIND(SHA256_Update), M_BIND(SHA512_Final), M_BIND(SHA512_Init),
		M_BIND(SHA512_Update), M_BIND(sk_num), M_BIND(SSL_get_fd), M_BIND(SSL_set_fd), M_BIND(X509_check_host), M_BIND(X509_check_issued),
		M_BIND(X509_NAME_get_index_by_NID), M_BIND(X509_STORE_CTX_get_error), M_BIND(X509_STORE_CTX_get_error_depth), M_BIND(X509_STORE_CTX_init),
		M_BIND(X509_STORE_load_locations), M_BIND(X509_STORE_set_flags), M_BIND(X509_verify_cert), M_BIND(OCSP_sendreq_new), M_BIND(RSA_new),
		M_BIND(RSAPublicKey_dup), M_BIND(BUF_strlcat), M_BIND(X509_get1_ocsp), M_BIND(SSL_get_peer_cert_chain), M_BIND(ASN1_STRING_data),
		M_BIND(SHA512), M_BIND(ERR_peek_error_line_data), M_BIND(BIO_free_all), M_BIND(EC_GROUP_clear_free), M_BIND(ERR_load_crypto_strings),
		M_BIND(ERR_print_errors_fp), M_BIND(EVP_CIPHER_CTX_free), M_BIND(OCSP_REQUEST_free), M_BIND(OCSP_RESPONSE_free), M_BIND(RSA_free),
		M_BIND(SSL_CTX_set_verify), M_BIND(X509_email_free), M_BIND(X509_STORE_CTX_free), M_BIND(X509_STORE_CTX_set_chain), M_BIND(X509_STORE_free),
		M_BIND(OCSP_cert_to_id), M_BIND(OCSP_request_add0_id), M_BIND(OCSP_response_get1_basic), M_BIND(sk_value), M_BIND(X509_STORE_CTX_get_current_cert),
		M_BIND(X509_STORE_add_lookup), M_BIND(X509_LOOKUP_file), M_BIND(X509_NAME_get_entry), M_BIND(X509_STORE_new), M_BIND(ERR_clear_error),
		M_BIND(ERR_put_error)
	};

	if (!lib_symbols(sizeof(openssl) / sizeof(symbol_t), openssl)) {
		return false;
	}

	return true;
}

/**
 * @brief	Close the dynamic sumbols handle.
 * @return	This function returns no value.
 */
void lib_unload(void) {

	dlclose(dynamic_lib_handle);

	return;
}

/**
 * @brief	Create a handle for the currently loaded program, and dynamically resolve all the symbols for external dependencies.
 * @return	0 for success, and a negative number when an error occurs.
 */
int lib_load(void) {

	char *lib_error = NULL;
	dynamic_lib_handle = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
	if (!dynamic_lib_handle || (lib_error = dlerror())) {
		if (lib_error) {
			log_critical("The dlerror() function returned: %s\n", lib_error);
		}
		return -1;
	}

	else if (!lib_load_openssl()) {
		return -1;
	}

	return 0;
}
