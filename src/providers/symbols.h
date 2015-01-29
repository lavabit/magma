
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

// Our macro for declaring external symbol binding points
#define M_BIND(x) 		{ \
                        .name = #x, \
                        .pointer = (void *)&x##_d \
                        }

//! MEMCACHED
memcached_return_t (*memcached_flush_d)(memcached_st *ptr, time_t expiration) __attribute__ ((common)) = NULL;
void (*memcached_free_d)(memcached_st *ptr) __attribute__ ((common)) = NULL;
const char * (*memcached_lib_version_d)(void) __attribute__ ((common)) = NULL;
memcached_st * (*memcached_create_d)(memcached_st *ptr) __attribute__ ((common)) = NULL;
const char * (*memcached_strerror_d)(const memcached_st *ptr, memcached_return_t rc) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_behavior_set_d)(memcached_st *ptr, const memcached_behavior_t flag, uint64_t data) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_delete_d)(memcached_st *ptr, const char *key, size_t key_length, time_t expiration) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_server_add_with_weight_d)(memcached_st *ptr, const char *hostname, in_port_t port, uint32_t weight) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_decrement_d)(memcached_st *ptr, const char *key, size_t key_length, uint32_t offset, uint64_t *value) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_increment_d)(memcached_st *ptr, const char *key, size_t key_length, uint32_t offset, uint64_t *value) __attribute__ ((common)) = NULL;
char * (*memcached_get_d)(memcached_st *ptr, const char *key, size_t key_length, size_t *value_length, uint32_t *flags, memcached_return_t *error) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_add_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_set_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_append_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_prepend_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_replace_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_cas_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags, uint64_t cas) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_decrement_with_initial_d)(memcached_st *ptr, const char *key, size_t key_length, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value) __attribute__ ((common)) = NULL;
memcached_return_t (*memcached_increment_with_initial_d)(memcached_st *ptr, const char *key, size_t key_length, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value) __attribute__ ((common)) = NULL;

//! BZIP
const char * (*BZ2_bzlibVersion_d)(void) __attribute__ ((common)) = NULL;
int (*BZ2_bzBuffToBuffDecompress_d)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen, int small, int verbosity) __attribute__ ((common)) = NULL;
int (*BZ2_bzBuffToBuffCompress_d)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen, int blockSize100k, int verbosity, int workFactor) __attribute__ ((common)) = NULL;

//! CLAMAV
void (*cl_shutdown_d)(void) __attribute__ ((common)) = NULL;
int (*lt_dlexit_d)(void) __attribute__ ((common)) = NULL;
const char * (*cl_retver_d)(void) __attribute__ ((common)) = NULL;
int (*cl_init_d)(unsigned int initoptions) __attribute__ ((common)) = NULL;
const char * (*cl_strerror_d)(int clerror) __attribute__ ((common)) = NULL;
struct cl_engine * (*cl_engine_new_d)(void) __attribute__ ((common)) = NULL;
int (*cl_statfree_d)(struct cl_stat *dbstat) __attribute__ ((common)) = NULL;
int (*cl_engine_free_d)(struct cl_engine *engine) __attribute__ ((common)) = NULL;
int (*cl_engine_compile_d)(struct cl_engine *engine) __attribute__ ((common)) = NULL;
int (*cl_statchkdir_d)(const struct cl_stat *dbstat) __attribute__ ((common)) = NULL;
int (*cl_statinidir_d)(const char *dirname, struct cl_stat *dbstat) __attribute__ ((common)) = NULL;
int (*cl_countsigs_d)(const char *path, unsigned int countoptions, unsigned int *sigs) __attribute__ ((common)) = NULL;
int (*cl_engine_set_num_d)(struct cl_engine *engine, enum cl_engine_field field, long long num) __attribute__ ((common)) = NULL;
int (*cl_engine_set_str_d)(struct cl_engine *engine, enum cl_engine_field field, const char *str) __attribute__ ((common)) = NULL;
int (*cl_load_d)(const char *path, struct cl_engine *engine, unsigned int *signo, unsigned int dboptions) __attribute__ ((common)) = NULL;
int (*cl_scandesc_d)(int desc, const char **virname, unsigned long int *scanned, const struct cl_engine *engine, unsigned int scanoptions) __attribute__ ((common)) = NULL;

//! DSPAM
const char * (*dspam_version_d)(void) __attribute__ ((common)) = NULL;
int (*dspam_detach_d)(DSPAM_CTX *CTX) __attribute__ ((common)) = NULL;
void (*dspam_destroy_d)(DSPAM_CTX * CTX) __attribute__ ((common)) = NULL;
int (*dspam_init_driver_d)(DRIVER_CTX *DTX) __attribute__ ((common)) = NULL;
int (*dspam_shutdown_driver_d)(DRIVER_CTX *DTX) __attribute__ ((common)) = NULL;
int (*dspam_attach_d)(DSPAM_CTX *CTX, void *dbh) __attribute__ ((common)) = NULL;
int (*dspam_process_d)(DSPAM_CTX * CTX, const char *message) __attribute__ ((common)) = NULL;
DSPAM_CTX * (*dspam_create_d)(const char *username, const char *group, const char *home, int operating_mode, u_int32_t flags) __attribute__ ((common)) = NULL;

