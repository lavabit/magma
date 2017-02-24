
/**
 * @file /check/magma/smtp/smtp_check.h
 *
 * @brief SMTP interface test functions.
 */

#ifndef SMTP_CHECK_H
#define SMTP_CHECK_H

// checkers_check.c
bool_t check_smtp_checkers_greylist_sthread(stringer_t*);

Suite * suite_check_smtp(void);

#endif

