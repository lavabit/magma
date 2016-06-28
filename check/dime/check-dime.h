/* DIME-specific extensions to the check library */

#ifndef CHECK_DIME_H
#define CHECK_DIME_H

#include <common/error.h>
#include "check-compat.h"

static inline void suite_add_test(Suite *s, const char *name, TFun func) {

    TCase *tc = tcase_create(name);
    tcase_add_test(tc, func);
    suite_add_tcase(s, tc);
}
#define suite_add_testfunc(s, func) \
    suite_add_test(s, #func, func)

#define ASSERT_DIME_NO_ERROR() \
    ck_assert_dime_noerror_impl(__FILE__, __LINE__)
static inline void ck_assert_dime_noerror_impl(const char *filename, int lineno) {
    if (get_last_error() != NULL) {
        dump_error_stack();
        ck_assert_msg(0, filename, lineno, "get_last_error() == NULL", "Unexpected error", (const char *)0);
    }
}

#endif