//! DKIM
// Note that dkim_getsighdr_d is used by the library, so were using dkim_getsighdrx_d.
DKIM_STAT (*dkim_eoh_d)(DKIM *dkim) __attribute__ ((common)) = NULL;
void (*dkim_close_d)(DKIM_LIB *lib) __attribute__ ((common)) = NULL;
DKIM_STAT (*dkim_free_d)(DKIM *dkim) __attribute__ ((common)) = NULL;
uint32_t (*dkim_libversion_d)(void) __attribute__ ((common)) = NULL;
DKIM_STAT (*dkim_eom_d)(DKIM *dkim, _Bool *testkey) __attribute__ ((common)) = NULL;
const char * (*dkim_getresultstr_d)(DKIM_STAT result) __attribute__ ((common)) = NULL;
DKIM_STAT (*dkim_body_d)(DKIM *dkim, u_char *buf, size_t len) __attribute__ ((common)) = NULL;
DKIM_STAT (*dkim_header_d)(DKIM *dkim, u_char *hdr, size_t len) __attribute__ ((common)) = NULL;
DKIM_STAT (*dkim_getsighdrx_d)(DKIM *dkim, u_char *buf, size_t len, size_t initial) __attribute__ ((common)) = NULL;
DKIM * (*dkim_verify_d)(DKIM_LIB *libhandle, const unsigned char *id, void *memclosure, DKIM_STAT *statp) __attribute__ ((common)) = NULL;
DKIM_LIB * (*dkim_init_d)(void *(*mallocf)(void *closure, size_t nbytes), void (*freef)(void *closure, void *p)) __attribute__ ((common)) = NULL;
DKIM * (*dkim_sign_d)(DKIM_LIB *libhandle, const unsigned char *id, void *memclosure, const dkim_sigkey_t secretkey, const unsigned char *selector, const unsigned char *domain, dkim_canon_t hdr_canon_alg, dkim_canon_t body_canon_alg, dkim_alg_t sign_alg,	off_t length, DKIM_STAT *statp) __attribute__ ((common)) = NULL;
DKIM_STAT (*dkim_chunk_d)(DKIM *dkim, unsigned char *chunkp, size_t len) __attribute__ ((common)) = NULL;

//! FreeType
void (*FT_Library_Version_Static_d)(FT_Int *amajor, FT_Int *aminor, FT_Int *apatch) __attribute__ ((common)) = NULL;

//! GD
const char * (*gd_version_d)(void) __attribute__ ((common)) = NULL;
void (*gdFree_d)(void *m) __attribute__ ((common)) = NULL;
void * (*gdImageGifPtr_d)(gdImagePtr im, int *size) __attribute__ ((common)) = NULL;
void (*gdImageDestroy_d)(gdImagePtr im) __attribute__ ((common)) = NULL;
void * (*gdImageJpegPtr_d)(gdImagePtr im, int *size, int quality) __attribute__ ((common)) = NULL;
void (*gdImageSetPixel_d)(gdImagePtr im, int x, int y, int color) __attribute__ ((common)) = NULL;
gdImagePtr (*gdImageCreate_d)(int sx, int sy) __attribute__ ((common)) = NULL;
int (*gdImageColorResolve_d)(gdImagePtr im, int r, int g, int b) __attribute__ ((common)) = NULL;
char * (*gdImageStringFT_d)(gdImage * im, int *brect, int fg, char *fontlist, double ptsize, double angle, int x, int y, char *string) __attribute__ ((common)) = NULL;

//! JPEG
const char * (*jpeg_version_d)(void) __attribute__ ((common)) = NULL;

//! LZO
const char * (*lzo_version_string_d)(void) __attribute__ ((common)) = NULL;
int (*__lzo_init_v2_d)(unsigned, int, int, int, int, int, int, int, int, int) __attribute__ ((common)) = NULL;
lzo_uint32 (*lzo_adler32_d)(lzo_uint32 _adler, const lzo_bytep _buf, lzo_uint _len) __attribute__ ((common)) = NULL;
int (*lzo1x_1_compress_d)(const lzo_byte *src, lzo_uint src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem) __attribute__ ((common)) = NULL;
int (*lzo1x_decompress_safe_d)(const lzo_byte *src, lzo_uint src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem) __attribute__ ((common)) = NULL;

