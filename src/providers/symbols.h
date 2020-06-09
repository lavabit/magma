
/**
 * @file /magma/providers/symbols.h
 *
 * @brief External function pointers/definitions.
 */

#ifndef MAGMA_PROVIDERS_SYMBOLS_H
#define MAGMA_PROVIDERS_SYMBOLS_H

// System Headers
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

// SPF
#include <spf2/spf.h>
#include <spf2/spf_dns_zone.h>

// ClamAV
#include <clamav.h>

// MariaDB Connector/C
#include <mariadb/mysql.h>
#include <mariadb/errmsg.h>

// OpenSSL
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/dh.h>
#include <openssl/ec.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>
#include <openssl/ocsp.h>
#include <openssl/curve25519.h>

// LZO
#include <lzo/lzodefs.h>
#include <lzo/lzoconf.h>
#include <lzo/lzoutil.h>
#include <lzo/lzo1x.h>

// XML2
#include <libxml2/libxml/xmlmemory.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/valid.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include <libxml2/libxml/parserInternals.h>
#include <libxml2/libxml/xmlerror.h>

// ZLIB
#include <zlib.h>

// BZIP
#include <bzlib.h>

// TOKYO
#include <tcutil.h>
#include <tcadb.h>

#include <tchdb.h>
#include <tcbdb.h>

// Memcached
#include <libmemcached/memcached.h>

// DKIM
#define lint
#include <opendkim/dkim.h>
#include <opendkim/dkim-types.h>
#include <opendkim/dkim-util.h>
#include <opendkim/dkim-test.h>
#undef lint

// DSPAM
#define CONFIG_DEFAULT ""
#define LOGDIR "~/"
#include <dspam/libdspam.h>
#include <dspam/mysql_drv.h>

// Jansson
#include <jansson.h>

// GD
#include <gd.h>

// PNG
#include <png.h>

// JPEG
#include <jpeglib.h>

// FreeType
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

// UTF8
#include <utf8proc.h>

// Our macro for declaring external symbol binding points
#define M_BIND(x) 		{ \
                        .name = #x, \
                        .pointer = (void *)&x##_d \
                        }

//! MEMCACHED
extern void (*memcached_free_d)(memcached_st *ptr);
extern const char * (*memcached_lib_version_d)(void);
extern memcached_st * (*memcached_create_d)(memcached_st *ptr);
extern memcached_return_t (*memcached_flush_d)(memcached_st *ptr, time_t expiration);
extern const char * (*memcached_strerror_d)(const memcached_st *ptr, memcached_return_t rc);
extern memcached_return_t (*memcached_behavior_set_d)(memcached_st *ptr, const memcached_behavior_t flag, uint64_t data);
extern memcached_return_t (*memcached_delete_d)(memcached_st *ptr, const char *key, size_t key_length, time_t expiration);
extern memcached_return_t (*memcached_server_add_with_weight_d)(memcached_st *ptr, const char *hostname, in_port_t port, uint32_t weight);
extern memcached_return_t (*memcached_decrement_d)(memcached_st *ptr, const char *key, size_t key_length, uint32_t offset, uint64_t *value);
extern memcached_return_t (*memcached_increment_d)(memcached_st *ptr, const char *key, size_t key_length, uint32_t offset, uint64_t *value);
extern char * (*memcached_get_d)(memcached_st *ptr, const char *key, size_t key_length, size_t *value_length, uint32_t *flags, memcached_return_t *error);
extern memcached_return_t (*memcached_add_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags);
extern memcached_return_t (*memcached_set_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags);
extern memcached_return_t (*memcached_append_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags);
extern memcached_return_t (*memcached_prepend_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags);
extern memcached_return_t (*memcached_replace_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags);
extern memcached_return_t (*memcached_cas_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags, uint64_t cas);
extern memcached_return_t (*memcached_decrement_with_initial_d)(memcached_st *ptr, const char *key, size_t key_length, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value);
extern memcached_return_t (*memcached_increment_with_initial_d)(memcached_st *ptr, const char *key, size_t key_length, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value);

//! BZIP
extern const char * (*BZ2_bzlibVersion_d)(void);
extern int (*BZ2_bzBuffToBuffDecompress_d)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen, int small, int verbosity);
extern int (*BZ2_bzBuffToBuffCompress_d)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen, int blockSize100k, int verbosity, int workFactor);

//! CLAMAV
extern int (*lt_dlexit_d)(void);
extern void (*cl_shutdown_d)(void);
extern const char * (*cl_retver_d)(void);
extern int (*cl_init_d)(unsigned int initoptions);
extern const char * (*cl_strerror_d)(int clerror);
extern struct cl_engine * (*cl_engine_new_d)(void);
extern int (*cl_statfree_d)(struct cl_stat *dbstat);
extern int (*cl_engine_free_d)(struct cl_engine *engine);
extern int (*cl_engine_compile_d)(struct cl_engine *engine);
extern int (*cl_statchkdir_d)(const struct cl_stat *dbstat);
extern int (*cl_statinidir_d)(const char *dirname, struct cl_stat *dbstat);
extern int (*cl_countsigs_d)(const char *path, unsigned int countoptions, unsigned int *sigs);
extern int (*cl_engine_set_num_d)(struct cl_engine *engine, enum cl_engine_field field, long long num);
extern int (*cl_engine_set_str_d)(struct cl_engine *engine, enum cl_engine_field field, const char *str);
extern int (*cl_load_d)(const char *path, struct cl_engine *engine, unsigned int *signo, unsigned int dboptions);
extern int (*cl_scandesc_d)(int desc, const char *filename, const char **virname, unsigned long int *scanned, const struct cl_engine *engine, struct cl_scan_options *scanoptions);

//! DSPAM
extern const char * (*dspam_version_d)(void);
extern int (*dspam_detach_d)(DSPAM_CTX *CTX);
extern void (*dspam_destroy_d)(DSPAM_CTX * CTX);
extern int (*dspam_init_driver_d)(DRIVER_CTX *DTX);
extern int (*dspam_shutdown_driver_d)(DRIVER_CTX *DTX);
extern int (*dspam_attach_d)(DSPAM_CTX *CTX, void *dbh);
extern int (*dspam_process_d)(DSPAM_CTX * CTX, const char *message);
extern DSPAM_CTX * (*dspam_create_d)(const char *username, const char *group, const char *home, int operating_mode, u_int32_t flags);

