
/**
 * @file /check/magma/users/users_check.h
 *
 * @brief Checks the code used to handle user data.
 */

#ifndef USER_CHECK_H
#define USER_CHECK_H

Suite * suite_check_users(void);

void check_users_auth_legacy_s(int);
void check_users_auth_stacie_s(int);
void check_users_auth_challenge_s(int);
void check_users_auth_response_s(int);
void check_users_auth_login_s(int);
void check_users_auth_locked_s(int);
void check_users_auth_username_s(int);
void check_users_auth_address_s(int);
void check_users_auth_inactivity_s(int);

#endif

