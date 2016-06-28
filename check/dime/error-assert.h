#ifndef GTEST_HELPERS_H
#define GTEST_HELPERS_H

#include "gtest/gtest.h"
#include "dime/common/error.h"

#define ASSERT_DIME_NO_ERROR() \
    assert_dime_no_error_impl(__FILE__, __LINE__)
static void assert_dime_no_error_impl(char const *filename, int lineno) {
    if (get_last_error() != NULL) {
        dump_error_stack();
        ASSERT_TRUE(false) << filename << lineno << "Unexpected DIME error";
    }
}

#endif
