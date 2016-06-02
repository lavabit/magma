
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

#include "magma.h"

//! MEMCACHED
memcached_return_t (*memcached_flush_d)(memcached_st *ptr, time_t expiration) = NULL;
void (*memcached_free_d)(memcached_st *ptr) = NULL;
const char * (*memcached_lib_version_d)(void) = NULL;
memcached_st * (*memcached_create_d)(memcached_st *ptr) = NULL;
const char * (*memcached_strerror_d)(const memcached_st *ptr, memcached_return_t rc) = NULL;
memcached_return_t (*memcached_behavior_set_d)(memcached_st *ptr, const memcached_behavior_t flag, uint64_t data) = NULL;
memcached_return_t (*memcached_delete_d)(memcached_st *ptr, const char *key, size_t key_length, time_t expiration) = NULL;
memcached_return_t (*memcached_server_add_with_weight_d)(memcached_st *ptr, const char *hostname, in_port_t port, uint32_t weight) = NULL;
memcached_return_t (*memcached_decrement_d)(memcached_st *ptr, const char *key, size_t key_length, uint32_t offset, uint64_t *value) = NULL;
memcached_return_t (*memcached_increment_d)(memcached_st *ptr, const char *key, size_t key_length, uint32_t offset, uint64_t *value) = NULL;
char * (*memcached_get_d)(memcached_st *ptr, const char *key, size_t key_length, size_t *value_length, uint32_t *flags, memcached_return_t *error) = NULL;
memcached_return_t (*memcached_add_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) = NULL;
memcached_return_t (*memcached_set_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) = NULL;
memcached_return_t (*memcached_append_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) = NULL;
memcached_return_t (*memcached_prepend_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) = NULL;
memcached_return_t (*memcached_replace_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags) = NULL;
memcached_return_t (*memcached_cas_d)(memcached_st *ptr, const char *key, size_t key_length, const char *value, size_t value_length, time_t expiration, uint32_t flags, uint64_t cas) = NULL;
memcached_return_t (*memcached_decrement_with_initial_d)(memcached_st *ptr, const char *key, size_t key_length, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value) = NULL;
memcached_return_t (*memcached_increment_with_initial_d)(memcached_st *ptr, const char *key, size_t key_length, uint64_t offset, uint64_t initial, time_t expiration, uint64_t *value) = NULL;

//! BZIP
const char * (*BZ2_bzlibVersion_d)(void) = NULL;
int (*BZ2_bzBuffToBuffDecompress_d)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen, int small, int verbosity) = NULL;
int (*BZ2_bzBuffToBuffCompress_d)(char *dest, unsigned int *destLen, char *source, unsigned int sourceLen, int blockSize100k, int verbosity, int workFactor) = NULL;

//! CLAMAV
void (*cl_shutdown_d)(void) = NULL;
int (*lt_dlexit_d)(void) = NULL;
const char * (*cl_retver_d)(void) = NULL;
int (*cl_init_d)(unsigned int initoptions) = NULL;
const char * (*cl_strerror_d)(int clerror) = NULL;
struct cl_engine * (*cl_engine_new_d)(void) = NULL;
int (*cl_statfree_d)(struct cl_stat *dbstat) = NULL;
int (*cl_engine_free_d)(struct cl_engine *engine) = NULL;
int (*cl_engine_compile_d)(struct cl_engine *engine) = NULL;
int (*cl_statchkdir_d)(const struct cl_stat *dbstat) = NULL;
int (*cl_statinidir_d)(const char *dirname, struct cl_stat *dbstat) = NULL;
int (*cl_countsigs_d)(const char *path, unsigned int countoptions, unsigned int *sigs) = NULL;
int (*cl_engine_set_num_d)(struct cl_engine *engine, enum cl_engine_field field, long long num) = NULL;
int (*cl_engine_set_str_d)(struct cl_engine *engine, enum cl_engine_field field, const char *str) = NULL;
int (*cl_load_d)(const char *path, struct cl_engine *engine, unsigned int *signo, unsigned int dboptions) = NULL;
int (*cl_scandesc_d)(int desc, const char **virname, unsigned long int *scanned, const struct cl_engine *engine, unsigned int scanoptions) = NULL;

