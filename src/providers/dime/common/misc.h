#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include "dime/common/error.h"

#define ANSI_COLOR_RED   "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ALERT_PRINT(fp, x) { fprintf(fp, ANSI_COLOR_RED); x; fprintf(fp, ANSI_COLOR_RESET); fflush(fp); }

#define SHA_160_SIZE 20
#define SHA_256_SIZE 32
#define SHA_512_SIZE 64

#define SHA_512_B64_SIZE 86

#define B64_ENCODED_LEN(len) ((((len) + (((len) % 3) ? (3 - ((len) % 3)) : 0)) / 3) * 4)
#define B64_DECODED_LEN(len) ((len) * 3 / 4)

#define chr_isspace(c) isspace((unsigned char)(c))
#define chr_isprint(c) isprint((unsigned char)(c))

typedef struct {
    void *data;
    size_t len;
} sha_databuf_t;


extern int _verbose;


// Debugging level operations.
PUBLIC_FUNC_DECL(void,            set_dbg_level,             unsigned int level);
PUBLIC_FUNC_DECL(unsigned int,    get_dbg_level,             void);

// Network order encoding operations.
PUBLIC_FUNC_DECL(uint32_t,        int_no_get_4b,             const void *buf);
PUBLIC_FUNC_DECL(uint32_t,        int_no_get_3b,             const void *buf);
PUBLIC_FUNC_DECL(uint16_t,        int_no_get_2b,             const void *buf);
PUBLIC_FUNC_DECL(void,            int_no_put_4b,             void *buf, uint32_t val);
PUBLIC_FUNC_DECL(void,            int_no_put_3b,             void *buf, uint32_t val);
PUBLIC_FUNC_DECL(void,            int_no_put_2b,             void *buf, uint16_t val);

// Managing dynamically re-sized data buffers.
PUBLIC_FUNC_DECL_VA(int,          str_printf,                char **sbuf, const char *fmt);
PUBLIC_FUNC_DECL(size_t,          mem_append,                unsigned char **buf, size_t *blen, const unsigned char *data, size_t dlen);

// Pointer chain operations.
PUBLIC_FUNC_DECL(void *,          ptr_chain_add,             void *buf, const void *addr);
PUBLIC_FUNC_DECL(void,            ptr_chain_free,            void *buf);
PUBLIC_FUNC_DECL(int,             count_ptr_chain,           void *buf);
PUBLIC_FUNC_DECL(void *,          ptr_chain_clone,           void *buf);

// Miscellaneous/no category.
PUBLIC_FUNC_DECL(char *,          get_chr_date,              time_t time, int local);
PUBLIC_FUNC_DECL(int,             is_buf_zeroed,             void *buf, size_t len);
PUBLIC_FUNC_DECL(void,            secure_wipe,               void *buf, size_t len);

// Character encoding functions.
PUBLIC_FUNC_DECL(unsigned char *, b64decode,                 const char *buf, size_t len, size_t *outlen);
PUBLIC_FUNC_DECL(unsigned char *, b64decode_nopad,           const char *buf, size_t len, size_t *outlen);
PUBLIC_FUNC_DECL(char *,          b64encode,                 const unsigned char *buf, size_t len);
PUBLIC_FUNC_DECL(char *,          b64encode_nopad,           const unsigned char *buf, size_t len);
PUBLIC_FUNC_DECL(char *,          hex_encode,                const unsigned char *buf, size_t len);

// Functions for dumping data and printing debugging messages.
PUBLIC_FUNC_DECL(void,            dump_buf,                  const unsigned char *buf, size_t len, int all_hex);
PUBLIC_FUNC_DECL(void,            dump_buf_outer,            const unsigned char *buf, size_t len, size_t nouter, int all_hex);
// Special declaration for variable argument function.
PUBLIC_FUNC_DECL_VA(void,         dbgprint,                  unsigned int dbglevel, const char *fmt);

// File I/O helpers.
PUBLIC_FUNC_DECL(int,             write_pem_data,           const char *b64_data, const char *checksum, const char *tag, const char *filename);
PUBLIC_FUNC_DECL(unsigned char *, read_file_data,           const char *filename, size_t *fsize);
PUBLIC_FUNC_DECL(char *,          read_pem_data,             const char *pemfile, const char *tag, int nospace);

// Various cryptographic operations.
PUBLIC_FUNC_DECL(int,             compute_sha_hash,          size_t nbits, const unsigned char *buf, size_t blen, unsigned char *outbuf);
PUBLIC_FUNC_DECL(int,             compute_sha_hash_multibuf, size_t nbits, sha_databuf_t *bufs, unsigned char *outbuf);
PUBLIC_FUNC_DECL(RSA *,           decode_rsa_pubkey,         unsigned char *data, size_t dlen);
PUBLIC_FUNC_DECL(unsigned char *, encode_rsa_pubkey,         RSA *pubkey, size_t *enclen);
PUBLIC_FUNC_DECL(int,             get_x509_cert_sha_hash,    X509 *cert, size_t nbits, unsigned char *out);

// Checksum function.
PUBLIC_FUNC_DECL(int,             _compute_crc24_checksum,   void *buffer, size_t length);

#endif
