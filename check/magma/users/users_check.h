
/**
 * @file /check/magma/users/users_check.h
 *
 * @brief Checks the code used to handle user data.
 */

#ifndef USER_CHECK_H
#define USER_CHECK_H

extern stringer_t *check_username;
extern stringer_t *check_password;

Suite * suite_check_users(void);

void check_users_auth_legacy_s(int);
void check_users_auth_stacie_s(int);
void check_users_auth_challenge_s(int);
void check_users_auth_response_s(int);
void check_users_auth_login_s(int);
void check_users_auth_username_s(int);
void check_users_auth_address_s(int);
void check_users_register_s(int);

#endif