//! DSPAM
const char * (*dspam_version_d)(void) = NULL;
int (*dspam_detach_d)(DSPAM_CTX *CTX) = NULL;
void (*dspam_destroy_d)(DSPAM_CTX * CTX) = NULL;
int (*dspam_init_driver_d)(DRIVER_CTX *DTX) = NULL;
int (*dspam_shutdown_driver_d)(DRIVER_CTX *DTX) = NULL;
int (*dspam_attach_d)(DSPAM_CTX *CTX, void *dbh) = NULL;
int (*dspam_process_d)(DSPAM_CTX * CTX, const char *message) = NULL;
DSPAM_CTX * (*dspam_create_d)(const char *username, const char *group, const char *home, int operating_mode, u_int32_t flags) = NULL;

//! DKIM
/// @note that dkim_getsighdr_d is used by the library, so were using dkim_getsighdrx_d.
DKIM_STAT (*dkim_eoh_d)(DKIM *dkim) = NULL;
void (*dkim_close_d)(DKIM_LIB *lib) = NULL;
DKIM_STAT (*dkim_free_d)(DKIM *dkim) = NULL;
uint32_t (*dkim_libversion_d)(void) = NULL;
DKIM_STAT (*dkim_eom_d)(DKIM *dkim, _Bool *testkey) = NULL;
const char * (*dkim_getresultstr_d)(DKIM_STAT result) = NULL;
DKIM_STAT (*dkim_body_d)(DKIM *dkim, u_char *buf, size_t len) = NULL;
DKIM_STAT (*dkim_header_d)(DKIM *dkim, u_char *hdr, size_t len) = NULL;
DKIM_STAT (*dkim_getsighdrx_d)(DKIM *dkim, u_char *buf, size_t len, size_t initial) = NULL;
DKIM * (*dkim_verify_d)(DKIM_LIB *libhandle, const unsigned char *id, void *memclosure, DKIM_STAT *statp) = NULL;
DKIM_LIB * (*dkim_init_d)(void *(*mallocf)(void *closure, size_t nbytes), void (*freef)(void *closure, void *p)) = NULL;
DKIM * (*dkim_sign_d)(DKIM_LIB *libhandle, const unsigned char *id, void *memclosure, const dkim_sigkey_t secretkey, const unsigned char *selector, const unsigned char *domain, dkim_canon_t hdr_canon_alg, dkim_canon_t body_canon_alg, dkim_alg_t sign_alg,	off_t length, DKIM_STAT *statp) = NULL;
DKIM_STAT (*dkim_chunk_d)(DKIM *dkim, unsigned char *chunkp, size_t len) = NULL;

//! FreeType
void (*FT_Library_Version_Static_d)(FT_Int *amajor, FT_Int *aminor, FT_Int *apatch) = NULL;

//! GD
const char * (*gd_version_d)(void) = NULL;
void (*gdFree_d)(void *m) = NULL;
void * (*gdImageGifPtr_d)(gdImagePtr im, int *size) = NULL;
void (*gdImageDestroy_d)(gdImagePtr im) = NULL;
void * (*gdImageJpegPtr_d)(gdImagePtr im, int *size, int quality) = NULL;
void (*gdImageSetPixel_d)(gdImagePtr im, int x, int y, int color) = NULL;
gdImagePtr (*gdImageCreate_d)(int sx, int sy) = NULL;
int (*gdImageColorResolve_d)(gdImagePtr im, int r, int g, int b) = NULL;
char * (*gdImageStringFT_d)(gdImage * im, int *brect, int fg, char *fontlist, double ptsize, double angle, int x, int y, char *string) = NULL;

//! JPEG
const char * (*jpeg_version_d)(void) = NULL;

//! LZO
const char * (*lzo_version_string_d)(void) = NULL;
int (*__lzo_init_v2_d)(unsigned, int, int, int, int, int, int, int, int, int) = NULL;
lzo_uint32 (*lzo_adler32_d)(lzo_uint32 _adler, const lzo_bytep _buf, lzo_uint _len) = NULL;
int (*lzo1x_1_compress_d)(const lzo_byte *src, lzo_uint src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem) = NULL;
int (*lzo1x_decompress_safe_d)(const lzo_byte *src, lzo_uint src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem) = NULL;

