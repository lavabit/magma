
/**
 * @file /check/magma/mail/mail_check.h
 */

#ifndef MAIL_CHECK_H
#define MAIL_CHECK_H

/// store_check.c
bool_t   check_mail_store_encrypted_sthread(stringer_t *errmsg);
bool_t   check_mail_store_plaintext_sthread(stringer_t *errmsg);

/// load_check.c
bool_t   check_mail_load_sthread(stringer_t *errmsg);

/// mail_check.c
Suite *  suite_check_mail(void);

#endif

