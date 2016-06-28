#include <stdio.h>

extern "C" {
#include "dime/common/misc.h"
}
#include "gtest/gtest.h"

/* 24 void set_dbg_level(unsigned int level);
 25 unsigned int get_dbg_level(void);
 26 int str_printf(char **sbuf, char *fmt, ...);
 27 size_t mem_append(unsigned char **buf, size_t *blen, const unsigned char *data, size_t dlen);
 28 void ptr_chain_free(void *buf);
 29 void * ptr_chain_add(void *buf, const void *addr);
 30 char *get_chr_date(time_t time, int local);
 31 unsigned char * base64_decode(const char *buf, size_t len, size_t *outlen);
 32 char * base64_encode(const unsigned char *buf, size_t len);
 33 void dump_buf(const unsigned char *buf, size_t len, int all_hex);
 34 void dump_buf_outer(const unsigned char *buf, size_t len, size_t nouter, int all_hex);
 35 void dbgprint(unsigned int dbglevel, const char *fmt, ...);
 36 int compute_sha_hash(size_t nbits, const unsigned char *buf, size_t blen, unsigned char *outbuf);
 37 RSA * decode_rsa_pubkey(unsigned char *data, size_t dlen);
 38 unsigned char *encode_rsa_pubkey(RSA *pubkey, size_t *enclen);
 39 int get_x509_cert_sha_hash(X509 *cert, size_t nbits, unsigned char *out); */


TEST(DIME, check_debug_level)
{
    for (unsigned int i = 0; i < 100; i++) {
        set_dbg_level(i);
        ASSERT_EQ(i, get_dbg_level());
    }
}

TEST(DIME, check_base64_macros)
{
    ASSERT_EQ(0, B64_ENCODED_LEN(0));
    ASSERT_EQ(4, B64_ENCODED_LEN(1));
    ASSERT_EQ(4, B64_ENCODED_LEN(2));
    ASSERT_EQ(4, B64_ENCODED_LEN(3));
    ASSERT_EQ(40, B64_ENCODED_LEN(30));
    ASSERT_EQ(44, B64_ENCODED_LEN(31));
    ASSERT_EQ(40, B64_ENCODED_LEN(10 + 10 + 10));

    ASSERT_EQ(0, B64_DECODED_LEN(0));
    ASSERT_EQ(1, B64_DECODED_LEN(2));
    ASSERT_EQ(2, B64_DECODED_LEN(3));
    ASSERT_EQ(3, B64_DECODED_LEN(4));
    ASSERT_EQ(57, B64_DECODED_LEN(76));
    ASSERT_EQ(30 + 30 + 30, B64_DECODED_LEN(40 + 40 + 40));
}
