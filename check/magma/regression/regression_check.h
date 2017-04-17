/**
 * @file /check/magma/regression/regression_check.h
 *
 * @brief Regression unit tests
 */

#ifndef REGRESSION_CHECK_H
#define REGRESSION_CHECK_H

/// regression_check_helpers.c
void check_regression_file_descriptors_leak_test(void);
bool_t check_client_dot_stuff(client_t *client, chr_t *token);
bool_t check_regression_smtp_dot_stuffing_sthread(stringer_t *errmsg);
bool_t check_regression_imap_search_range_sthread(stringer_t *errmsg);

Suite * suite_check_regression(void);

#endif