//! MYSQL
void (*my_once_free_d)(void) = NULL;
void (*mysql_server_end_d)(void) = NULL;
void (*mysql_thread_end_d)(void) = NULL;
int (*mysql_ping_d)(MYSQL *mysql) = NULL;
void (*mysql_close_d)(MYSQL *mysql) = NULL;
my_bool (*mysql_thread_init_d)(void) = NULL;
const char * (*mysql_get_server_info_d)(MYSQL *mysql) = NULL;
MYSQL * (*mysql_init_d)(MYSQL *mysql) = NULL;
unsigned int (*mysql_thread_safe_d)(void) = NULL;
int (*mysql_stmt_fetch_d)(MYSQL_STMT *stmt) = NULL;
my_bool (*mysql_stmt_close_d)(MYSQL_STMT *) = NULL;
unsigned int (*mysql_errno_d)(MYSQL *mysql) = NULL;
const char * (*mysql_error_d)(MYSQL *mysql) = NULL;
int (*mysql_stmt_execute_d)(MYSQL_STMT *stmt) = NULL;
my_bool	(*mysql_embedded_d)(void) = NULL;
void (*mysql_free_result_d)(MYSQL_RES *result) = NULL;
my_bool (*mysql_stmt_reset_d)(MYSQL_STMT *stmt) = NULL;
my_ulonglong (*mysql_insert_id_d)(MYSQL *mysql) = NULL;
unsigned long (*mysql_thread_id_d)(MYSQL *mysql) = NULL;
MYSQL_STMT * (*mysql_stmt_init_d)(MYSQL * mysql) = NULL;
MYSQL_ROW (*mysql_fetch_row_d)(MYSQL_RES *result) = NULL;
unsigned long (*mysql_get_client_version_d)(void) = NULL;
MYSQL_RES * (*mysql_store_result_d)(MYSQL * mysql) = NULL;
int (*mysql_stmt_store_result_d)(MYSQL_STMT *stmt) = NULL;
my_ulonglong (*mysql_affected_rows_d)(MYSQL *mysql) = NULL;
my_ulonglong (*mysql_num_rows_d)(MYSQL_RES *result) = NULL;
unsigned int (*mysql_stmt_errno_d)(MYSQL_STMT *stmt) = NULL;
const char * (*mysql_stmt_error_d)(MYSQL_STMT * stmt) = NULL;
my_bool (*mysql_stmt_free_result_d)(MYSQL_STMT *stmt) = NULL;
unsigned int (*mysql_num_fields_d)(MYSQL_RES *result) = NULL;
my_ulonglong (*mysql_stmt_num_rows_d)(MYSQL_STMT *stmt) = NULL;
MYSQL_FIELD * (*mysql_fetch_field_d)(MYSQL_RES * result) = NULL;
const char * (*mysql_character_set_name_d)(MYSQL *mysql) = NULL;
my_ulonglong (*mysql_stmt_insert_id_d)(MYSQL_STMT *stmt) = NULL;
my_ulonglong (*mysql_stmt_affected_rows_d)(MYSQL_STMT *stmt) = NULL;
MYSQL_RES * (*mysql_stmt_result_metadata_d)(MYSQL_STMT * stmt) = NULL;
int (*mysql_server_init_d)(int argc, char **argv, char **groups) = NULL;
int (*mysql_set_character_set_d)(MYSQL *mysql, const char *csname) = NULL;
my_bool (*mysql_stmt_bind_param_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind) = NULL;
int (*mysql_options_d)(MYSQL *mysql, enum mysql_option option, const void *arg) = NULL;
int (*mysql_real_query_d)(MYSQL *mysql, const char *query, unsigned long length) = NULL;
int (*mysql_stmt_prepare_d)(MYSQL_STMT *stmt, const char *query, unsigned long length) = NULL;
unsigned long (*mysql_escape_string_d)(char *to, const char *from, unsigned long length) = NULL;
my_bool (*mysql_stmt_attr_set_d)(MYSQL_STMT *stmt, enum enum_stmt_attr_type attr_type, const void *attr) = NULL;
my_bool (*mysql_stmt_bind_result_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind) = NULL;
MYSQL * (*mysql_real_connect_d)(MYSQL * mysql, const char *name, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag) = NULL;

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
int (*SSL_shutdown_d)(SSL *ssl) = NULL;
void (*BIO_sock_cleanup_d)(void) = NULL;
void (*ERR_free_strings_d)(void) = NULL;
SSL * (*SSL_new_d)(SSL_CTX * ctx) = NULL;
const EVP_MD * (*EVP_md4_d)(void) = NULL;
const EVP_MD * (*EVP_md5_d)(void) = NULL;
const EVP_MD * (*EVP_sha_d)(void) = NULL;
void (*COMP_zlib_cleanup_d)(void) = NULL;
const EVP_MD * (*EVP_sha1_d)(void) = NULL;
void (*EC_KEY_free_d)(EC_KEY *key) = NULL;
const char * (*OBJ_nid2sn_d)(int n) = NULL;
const EVP_MD * (*EVP_sha224_d)(void) = NULL;
const EVP_MD * (*EVP_sha256_d)(void) = NULL;
const EVP_MD * (*EVP_sha384_d)(void) = NULL;
const EVP_MD * (*EVP_sha512_d)(void) = NULL;
void (*OBJ_NAME_cleanup_d)(int type) = NULL;
void (*SSL_CTX_free_d)(SSL_CTX *ctx) = NULL;
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
const BIGNUM * (*EC_KEY_get0_private_key_d)(const EC_KEY *key) = NULL;
const EVP_CIPHER * (*EVP_get_cipherbyname_d)(const char *name) = NULL;
const EC_POINT * (*EC_KEY_get0_public_key_d)(const EC_KEY *key) = NULL;
int (*EC_GROUP_precompute_mult_d)(EC_GROUP *group, BN_CTX *ctx) = NULL;
int (*EC_KEY_set_private_key_d)(EC_KEY *key, const BIGNUM *prv) = NULL;
int (*EVP_CIPHER_CTX_set_padding_d)(EVP_CIPHER_CTX *c, int pad) = NULL;
STACK_OF(SSL_COMP) * (*SSL_COMP_get_compression_methods_d)(void) = NULL;
int (*BIO_vprintf_d)(BIO *bio, const char *format, va_list args) = NULL;
int (*EC_KEY_set_public_key_d)(EC_KEY *key, const EC_POINT *pub) = NULL;
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

