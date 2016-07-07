
/**
 * @file /magma/providers/symbols.h
 *
 * @brief External function pointers/definitions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PROVIDERS_SYMBOLS_H
#define MAGMA_PROVIDERS_SYMBOLS_H

// OpenSSL
#include <openssl/conf.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/ec.h>
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/ocsp.h>

//! OPENSSL
extern DH * (*DH_new_d)(void);
extern char **SSL_version_str_d;
extern int (*SSL_connect_d)(SSL *ssl);
extern const SSL_METHOD * (*SSLv23_client_method_d)(void);
extern const SSL_METHOD * (*TLSv1_server_method_d)(void);
extern void (*DH_free_d)(DH *dh);
extern int (*RAND_status_d)(void);
extern void (*EVP_cleanup_d)(void);
extern void (*OBJ_cleanup_d)(void);
extern void (*BN_free_d)(BIGNUM *a);
extern void (*RAND_cleanup_d)(void);
extern void (*SSL_free_d)(SSL *ssl);
extern int (*SSL_accept_d)(SSL *ssl);
extern EC_KEY * (*EC_KEY_new_d)(void);
extern void (*CRYPTO_free_d) (void *);
extern void (*ENGINE_cleanup_d)(void);
extern int (*CRYPTO_num_locks_d)(void);
extern int (*SSL_library_init_d)(void);
extern int (*SSL_want_d)(const SSL *s);
extern int (*SSL_shutdown_d)(SSL *ssl);
extern void (*BIO_sock_cleanup_d)(void);
extern void (*ERR_free_strings_d)(void);
extern SSL * (*SSL_new_d)(SSL_CTX * ctx);
extern const EVP_MD * (*EVP_md4_d)(void);
extern const EVP_MD * (*EVP_md5_d)(void);
extern const EVP_MD * (*EVP_sha_d)(void);
extern void (*COMP_zlib_cleanup_d)(void);
extern const EVP_MD * (*EVP_sha1_d)(void);
extern void (*EC_KEY_free_d)(EC_KEY *key);
extern int (*SSL_get_rfd_d)(const SSL *s);
extern const char * (*OBJ_nid2sn_d)(int n);
extern const EVP_MD * (*EVP_sha224_d)(void);
extern const EVP_MD * (*EVP_sha256_d)(void);
extern const EVP_MD * (*EVP_sha384_d)(void);
extern const EVP_MD * (*EVP_sha512_d)(void);
extern void (*OBJ_NAME_cleanup_d)(int type);
extern void (*SSL_CTX_free_d)(SSL_CTX *ctx);
extern int (*SSL_pending_d)(const SSL *ssl);
extern int	(*BN_num_bits_d)(const BIGNUM *);
extern int (*X509_get_ext_count_d) (X509 *x);
extern char * (*BN_bn2hex_d)(const BIGNUM *a);
extern int (*EVP_MD_size_d)(const EVP_MD *md);
extern unsigned long (*ERR_get_error_d)(void);
extern void (*CONF_modules_unload_d)(int all);
extern void (*HMAC_CTX_init_d)(HMAC_CTX *ctx);
extern void (*SSL_load_error_strings_d)(void);
extern int (*EVP_MD_type_d)(const EVP_MD *md);
extern const EVP_MD * (*EVP_ripemd160_d)(void);
extern const char * (*SSLeay_version_d)(int t);
extern BIO * (*SSL_get_wbio_d)(const SSL * ssl);
extern void (*EC_GROUP_free_d)(EC_GROUP *group);
extern void (*EC_POINT_free_d)(EC_POINT *point);
extern int (*EC_KEY_generate_key_d)(EC_KEY *key);
extern void (*ASN1_STRING_TABLE_cleanup_d)(void);
extern void (*HMAC_CTX_cleanup_d)(HMAC_CTX *ctx);
extern int (*SSL_get_shutdown_d)(const SSL *ssl);
extern void (*CRYPTO_cleanup_all_ex_data_d)(void);
extern void (*EVP_MD_CTX_init_d)(EVP_MD_CTX *ctx);
extern int (*EC_KEY_check_key_d)(const EC_KEY *key);
extern int (*EVP_MD_CTX_cleanup_d)(EVP_MD_CTX *ctx);
extern int 	(*SSL_peek_d)(SSL *ssl,void *buf,int num);
extern X509_NAME *	(*X509_get_subject_name_d)(X509 *a);
extern EC_KEY * (*EC_KEY_new_by_curve_name_d)(int nid);
extern int (*BN_hex2bn_d)(BIGNUM **a, const char *str);
extern int (*SSL_read_d)(SSL *ssl, void *buf, int num);
extern int (*RAND_bytes_d)(unsigned char *buf, int num);
extern void (*EVP_CIPHER_CTX_init_d)(EVP_CIPHER_CTX *a);
extern int (*EVP_CIPHER_nid_d)(const EVP_CIPHER *cipher);
extern void (*OPENSSL_add_all_algorithms_noconf_d)(void);
extern int	(*SSL_get_error_d)(const SSL *s,int ret_code);
extern const SSL_METHOD * (*SSLv23_server_method_d)(void);
extern X509 * (*SSL_get_peer_certificate_d)(const SSL *s);
extern int (*EVP_CIPHER_CTX_cleanup_d)(EVP_CIPHER_CTX *a);
extern BIO * (*BIO_new_socket_d)(int sock, int close_flag);
extern EC_GROUP * (*EC_GROUP_new_by_curve_name_d)(int nid);
extern EC_POINT * (*EC_POINT_new_d)(const EC_GROUP *group);
extern int	(*BN_bn2bin_d)(const BIGNUM *, unsigned char *);
extern X509_EXTENSION * (*X509_get_ext_d) (X509 *x, int loc);
extern SSL_CTX * (*SSL_CTX_new_d)(const SSL_METHOD * method);
extern void (*SSL_set_bio_d)(SSL *ssl, BIO *rbio, BIO *wbio);
extern int (*SSL_CTX_check_private_key_d)(const SSL_CTX *ctx);
extern int (*SSL_write_d)(SSL *ssl, const void *buf, int num);
extern void (*sk_pop_free_d)(_STACK *st, void(*func)(void *));
extern int (*EVP_CIPHER_iv_length_d)(const EVP_CIPHER *cipher);
extern char * (*ERR_error_string_d)(unsigned long e, char *buf);
extern int (*EVP_CIPHER_block_size_d)(const EVP_CIPHER *cipher);
extern int (*EVP_CIPHER_key_length_d)(const EVP_CIPHER *cipher);
extern const EC_GROUP * (*EC_KEY_get0_group_d)(const EC_KEY *key);
extern const EVP_MD * (*EVP_get_digestbyname_d)(const char *name);
extern int	(*SSL_CTX_set_cipher_list_d)(SSL_CTX *,const char *str);
extern int (*EVP_CIPHER_CTX_iv_length_d)(const EVP_CIPHER_CTX *ctx);
extern int (*EVP_DigestInit_d)(EVP_MD_CTX *ctx, const EVP_MD *type);
extern int (*EC_KEY_set_group_d)(EC_KEY *key, const EC_GROUP *group);
extern int (*EVP_CIPHER_CTX_block_size_d)(const EVP_CIPHER_CTX *ctx);
extern int (*EVP_CIPHER_CTX_key_length_d)(const EVP_CIPHER_CTX *ctx);
extern int (*RAND_load_file_d)(const char *filename, long max_bytes);
extern void (*ERR_remove_thread_state_d)(const CRYPTO_THREADID *tid);
extern unsigned long (*EVP_CIPHER_flags_d)(const EVP_CIPHER *cipher);
extern const BIGNUM * (*EC_KEY_get0_private_key_d)(const EC_KEY *key);
extern const EVP_CIPHER * (*EVP_get_cipherbyname_d)(const char *name);
extern const EC_POINT * (*EC_KEY_get0_public_key_d)(const EC_KEY *key);
extern int (*EC_GROUP_precompute_mult_d)(EC_GROUP *group, BN_CTX *ctx);
extern int (*EC_KEY_set_private_key_d)(EC_KEY *key, const BIGNUM *prv);
extern int (*EVP_CIPHER_CTX_set_padding_d)(EVP_CIPHER_CTX *c, int pad);
extern STACK_OF(SSL_COMP) * (*SSL_COMP_get_compression_methods_d)(void);
extern int (*BIO_vprintf_d)(BIO *bio, const char *format, va_list args);
extern int (*EC_KEY_set_public_key_d)(EC_KEY *key, const EC_POINT *pub);
extern unsigned long (*EVP_CIPHER_CTX_flags_d)(const EVP_CIPHER_CTX *ctx);
extern void (*CRYPTO_set_id_callback_d)(unsigned long(*id_function)(void));
extern long (*SSL_CTX_ctrl_d)(SSL_CTX *ctx, int cmd, long larg, void *parg);
extern void (*ERR_error_string_n_d)(unsigned long e, char *buf, size_t len);
extern BIGNUM * (*BN_bin2bn_d)(const unsigned char *s, int len, BIGNUM *ret);
extern int (*EVP_DigestUpdate_d)(EVP_MD_CTX *ctx, const void *d, size_t cnt);
extern int (*HMAC_Final_d)(HMAC_CTX *ctx, unsigned char *md, unsigned int *len);
extern int (*HMAC_Update_d)(HMAC_CTX *ctx, const unsigned char *data, size_t len);
extern int (*SSL_CTX_use_certificate_chain_file_d)(SSL_CTX *ctx, const char *file);
extern int (*EVP_DigestFinal_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
extern int (*EVP_DigestInit_ex_d)(EVP_MD_CTX *ctx, const EVP_MD *type, ENGINE *impl);
extern int (*SSL_CTX_use_PrivateKey_file_d)(SSL_CTX *ctx, const char *file, int type);
extern int (*EVP_CIPHER_CTX_ctrl_d)(EVP_CIPHER_CTX *ctx, int type, int arg, void *ptr);
extern int (*X509_NAME_get_text_by_NID_d)(X509_NAME *name, int nid, char *buf,int len);
extern int (*EVP_DigestFinal_ex_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
extern int (*EVP_EncryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);
extern int (*EVP_DecryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl);
extern void (*EC_GROUP_set_point_conversion_form_d)(EC_GROUP *, point_conversion_form_t);
extern int	(*DH_generate_parameters_ex_d)(DH *dh, int prime_len,int generator, BN_GENCB *cb);
extern EC_POINT * (*EC_POINT_hex2point_d)(const EC_GROUP *, const char *, EC_POINT *, BN_CTX *);
extern int (*SSL_CTX_load_verify_locations_d)(SSL_CTX *ctx, const char *CAfile, const char *CApath);
extern int (*HMAC_Init_ex_d)(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md, ENGINE *impl);
extern void (*SSL_CTX_set_tmp_dh_callback_d)(SSL_CTX *ctx, DH *(*dh)(SSL *ssl,int is_export, int keylength)) ;
extern char * (*EC_POINT_point2hex_d)(const EC_GROUP *, const EC_POINT *, point_conversion_form_t form, BN_CTX *);
extern void (*CRYPTO_set_locking_callback_d)(void(*locking_function)(int mode, int n, const char *file, int line));
extern void (*SSL_CTX_set_tmp_ecdh_callback_d)(SSL_CTX *ctx, EC_KEY *(*ecdh)(SSL *ssl,int is_export, int keylength));
extern int (*EVP_DecryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl);
extern int (*EVP_EncryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl);
extern int (*EC_POINT_oct2point_d)(const EC_GROUP *group, EC_POINT *p, const unsigned char *buf, size_t len, BN_CTX *ctx);
extern int (*EVP_Digest_d)(const void *data, size_t count, unsigned char *md, unsigned int *size, const EVP_MD *type, ENGINE *impl);
extern int (*EVP_DecryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv);
extern int (*EVP_EncryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv);
extern size_t (*EC_POINT_point2oct_d)(const EC_GROUP *group, const EC_POINT *p, point_conversion_form_t form, unsigned char *buf, size_t len, BN_CTX *ctx);
extern int (*ECDH_compute_key_d)(void *out, size_t outlen, const EC_POINT *pub_key, EC_KEY *ecdh, void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen));
extern void *(*sk_pop_d)(_STACK *st);
extern int (*i2d_OCSP_RESPONSE_d)(OCSP_RESPONSE *a, unsigned char **out);
extern ECDSA_SIG * (*ECDSA_do_sign_d)(const unsigned char *dgst, int dgst_len, EC_KEY *eckey);
extern void (*ECDSA_SIG_free_d)(ECDSA_SIG *a);
extern int (*i2d_OCSP_CERTID_d)(OCSP_CERTID *a, unsigned char **out);
extern OCSP_RESPONSE * (*d2i_OCSP_RESPONSE_d)(OCSP_RESPONSE **a, const unsigned char **in, long len);
extern OCSP_REQUEST * (*OCSP_REQUEST_new_d)(void);
extern void (*OCSP_BASICRESP_free_d)(OCSP_BASICRESP *a);
extern int (*i2d_X509_d)(X509 *a, unsigned char **out);
extern long (*SSL_CTX_callback_ctrl_d)(SSL_CTX *, int, void (*)(void));
extern long (*SSL_ctrl_d)(SSL *s, int cmd, long larg, void *parg);
extern ASN1_STRING * (*X509_NAME_ENTRY_get_data_d)(X509_NAME_ENTRY *ne);
extern BIGNUM * (*ASN1_INTEGER_to_BN_d)(const ASN1_INTEGER *ai, BIGNUM *bn);
extern BIO * (*BIO_new_fp_d)(FILE *stream, int close_flag);
extern char * (*X509_NAME_oneline_d)(X509_NAME *a, char *buf, int len);
extern const char * (*OCSP_response_status_str_d)(long s);
extern const char * (*X509_verify_cert_error_string_d)(long n);
extern const EVP_CIPHER * (*EVP_aes_256_cbc_d)(void);
extern EC_KEY * (*d2i_ECPrivateKey_d)(EC_KEY **key, const unsigned char **in, long len);
extern EC_KEY * (*o2i_ECPublicKey_d)(EC_KEY **key, const unsigned char **in, long len);
extern ECDSA_SIG * (*d2i_ECDSA_SIG_d)(ECDSA_SIG **sig, const unsigned char **pp, long len);
extern EVP_CIPHER_CTX * (*EVP_CIPHER_CTX_new_d)(void);
extern EVP_PKEY * (*EVP_PKEY_new_d)(void);
extern int (*ASN1_GENERALIZEDTIME_print_d)(BIO *fp, const ASN1_GENERALIZEDTIME *a);
extern int (*BIO_free_d)(BIO *a);
extern int (*ECDSA_do_verify_d)(const unsigned char *dgst, int dgst_len, const ECDSA_SIG *sig, EC_KEY *eckey);
extern int (*EVP_PKEY_set1_RSA_d)(EVP_PKEY *pkey, struct rsa_st *key);
extern int (*EVP_VerifyFinal_d)(EVP_MD_CTX *ctx, const unsigned char *sigbuf, unsigned int siglen, EVP_PKEY *pkey);
extern int (*i2d_ECDSA_SIG_d)(const ECDSA_SIG *sig, unsigned char **pp);
extern int (*i2d_ECPrivateKey_d)(EC_KEY *key, unsigned char **out);
extern int (*i2o_ECPublicKey_d)(EC_KEY *key, unsigned char **out);
extern int (*OCSP_basic_verify_d)(void *bs, struct stack_st_X509 *certs, struct x509_store_st *st, unsigned long flags);
extern int (*OCSP_check_nonce_d)(void *req, void *bs);
extern int (*OCSP_check_validity_d)(ASN1_GENERALIZEDTIME *thisupd, ASN1_GENERALIZEDTIME *nextupd, long sec, long maxsec);
extern int (*OCSP_parse_url_d)(const char *url, char **phost, char **pport, char **ppath, int *pssl);
extern int (*OCSP_REQ_CTX_add1_header_d)(OCSP_REQ_CTX *rctx, const char *name, const char *value);
extern int (*OCSP_REQ_CTX_set1_req_d)(OCSP_REQ_CTX *rctx, void *req);
extern int (*OCSP_request_add1_nonce_d)(void *req, unsigned char *val, int len);
extern int (*OCSP_REQUEST_print_d)(BIO *bp, void *a, unsigned long flags);
extern int (*OCSP_resp_find_status_d)(void *bs, void *id, int *status, int *reason, ASN1_GENERALIZEDTIME **revtime, ASN1_GENERALIZEDTIME **thisupd, ASN1_GENERALIZEDTIME **nextupd);
extern int (*OCSP_RESPONSE_print_d)(BIO *bp, OCSP_RESPONSE *o, unsigned long flags);
extern int (*OCSP_response_status_d)(OCSP_RESPONSE *resp);
extern int (*OCSP_sendreq_nbio_d)(OCSP_RESPONSE **presp, OCSP_REQ_CTX *rctx);
extern int (*SHA1_Final_d)(unsigned char *md, SHA_CTX *c);
extern int (*SHA1_Init_d)(SHA_CTX *c);
extern int (*SHA1_Update_d)(SHA_CTX *c, const void *data, size_t len);
extern int (*SHA256_Final_d)(unsigned char *md, SHA256_CTX *c);
extern int (*SHA256_Init_d)(SHA256_CTX *c);
extern int (*SHA256_Update_d)(SHA256_CTX *c, const void *data, size_t len);
extern int (*SHA512_Final_d)(unsigned char *md, SHA512_CTX *c);
extern int (*SHA512_Init_d)(SHA512_CTX *c);
extern int (*SHA512_Update_d)(SHA512_CTX *c, const void *data, size_t len);
extern int (*sk_num_d)(const _STACK *);
extern int (*SSL_get_fd_d)(const SSL *s);
extern int (*SSL_set_fd_d)(SSL *s, int fd);
extern int (*X509_check_host_d)(X509 *x, const char *chk, size_t chklen, unsigned int flags, char **peername);
extern int (*X509_check_issued_d)(X509 *issuer, X509 *subject);
extern int (*X509_NAME_get_index_by_NID_d)(X509_NAME *name, int nid, int lastpos);
extern int (*X509_STORE_CTX_get_error_d)(X509_STORE_CTX *ctx);
extern int (*X509_STORE_CTX_get_error_depth_d)(X509_STORE_CTX *ctx);
extern int (*X509_STORE_CTX_init_d)(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509, STACK_OF(X509) *chain);
extern int (*X509_STORE_load_locations_d)(X509_STORE *ctx, const char *file, const char *path);
extern int (*X509_STORE_set_flags_d)(X509_STORE *ctx, unsigned long flags);
extern int (*X509_verify_cert_d)(X509_STORE_CTX *ctx);
extern OCSP_REQ_CTX * (*OCSP_sendreq_new_d)(BIO *io, const char *path, void *req, int maxline);
extern RSA * (*RSA_new_d)(void);
extern RSA * (*RSAPublicKey_dup_d)(RSA *rsa);
extern size_t (*BUF_strlcat_d)(char *dst, const char *src, size_t siz);
extern struct stack_st_OPENSSL_STRING * (*X509_get1_ocsp_d)(X509 *x);
extern struct stack_st_X509 * (*SSL_get_peer_cert_chain_d)(const SSL *s);
extern unsigned char * (*ASN1_STRING_data_d)(ASN1_STRING *x);
extern unsigned char * (*SHA512_d)(const unsigned char *d, size_t n, unsigned char *md);
extern unsigned long (*ERR_peek_error_line_data_d)(const char **file, int *line, const char **data, int *flags);
extern void (*BIO_free_all_d)(BIO *a);
extern void (*EC_GROUP_clear_free_d)(EC_GROUP *group);
extern void (*ERR_load_crypto_strings_d)(void);
extern void (*ERR_print_errors_fp_d)(FILE *fp);
extern void (*EVP_CIPHER_CTX_free_d)(EVP_CIPHER_CTX *a);
extern void (*OCSP_REQUEST_free_d)(OCSP_REQUEST *a);
extern void (*OCSP_RESPONSE_free_d)(OCSP_RESPONSE *a);
extern void (*RSA_free_d)(RSA *r);
extern void (*SSL_CTX_set_verify_d)(SSL_CTX *ctx, int mode, int (*cb) (int, X509_STORE_CTX *));
extern void (*X509_email_free_d)(struct stack_st_OPENSSL_STRING *sk);
extern void (*X509_STORE_CTX_free_d)(X509_STORE_CTX *ctx);
extern void (*X509_STORE_CTX_set_chain_d)(struct x509_store_ctx_st *ctx, struct stack_st_X509 *sk);
extern void (*X509_STORE_free_d)(X509_STORE *v);
extern void * (*OCSP_cert_to_id_d)(const EVP_MD *dgst, X509 *subject, X509 *issuer);
extern void * (*OCSP_request_add0_id_d)(void *req, void *cid);
extern void * (*OCSP_response_get1_basic_d)(OCSP_RESPONSE *resp);
extern void * (*sk_value_d)(const _STACK *, int);
extern X509 * (*X509_STORE_CTX_get_current_cert_d)(X509_STORE_CTX *ctx);
extern X509_LOOKUP * (*X509_STORE_add_lookup_d)(X509_STORE *v, X509_LOOKUP_METHOD *m);
extern X509_LOOKUP_METHOD * (*X509_LOOKUP_file_d)(void);
extern X509_NAME_ENTRY * (*X509_NAME_get_entry_d)(X509_NAME *name, int loc);
extern X509_STORE * (*X509_STORE_new_d)(void);
extern X509_STORE_CTX * (*X509_STORE_CTX_new_d)(void);
extern void (*ERR_clear_error_d)(void);
extern void (*ERR_put_error_d)(int lib, int func, int reason, const char *file, int line);

/// symbols.c
int      lib_load(void);
void     lib_unload(void);

#endif

