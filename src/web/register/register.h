
/**
 * @file /magma/web/register/register.h
 *
 * @brief	Functions for handling the registration process.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_WEB_REGISTER_H
#define MAGMA_WEB_REGISTER_H

#define REGISTER_PASSWORD_MIN_LENGTH	5
#define REGISTER_PASSWORD_MAX_LENGTH	200
#define REGISTER_USERNAME_MIN_LENGTH	1
#define REGISTER_USERNAME_MAX_LENGTH	200

// Stores information about new user registrations.
typedef struct {
	uint64_t usernum;
	uint16_t plan;
	stringer_t *username, *password, *hvf_value, *hvf_input, *name;
} register_session_t;

/// datatier.c
bool_t   register_data_check_username(stringer_t *username);
inx_t *  register_data_fetch_blocklist(void);
bool_t register_data_insert_user(
	connection_t *con,
	uint16_t plan,
	stringer_t *username,
	stringer_t *password,
	int_t transaction,
	uint64_t *outuser);

/// abuse.c
bool_t  register_abuse_check_blocklist(connection_t *con);
bool_t   register_abuse_checks(connection_t *con);
void    register_abuse_increment_history(connection_t *con);
void    register_blocklist_free(void);
void    register_blocklist_update(void);

/// captcha.c
stringer_t *  register_captcha_generate(stringer_t *value);
stringer_t *  register_captcha_random_font(void);
void          register_captcha_write_noise(gdImagePtr image, int_t x, int_t y);

/// sessions.c
bool_t                register_session_cache(connection_t *con, register_session_t *session);
void                  register_session_free(register_session_t *session);
register_session_t *  register_session_generate(void);
register_session_t *  register_session_get(connection_t *con, stringer_t *name);

/// business.c
chr_t *  register_business_step1(connection_t *con, register_session_t *reg);
chr_t *  register_business_step2(connection_t *con, register_session_t *reg);
bool_t   register_business_validate_password(stringer_t *password);
int_t    register_business_validate_username(stringer_t *username);

/// register.c
void          register_print_captcha(connection_t *con, register_session_t *reg);
void          register_print_message(connection_t *con, chr_t *message);
void          register_print_step1(connection_t *con, register_session_t *reg, chr_t *message);
void          register_print_step2(connection_t *con, register_session_t *reg, chr_t *message);
void          register_print_step3(connection_t *con);
void          register_process(connection_t *con);

#endif