//! DKIM
/// Note that dkim_getsighdr_d is used by the library, so were using dkim_getsighdrx_d.
/// Note that dkim_test_dns_put_d is only used by the verification unit test, to load public keys which may no
/// longer be available on the public internet.
extern DKIM_STAT (*dkim_eoh_d)(DKIM *dkim);
extern void (*dkim_close_d)(DKIM_LIB *lib);
extern uint32_t (*dkim_libversion_d)(void);
extern DKIM_STAT (*dkim_free_d)(DKIM *dkim);
extern const char * (*dkim_geterror_d)(DKIM *dkim);
extern DKIM_STAT (*dkim_eom_d)(DKIM *dkim, _Bool *testkey);
extern const char * (*dkim_getresultstr_d)(DKIM_STAT result);
extern DKIM_STAT (*dkim_body_d)(DKIM *dkim, u_char *buf, size_t len);
extern DKIM_STAT (*dkim_header_d)(DKIM *dkim, u_char *hdr, size_t len);
extern void (*dkim_mfree_d)(DKIM_LIB *libhandle, void *closure, void *ptr);
extern DKIM_STAT (*dkim_chunk_d)(DKIM *dkim, unsigned char *chunkp, size_t len);
extern DKIM_STAT (*dkim_getsighdrx_d)(DKIM *dkim, u_char *buf, size_t len, size_t initial);
extern int (*dkim_test_dns_put_d)(DKIM *dkim, int class, int type, int prec, u_char *name, u_char *data);
extern DKIM * (*dkim_verify_d)(DKIM_LIB *libhandle, const unsigned char *id, void *memclosure, DKIM_STAT *statp);
extern DKIM_LIB * (*dkim_init_d)(void *(*mallocf)(void *closure, size_t nbytes), void (*freef)(void *closure, void *p));
extern DKIM * (*dkim_sign_d)(DKIM_LIB *libhandle, const unsigned char *id, void *memclosure, const dkim_sigkey_t secretkey, const unsigned char *selector, const unsigned char *domain, dkim_canon_t hdr_canon_alg, dkim_canon_t body_canon_alg, dkim_alg_t sign_alg,	off_t length, DKIM_STAT *statp);

//! FreeType
extern FT_Error (*FT_Done_FreeType_d)(FT_Library library);
extern FT_Error (*FT_Init_FreeType_d)(FT_Library *alibrary);
extern void (*FT_Library_Version_d)(FT_Library library, FT_Int *amajor, FT_Int *aminor, FT_Int *apatch);

//! GD
extern void (*gdFree_d)(void *m);
extern const char * (*gdVersionString_d)(void);
extern void (*gdImageDestroy_d)(gdImagePtr im);
extern gdImagePtr (*gdImageCreate_d)(int sx, int sy);
extern void * (*gdImageGifPtr_d)(gdImagePtr im, int *size);
extern int (*gdImageColorResolve_d)(gdImagePtr im, int r, int g, int b);
extern void * (*gdImageJpegPtr_d)(gdImagePtr im, int *size, int quality);
extern void (*gdImageSetPixel_d)(gdImagePtr im, int x, int y, int color);
extern char * (*gdImageStringFT_d)(gdImage * im, int *brect, int fg, char *fontlist, double ptsize, double angle, int x, int y, char *string);

//! JPEG
extern const char * (*jpeg_version_d)(void);

//! LZO
extern const char * (*lzo_version_string_d)(void);
extern int (*__lzo_init_v2_d)(unsigned, int, int, int, int, int, int, int, int, int);
extern lzo_uint32 (*lzo_adler32_d)(lzo_uint32 _adler, const lzo_bytep _buf, lzo_uint _len);
extern int (*lzo1x_1_compress_d)(const lzo_byte *src, lzo_uint src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem);
extern int (*lzo1x_decompress_safe_d)(const lzo_byte *src, lzo_uint src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem);

//! MYSQL
extern void (*my_once_free_d)(void);
extern void (*mysql_server_end_d)(void);
extern void (*mysql_thread_end_d)(void);
extern int (*mysql_ping_d)(MYSQL *mysql);
extern void (*mysql_close_d)(MYSQL *mysql);
extern my_bool (*mysql_thread_init_d)(void);
extern MYSQL * (*mysql_init_d)(MYSQL *mysql);
extern unsigned int (*mysql_thread_safe_d)(void);
extern int (*mysql_stmt_fetch_d)(MYSQL_STMT *stmt);
extern my_bool (*mysql_stmt_close_d)(MYSQL_STMT *);
extern unsigned int (*mysql_errno_d)(MYSQL *mysql);
extern const char * (*mysql_error_d)(MYSQL *mysql);
extern int (*mysql_stmt_execute_d)(MYSQL_STMT *stmt);
extern my_bool (*mariadb_connection_d)(MYSQL *mysql);
extern void (*mysql_free_result_d)(MYSQL_RES *result);
extern my_bool (*mysql_stmt_reset_d)(MYSQL_STMT *stmt);
extern my_ulonglong (*mysql_insert_id_d)(MYSQL *mysql);
extern char * (*mysql_get_server_info_d)(MYSQL *mysql);
extern unsigned long (*mysql_thread_id_d)(MYSQL *mysql);
extern MYSQL_STMT * (*mysql_stmt_init_d)(MYSQL * mysql);
extern const char * (*mysql_get_connector_info_d)(void);
extern MYSQL_ROW (*mysql_fetch_row_d)(MYSQL_RES *result);
extern unsigned long (*mysql_get_client_version_d)(void);
extern MYSQL_RES * (*mysql_store_result_d)(MYSQL * mysql);
extern int (*mysql_stmt_store_result_d)(MYSQL_STMT *stmt);
extern my_ulonglong (*mysql_affected_rows_d)(MYSQL *mysql);
extern my_ulonglong (*mysql_num_rows_d)(MYSQL_RES *result);
extern unsigned int (*mysql_stmt_errno_d)(MYSQL_STMT *stmt);
extern const char * (*mysql_stmt_error_d)(MYSQL_STMT * stmt);
extern my_bool (*mysql_stmt_free_result_d)(MYSQL_STMT *stmt);
extern unsigned int (*mysql_num_fields_d)(MYSQL_RES *result);
extern my_ulonglong (*mysql_stmt_num_rows_d)(MYSQL_STMT *stmt);
extern MYSQL_FIELD * (*mysql_fetch_field_d)(MYSQL_RES * result);
extern const char * (*mysql_character_set_name_d)(MYSQL *mysql);
extern my_ulonglong (*mysql_stmt_insert_id_d)(MYSQL_STMT *stmt);
extern my_ulonglong (*mysql_stmt_affected_rows_d)(MYSQL_STMT *stmt);
extern MYSQL_RES * (*mysql_stmt_result_metadata_d)(MYSQL_STMT * stmt);
extern int (*mysql_server_init_d)(int argc, char **argv, char **groups);
extern int (*mysql_set_character_set_d)(MYSQL *mysql, const char *csname);
extern my_bool (*mysql_stmt_bind_param_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind);
extern my_bool (*mysql_stmt_bind_result_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind);
extern int (*mysql_options_d)(MYSQL *mysql, enum mysql_option option, const void *arg);
extern int (*mysql_real_query_d)(MYSQL *mysql, const char *query, unsigned long length);
extern int (*mysql_stmt_prepare_d)(MYSQL_STMT *stmt, const char *query, unsigned long length);
extern unsigned long (*mysql_escape_string_d)(char *to, const char *from, unsigned long length);
extern my_bool (*mysql_stmt_attr_set_d)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, const void *attr);
extern MYSQL * (*mysql_real_connect_d)(MYSQL * mysql, const char *name, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);