//! PNG
png_uint_32 (*png_access_version_number_d)(void) = NULL;

//! SPF
void (*SPF_server_free_d)(SPF_server_t *sp) = NULL;
void (*SPF_request_free_d)(SPF_request_t *sr) = NULL;
void (*SPF_response_free_d)(SPF_response_t *rp) = NULL;
const char * (*SPF_strreason_d)(SPF_reason_t reason) = NULL;
const char * (*SPF_strresult_d)(SPF_result_t result) = NULL;
const char * (*SPF_strerror_d)(SPF_errcode_t spf_err) = NULL;
SPF_reason_t (*SPF_response_reason_d)(SPF_response_t *rp) = NULL;
SPF_result_t (*SPF_response_result_d)(SPF_response_t *rp) = NULL;
SPF_request_t * (*SPF_request_new_d)(SPF_server_t * spf_server) = NULL;
void (*SPF_get_lib_version_d)(int *major, int *minor, int *patch) = NULL;
int (*SPF_request_set_env_from_d)(SPF_request_t *sr, const char *from) = NULL;
SPF_server_t * (*SPF_server_new_d)(SPF_server_dnstype_t dnstype, int debug) = NULL;
SPF_errcode_t (*SPF_request_set_helo_dom_d)(SPF_request_t *sr, const char *dom) = NULL;
SPF_errcode_t	(*SPF_request_set_ipv4_d)(SPF_request_t *sr, struct in_addr addr) = NULL;
SPF_errcode_t	(*SPF_request_set_ipv6_d)(SPF_request_t *sr, struct in6_addr addr) = NULL;
SPF_dns_server_t * (*SPF_dns_zone_new_d)(SPF_dns_server_t *layer_below, const char *name, int debug) = NULL;
SPF_errcode_t (*SPF_request_query_mailfrom_d)(SPF_request_t *spf_request, SPF_response_t **spf_responsep) = NULL;
SPF_errcode_t (*SPF_dns_zone_add_str_d)(SPF_dns_server_t *spf_dns_server, const char *domain, ns_type rr_type, SPF_dns_stat_t herrno, const char *data) = NULL;