//! MYSQL
void (*my_once_free_d)(void) __attribute__ ((common)) = NULL;
void (*mysql_server_end_d)(void) __attribute__ ((common)) = NULL;
void (*mysql_thread_end_d)(void) __attribute__ ((common)) = NULL;
int (*mysql_ping_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
void (*mysql_close_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
my_bool (*mysql_thread_init_d)(void) __attribute__ ((common)) = NULL;
const char * (*mysql_get_server_info_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
MYSQL * (*mysql_init_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
unsigned int (*mysql_thread_safe_d)(void) __attribute__ ((common)) = NULL;
int (*mysql_stmt_fetch_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
my_bool (*mysql_stmt_close_d)(MYSQL_STMT *) __attribute__ ((common)) = NULL;
unsigned int (*mysql_errno_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
const char * (*mysql_error_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
int (*mysql_stmt_execute_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
my_bool	(*mysql_embedded_d)(void) __attribute__ ((common)) = NULL;
void (*mysql_free_result_d)(MYSQL_RES *result) __attribute__ ((common)) = NULL;
my_bool (*mysql_stmt_reset_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
my_ulonglong (*mysql_insert_id_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
unsigned long (*mysql_thread_id_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
MYSQL_STMT * (*mysql_stmt_init_d)(MYSQL * mysql) __attribute__ ((common)) = NULL;
MYSQL_ROW (*mysql_fetch_row_d)(MYSQL_RES *result) __attribute__ ((common)) = NULL;
unsigned long (*mysql_get_client_version_d)(void) __attribute__ ((common)) = NULL;
MYSQL_RES * (*mysql_store_result_d)(MYSQL * mysql) __attribute__ ((common)) = NULL;
int (*mysql_stmt_store_result_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
my_ulonglong (*mysql_affected_rows_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
my_ulonglong (*mysql_num_rows_d)(MYSQL_RES *result) __attribute__ ((common)) = NULL;
unsigned int (*mysql_stmt_errno_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
const char * (*mysql_stmt_error_d)(MYSQL_STMT * stmt) __attribute__ ((common)) = NULL;
my_bool (*mysql_stmt_free_result_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
unsigned int (*mysql_num_fields_d)(MYSQL_RES *result) __attribute__ ((common)) = NULL;
my_ulonglong (*mysql_stmt_num_rows_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
MYSQL_FIELD * (*mysql_fetch_field_d)(MYSQL_RES * result) __attribute__ ((common)) = NULL;
const char * (*mysql_character_set_name_d)(MYSQL *mysql) __attribute__ ((common)) = NULL;
my_ulonglong (*mysql_stmt_insert_id_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
my_ulonglong (*mysql_stmt_affected_rows_d)(MYSQL_STMT *stmt) __attribute__ ((common)) = NULL;
MYSQL_RES * (*mysql_stmt_result_metadata_d)(MYSQL_STMT * stmt) __attribute__ ((common)) = NULL;
int (*mysql_server_init_d)(int argc, char **argv, char **groups) __attribute__ ((common)) = NULL;
int (*mysql_set_character_set_d)(MYSQL *mysql, const char *csname) __attribute__ ((common)) = NULL;
my_bool (*mysql_stmt_bind_param_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind) __attribute__ ((common)) = NULL;
int (*mysql_options_d)(MYSQL *mysql, enum mysql_option option, const void *arg) __attribute__ ((common)) = NULL;
int (*mysql_real_query_d)(MYSQL *mysql, const char *query, unsigned long length) __attribute__ ((common)) = NULL;
int (*mysql_stmt_prepare_d)(MYSQL_STMT *stmt, const char *query, unsigned long length) __attribute__ ((common)) = NULL;
unsigned long (*mysql_escape_string_d)(char *to, const char *from, unsigned long length) __attribute__ ((common)) = NULL;
my_bool (*mysql_stmt_attr_set_d)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, const void *attr) __attribute__ ((common)) = NULL;
my_bool (*mysql_stmt_bind_result_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind) __attribute__ ((common)) = NULL;const char * STDCALL mysql_character_set_name(MYSQL *mysql);
MYSQL * (*mysql_real_connect_d)(MYSQL * mysql, const char *name, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag) __attribute__ ((common)) = NULL;

//! OPENSSL
DH * (*DH_new_d)(void) __attribute__ ((common)) = NULL;
char **SSL_version_str_d __attribute__ ((common)) = NULL;
int (*SSL_connect_d)(SSL *ssl) __attribute__ ((common)) = NULL;
const SSL_METHOD * (*SSLv23_client_method_d)(void) __attribute__ ((common)) = NULL;
const SSL_METHOD * (*TLSv1_server_method_d)(void) __attribute__ ((common)) = NULL;
void (*DH_free_d)(DH *dh) __attribute__ ((common)) = NULL;
int (*RAND_status_d)(void) __attribute__ ((common)) = NULL;
void (*EVP_cleanup_d)(void) __attribute__ ((common)) = NULL;
void (*OBJ_cleanup_d)(void) __attribute__ ((common)) = NULL;
void (*BN_free_d)(BIGNUM *a) __attribute__ ((common)) = NULL;
void (*RAND_cleanup_d)(void) __attribute__ ((common)) = NULL;
void (*SSL_free_d)(SSL *ssl) __attribute__ ((common)) = NULL;
int (*SSL_accept_d)(SSL *ssl) __attribute__ ((common)) = NULL;
EC_KEY * (*EC_KEY_new_d)(void) __attribute__ ((common)) = NULL;
void (*CRYPTO_free_d) (void *) __attribute__ ((common)) = NULL;
void (*ENGINE_cleanup_d)(void) __attribute__ ((common)) = NULL;
int (*CRYPTO_num_locks_d)(void) __attribute__ ((common)) = NULL;
int (*SSL_library_init_d)(void) __attribute__ ((common)) = NULL;
int (*SSL_shutdown_d)(SSL *ssl) __attribute__ ((common)) = NULL;
void (*BIO_sock_cleanup_d)(void) __attribute__ ((common)) = NULL;
void (*ERR_free_strings_d)(void) __attribute__ ((common)) = NULL;
SSL * (*SSL_new_d)(SSL_CTX * ctx) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_md4_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_md5_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_sha_d)(void) __attribute__ ((common)) = NULL;
void (*COMP_zlib_cleanup_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_sha1_d)(void) __attribute__ ((common)) = NULL;
void (*EC_KEY_free_d)(EC_KEY *key) __attribute__ ((common)) = NULL;
const char * (*OBJ_nid2sn_d)(int n) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_sha224_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_sha256_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_sha384_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_sha512_d)(void) __attribute__ ((common)) = NULL;
void (*OBJ_NAME_cleanup_d)(int type) __attribute__ ((common)) = NULL;
void (*SSL_CTX_free_d)(SSL_CTX *ctx) __attribute__ ((common)) = NULL;
int	(*BN_num_bits_d)(const BIGNUM *) __attribute__ ((common)) = NULL;
int (*X509_get_ext_count_d) (X509 *x) __attribute__ ((common)) = NULL;
char * (*BN_bn2hex_d)(const BIGNUM *a) __attribute__ ((common)) = NULL;
int (*EVP_MD_size_d)(const EVP_MD *md) __attribute__ ((common)) = NULL;
unsigned long (*ERR_get_error_d)(void) __attribute__ ((common)) = NULL;
void (*CONF_modules_unload_d)(int all) __attribute__ ((common)) = NULL;
void (*HMAC_CTX_init_d)(HMAC_CTX *ctx) __attribute__ ((common)) = NULL;
void (*SSL_load_error_strings_d)(void) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_ripemd160_d)(void) __attribute__ ((common)) = NULL;
const char * (*SSLeay_version_d)(int t) __attribute__ ((common)) = NULL;
BIO * (*SSL_get_wbio_d)(const SSL * ssl) __attribute__ ((common)) = NULL;
void (*EC_GROUP_free_d)(EC_GROUP *group) __attribute__ ((common)) = NULL;
void (*EC_POINT_free_d)(EC_POINT *point) __attribute__ ((common)) = NULL;
int (*EC_KEY_generate_key_d)(EC_KEY *key) __attribute__ ((common)) = NULL;
void (*ASN1_STRING_TABLE_cleanup_d)(void) __attribute__ ((common)) = NULL;
void (*HMAC_CTX_cleanup_d)(HMAC_CTX *ctx) __attribute__ ((common)) = NULL;
int (*SSL_get_shutdown_d)(const SSL *ssl) __attribute__ ((common)) = NULL;
void (*CRYPTO_cleanup_all_ex_data_d)(void) __attribute__ ((common)) = NULL;
void (*EVP_MD_CTX_init_d)(EVP_MD_CTX *ctx) __attribute__ ((common)) = NULL;
int (*EC_KEY_check_key_d)(const EC_KEY *key) __attribute__ ((common)) = NULL;
int (*EVP_MD_CTX_cleanup_d)(EVP_MD_CTX *ctx) __attribute__ ((common)) = NULL;
void (*ERR_remove_state_d)(unsigned long pid) __attribute__ ((common)) = NULL;
int 	(*SSL_peek_d)(SSL *ssl,void *buf,int num) __attribute__ ((common)) = NULL;
X509_NAME *	(*X509_get_subject_name_d)(X509 *a) __attribute__ ((common)) = NULL;
EC_KEY * (*EC_KEY_new_by_curve_name_d)(int nid) __attribute__ ((common)) = NULL;
int (*BN_hex2bn_d)(BIGNUM **a, const char *str) __attribute__ ((common)) = NULL;
int (*SSL_read_d)(SSL *ssl, void *buf, int num) __attribute__ ((common)) = NULL;
int (*RAND_bytes_d)(unsigned char *buf, int num) __attribute__ ((common)) = NULL;
void (*EVP_CIPHER_CTX_init_d)(EVP_CIPHER_CTX *a) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_nid_d)(const EVP_CIPHER *cipher) __attribute__ ((common)) = NULL;
void (*OPENSSL_add_all_algorithms_noconf_d)(void) __attribute__ ((common)) = NULL;
int	(*SSL_get_error_d)(const SSL *s,int ret_code) __attribute__ ((common)) = NULL;
const SSL_METHOD * (*SSLv23_server_method_d)(void) __attribute__ ((common)) = NULL;
X509 * (*SSL_get_peer_certificate_d)(const SSL *s) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_CTX_cleanup_d)(EVP_CIPHER_CTX *a) __attribute__ ((common)) = NULL;
BIO * (*BIO_new_socket_d)(int sock, int close_flag) __attribute__ ((common)) = NULL;
EC_GROUP * (*EC_GROUP_new_by_curve_name_d)(int nid) __attribute__ ((common)) = NULL;
EC_POINT * (*EC_POINT_new_d)(const EC_GROUP *group) __attribute__ ((common)) = NULL;
int	(*BN_bn2bin_d)(const BIGNUM *, unsigned char *) __attribute__ ((common)) = NULL;
X509_EXTENSION * (*X509_get_ext_d) (X509 *x, int loc) __attribute__ ((common)) = NULL;
SSL_CTX * (*SSL_CTX_new_d)(const SSL_METHOD * method) __attribute__ ((common)) = NULL;
void (*SSL_set_bio_d)(SSL *ssl, BIO *rbio, BIO *wbio) __attribute__ ((common)) = NULL;
int (*SSL_CTX_check_private_key_d)(const SSL_CTX *ctx) __attribute__ ((common)) = NULL;
int (*SSL_write_d)(SSL *ssl, const void *buf, int num) __attribute__ ((common)) = NULL;
void (*sk_pop_free_d)(_STACK *st, void(*func)(void *)) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_iv_length_d)(const EVP_CIPHER *cipher) __attribute__ ((common)) = NULL;
char * (*ERR_error_string_d)(unsigned long e, char *buf) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_block_size_d)(const EVP_CIPHER *cipher) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_key_length_d)(const EVP_CIPHER *cipher) __attribute__ ((common)) = NULL;
const EC_GROUP * (*EC_KEY_get0_group_d)(const EC_KEY *key) __attribute__ ((common)) = NULL;
const EVP_MD * (*EVP_get_digestbyname_d)(const char *name) __attribute__ ((common)) = NULL;
int	(*SSL_CTX_set_cipher_list_d)(SSL_CTX *,const char *str) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_CTX_iv_length_d)(const EVP_CIPHER_CTX *ctx) __attribute__ ((common)) = NULL;
int (*EVP_DigestInit_d)(EVP_MD_CTX *ctx, const EVP_MD *type) __attribute__ ((common)) = NULL;
int (*EC_KEY_set_group_d)(EC_KEY *key, const EC_GROUP *group) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_CTX_block_size_d)(const EVP_CIPHER_CTX *ctx) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_CTX_key_length_d)(const EVP_CIPHER_CTX *ctx) __attribute__ ((common)) = NULL;
int (*RAND_load_file_d)(const char *filename, long max_bytes) __attribute__ ((common)) = NULL;
const BIGNUM * (*EC_KEY_get0_private_key_d)(const EC_KEY *key) __attribute__ ((common)) = NULL;
const EVP_CIPHER * (*EVP_get_cipherbyname_d)(const char *name) __attribute__ ((common)) = NULL;
const EC_POINT * (*EC_KEY_get0_public_key_d)(const EC_KEY *key) __attribute__ ((common)) = NULL;
int (*EC_GROUP_precompute_mult_d)(EC_GROUP *group, BN_CTX *ctx) __attribute__ ((common)) = NULL;
int (*EC_KEY_set_private_key_d)(EC_KEY *key, const BIGNUM *prv) __attribute__ ((common)) = NULL;
int (*EVP_CIPHER_CTX_set_padding_d)(EVP_CIPHER_CTX *c, int pad) __attribute__ ((common)) = NULL;
STACK_OF(SSL_COMP) * (*SSL_COMP_get_compression_methods_d)(void) __attribute__ ((common)) = NULL;
int (*BIO_vprintf_d)(BIO *bio, const char *format, va_list args) __attribute__ ((common)) = NULL;
int (*EC_KEY_set_public_key_d)(EC_KEY *key, const EC_POINT *pub) __attribute__ ((common)) = NULL;
void (*CRYPTO_set_id_callback_d)(unsigned long(*id_function)(void)) __attribute__ ((common)) = NULL;
long (*SSL_CTX_ctrl_d)(SSL_CTX *ctx, int cmd, long larg, void *parg) __attribute__ ((common)) = NULL;
void (*ERR_error_string_n_d)(unsigned long e, char *buf, size_t len) __attribute__ ((common)) = NULL;
BIGNUM * (*BN_bin2bn_d)(const unsigned char *s, int len, BIGNUM *ret) __attribute__ ((common)) = NULL;
int (*EVP_DigestUpdate_d)(EVP_MD_CTX *ctx, const void *d, size_t cnt) __attribute__ ((common)) = NULL;
int (*HMAC_Final_d)(HMAC_CTX *ctx, unsigned char *md, unsigned int *len) __attribute__ ((common)) = NULL;
int (*HMAC_Update_d)(HMAC_CTX *ctx, const unsigned char *data, size_t len) __attribute__ ((common)) = NULL;
int (*SSL_CTX_use_certificate_chain_file_d)(SSL_CTX *ctx, const char *file) __attribute__ ((common)) = NULL;
int (*EVP_DigestFinal_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) __attribute__ ((common)) = NULL;
int (*EVP_DigestInit_ex_d)(EVP_MD_CTX *ctx, const EVP_MD *type, ENGINE *impl) __attribute__ ((common)) = NULL;
int (*SSL_CTX_use_PrivateKey_file_d)(SSL_CTX *ctx, const char *file, int type) __attribute__ ((common)) = NULL;
int (*X509_NAME_get_text_by_NID_d)(X509_NAME *name, int nid, char *buf,int len) __attribute__ ((common)) = NULL;
int (*EVP_DigestFinal_ex_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) __attribute__ ((common)) = NULL;
int (*EVP_EncryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl) __attribute__ ((common)) = NULL;
int (*EVP_DecryptFinal_ex_d)(EVP_CIPHER_CTX *ctx, unsigned char *outm, int *outl) __attribute__ ((common)) = NULL;
void (*EC_GROUP_set_point_conversion_form_d)(EC_GROUP *, point_conversion_form_t) __attribute__ ((common)) = NULL;
int	(*DH_generate_parameters_ex_d)(DH *dh, int prime_len,int generator, BN_GENCB *cb) __attribute__ ((common)) = NULL;
EC_POINT * (*EC_POINT_hex2point_d)(const EC_GROUP *, const char *, EC_POINT *, BN_CTX *) __attribute__ ((common)) = NULL;
int (*SSL_CTX_load_verify_locations_d)(SSL_CTX *ctx, const char *CAfile, const char *CApath) __attribute__ ((common)) = NULL;
int (*HMAC_Init_ex_d)(HMAC_CTX *ctx, const void *key, int len, const EVP_MD *md, ENGINE *impl) __attribute__ ((common)) = NULL;
void (*SSL_CTX_set_tmp_dh_callback_d)(SSL_CTX *ctx, DH *(*dh)(SSL *ssl,int is_export, int keylength))  __attribute__ ((common)) = NULL;
char * (*EC_POINT_point2hex_d)(const EC_GROUP *, const EC_POINT *, point_conversion_form_t form, BN_CTX *) __attribute__ ((common)) = NULL;
void (*CRYPTO_set_locking_callback_d)(void(*locking_function)(int mode, int n, const char *file, int line)) __attribute__ ((common)) = NULL;
void (*SSL_CTX_set_tmp_ecdh_callback_d)(SSL_CTX *ctx, EC_KEY *(*ecdh)(SSL *ssl,int is_export, int keylength)) __attribute__ ((common)) = NULL;
int (*EVP_DecryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl) __attribute__ ((common)) = NULL;
int (*EVP_EncryptUpdate_d)(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, const unsigned char *in, int inl) __attribute__ ((common)) = NULL;
int (*EC_POINT_oct2point_d)(const EC_GROUP *group, EC_POINT *p, const unsigned char *buf, size_t len, BN_CTX *ctx) __attribute__ ((common)) = NULL;
int (*EVP_Digest_d)(const void *data, size_t count, unsigned char *md, unsigned int *size, const EVP_MD *type, ENGINE *impl) __attribute__ ((common)) = NULL;
int (*EVP_DecryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv) __attribute__ ((common)) = NULL;
int (*EVP_EncryptInit_ex_d)(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher, ENGINE *impl, const unsigned char *key, const unsigned char *iv) __attribute__ ((common)) = NULL;
size_t (*EC_POINT_point2oct_d)(const EC_GROUP *group, const EC_POINT *p, point_conversion_form_t form, unsigned char *buf, size_t len, BN_CTX *ctx) __attribute__ ((common)) = NULL;
int (*ECDH_compute_key_d)(void *out, size_t outlen, const EC_POINT *pub_key, EC_KEY *ecdh, void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen)) __attribute__ ((common)) = NULL;

//! PNG
png_uint_32 (*png_access_version_number_d)(void) __attribute__ ((common)) = NULL;

//! SPF
void (*SPF_server_free_d)(SPF_server_t *sp) __attribute__ ((common)) = NULL;
void (*SPF_request_free_d)(SPF_request_t *sr) __attribute__ ((common)) = NULL;
void (*SPF_response_free_d)(SPF_response_t *rp) __attribute__ ((common)) = NULL;
const char * (*SPF_strreason_d)(SPF_reason_t reason) __attribute__ ((common)) = NULL;
const char * (*SPF_strresult_d)(SPF_result_t result) __attribute__ ((common)) = NULL;
const char * (*SPF_strerror_d)(SPF_errcode_t spf_err) __attribute__ ((common)) = NULL;
SPF_reason_t (*SPF_response_reason_d)(SPF_response_t *rp) __attribute__ ((common)) = NULL;
SPF_result_t (*SPF_response_result_d)(SPF_response_t *rp) __attribute__ ((common)) = NULL;
SPF_request_t * (*SPF_request_new_d)(SPF_server_t * spf_server) __attribute__ ((common)) = NULL;
void (*SPF_get_lib_version_d)(int *major, int *minor, int *patch) __attribute__ ((common)) = NULL;
int (*SPF_request_set_env_from_d)(SPF_request_t *sr, const char *from) __attribute__ ((common)) = NULL;
SPF_server_t * (*SPF_server_new_d)(SPF_server_dnstype_t dnstype, int debug) __attribute__ ((common)) = NULL;
SPF_errcode_t (*SPF_request_set_helo_dom_d)(SPF_request_t *sr, const char *dom) __attribute__ ((common)) = NULL;
SPF_errcode_t	(*SPF_request_set_ipv4_d)(SPF_request_t *sr, struct in_addr addr) __attribute__ ((common)) = NULL;
SPF_errcode_t	(*SPF_request_set_ipv6_d)(SPF_request_t *sr, struct in6_addr addr) __attribute__ ((common)) = NULL;
SPF_dns_server_t * (*SPF_dns_zone_new_d)(SPF_dns_server_t *layer_below, const char *name, int debug) __attribute__ ((common)) = NULL;
SPF_errcode_t (*SPF_request_query_mailfrom_d)(SPF_request_t *spf_request, SPF_response_t **spf_responsep) __attribute__ ((common)) = NULL;
SPF_errcode_t (*SPF_dns_zone_add_str_d)(SPF_dns_server_t *spf_dns_server, const char *domain, ns_type rr_type, SPF_dns_stat_t herrno, const char *data) __attribute__ ((common)) = NULL;

//! TOKYO
char **tcversion_d __attribute__ ((common)) = NULL;
TCHDB * (*tchdbnew_d)(void) __attribute__ ((common)) = NULL;
void (*tcfree_d)(void *ptr) __attribute__ ((common)) = NULL;
void (*tchdbdel_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
bool (*tchdbsync_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
int (*tchdbecode_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
void (*tcndbdel_d)(TCNDB *tree) __attribute__ ((common)) = NULL;
bool (*tchdbclose_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
void (*tclistdel_d)(TCLIST *list) __attribute__ ((common)) = NULL;
TCNDB * (*tcndbdup_d)(TCNDB *ndb) __attribute__ ((common)) = NULL;
bool (*tchdbsetmutex_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
uint64_t (*tchdbfsiz_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
uint64_t (*tchdbrnum_d)(TCHDB *hdb) __attribute__ ((common)) = NULL;
uint64_t (*tcndbrnum_d)(TCNDB *ndb) __attribute__ ((common)) = NULL;
void (*tcndbiterinit_d)(TCNDB *ndb) __attribute__ ((common)) = NULL;
char * (*tcndbiternext2_d)(TCNDB *ndb) __attribute__ ((common)) = NULL;
int (*tclistnum_d)(const TCLIST *list) __attribute__ ((common)) = NULL;
const char * (*tchdberrmsg_d)(int ecode) __attribute__ ((common)) = NULL;
const char * (*tchdbpath_d)(TCHDB * hdb) __attribute__ ((common)) = NULL;
TCLIST * (*tctreekeys_d)(const TCTREE * tree) __attribute__ ((common)) = NULL;
TCLIST * (*tctreevals_d)(const TCTREE * tree) __attribute__ ((common)) = NULL;
TCNDB * (*tcndbnew2_d)(TCCMP cmp, void *cmpop) __attribute__ ((common)) = NULL;
bool (*tchdbdefrag_d)(TCHDB *hdb, int64_t step) __attribute__ ((common)) = NULL;
bool (*tchdbsetdfunit_d)(TCHDB *hdb, int32_t dfunit) __attribute__ ((common)) = NULL;
bool (*tchdbout_d)(TCHDB *hdb, const void *kbuf, int ksiz) __attribute__ ((common)) = NULL;
bool (*tcndbout_d)(TCNDB *ndb, const void *kbuf, int ksiz) __attribute__ ((common)) = NULL;
bool (*tchdbopen_d)(TCHDB *hdb, const char *path, int omode) __attribute__ ((common)) = NULL;
const void * (*tclistval_d)(const TCLIST * list, int index, int *sp) __attribute__ ((common)) = NULL;
void * (*tchdbget_d)(TCHDB * hdb, const void *kbuf, int ksiz, int *sp) __attribute__ ((common)) = NULL;
void * (*tcndbget3_d)(TCNDB *ndb, const void *kbuf, int ksiz, int *sp) __attribute__ ((common)) = NULL;
void * (*tcndbget_d)(TCNDB * ndb, const void *kbuf, int ksiz, int *sp) __attribute__ ((common)) = NULL;
TCLIST * (*tcndbfwmkeys_d)(TCNDB * ndb, const void *pbuf, int psiz, int max) __attribute__ ((common)) = NULL;
bool (*tchdbtune_d)(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts) __attribute__ ((common)) = NULL;
bool (*tchdboptimize_d)(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts) __attribute__ ((common)) = NULL;
bool (*tcndbputkeep_d)(TCNDB *ndb, const void *kbuf, int ksiz, const void *vbuf, int vsiz) __attribute__ ((common)) = NULL;
bool (*tchdbputasync_d)(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz) __attribute__ ((common)) = NULL;
bool (*tcndbgetboth_d)(TCNDB *ndb, const void *kbuf, int ksiz, void **rkbuf, int *rksiz, void **rvbuf, int *rvsiz) __attribute__ ((common)) = NULL;

//! Jansson
const char * (*jansson_version_d)(void) __attribute__ ((common)) = NULL;
int (*json_array_append_d)(json_t *array, json_t *value) __attribute__ ((common)) = NULL;
int (*json_array_insert_d)(json_t *array, size_t index, json_t *value) __attribute__ ((common)) = NULL;
int (*json_array_set_d)(json_t *array, size_t index, json_t *value) __attribute__ ((common)) = NULL;
json_t * (*json_array_d)(void) __attribute__ ((common)) = NULL;
int (*json_array_append_new_d)(json_t *array, json_t *value) __attribute__ ((common)) = NULL;
int (*json_array_clear_d)(json_t *array) __attribute__ ((common)) = NULL;
int (*json_array_extend_d)(json_t *array, json_t *other) __attribute__ ((common)) = NULL;
json_t * (*json_array_get_d)(const json_t *array, size_t index) __attribute__ ((common)) = NULL;
int (*json_array_insert_new_d)(json_t *array, size_t index, json_t *value) __attribute__ ((common)) = NULL;
int (*json_array_remove_d)(json_t *array, size_t index) __attribute__ ((common)) = NULL;
int (*json_array_set_new_d)(json_t *array, size_t index, json_t *value) __attribute__ ((common)) = NULL;
size_t (*json_array_size_d)(const json_t *array) __attribute__ ((common)) = NULL;
json_t * (*json_copy_d)(json_t *value) __attribute__ ((common)) = NULL;
void (*json_decref_d)(json_t *json) __attribute__ ((common)) = NULL;
json_t * (*json_deep_copy_d)(json_t *value) __attribute__ ((common)) = NULL;
void (*json_delete_d)(json_t *json) __attribute__ ((common)) = NULL;
int (*json_dump_file_d)(const json_t *json, const char *path, size_t flags) __attribute__ ((common)) = NULL;
int (*json_dumpf_d)(const json_t *json, FILE *output, size_t flags) __attribute__ ((common)) = NULL;
char * (*json_dumps_d)(const json_t *json, size_t flags) __attribute__ ((common)) = NULL;
int (*json_equal_d)(json_t *value1, json_t *value2) __attribute__ ((common)) = NULL;
json_t * (*json_false_d)(void) __attribute__ ((common)) = NULL;
const char * (*json_type_string_d)(json_t *json) __attribute__ ((common)) = NULL;
json_t * (*json_incref_d)(json_t *json) __attribute__ ((common)) = NULL;
json_t * (*json_integer_d)(json_int_t value) __attribute__ ((common)) = NULL;
int (*json_integer_set_d)(json_t *integer, json_int_t value) __attribute__ ((common)) = NULL;
json_int_t (*json_integer_value_d)(const json_t *integer) __attribute__ ((common)) = NULL;
json_t * (*json_load_file_d)(const char *path, size_t flags, json_error_t *error) __attribute__ ((common)) = NULL;
json_t * (*json_loadf_d)(FILE *input, size_t flags, json_error_t *error) __attribute__ ((common)) = NULL;
json_t * (*json_loads_d)(const char *input, size_t flags, json_error_t *error) __attribute__ ((common)) = NULL;
json_t * (*json_null_d)(void) __attribute__ ((common)) = NULL;
double (*json_number_value_d)(const json_t *json) __attribute__ ((common)) = NULL;
json_t * (*json_object_d)(void) __attribute__ ((common)) = NULL;
int (*json_object_clear_d)(json_t *object) __attribute__ ((common)) = NULL;
int (*json_object_del_d)(json_t *object, const char *key) __attribute__ ((common)) = NULL;
json_t * (*json_object_get_d)(const json_t *object, const char *key) __attribute__ ((common)) = NULL;
void * (*json_object_iter_d)(json_t *object) __attribute__ ((common)) = NULL;
void * (*json_object_iter_at_d)(json_t *object, const char *key) __attribute__ ((common)) = NULL;
const char * (*json_object_iter_key_d)(void *iter) __attribute__ ((common)) = NULL;
void * (*json_object_iter_next_d)(json_t *object, void *iter) __attribute__ ((common)) = NULL;
int (*json_object_iter_set_d)(json_t *object, void *iter, json_t *value) __attribute__ ((common)) = NULL;
int (*json_object_iter_set_new_d)(json_t *object, void *iter, json_t *value) __attribute__ ((common)) = NULL;
json_t * (*json_object_iter_value_d)(void *iter) __attribute__ ((common)) = NULL;
int (*json_object_set_d)(json_t *object, const char *key, json_t *value) __attribute__ ((common)) = NULL;
int (*json_object_set_new_d)(json_t *object, const char *key, json_t *value) __attribute__ ((common)) = NULL;
int (*json_object_set_new_nocheck_d)(json_t *object, const char *key, json_t *value) __attribute__ ((common)) = NULL;
int (*json_object_set_nocheck_d)(json_t *object, const char *key, json_t *value) __attribute__ ((common)) = NULL;
size_t (*json_object_size_d)(const json_t *object) __attribute__ ((common)) = NULL;
int (*json_object_update_d)(json_t *object, json_t *other) __attribute__ ((common)) = NULL;
json_t * (*json_pack_d)(const char *fmt, ...) __attribute__ ((common)) = NULL;
json_t * (*json_pack_ex_d)(json_error_t *error, size_t flags, const char *fmt, ...) __attribute__ ((common)) = NULL;
json_t * (*json_real_d)(double value) __attribute__ ((common)) = NULL;
int (*json_real_set_d)(json_t *real, double value) __attribute__ ((common)) = NULL;
double (*json_real_value_d)(const json_t *real) __attribute__ ((common)) = NULL;
void (*json_set_alloc_funcs_d)(json_malloc_t malloc_fn, json_free_t free_fn) __attribute__ ((common)) = NULL;
json_t * (*json_string_d)(const char *value) __attribute__ ((common)) = NULL;
json_t * (*json_string_nocheck_d)(const char *value) __attribute__ ((common)) = NULL;
int (*json_string_set_d)(json_t *string, const char *value) __attribute__ ((common)) = NULL;
int (*json_string_set_nocheck_d)(json_t *string, const char *value) __attribute__ ((common)) = NULL;
const char * (*json_string_value_d)(const json_t *string) __attribute__ ((common)) = NULL;
json_t * (*json_true_d)(void) __attribute__ ((common)) = NULL;
int (*json_unpack_d)(json_t *root, const char *fmt, ...) __attribute__ ((common)) = NULL;
int (*json_unpack_ex_d)(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...) __attribute__ ((common)) = NULL;
json_t * (*json_vpack_ex_d)(json_error_t *error, size_t flags, const char *fmt, va_list ap) __attribute__ ((common)) = NULL;
int (*json_vunpack_ex_d)(json_t *root, json_error_t *error, size_t flags, const char *fmt, va_list ap) __attribute__ ((common)) = NULL;

//! XML
char **xmlParserVersion_d __attribute__ ((common)) = NULL;
void (*xmlInitParser_d)(void) __attribute__ ((common)) = NULL;
void (*xmlMemoryDump_d)(void) __attribute__ ((common)) = NULL;
void (*xmlCleanupParser_d)(void) __attribute__ ((common)) = NULL;
void (*xmlCleanupGlobals_d)(void) __attribute__ ((common)) = NULL;
void (*xmlFreeDoc_d)(xmlDocPtr doc) __attribute__ ((common)) = NULL;
void (*xmlFreeNode_d)(xmlNodePtr cur) __attribute__ ((common)) = NULL;
xmlBufferPtr (*xmlBufferCreate_d)(void) __attribute__ ((common)) = NULL;
void (*xmlBufferFree_d)(xmlBufferPtr buf) __attribute__ ((common)) = NULL;
xmlParserCtxtPtr (*xmlNewParserCtxt_d)(void) __attribute__ ((common)) = NULL;
int (*xmlBufferLength_d)(const xmlBufferPtr buf) __attribute__ ((common)) = NULL;
void (*xmlFreeParserCtxt_d)(xmlParserCtxtPtr ctx) __attribute__ ((common)) = NULL;
void (*xmlXPathFreeObject_d)(xmlXPathObjectPtr obj) __attribute__ ((common)) = NULL;
void (*xmlXPathFreeContext_d)(xmlXPathContextPtr ctx) __attribute__ ((common)) = NULL;
xmlXPathContextPtr (*xmlXPathNewContext_d)(xmlDocPtr doc) __attribute__ ((common)) = NULL;
xmlNodePtr (*xmlNewNode_d)(xmlNsPtr ns, const xmlChar *name) __attribute__ ((common)) = NULL;
const xmlChar * (*xmlBufferContent_d)(const xmlBufferPtr buf) __attribute__ ((common)) = NULL;
xmlNodePtr (*xmlAddSibling_d)(xmlNodePtr cur, xmlNodePtr elem) __attribute__ ((common)) = NULL;
int (*xmlNodeBufGetContent_d)(xmlBufferPtr buffer, xmlNodePtr cur) __attribute__ ((common)) = NULL;
void (*xmlNodeSetContent_d)(xmlNodePtr cur, const xmlChar *content) __attribute__ ((common)) = NULL;
xmlChar * (*xmlEncodeEntitiesReentrant_d)(xmlDocPtr doc, const xmlChar * input) __attribute__ ((common)) = NULL;
void (*xmlDocDumpFormatMemory_d)(xmlDocPtr cur, xmlChar **mem, int *size, int format) __attribute__ ((common)) = NULL;
xmlAttrPtr (*xmlSetProp_d)(xmlNodePtr node, const xmlChar *name, const xmlChar *value) __attribute__ ((common)) = NULL;
xmlXPathObjectPtr (*xmlXPathEvalExpression_d)(const xmlChar *xpath, xmlXPathContextPtr ctx) __attribute__ ((common)) = NULL;
int (*xmlXPathRegisterNs_d)(xmlXPathContextPtr ctxt, const xmlChar *prefix, const xmlChar *ns_uri) __attribute__ ((common)) = NULL;
xmlDocPtr (*xmlCtxtReadMemory_d)(xmlParserCtxtPtr ctxt, const char *buffer, int size, const char *url, const char *encoding, int options) __attribute__ ((common)) = NULL;

//! ZLIB
const char * (*zlibVersion_d)(void) __attribute__ ((common)) = NULL;
uLong (*compressBound_d)(uLong sourceLen) __attribute__ ((common)) = NULL;
int (*uncompress_d)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) __attribute__ ((common)) = NULL;
int (*compress2_d)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level) __attribute__ ((common)) = NULL;

#endif

