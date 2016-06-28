#include "dime/common/misc.h"
#include "dime/common/error.h"

void int_no_put_4b(void *buf, uint32_t val) {
    PUBLIC_FUNC_IMPL_VOID(int_no_put_4b, buf, val);
}

void int_no_put_3b(void *buf, uint32_t val) {
    PUBLIC_FUNC_IMPL_VOID(int_no_put_3b, buf, val);
}

void int_no_put_2b(void *buf, uint16_t val) {
    PUBLIC_FUNC_IMPL_VOID(int_no_put_2b, buf, val);
}

uint32_t int_no_get_4b(const void *buf) {
    PUBLIC_FUNC_IMPL(int_no_get_4b, buf);
}

uint32_t int_no_get_3b(const void *buf) {
    PUBLIC_FUNC_IMPL(int_no_get_3b, buf);
}

uint16_t int_no_get_2b(const void *buf) {
    PUBLIC_FUNC_IMPL(int_no_get_2b, buf);
}

char *b64encode(const unsigned char *buf, size_t len) {
    PUBLIC_FUNC_IMPL(b64encode, buf, len);
}
char *b64encode_nopad(const unsigned char *buf, size_t len) {
    PUBLIC_FUNC_IMPL(b64encode_nopad, buf, len);
}

unsigned char *b64decode(const char *buf, size_t len, size_t *outlen) {
    PUBLIC_FUNC_IMPL(b64decode, buf, len, outlen);
}

unsigned char *b64decode_nopad(const char *buf, size_t len, size_t *outlen) {
    PUBLIC_FUNC_IMPL(b64decode_nopad, buf, len, outlen);
}

char *hex_encode(const unsigned char *buf, size_t len) {
    PUBLIC_FUNC_IMPL(hex_encode, buf, len);
}

void set_dbg_level(unsigned int level) {
    PUBLIC_FUNC_IMPL_VOID(set_dbg_level, level);
}

unsigned int get_dbg_level(void) {
    PUBLIC_FUNC_IMPL(get_dbg_level, );
}

int str_printf(char **sbuf, const char *fmt, ...) {
    PUBLIC_FUNC_IMPL_VA2_RET(int, str_printf, sbuf, fmt);
}

size_t mem_append(unsigned char **buf, size_t *blen, const unsigned char *data, size_t dlen) {
    PUBLIC_FUNC_IMPL(mem_append, buf, blen, data, dlen);
}

void ptr_chain_free(void *buf) {
    PUBLIC_FUNC_IMPL_VOID(ptr_chain_free, buf);
}

void *ptr_chain_add(void *buf, const void *addr) {
    PUBLIC_FUNC_IMPL(ptr_chain_add, buf, addr);
}

int count_ptr_chain(void *buf) {
    PUBLIC_FUNC_IMPL(count_ptr_chain, buf);
}

void *ptr_chain_clone(void *buf) {
    PUBLIC_FUNC_IMPL(ptr_chain_clone, buf);
}

char *get_chr_date(time_t time, int local) {
    PUBLIC_FUNC_IMPL(get_chr_date, time, local);
}

void dump_buf(const unsigned char *buf, size_t len, int all_hex) {
    PUBLIC_FUNC_IMPL_VOID(dump_buf, buf, len, all_hex);
}

void dump_buf_outer(const unsigned char *buf, size_t len, size_t nouter, int all_hex) {
    PUBLIC_FUNC_IMPL_VOID(dump_buf_outer, buf, len, nouter, all_hex);
}

void dbgprint(unsigned int dbglevel, const char *fmt, ...) {
    PUBLIC_FUNC_IMPL_VA2(dbgprint, dbglevel, fmt);
}

int compute_sha_hash(size_t nbits, const unsigned char *buf, size_t blen, unsigned char *outbuf) {
    PUBLIC_FUNC_IMPL(compute_sha_hash, nbits, buf, blen, outbuf);
}

int compute_sha_hash_multibuf(size_t nbits, sha_databuf_t *bufs, unsigned char *outbuf) {
    PUBLIC_FUNC_IMPL(compute_sha_hash_multibuf, nbits, bufs, outbuf);
}

RSA *decode_rsa_pubkey(unsigned char *data, size_t dlen) {
    PUBLIC_FUNC_IMPL(decode_rsa_pubkey, data, dlen);
}

unsigned char *encode_rsa_pubkey(RSA *pubkey, size_t *enclen) {
    PUBLIC_FUNC_IMPL(encode_rsa_pubkey, pubkey, enclen);
}

int get_x509_cert_sha_hash(X509 *cert, size_t nbits, unsigned char *out) {
    PUBLIC_FUNC_IMPL(get_x509_cert_sha_hash, cert, nbits, out);
}

int is_buf_zeroed(void *buf, size_t len) {
    PUBLIC_FUNC_IMPL(is_buf_zeroed, buf, len);
}

unsigned char *read_file_data(const char *filename, size_t *fsize) {
    PUBLIC_FUNC_IMPL(read_file_data, filename, fsize);
}

char *read_pem_data(const char *pemfile, const char *tag, int nospace) {
    PUBLIC_FUNC_IMPL(read_pem_data, pemfile, tag, nospace);
}

void secure_wipe(void *buf, size_t len) {
    PUBLIC_FUNC_IMPL_VOID(secure_wipe, buf, len);
}
