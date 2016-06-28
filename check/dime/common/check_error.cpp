#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include <openssl/err.h>
#include <openssl/rand.h>

extern "C" {
#include "dime/common/error.h"
}
#include "gtest/gtest.h"

#define ERRMSG "error:02001002:system library:fopen:No such file or directory:filename:100"
#define SEP    " | "

TEST(DIME, test_openssl_error_1)
{
    ERR_clear_error();
    ERR_put_error(ERR_LIB_SYS, SYS_F_FOPEN, ERR_R_SYS_LIB, "filename", 100);

    PUSH_ERROR_OPENSSL();

    const errinfo_t *last_error = get_last_error();
    ASSERT_TRUE(NULL != last_error);
    ASSERT_EQ(ERR_OPENSSL, last_error->errcode);
    ASSERT_STREQ(ERRMSG, last_error->auxmsg);
}

TEST(DIME, test_openssl_error_5)
{
    ERR_clear_error();
    for (int i = 0; i < 5; i++) {
        ERR_put_error(ERR_LIB_SYS, SYS_F_FOPEN, ERR_R_SYS_LIB, "filename", 100);
    }

    PUSH_ERROR_OPENSSL();

    const errinfo_t *last_error = get_last_error();
    ASSERT_TRUE(NULL != last_error);
    ASSERT_EQ(ERR_OPENSSL, last_error->errcode);
    ASSERT_STREQ(ERRMSG SEP ERRMSG SEP ERRMSG SEP ERRMSG SEP ERRMSG, last_error->auxmsg);
}

TEST(DIME, test_openssl_error_100)
{
    ERR_clear_error();
    for (int i = 0; i < 100; i++) {
        ERR_put_error(ERR_LIB_SYS, SYS_F_FOPEN, ERR_R_SYS_LIB, "filename", 100 + i);
    }

    PUSH_ERROR_OPENSSL();

    const errinfo_t *last_error = get_last_error();
    ASSERT_TRUE(NULL != last_error);
    ASSERT_EQ(ERR_OPENSSL, last_error->errcode);
    ASSERT_EQ(sizeof(last_error->auxmsg) - 1, strlen(last_error->auxmsg));
}

/**
 * In an old version of the code that used strncat, this caused a SIGSEGV.
 */
TEST(DIME, test_openssl_error_longfilename)
{
    ERR_clear_error();
#define fn10   "1234567890"
#define fn50   fn10 fn10 fn10 fn10 fn10
#define fn250  fn50 fn50 fn50 fn50 fn50
#define fn1000 fn250 fn250 fn250 fn250
    ERR_put_error(ERR_LIB_SYS, SYS_F_FOPEN, ERR_R_SYS_LIB, fn1000, 1);
    ERR_put_error(ERR_LIB_SYS, SYS_F_FOPEN, ERR_R_SYS_LIB, fn1000, 1);

    PUSH_ERROR_OPENSSL();

    const errinfo_t *last_error = get_last_error();
    ASSERT_TRUE(NULL != last_error);
    ASSERT_EQ(ERR_OPENSSL, last_error->errcode);
    ASSERT_EQ(sizeof(last_error->auxmsg) - 1, strlen(last_error->auxmsg));
}
