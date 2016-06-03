
/**
 * @file /magma/web/register/sessions.c
 *
 * @brief	Functions for handling the internal session structure used by new user registration attemptps.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Destroy a registration session and all its associated data.
 * @return	This function returns no value.
 */
void register_session_free(register_session_t *session) {

	if (!session) {
		return;
	}

	// This structure is allocated with mm_alloc() which makes sure that everything is initially zero'ed out.
	st_cleanup(session->username);
	st_cleanup(session->password);
	st_cleanup(session->hvf_value);
	st_cleanup(session->hvf_input);
	st_cleanup(session->name);
	mm_free(session);

	return;
}

/**
 * @brief	Generate a new registration session.
 * @return	NULL on failure, or a pointer to a new randomly named session on success.
 */
register_session_t * register_session_generate(void) {

	register_session_t *session;

	if (!(session = mm_alloc(sizeof(register_session_t)))) {
		log_pedantic("Unable to allocate new registration session.");
		return NULL;
	}

	if (!(session->name = rand_choices("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 32))) {
		register_session_free(session);
		return NULL;
	}

	return session;
}

/**
 * @brief	Retrieve a registration session by a data key supplied in the user's http POST request.
 * @note	The session is stored under the parent key "lavad.register.session."
 * @param	con		the client connection underlying the user request.
 * @param	name	a managed string containing the registration session identifier.
 * @return	NULL on failure, or a pointer to the user's registration session on success.
 */
register_session_t * register_session_get(connection_t *con, stringer_t *name) {

	chr_t key[256];
	size_t keylen;
	stringer_t *data;
	serialization_t serial;
	register_session_t *session;

	// Build the key.
	keylen = snprintf(key, 256, "lavad.register.session.%s.%.*s", st_char_get(con_addr_presentation(con, MANAGEDBUF(64))), st_length_int(name), st_char_get(name));

	// Pull it. If no session is found, generate it.
	if (!(data = cache_get(NULLER(key)))) {
		return NULL;
	}

	// Setup the deserialization structure.
	mm_wipe(&serial, sizeof(serialization_t));
	serial.data = data;

	// Deserialize the session.
	if (!(session = mm_alloc(sizeof(register_session_t))) || !deserialize_uint16(&serial, &(session->plan)) ||
		!deserialize_st(&serial, &(session->username)) || !deserialize_st(&serial, &(session->password)) ||
		!deserialize_st(&serial, &(session->hvf_value)) || !deserialize_st(&serial, &(session->hvf_input)) ||
		!deserialize_uint64(&serial, &(session->usernum)) || !(session->name = st_dupe(name))) {
		log_pedantic("Unable to deserialize a registration session. {key = %s}", key);

		if (!session) {
			register_session_free(session);
		}

		st_free(data);
		return NULL;
	}

	st_free(data);

	return session;
}

/**
 * @brief	Save a registration session's state into the cache.
 * @note	The session is stored under the parent key "lavad.register.session."
 * @param	con			a pointer to the client connection underlying the user session.
 * @param	session		a pointer to the registration session to be persisted.
 * @return	false on failure or true if the session was cached successfully.
 */
bool_t register_session_cache(connection_t *con, register_session_t *session) {

	char key[256];
	size_t keylen;
	stringer_t *data = NULL;

	// Serialize the session.
	if (!serialize_uint16(&data, session->plan) || !serialize_st(&data, session->username) || !serialize_st(&data, session->password) ||
		!serialize_st(&data, session->hvf_value) || !serialize_st(&data, session->hvf_input) ||
		!serialize_uint64(&data, session->usernum) || !data) {
		log_pedantic("Unable to serialize a registration session.");
		st_cleanup(data);
		return false;
	}

	// Build the key.
	keylen = snprintf(key, 256, "lavad.register.session.%s.%.*s", st_char_get(con_addr_presentation(con, MANAGEDBUF(64))), st_length_int(session->name), st_char_get(session->name));

	// Store it.
	if (cache_set(NULLER(key), data, 3600) != 1) {
		log_pedantic("Unable to cache registration session.");
		st_free(data);
		return false;
	}

	st_free(data);

	return true;
}
