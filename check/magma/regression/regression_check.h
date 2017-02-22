/**
 * @file /check/magma/regression/regression_check.h
 *
 * @brief Regression unit tests
 */

#ifndef REGRESSION_CHECK_H
#define REGRESSION_CHECK_H

/// regression_check.c
bool_t check_regression_file_descriptors_leak_mthread(void);
Suite * suite_check_regression(void);

#endif