//! OPENSSL
extern DH * (*DH_new_d)(void);
extern char **SSL_version_str_d;
extern RSA * (*RSA_new_d)(void);
extern void (*DH_free_d)(DH *dh);
extern int (*BIO_free_d)(BIO *a);
extern int (*RAND_status_d)(void);
extern void (*RSA_free_d)(RSA *r);
extern void (*EVP_cleanup_d)(void);
extern void (*OBJ_cleanup_d)(void);
extern void (*BN_free_d)(BIGNUM *a);
extern void (*RAND_cleanup_d)(void);
extern void (*SSL_free_d)(SSL *ssl);
extern int (*SSL_accept_d)(SSL *ssl);
extern void *(*sk_pop_d)(_STACK *st);
extern BN_CTX * (*BN_CTX_new_d)(void);
extern int (*SSL_connect_d)(SSL *ssl);
extern EC_KEY * (*EC_KEY_new_d)(void);
extern void (*CRYPTO_free_d) (void *);
extern void (*ENGINE_cleanup_d)(void);
extern int (*SHA1_Init_d)(SHA_CTX *c);
extern void (*BIO_free_all_d)(BIO *a);
extern int (*CRYPTO_num_locks_d)(void);
extern int (*SSL_library_init_d)(void);
extern int (*SSL_want_d)(const SSL *s);
extern int (*SSL_shutdown_d)(SSL *ssl);
extern void (*ERR_clear_error_d)(void);
extern int (*sk_num_d)(const _STACK *);
extern void (*BIO_sock_cleanup_d)(void);
extern void (*ERR_free_strings_d)(void);
extern SSL * (*SSL_new_d)(SSL_CTX * ctx);
extern const EVP_MD * (*EVP_md4_d)(void);
extern const EVP_MD * (*EVP_md5_d)(void);
extern const EVP_MD * (*EVP_sha_d)(void);
extern void (*COMP_zlib_cleanup_d)(void);
extern int (*SSL_get_fd_d)(const SSL *s);
extern int (*SSL_do_handshake_d)(SSL *s);
extern void (*BN_CTX_free_d)(BN_CTX *ctx);
extern const EVP_MD * (*EVP_sha1_d)(void);
extern void (*EC_KEY_free_d)(EC_KEY *key);
extern int (*SSL_get_rfd_d)(const SSL *s);
extern EVP_PKEY * (*EVP_PKEY_new_d)(void);
extern void (*BN_CTX_start_d)(BN_CTX *ctx);
extern const char * (*OBJ_nid2sn_d)(int n);
extern int (*SHA256_Init_d)(SHA256_CTX *c);
extern int (*SHA512_Init_d)(SHA512_CTX *c);
extern int (*SSL_set_fd_d)(SSL *s, int fd);
extern const EVP_MD * (*EVP_sha224_d)(void);
extern const EVP_MD * (*EVP_sha256_d)(void);
extern const EVP_MD * (*EVP_sha384_d)(void);
extern const EVP_MD * (*EVP_sha512_d)(void);
extern void (*OBJ_NAME_cleanup_d)(int type);
extern void (*SSL_CTX_free_d)(SSL_CTX *ctx);
extern int (*SSL_pending_d)(const SSL *ssl);
extern int (*BN_num_bits_d)(const BIGNUM *);
extern int (*X509_get_ext_count_d) (X509 *x);
extern RSA * (*RSAPublicKey_dup_d)(RSA *rsa);
extern char * (*BN_bn2dec_d)(const BIGNUM *a);
extern char * (*BN_bn2hex_d)(const BIGNUM *a);
extern int (*EVP_MD_size_d)(const EVP_MD *md);
extern unsigned long (*ERR_get_error_d)(void);
extern void (*CONF_modules_unload_d)(int all);
extern void (*HMAC_CTX_init_d)(HMAC_CTX *ctx);
extern void (*SSL_load_error_strings_d)(void);
extern int (*EVP_MD_type_d)(const EVP_MD *md);
extern void (*ECDSA_SIG_free_d)(ECDSA_SIG *a);
extern X509_STORE * (*X509_STORE_new_d)(void);
extern void (*SSL_set_accept_state_d)(SSL *s);
extern void (*SSL_set_connect_state_d)(SSL *s);
extern unsigned long (*ERR_peek_error_d)(void);
extern const EVP_MD * (*EVP_ripemd160_d)(void);
extern const char * (*SSLeay_version_d)(int t);
extern void (*ERR_load_crypto_strings_d)(void);
extern void (*ERR_print_errors_fp_d)(FILE *fp);
extern BIO * (*SSL_get_wbio_d)(const SSL * ssl);
extern void (*EC_GROUP_free_d)(EC_GROUP *group);
extern void (*EC_POINT_free_d)(EC_POINT *point);
extern void (*X509_STORE_free_d)(X509_STORE *v);
extern int (*SSL_get_read_ahead_d)(const SSL *s);
extern int (*DH_check_d)(const DH *dh, int *ret);
extern int (*EC_KEY_generate_key_d)(EC_KEY *key);
extern void (*ASN1_STRING_TABLE_cleanup_d)(void);
extern void (*HMAC_CTX_cleanup_d)(HMAC_CTX *ctx);
extern int (*SSL_get_shutdown_d)(const SSL *ssl);
extern void * (*sk_value_d)(const _STACK *, int);
extern void (*CRYPTO_cleanup_all_ex_data_d)(void);
extern void (*EVP_MD_CTX_init_d)(EVP_MD_CTX *ctx);
extern OCSP_REQUEST * (*OCSP_REQUEST_new_d)(void);
extern int (*EC_KEY_check_key_d)(const EC_KEY *key);
extern int (*EVP_MD_CTX_cleanup_d)(EVP_MD_CTX *ctx);
extern void (*OCSP_REQUEST_free_d)(OCSP_REQUEST *a);
extern const EVP_CIPHER * (*EVP_aes_256_gcm_d)(void);
extern const EVP_CIPHER * (*EVP_aes_256_cbc_d)(void);
extern int (*SSL_peek_d)(SSL *ssl,void *buf,int num);
extern void (*SSL_set_read_ahead_d)(SSL *s, int yes);
extern EVP_CIPHER_CTX * (*EVP_CIPHER_CTX_new_d)(void);
extern int (*X509_verify_cert_d)(X509_STORE_CTX *ctx);
extern void (*EC_GROUP_clear_free_d)(EC_GROUP *group);
extern void (*OCSP_RESPONSE_free_d)(OCSP_RESPONSE *a);
extern X509_STORE_CTX * (*X509_STORE_CTX_new_d)(void);
extern X509_NAME * (*X509_get_subject_name_d)(X509 *a);
extern EC_KEY * (*EC_KEY_new_by_curve_name_d)(int nid);
extern int (*BN_hex2bn_d)(BIGNUM **a, const char *str);
extern int (*SSL_read_d)(SSL *ssl, void *buf, int num);
extern int (*i2d_X509_d)(X509 *a, unsigned char **out);
extern const char * (*SSL_get_version_d)(const SSL *s);
extern int (*RAND_bytes_d)(unsigned char *buf, int num);
extern void (*EVP_CIPHER_CTX_init_d)(EVP_CIPHER_CTX *a);
extern void (*OCSP_BASICRESP_free_d)(OCSP_BASICRESP *a);
extern void (*EVP_CIPHER_CTX_free_d)(EVP_CIPHER_CTX *a);
extern X509_LOOKUP_METHOD * (*X509_LOOKUP_file_d)(void);
extern int (*BN_cmp_d)(const BIGNUM *a, const BIGNUM *b);
extern const SSL_METHOD * (*TLSv1_server_method_d)(void);
extern int (*EVP_CIPHER_nid_d)(const EVP_CIPHER *cipher);
extern void (*OPENSSL_add_all_algorithms_noconf_d)(void);
extern int (*SSL_get_error_d)(const SSL *s,int ret_code);
extern const SSL_METHOD * (*SSLv23_client_method_d)(void);
extern const SSL_METHOD * (*SSLv23_server_method_d)(void);
extern X509 * (*SSL_get_peer_certificate_d)(const SSL *s);
extern int (*EVP_CIPHER_CTX_cleanup_d)(EVP_CIPHER_CTX *a);
extern const char * (*OCSP_response_status_str_d)(long s);
extern int (*OCSP_response_status_d)(OCSP_RESPONSE *resp);
extern int (*SHA1_Final_d)(unsigned char *md, SHA_CTX *c);
extern void (*X509_STORE_CTX_free_d)(X509_STORE_CTX *ctx);
extern BIO * (*BIO_new_socket_d)(int sock, int close_flag);
extern EC_GROUP * (*EC_GROUP_new_by_curve_name_d)(int nid);
extern EC_POINT * (*EC_POINT_new_d)(const EC_GROUP *group);
extern int (*BN_bn2bin_d)(const BIGNUM *, unsigned char *);
extern BIO * (*BIO_new_fp_d)(FILE *stream, int close_flag);
extern X509_EXTENSION * (*X509_get_ext_d) (X509 *x, int loc);
extern SSL_CTX * (*SSL_CTX_new_d)(const SSL_METHOD * method);
extern void (*SSL_set_bio_d)(SSL *ssl, BIO *rbio, BIO *wbio);
extern unsigned char * (*ASN1_STRING_data_d)(ASN1_STRING *x);
extern int (*BN_bn2mpi_d)(const BIGNUM *a, unsigned char *to);
extern int (*SSL_CTX_check_private_key_d)(const SSL_CTX *ctx);
extern int (*SSL_write_d)(SSL *ssl, const void *buf, int num);
extern void (*sk_pop_free_d)(_STACK *st, void(*func)(void *));
extern int (*X509_STORE_CTX_get_error_d)(X509_STORE_CTX *ctx);
extern int (*EVP_CIPHER_iv_length_d)(const EVP_CIPHER *cipher);
extern const char * (*X509_verify_cert_error_string_d)(long n);
extern int (*SHA256_Final_d)(unsigned char *md, SHA256_CTX *c);
extern int (*SHA512_Final_d)(unsigned char *md, SHA512_CTX *c);
extern int (*X509_check_issued_d)(X509 *issuer, X509 *subject);
extern int (*EVP_CIPHER_block_size_d)(const EVP_CIPHER *cipher);
extern int (*EVP_CIPHER_key_length_d)(const EVP_CIPHER *cipher);
extern const EC_GROUP * (*EC_KEY_get0_group_d)(const EC_KEY *key);
extern const EVP_MD * (*EVP_get_digestbyname_d)(const char *name);
extern long (*SSL_ctrl_d)(SSL *s, int cmd, long larg, void *parg);
extern int (*i2o_ECPublicKey_d)(EC_KEY *key, unsigned char **out);
extern int (*SSL_CTX_set_cipher_list_d)(SSL_CTX *,const char *str);
extern int (*i2d_ECPrivateKey_d)(EC_KEY *key, unsigned char **out);
extern const SSL_CIPHER * (*SSL_get_current_cipher_d)(const SSL *s);
extern char * (*SSL_CIPHER_get_version_d)(const SSL_CIPHER *cipher);
extern int (*EVP_CIPHER_CTX_iv_length_d)(const EVP_CIPHER_CTX *ctx);
extern int (*EVP_DigestInit_d)(EVP_MD_CTX *ctx, const EVP_MD *type);
extern int (*X509_STORE_CTX_get_error_depth_d)(X509_STORE_CTX *ctx);
extern int (*EC_KEY_set_group_d)(EC_KEY *key, const EC_GROUP *group);
extern int (*EVP_CIPHER_CTX_block_size_d)(const EVP_CIPHER_CTX *ctx);
extern int (*EVP_CIPHER_CTX_key_length_d)(const EVP_CIPHER_CTX *ctx);
extern int (*RAND_load_file_d)(const char *filename, long max_bytes);
extern void (*ERR_remove_thread_state_d)(const CRYPTO_THREADID *tid);
extern unsigned long (*EVP_CIPHER_flags_d)(const EVP_CIPHER *cipher);
extern int (*i2d_OCSP_CERTID_d)(OCSP_CERTID *a, unsigned char **out);
extern struct stack_st_OPENSSL_STRING * (*X509_get1_ocsp_d)(X509 *x);
extern void (*X509_email_free_d)(struct stack_st_OPENSSL_STRING *sk);
extern const BIGNUM * (*EC_KEY_get0_private_key_d)(const EC_KEY *key);
extern const EVP_CIPHER * (*EVP_get_cipherbyname_d)(const char *name);
extern int (*EVP_PKEY_set1_RSA_d)(EVP_PKEY *pkey, struct rsa_st *key);
extern int (*SHA1_Update_d)(SHA_CTX *c, const void *data, size_t len);
extern const char * (*SSL_CIPHER_get_name_d)(const SSL_CIPHER *cipher);
extern const EC_POINT * (*EC_KEY_get0_public_key_d)(const EC_KEY *key);
extern int (*EC_GROUP_precompute_mult_d)(EC_GROUP *group, BN_CTX *ctx);
extern int (*EC_KEY_set_private_key_d)(EC_KEY *key, const BIGNUM *prv);
extern int (*EVP_CIPHER_CTX_set_padding_d)(EVP_CIPHER_CTX *c, int pad);
extern long (*SSL_CTX_callback_ctrl_d)(SSL_CTX *, int, void (*)(void));
extern char * (*X509_NAME_oneline_d)(X509_NAME *a, char *buf, int len);
extern size_t (*BUF_strlcat_d)(char *dst, const char *src, size_t siz);
extern int (*OCSP_check_nonce_d)(OCSP_REQUEST *req, OCSP_BASICRESP *bs);
extern STACK_OF(SSL_COMP) * (*SSL_COMP_get_compression_methods_d)(void);
extern int (*BIO_vprintf_d)(BIO *bio, const char *format, va_list args);
extern int (*EC_KEY_set_public_key_d)(EC_KEY *key, const EC_POINT *pub);
extern ASN1_STRING * (*X509_NAME_ENTRY_get_data_d)(X509_NAME_ENTRY *ne);
extern int (*i2d_ECDSA_SIG_d)(const ECDSA_SIG *sig, unsigned char **pp);
extern X509 * (*X509_STORE_CTX_get_current_cert_d)(X509_STORE_CTX *ctx);
extern int (*SSL_CIPHER_get_bits_d)(const SSL_CIPHER *c, int *alg_bits);
extern int (*i2d_OCSP_RESPONSE_d)(OCSP_RESPONSE *a, unsigned char **out);
extern struct stack_st_X509 * (*SSL_get_peer_cert_chain_d)(const SSL *s);
extern unsigned long (*EVP_CIPHER_CTX_flags_d)(const EVP_CIPHER_CTX *ctx);
extern OCSP_BASICRESP * (*OCSP_response_get1_basic_d)(OCSP_RESPONSE *resp);
extern void (*CRYPTO_set_id_callback_d)(unsigned long(*id_function)(void));
extern int (*SHA256_Update_d)(SHA256_CTX *c, const void *data, size_t len);
extern int (*SHA512_Update_d)(SHA512_CTX *c, const void *data, size_t len);
extern int (*X509_STORE_set_flags_d)(X509_STORE *ctx, unsigned long flags);
extern point_conversion_form_t (*EC_KEY_get_conv_form_d)(const EC_KEY *key);
extern long (*SSL_CTX_ctrl_d)(SSL_CTX *ctx, int cmd, long larg, void *parg);
extern void (*ERR_error_string_n_d)(unsigned long e, char *buf, size_t len);
extern BIGNUM * (*ASN1_INTEGER_to_BN_d)(const ASN1_INTEGER *ai, BIGNUM *bn);
extern X509_NAME_ENTRY * (*X509_NAME_get_entry_d)(X509_NAME *name, int loc);
extern int (*OCSP_REQ_CTX_set1_req_d)(OCSP_REQ_CTX *rctx, OCSP_REQUEST *req);
extern BIGNUM * (*BN_mpi2bn_d)(const unsigned char *s, int len, BIGNUM *ret);
extern BIGNUM * (*BN_bin2bn_d)(const unsigned char *s, int len, BIGNUM *ret);
extern int (*EVP_DigestUpdate_d)(EVP_MD_CTX *ctx, const void *d, size_t cnt);
extern int (*OCSP_sendreq_nbio_d)(OCSP_RESPONSE **presp, OCSP_REQ_CTX *rctx);
extern int (*HMAC_Final_d)(HMAC_CTX *ctx, unsigned char *md, unsigned int *len);
extern int (*OCSP_REQUEST_print_d)(BIO *bp, OCSP_REQUEST *a, unsigned long flags);
extern int (*HMAC_Update_d)(HMAC_CTX *ctx, const unsigned char *data, size_t len);
extern int (*X509_NAME_get_index_by_NID_d)(X509_NAME *name, int nid, int lastpos);
extern int (*SSL_CTX_use_certificate_chain_file_d)(SSL_CTX *ctx, const char *file);
extern int (*ASN1_GENERALIZEDTIME_print_d)(BIO *fp, const ASN1_GENERALIZEDTIME *a);
extern OCSP_ONEREQ * (*OCSP_request_add0_id_d)(OCSP_REQUEST *req, OCSP_CERTID *cid);
extern void (*EC_KEY_set_conv_form_d)(EC_KEY *eckey, point_conversion_form_t cform);
extern int (*OCSP_RESPONSE_print_d)(BIO *bp, OCSP_RESPONSE *o, unsigned long flags);
extern int (*EVP_DigestFinal_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
extern int (*EVP_DigestInit_ex_d)(EVP_MD_CTX *ctx, const EVP_MD *type, ENGINE *impl);
extern int (*SSL_CTX_use_PrivateKey_file_d)(SSL_CTX *ctx, const char *file, int type);
extern X509_LOOKUP * (*X509_STORE_add_lookup_d)(X509_STORE *v, X509_LOOKUP_METHOD *m);
extern int (*EVP_CIPHER_CTX_ctrl_d)(EVP_CIPHER_CTX *ctx, int type, int arg, void *ptr);
extern int (*X509_NAME_get_text_by_NID_d)(X509_NAME *name, int nid, char *buf,int len);
extern EC_KEY * (*o2i_ECPublicKey_d)(EC_KEY **key, const unsigned char **in, long len);
extern int (*OCSP_request_add1_nonce_d)(OCSP_REQUEST *req, unsigned char *val, int len);
extern int (*EVP_DigestFinal_ex_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
extern int (*EVP_EncryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl);
extern EC_KEY * (*d2i_ECPrivateKey_d)(EC_KEY **key, const unsigned char **in, long len);
extern unsigned char * (*SHA512_d)(const unsigned char *d, size_t n, unsigned char *md);
extern int (*EVP_DecryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl);
extern void (*EC_GROUP_set_point_conversion_form_d)(EC_GROUP *, point_conversion_form_t);
extern void (*ED25519_keypair_d)(uint8_t out_public_key[32], uint8_t out_private_key[64]);
extern void (*ERR_put_error_d)(int lib, int func, int reason, const char *file, int line);
extern OCSP_CERTID * (*OCSP_cert_to_id_d)(const EVP_MD *dgst, X509 *subject, X509 *issuer);
extern ECDSA_SIG * (*d2i_ECDSA_SIG_d)(ECDSA_SIG **sig, const unsigned char **pp, long len);
extern int (*DH_generate_parameters_ex_d)(DH *dh, int prime_len,int generator, BN_GENCB *cb);
extern ECDSA_SIG * (*ECDSA_do_sign_d)(const unsigned char *dgst, int dgst_len, EC_KEY *eckey);
extern int (*X509_STORE_load_locations_d)(X509_STORE *ctx, const char *file, const char *path);
extern void (*SSL_CTX_set_verify_d)(SSL_CTX *ctx, int mode, int (*cb) (int, X509_STORE_CTX *));
extern EC_POINT * (*EC_POINT_hex2point_d)(const EC_GROUP *, const char *, EC_POINT *, BN_CTX *);
extern int (*CRYPTO_set_locked_mem_functions_d)(void *(*m) (size_t), void (*free_func) (void *));
extern int (*OCSP_REQ_CTX_add1_header_d)(OCSP_REQ_CTX *rctx, const char *name, const char *value);
extern void (*X509_STORE_CTX_set_chain_d)(struct x509_store_ctx_st *ctx, struct stack_st_X509 *sk);
extern int (*SSL_CTX_load_verify_locations_d)(SSL_CTX *ctx, const char *CAfile, const char *CApath);
extern OCSP_RESPONSE * (*d2i_OCSP_RESPONSE_d)(OCSP_RESPONSE **a, const unsigned char **in, long len);
extern int (*OCSP_parse_url_d)(const char *url, char **phost, char **pport, char **ppath, int *pssl);
extern int (*HMAC_Init_ex_d)(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md, ENGINE *impl);
extern OCSP_REQ_CTX * (*OCSP_sendreq_new_d)(BIO *io, const char *path, OCSP_REQUEST *req, int maxline);
extern int (*EC_POINT_cmp_d)(const EC_GROUP *group, const EC_POINT *a, const EC_POINT *b, BN_CTX *ctx);
extern void (*SSL_CTX_set_tmp_dh_callback_d)(SSL_CTX *ctx, DH *(*dh)(SSL *ssl,int is_export, int keylength)) ;
extern int (*ECDSA_do_verify_d)(const unsigned char *dgst, int dgst_len, const ECDSA_SIG *sig, EC_KEY *eckey);
extern int (*X509_check_host_d)(X509 *x, const char *chk, size_t chklen, unsigned int flags, char **peername);
extern int (*CRYPTO_set_mem_functions_d)(void *(*m) (size_t), void *(*r) (void *, size_t), void (*f) (void *));
extern int (*X509_STORE_CTX_init_d)(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509, STACK_OF(X509) *chain);
extern unsigned long (*ERR_peek_error_line_data_d)(const char **file, int *line, const char **data, int *flags);
extern char * (*EC_POINT_point2hex_d)(const EC_GROUP *, const EC_POINT *, point_conversion_form_t form, BN_CTX *);
extern int (*OCSP_basic_verify_d)(OCSP_BASICRESP *bs, STACK_OF(X509) *certs, X509_STORE *st, unsigned long flags);
extern void (*CRYPTO_set_locking_callback_d)(void(*locking_function)(int mode, int n, const char *file, int line));
extern int (*EVP_VerifyFinal_d)(EVP_MD_CTX *ctx, const unsigned char *sigbuf, unsigned int siglen, EVP_PKEY *pkey);
extern void (*SSL_CTX_set_tmp_ecdh_callback_d)(SSL_CTX *ctx, EC_KEY *(*ecdh)(SSL *ssl,int is_export, int keylength));
extern int (*EVP_DecryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl);
extern int (*EVP_EncryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl);
extern int (*OCSP_check_validity_d)(ASN1_GENERALIZEDTIME *thisupd, ASN1_GENERALIZEDTIME *nextupd, long sec, long maxsec);
extern int (*ED25519_sign_d)(uint8_t *out_sig, const uint8_t *message, size_t message_len, const uint8_t private_key[64]);
extern int (*EC_POINT_oct2point_d)(const EC_GROUP *group, EC_POINT *p, const unsigned char *buf, size_t len, BN_CTX *ctx);
extern void (*ED25519_keypair_from_seed_d)(uint8_t out_public_key[32], uint8_t out_private_key[64], const uint8_t seed[32]);
extern int (*EVP_Digest_d)(const void *data, size_t count, unsigned char *md, unsigned int *size, const EVP_MD *type, ENGINE *impl);
extern int (*ED25519_verify_d)(const uint8_t *message, size_t message_len, const uint8_t signature[64], const uint8_t public_key[32]);
extern int (*EVP_DecryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv);
extern int (*EVP_EncryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv);
extern int (*EC_POINT_mul_d)(const EC_GROUP *group, EC_POINT *r, const BIGNUM *g_scalar, const EC_POINT *point, const BIGNUM *p_scalar, BN_CTX *ctx);
extern size_t (*EC_POINT_point2oct_d)(const EC_GROUP *group, const EC_POINT *p, point_conversion_form_t form, unsigned char *buf, size_t len, BN_CTX *ctx);
extern int (*ECDH_compute_key_d)(void *out, size_t outlen, const EC_POINT *pub_key, EC_KEY *ecdh, void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen));
extern int (*OCSP_resp_find_status_d)(OCSP_BASICRESP *bs, OCSP_CERTID *id, int *status, int *reason, ASN1_GENERALIZEDTIME **revtime, ASN1_GENERALIZEDTIME **thisupd, ASN1_GENERALIZEDTIME **nextupd);

//! PNG
extern png_uint_32 (*png_access_version_number_d)(void);

//! SPF
extern void (*SPF_server_free_d)(SPF_server_t *sp);
extern void (*SPF_request_free_d)(SPF_request_t *sr);
extern void (*SPF_response_free_d)(SPF_response_t *rp);
extern const char * (*SPF_strreason_d)(SPF_reason_t reason);
extern const char * (*SPF_strresult_d)(SPF_result_t result);
extern const char * (*SPF_strerror_d)(SPF_errcode_t spf_err);
extern SPF_reason_t (*SPF_response_reason_d)(SPF_response_t *rp);
extern SPF_result_t (*SPF_response_result_d)(SPF_response_t *rp);
extern SPF_request_t * (*SPF_request_new_d)(SPF_server_t * spf_server);
extern void (*SPF_get_lib_version_d)(int *major, int *minor, int *patch);
extern int (*SPF_request_set_env_from_d)(SPF_request_t *sr, const char *from);
extern SPF_server_t * (*SPF_server_new_d)(SPF_server_dnstype_t dnstype, int debug);
extern SPF_errcode_t (*SPF_request_set_helo_dom_d)(SPF_request_t *sr, const char *dom);
extern SPF_errcode_t	(*SPF_request_set_ipv4_d)(SPF_request_t *sr, struct in_addr addr);
extern SPF_errcode_t	(*SPF_request_set_ipv6_d)(SPF_request_t *sr, struct in6_addr addr);
extern SPF_dns_server_t * (*SPF_dns_zone_new_d)(SPF_dns_server_t *layer_below, const char *name, int debug);
extern SPF_errcode_t (*SPF_request_query_mailfrom_d)(SPF_request_t *spf_request, SPF_response_t **spf_responsep);
extern SPF_errcode_t (*SPF_dns_zone_add_str_d)(SPF_dns_server_t *spf_dns_server, const char *domain, ns_type rr_type, SPF_dns_stat_t herrno, const char *data);

//! TOKYO
extern char **tcversion_d;
extern TCHDB * (*tchdbnew_d)(void);
extern void (*tcfree_d)(void *ptr);
extern void (*tchdbdel_d)(TCHDB *hdb);
extern bool (*tchdbsync_d)(TCHDB *hdb);
extern int (*tchdbecode_d)(TCHDB *hdb);
extern void (*tcndbdel_d)(TCNDB *tree);
extern bool (*tchdbclose_d)(TCHDB *hdb);
extern void (*tclistdel_d)(TCLIST *list);
extern TCNDB * (*tcndbdup_d)(TCNDB *ndb);
extern void (*tctreeclear_d)(TCTREE *tree);
extern bool (*tchdbsetmutex_d)(TCHDB *hdb);
extern uint64_t (*tchdbfsiz_d)(TCHDB *hdb);
extern uint64_t (*tchdbrnum_d)(TCHDB *hdb);
extern uint64_t (*tcndbrnum_d)(TCNDB *ndb);
extern void (*tcndbiterinit_d)(TCNDB *ndb);
extern char * (*tcndbiternext2_d)(TCNDB *ndb);
extern int (*tclistnum_d)(const TCLIST *list);
extern const char * (*tchdberrmsg_d)(int ecode);
extern const char * (*tchdbpath_d)(TCHDB * hdb);
extern TCLIST * (*tctreekeys_d)(const TCTREE * tree);
extern TCLIST * (*tctreevals_d)(const TCTREE * tree);
extern TCNDB * (*tcndbnew2_d)(TCCMP cmp, void *cmpop);
extern bool (*tchdbdefrag_d)(TCHDB *hdb, int64_t step);
extern bool (*tchdbsetdfunit_d)(TCHDB *hdb, int32_t dfunit);
extern bool (*tchdbout_d)(TCHDB *hdb, const void *kbuf, int ksiz);
extern bool (*tcndbout_d)(TCNDB *ndb, const void *kbuf, int ksiz);
extern bool (*tchdbopen_d)(TCHDB *hdb, const char *path, int omode);
extern const void * (*tclistval_d)(const TCLIST * list, int index, int *sp);
extern void * (*tchdbget_d)(TCHDB * hdb, const void *kbuf, int ksiz, int *sp);
extern void * (*tcndbget3_d)(TCNDB *ndb, const void *kbuf, int ksiz, int *sp);
extern void * (*tcndbget_d)(TCNDB * ndb, const void *kbuf, int ksiz, int *sp);
extern TCLIST * (*tcndbfwmkeys_d)(TCNDB * ndb, const void *pbuf, int psiz, int max);
extern bool (*tchdbtune_d)(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts);
extern bool (*tchdboptimize_d)(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts);
extern bool (*tcndbputkeep_d)(TCNDB *ndb, const void *kbuf, int ksiz, const void *vbuf, int vsiz);
extern bool (*tchdbputasync_d)(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz);
extern bool (*tcndbgetboth_d)(TCNDB *ndb, const void *kbuf, int ksiz, void **rkbuf, int *rksiz, void **rvbuf, int *rvsiz);

//! Jansson
extern json_t * (*json_null_d)(void);
extern json_t * (*json_true_d)(void);
extern json_t * (*json_array_d)(void);
extern json_t * (*json_false_d)(void);
extern json_t * (*json_object_d)(void);
extern void (*json_decref_d)(json_t *json);
extern void (*json_delete_d)(json_t *json);
extern json_t * (*json_real_d)(double value);
extern json_t * (*json_copy_d)(json_t *value);
extern const char * (*jansson_version_d)(void);
extern json_t * (*json_incref_d)(json_t *json);
extern int (*json_array_clear_d)(json_t *array);
extern int (*json_object_clear_d)(json_t *object);
extern json_t * (*json_deep_copy_d)(json_t *value);
extern json_t * (*json_integer_d)(json_int_t value);
extern void * (*json_object_iter_d)(json_t *object);
extern json_t * (*json_string_d)(const char *value);
extern json_t * (*json_pack_d)(const char *fmt, ...);
extern double (*json_real_value_d)(const json_t *real);
extern size_t (*json_array_size_d)(const json_t *array);
extern const char * (*json_type_string_d)(json_t *json);
extern json_t * (*json_object_iter_value_d)(void *iter);
extern double (*json_number_value_d)(const json_t *json);
extern const char * (*json_object_iter_key_d)(void *iter);
extern size_t (*json_object_size_d)(const json_t *object);
extern int (*json_real_set_d)(json_t *real, double value);
extern int (*json_equal_d)(json_t *value1, json_t *value2);
extern json_t * (*json_string_nocheck_d)(const char *value);
extern int (*json_array_remove_d)(json_t *array, size_t index);
extern int (*json_array_append_d)(json_t *array, json_t *value);
extern int (*json_array_extend_d)(json_t *array, json_t *other);
extern char * (*json_dumps_d)(const json_t *json, size_t flags);
extern int (*json_unpack_d)(json_t *root, const char *fmt, ...);
extern json_int_t (*json_integer_value_d)(const json_t *integer);
extern int (*json_object_del_d)(json_t *object, const char *key);
extern const char * (*json_string_value_d)(const json_t *string);
extern int (*json_object_update_d)(json_t *object, json_t *other);
extern int (*json_string_set_d)(json_t *string, const char *value);
extern int (*json_array_append_new_d)(json_t *array, json_t *value);
extern int (*json_integer_set_d)(json_t *integer, json_int_t value);
extern void * (*json_object_iter_next_d)(json_t *object, void *iter);
extern json_t * (*json_array_get_d)(const json_t *array, size_t index);
extern void * (*json_object_iter_at_d)(json_t *object, const char *key);
extern int (*json_array_set_d)(json_t *array, size_t index, json_t *value);
extern int (*json_dumpf_d)(const json_t *json, FILE *output, size_t flags);
extern int (*json_string_set_nocheck_d)(json_t *string, const char *value);
extern json_t * (*json_object_get_d)(const json_t *object, const char *key);
extern int (*json_array_insert_d)(json_t *array, size_t index, json_t *value);
extern int (*json_array_set_new_d)(json_t *array, size_t index, json_t *value);
extern json_t * (*json_loadf_d)(FILE *input, size_t flags, json_error_t *error);
extern int (*json_object_iter_set_d)(json_t *object, void *iter, json_t *value);
extern int (*json_object_set_d)(json_t *object, const char *key, json_t *value);
extern int (*json_array_insert_new_d)(json_t *array, size_t index, json_t *value);
extern int (*json_dump_file_d)(const json_t *json, const char *path, size_t flags);
extern int (*json_object_iter_set_new_d)(json_t *object, void *iter, json_t *value);
extern int (*json_object_set_new_d)(json_t *object, const char *key, json_t *value);
extern void (*json_set_alloc_funcs_d)(json_malloc_t malloc_fn, json_free_t free_fn);
extern json_t * (*json_loads_d)(const char *input, size_t flags, json_error_t *error);
extern int (*json_object_set_nocheck_d)(json_t *object, const char *key, json_t *value);
extern json_t * (*json_load_file_d)(const char *path, size_t flags, json_error_t *error);
extern json_t * (*json_pack_ex_d)(json_error_t *error, size_t flags, const char *fmt, ...);
extern int (*json_object_set_new_nocheck_d)(json_t *object, const char *key, json_t *value);
extern json_t * (*json_vpack_ex_d)(json_error_t *error, size_t flags, const char *fmt, va_list ap);
extern int (*json_unpack_ex_d)(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...);
extern int (*json_vunpack_ex_d)(json_t *root, json_error_t *error, size_t flags, const char *fmt, va_list ap);

//! UTF8
extern const char * (*utf8proc_version_d)(void);
extern const char * (*utf8proc_errmsg_d)(utf8proc_ssize_t errcode);
extern const char * (*utf8proc_category_string_d)(utf8proc_int32_t c);
extern utf8proc_category_t (*utf8proc_category_d)(utf8proc_int32_t c);
extern const utf8proc_property_t * (*utf8proc_get_property_d)(utf8proc_int32_t uc);
extern utf8proc_ssize_t (*utf8proc_iterate_d)(const utf8proc_uint8_t *str, utf8proc_ssize_t strlen, utf8proc_int32_t *codepoint_ref);

//! XML
extern char **xmlParserVersion_d;
extern void (*xmlInitParser_d)(void);
extern void (*xmlMemoryDump_d)(void);
extern void (*xmlCleanupParser_d)(void);
extern void (*xmlCleanupGlobals_d)(void);
extern void (*xmlFreeDoc_d)(xmlDocPtr doc);
extern void (*xmlFreeNode_d)(xmlNodePtr cur);
extern xmlBufferPtr (*xmlBufferCreate_d)(void);
extern void (*xmlBufferFree_d)(xmlBufferPtr buf);
extern xmlParserCtxtPtr (*xmlNewParserCtxt_d)(void);
extern int (*xmlBufferLength_d)(const xmlBuffer *buf);
extern void (*xmlFreeParserCtxt_d)(xmlParserCtxtPtr ctx);
extern void (*xmlXPathFreeObject_d)(xmlXPathObjectPtr obj);
extern void (*xmlXPathFreeContext_d)(xmlXPathContextPtr ctx);
extern xmlXPathContextPtr (*xmlXPathNewContext_d)(xmlDocPtr doc);
extern const xmlChar * (*xmlBufferContent_d)(const xmlBuffer *buf);
extern xmlNodePtr (*xmlNewNode_d)(xmlNsPtr ns, const xmlChar *name);
extern xmlNodePtr (*xmlAddSibling_d)(xmlNodePtr cur, xmlNodePtr elem);
extern void (*xmlNodeSetContent_d)(xmlNodePtr cur, const xmlChar *content);
extern int (*xmlNodeBufGetContent_d)(xmlBufferPtr buffer, const xmlNode *cur);
extern xmlChar * (*xmlEncodeEntitiesReentrant_d)(xmlDocPtr doc, const xmlChar * input);
extern void (*xmlDocDumpFormatMemory_d)(xmlDocPtr cur, xmlChar **mem, int *size, int format);
extern xmlAttrPtr (*xmlSetProp_d)(xmlNodePtr node, const xmlChar *name, const xmlChar *value);
extern xmlXPathObjectPtr (*xmlXPathEvalExpression_d)(const xmlChar *xpath, xmlXPathContextPtr ctx);
extern int (*xmlXPathRegisterNs_d)(xmlXPathContextPtr ctxt, const xmlChar *prefix, const xmlChar *ns_uri);
extern xmlDocPtr (*xmlCtxtReadMemory_d)(xmlParserCtxtPtr ctxt, const char *buffer, int size, const char *url, const char *encoding, int options);

//! ZLIB
extern const char * (*zlibVersion_d)(void);
extern uLong (*compressBound_d)(uLong sourceLen);
extern int (*uncompress_d)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
extern int (*compress2_d)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level);

#endif

