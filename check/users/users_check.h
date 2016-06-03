
/**
 * @file /check/users/users_check.h
 *
 * @brief Checks the code used to handle user data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef USER_CHECK_H
#define USER_CHECK_H

Suite * suite_check_users(void);

void check_users_auth_legacy_s(int);
void check_users_auth_stacie_s(int);
void check_users_auth_challenge_s(int);
void check_users_auth_login_s(int);

#endif