//! TOKYO
char **tcversion_d = NULL;
TCHDB * (*tchdbnew_d)(void) = NULL;
void (*tcfree_d)(void *ptr) = NULL;
void (*tchdbdel_d)(TCHDB *hdb) = NULL;
bool (*tchdbsync_d)(TCHDB *hdb) = NULL;
int (*tchdbecode_d)(TCHDB *hdb) = NULL;
void (*tcndbdel_d)(TCNDB *tree) = NULL;
bool (*tchdbclose_d)(TCHDB *hdb) = NULL;
void (*tclistdel_d)(TCLIST *list) = NULL;
TCNDB * (*tcndbdup_d)(TCNDB *ndb) = NULL;
bool (*tchdbsetmutex_d)(TCHDB *hdb) = NULL;
uint64_t (*tchdbfsiz_d)(TCHDB *hdb) = NULL;
uint64_t (*tchdbrnum_d)(TCHDB *hdb) = NULL;
uint64_t (*tcndbrnum_d)(TCNDB *ndb) = NULL;
void (*tcndbiterinit_d)(TCNDB *ndb) = NULL;
char * (*tcndbiternext2_d)(TCNDB *ndb) = NULL;
int (*tclistnum_d)(const TCLIST *list) = NULL;
const char * (*tchdberrmsg_d)(int ecode) = NULL;
const char * (*tchdbpath_d)(TCHDB * hdb) = NULL;
TCLIST * (*tctreekeys_d)(const TCTREE * tree) = NULL;
TCLIST * (*tctreevals_d)(const TCTREE * tree) = NULL;
TCNDB * (*tcndbnew2_d)(TCCMP cmp, void *cmpop) = NULL;
bool (*tchdbdefrag_d)(TCHDB *hdb, int64_t step) = NULL;
bool (*tchdbsetdfunit_d)(TCHDB *hdb, int32_t dfunit) = NULL;
bool (*tchdbout_d)(TCHDB *hdb, const void *kbuf, int ksiz) = NULL;
bool (*tcndbout_d)(TCNDB *ndb, const void *kbuf, int ksiz) = NULL;
bool (*tchdbopen_d)(TCHDB *hdb, const char *path, int omode) = NULL;
const void * (*tclistval_d)(const TCLIST * list, int index, int *sp) = NULL;
void * (*tchdbget_d)(TCHDB * hdb, const void *kbuf, int ksiz, int *sp) = NULL;
void * (*tcndbget3_d)(TCNDB *ndb, const void *kbuf, int ksiz, int *sp) = NULL;
void * (*tcndbget_d)(TCNDB * ndb, const void *kbuf, int ksiz, int *sp) = NULL;
TCLIST * (*tcndbfwmkeys_d)(TCNDB * ndb, const void *pbuf, int psiz, int max) = NULL;
bool (*tchdbtune_d)(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts) = NULL;
bool (*tchdboptimize_d)(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts) = NULL;
bool (*tcndbputkeep_d)(TCNDB *ndb, const void *kbuf, int ksiz, const void *vbuf, int vsiz) = NULL;
bool (*tchdbputasync_d)(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz) = NULL;
bool (*tcndbgetboth_d)(TCNDB *ndb, const void *kbuf, int ksiz, void **rkbuf, int *rksiz, void **rvbuf, int *rvsiz) = NULL;

