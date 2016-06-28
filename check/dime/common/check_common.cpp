#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include <openssl/rand.h>

extern "C" {
#include "dime/common/error.h"
}
#include "gtest/gtest.h"

#ifdef ERROR_API_FINISHED
/** @brief Compute positive sum of two non-negative ints or raise errinfo error.
 * @param a first addend
 * @param b second addend
 * @return the sum if both addends and the sum are positive; else return -1 and
 * raise errinfo error.  */
int
calc_nonnegative_sum(int a, int b)
{
    /* cleanup error stack if needed */
    BEGIN_PUBLIC_FUNC
    if (a < 0)
        RET_INT_ERROR_EX(CMN_ERRCODE_BADARG, "`a' is negative");
    if (b < 0)
        RET_INT_ERROR_EX(CMN_ERRCODE_BADARG, "'b' is negative");
    const int res = a + b;
    if (res < 0)
        RET_INT_ERROR(CMN_ERRCODE_INTOVERFLOW);
    return res;
}

START_TEST(errinfo_test)
{
    /* static checks of error codes and texts */
    ASSERT_NE(0, CMN_ERRCODE_BADARG);
    ASSERT_NE(0, CMN_ERRCODE_INTOVERFLOW);
    ASSERT_STREQ("no error", get_error_string(0));
    ASSERT_STREQ("invalid argument", get_error_string(CMN_ERRCODE_BADARG));
    ASSERT_STREQ("int overflow", get_error_string(CMN_ERRCODE_INTOVERFLOW));

    /* "no error" check */
    ASSERT_EQ(3, calc_nonnegative_sum(1, 2));
    ASSERT_EQ(0, get_last_error_code());
    ASSERT_EQ(0, get_first_error());
    ASSERT_EQ(0, pop_last_error());

    /* "bad argument" error checks */
    ASSERT_EQ(-1, calc_nonnegative_sum(-1, 2));
    ASSERT_EQ(CMN_ERRCODE_BADARG, get_last_error_code());
    ASSERT_NE(0, get_first_error());
    ASSERT_EQ(CMN_ERRCODE_BADARG, get_error_code(get_first_error()));
    ASSERT_STREQ("`a' is negative", get_error_msg(get_first_error()));

    ASSERT_EQ(-1, calc_nonnegative_sum(-1, -2));
    ASSERT_EQ(CMN_ERRCODE_BADARG, get_last_error_code());
    ASSERT_NE(0, get_first_error());
    ASSERT_EQ(CMN_ERRCODE_BADARG, get_error_code(get_first_error()));
    ASSERT_STREQ("`a' is negative", get_error_msg(get_first_error()));

    ASSERT_EQ(-1, calc_nonnegative_sum(1, -2));
    ASSERT_EQ(CMN_ERRCODE_BADARG, get_last_error_code());
    ASSERT_NE(0, get_first_error());
    ASSERT_EQ(CMN_ERRCODE_BADARG, get_error_code(get_first_error()));
    ASSERT_STREQ("`b' is negative", get_error_msg(get_first_error()));

    /* int overflow checks */
    ASSERT_EQ(-1, calc_nonnegative_sum(INT_MAX, INT_MAX));
    ASSERT_EQ(CMN_ERRCODE_INTOVERFLOW, get_last_error_code());
    ASSERT_NE(0, get_first_error());
    ASSERT_EQ(CMN_ERRCODE_INTOVERFLOW, get_error_code(get_first_error()));
    ASSERT_EQ(NULL, get_error_msg(get_first_error()));
}
#endif /* ERROR_API_FINISHED */