//! Jansson
const char * (*jansson_version_d)(void) = NULL;
int (*json_array_append_d)(json_t *array, json_t *value) = NULL;
int (*json_array_insert_d)(json_t *array, size_t index, json_t *value) = NULL;
int (*json_array_set_d)(json_t *array, size_t index, json_t *value) = NULL;
json_t * (*json_array_d)(void) = NULL;
int (*json_array_append_new_d)(json_t *array, json_t *value) = NULL;
int (*json_array_clear_d)(json_t *array) = NULL;
int (*json_array_extend_d)(json_t *array, json_t *other) = NULL;
json_t * (*json_array_get_d)(const json_t *array, size_t index) = NULL;
int (*json_array_insert_new_d)(json_t *array, size_t index, json_t *value) = NULL;
int (*json_array_remove_d)(json_t *array, size_t index) = NULL;
int (*json_array_set_new_d)(json_t *array, size_t index, json_t *value) = NULL;
size_t (*json_array_size_d)(const json_t *array) = NULL;
json_t * (*json_copy_d)(json_t *value) = NULL;
void (*json_decref_d)(json_t *json) = NULL;
json_t * (*json_deep_copy_d)(json_t *value) = NULL;
void (*json_delete_d)(json_t *json) = NULL;
int (*json_dump_file_d)(const json_t *json, const char *path, size_t flags) = NULL;
int (*json_dumpf_d)(const json_t *json, FILE *output, size_t flags) = NULL;
char * (*json_dumps_d)(const json_t *json, size_t flags) = NULL;
int (*json_equal_d)(json_t *value1, json_t *value2) = NULL;
json_t * (*json_false_d)(void) = NULL;
const char * (*json_type_string_d)(json_t *json) = NULL;
json_t * (*json_incref_d)(json_t *json) = NULL;
json_t * (*json_integer_d)(json_int_t value) = NULL;
int (*json_integer_set_d)(json_t *integer, json_int_t value) = NULL;
json_int_t (*json_integer_value_d)(const json_t *integer) = NULL;
json_t * (*json_load_file_d)(const char *path, size_t flags, json_error_t *error) = NULL;
json_t * (*json_loadf_d)(FILE *input, size_t flags, json_error_t *error) = NULL;
json_t * (*json_loads_d)(const char *input, size_t flags, json_error_t *error) = NULL;
json_t * (*json_null_d)(void) = NULL;
double (*json_number_value_d)(const json_t *json) = NULL;
json_t * (*json_object_d)(void) = NULL;
int (*json_object_clear_d)(json_t *object) = NULL;
int (*json_object_del_d)(json_t *object, const char *key) = NULL;
json_t * (*json_object_get_d)(const json_t *object, const char *key) = NULL;
void * (*json_object_iter_d)(json_t *object) = NULL;
void * (*json_object_iter_at_d)(json_t *object, const char *key) = NULL;
const char * (*json_object_iter_key_d)(void *iter) = NULL;
void * (*json_object_iter_next_d)(json_t *object, void *iter) = NULL;
int (*json_object_iter_set_d)(json_t *object, void *iter, json_t *value) = NULL;
int (*json_object_iter_set_new_d)(json_t *object, void *iter, json_t *value) = NULL;
json_t * (*json_object_iter_value_d)(void *iter) = NULL;
int (*json_object_set_d)(json_t *object, const char *key, json_t *value) = NULL;
int (*json_object_set_new_d)(json_t *object, const char *key, json_t *value) = NULL;
int (*json_object_set_new_nocheck_d)(json_t *object, const char *key, json_t *value) = NULL;
int (*json_object_set_nocheck_d)(json_t *object, const char *key, json_t *value) = NULL;
size_t (*json_object_size_d)(const json_t *object) = NULL;
int (*json_object_update_d)(json_t *object, json_t *other) = NULL;
json_t * (*json_pack_d)(const char *fmt, ...) = NULL;
json_t * (*json_pack_ex_d)(json_error_t *error, size_t flags, const char *fmt, ...) = NULL;
json_t * (*json_real_d)(double value) = NULL;
int (*json_real_set_d)(json_t *real, double value) = NULL;
double (*json_real_value_d)(const json_t *real) = NULL;
void (*json_set_alloc_funcs_d)(json_malloc_t malloc_fn, json_free_t free_fn) = NULL;
json_t * (*json_string_d)(const char *value) = NULL;
json_t * (*json_string_nocheck_d)(const char *value) = NULL;
int (*json_string_set_d)(json_t *string, const char *value) = NULL;
int (*json_string_set_nocheck_d)(json_t *string, const char *value) = NULL;
const char * (*json_string_value_d)(const json_t *string) = NULL;
json_t * (*json_true_d)(void) = NULL;
int (*json_unpack_d)(json_t *root, const char *fmt, ...) = NULL;
int (*json_unpack_ex_d)(json_t *root, json_error_t *error, size_t flags, const char *fmt, ...) = NULL;
json_t * (*json_vpack_ex_d)(json_error_t *error, size_t flags, const char *fmt, va_list ap) = NULL;
int (*json_vunpack_ex_d)(json_t *root, json_error_t *error, size_t flags, const char *fmt, va_list ap) = NULL;

//! UTF8
const char * (*utf8proc_release_d)(void) = NULL;
const char * (*utf8proc_errmsg_d)(utf8proc_ssize_t errcode) = NULL;
const char * (*utf8proc_category_string_d)(utf8proc_int32_t c) = NULL;
utf8proc_category_t (*utf8proc_category_d)(utf8proc_int32_t c) = NULL;
const utf8proc_property_t * (*utf8proc_get_property_d)(utf8proc_int32_t uc) = NULL;
utf8proc_ssize_t (*utf8proc_iterate_d)(const utf8proc_uint8_t *str, utf8proc_ssize_t strlen, utf8proc_int32_t *codepoint_ref) = NULL;

//! XML
char **xmlParserVersion_d = NULL;
void (*xmlInitParser_d)(void) = NULL;
void (*xmlMemoryDump_d)(void) = NULL;
void (*xmlCleanupParser_d)(void) = NULL;
void (*xmlCleanupGlobals_d)(void) = NULL;
void (*xmlFreeDoc_d)(xmlDocPtr doc) = NULL;
void (*xmlFreeNode_d)(xmlNodePtr cur) = NULL;
xmlBufferPtr (*xmlBufferCreate_d)(void) = NULL;
void (*xmlBufferFree_d)(xmlBufferPtr buf) = NULL;
xmlParserCtxtPtr (*xmlNewParserCtxt_d)(void) = NULL;
int (*xmlBufferLength_d)(const xmlBufferPtr buf) = NULL;
void (*xmlFreeParserCtxt_d)(xmlParserCtxtPtr ctx) = NULL;
void (*xmlXPathFreeObject_d)(xmlXPathObjectPtr obj) = NULL;
void (*xmlXPathFreeContext_d)(xmlXPathContextPtr ctx) = NULL;
xmlXPathContextPtr (*xmlXPathNewContext_d)(xmlDocPtr doc) = NULL;
xmlNodePtr (*xmlNewNode_d)(xmlNsPtr ns, const xmlChar *name) = NULL;
const xmlChar * (*xmlBufferContent_d)(const xmlBufferPtr buf) = NULL;
xmlNodePtr (*xmlAddSibling_d)(xmlNodePtr cur, xmlNodePtr elem) = NULL;
int (*xmlNodeBufGetContent_d)(xmlBufferPtr buffer, xmlNodePtr cur) = NULL;
void (*xmlNodeSetContent_d)(xmlNodePtr cur, const xmlChar *content) = NULL;
xmlChar * (*xmlEncodeEntitiesReentrant_d)(xmlDocPtr doc, const xmlChar * input) = NULL;
void (*xmlDocDumpFormatMemory_d)(xmlDocPtr cur, xmlChar **mem, int *size, int format) = NULL;
xmlAttrPtr (*xmlSetProp_d)(xmlNodePtr node, const xmlChar *name, const xmlChar *value) = NULL;
xmlXPathObjectPtr (*xmlXPathEvalExpression_d)(const xmlChar *xpath, xmlXPathContextPtr ctx) = NULL;
int (*xmlXPathRegisterNs_d)(xmlXPathContextPtr ctxt, const xmlChar *prefix, const xmlChar *ns_uri) = NULL;
xmlDocPtr (*xmlCtxtReadMemory_d)(xmlParserCtxtPtr ctxt, const char *buffer, int size, const char *url, const char *encoding, int options) = NULL;

//! ZLIB
const char * (*zlibVersion_d)(void) = NULL;
uLong (*compressBound_d)(uLong sourceLen) = NULL;
int (*uncompress_d)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) = NULL;
int (*compress2_d)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level) = NULL;

void *lib_magma = NULL;

/**
 * @brief	Initialize and bind an import symbol table.
 * @see		dlsym()
 * @param	count	the number of symbols in the table.
 * @param	symbols	the symbol table to be patched.
 * @return	true on success or false on failure.
 */
bool_t lib_symbols(size_t count, symbol_t symbols[]) {

	if (!count || !lib_magma) {
		log_critical("An invalid request was made.");
		return false;
	}

	// Scans the symbols array to ensure none of the symbols/pointers have been referenced twice.
	for (size_t i = 0; i < count; i++) {
		for (size_t j = i + 1; j < count; j++) {
			if (symbols[i].pointer == symbols[j].pointer) {
				log_critical("A dynamic function pointer has been referenced twice. {name = %s / pointer = %p}", symbols[i].name, symbols[i].pointer);
				return false;
			}
			else if (!st_cmp_cs_eq(NULLER(symbols[i].name), NULLER(symbols[j].name))) {
				log_critical("A dynamically loaded symbol has been referenced twice. {name = %s / pointer = %p}", symbols[i].name, symbols[i].pointer);
				return false;
			}
		}
	}

	// Loop through and setup the function pointers.
	for (size_t i = 0; i < count; i++) {
		if ((*(symbols[i].pointer) = dlsym(lib_magma, symbols[i].name)) == NULL) {
			log_critical("Unable to establish a pointer to the function %s.", symbols[i].name);
			log_critical("%s", dlerror());
			return false;
		}
	}

	return true;
}

/**
 * @brief	Unload magmad.so from memory.
 * @return	This function returns no value.
 */
void lib_unload(void) {

	if (magma.library.unload) {
		dlclose(lib_magma);
	}

	return;
}

/**
 * @brief	Load magmad.so dynamically and resolve all external dependencies from 3rd party providers.
 * @return	false on failure or true on success.
 */
bool_t lib_load(void) {

	chr_t *lib_error = NULL;

	if (!magma.config.file) {
		log_critical("The Magma shared library path is empty.");
		return false;
	}
	else if (!st_cmp_ci_eq(NULLER(magma.library.file), CONSTANT("NULL"))) {
		lib_magma = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
	}
	else {
		lib_magma = dlopen(magma.library.file, RTLD_NOW | RTLD_GLOBAL);
	}

	if (!lib_magma || (lib_error = dlerror())) {
		if (lib_error) {
			log_critical("The dlerror() function returned: %s", lib_error);
		}
		return false;
	}

	else if (!lib_load_bzip() || !lib_load_cache() || !lib_load_clamav() || !lib_load_dkim() || !lib_load_dspam() ||
		!lib_load_freetype() || !lib_load_gd() || !lib_load_jansson() || !lib_load_jpeg() || !lib_load_lzo() || !lib_load_utf8proc() ||
		!lib_load_mysql() || !lib_load_openssl() || !lib_load_png() || !lib_load_spf() || !lib_load_tokyo() || !lib_load_xml() ||
		!lib_load_zlib()) {
		return false;
	}

	log_pedantic("-------------------------------- VERSIONS --------------------------------\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n",
			"MAGMA:", build_version(),
			"COMMIT:", build_commit(),
			"TIMESTAMP:", build_stamp(),
			"PLATFORM:", st_char_get(host_platform(MANAGEDBUF(128))),
			"KERNEL:", st_char_get(host_version(MANAGEDBUF(128))),
			"DATABASE:", serv_type_mysql(),
			"BUILD:", serv_version_mysql(),
			"SCHEMA:", serv_schema_mysql(),
			"ENCODING:", serv_charset_mysql(),
			"BZIP:", lib_version_bzip(),
			"CLAMAV:", lib_version_clamav(),
			"DKIM:", lib_version_dkim(),
			"DSPAM:", lib_version_dspam(),
			"FREETYPE", lib_version_freetype(),
			"GD", lib_version_gd(),
			"GLIBC:", gnu_get_libc_version(),
			"JANSSON:", lib_version_jansson(),
			"JPEG", lib_version_jpeg(),
			"LZO:", lib_version_lzo(),
			"MEMCACHED:", lib_version_cache(),
			"MYSQL:", lib_version_mysql(),
			"OPENSSL:", lib_version_openssl(),
			"PNG", lib_version_png(),
			"SPF:", lib_version_spf(),
			"TOKYO:", lib_version_tokyo(),
			"UTF8:", lib_version_utf8proc(),
			"XML:",	lib_version_xml(),
			"ZLIB:", lib_version_zlib());
	return true;
}
