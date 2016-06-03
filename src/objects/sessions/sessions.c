
/**
 * @file /magma/objects/sessions/sessions.c
 *
 * @brief	Routines to handle web sessions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

struct {
	uint64_t number;
	pthread_mutex_t lock;
} sessions = {
	.number = 1,
	.lock = PTHREAD_MUTEX_INITIALIZER
};

/**
 * @brief	Generate a random 12 digit 64-bit web session key.
 * @return	the randomly generated web session key as an unsigned 64 bit integer.
 */
uint64_t sess_key(void) {

	uint64_t result = 0;

	for (uint64_t i = 0; i < 100 && uint64_digits(result) < 12; i++) {
		result = rand_get_uint64();
	}

#ifdef MAGMA_PEDANTIC
	if (result < uint64_digits(result)) {
		log_pedantic("The session key generator failed to produce a result of sufficient length.");
	}
#endif

	return result;
}

/**
 * @brief	Reserve a unique web session identifier.
 * @return	a number containing a unique web session identifier.
 */
uint64_t sess_number(void) {

	uint64_t result;

	mutex_get_lock(&(sessions.lock));
	result = sessions.number++;
	mutex_unlock(&(sessions.lock));

	return result;
}

/**
 * @brief	Destroy a web session and its associated data.
 * @param	sess	a pointer to the web session to be destroyed.
 * @return	This function returns no value.
 */
void sess_destroy(session_t *sess) {

	if (sess) {

		if (sess->user) {
			meta_remove(sess->user->username, META_PROT_WEB);
		}

		st_cleanup(sess->warden.token);
		st_cleanup(sess->warden.agent);

		// Release the credential.
		if (sess->warden.cred) {
			credential_free(sess->warden.cred);
			sess->warden.cred = NULL;
		}

		st_cleanup(sess->request.host);
		st_cleanup(sess->request.path);
		st_cleanup(sess->request.application);
		inx_cleanup(sess->compositions);

		mutex_destroy(&(sess->lock));
		mm_free(sess);
	}

	return;
}

/**
 * @brief	Increment the web session's reference counter and update its timestamp.
 * @param	sess	a pointer to the web session to be updated.
 * @return	This function returns no value.
 */
void sess_ref_add(session_t *sess) {

	if (sess) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(sess->lock));

		// Increment the web counter.
		sess->refs.web++;

		// Update the activity time stamp.
		sess->refs.stamp = time(NULL);

		// Release the reference counter lock.
		mutex_unlock(&(sess->lock));

	}
	return;
}

/**
 * @brief	Decrement the web session's reference counter and update its timestamp.
 * @param	sess	a pointer to the web session to be updated.
 * @return	This function returns no value.
 */
void sess_ref_dec(session_t *sess) {

	if (sess) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(sess->lock));

		// Decrement the web counter.
		sess->refs.web--;

		// Update the activity time stamp.
		sess->refs.stamp = time(NULL);

		// Release the reference counter lock.
		mutex_unlock(&(sess->lock));

	}
	return;
}

/**
 * @brief	Get the reference count of a web session.
 * @param	sess	a pointer to the web session to be queried.
 * @return	the total number of references to the specified web session.
 */
uint64_t sess_ref_total(session_t *sess) {

	uint64_t result = 0;

	if (sess) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(sess->lock));

		// Sum the total.
		result = sess->refs.web;

		// Release the reference counter lock.
		mutex_unlock(&(sess->lock));

	}

	return result;
}

/**
 * @brief	Get the timestamp for a web session's reference counter.
 * @param	sess	a pointer to the web session to be queried.
 * @return	the last-modified UTC timestamp value of the specified web session.
 */
time_t sess_ref_stamp(session_t *sess) {

	time_t result = 0;

	if (sess) {
		mutex_get_lock(&(sess->lock));
		result = sess->refs.stamp;
		mutex_unlock(&(sess->lock));
	}

	return result;
}

/**
 * @brief 	Update a web session's refresh stamp and prevent redundant refreshing of a web session.
 * @param	sess	a pointer to the web session to be touched.
 * @return	This function returns no value.
 */
void sess_refresh_flush(session_t *sess) {

	if (sess) {
		mutex_get_lock(&(sess->lock));
		sess->refresh.trigger = false;
		sess->refresh.stamp = time(NULL);
		mutex_unlock(&(sess->lock));
	}

	return;
}

/**
 * @brief	Get the timestamp for the last time the session was refreshed.
 * @param	sess	a pointer to the web session to be queried.
 * @return	the last-refreshed UTC timestamp value of the specified web session.
 */
time_t sess_refresh_stamp(session_t *sess) {

	time_t result = 0;

	if (sess) {
		mutex_get_lock(&(sess->lock));
		result = sess->refresh.stamp;
		mutex_unlock(&(sess->lock));
	}

	return result;
}

/**
 * @brief	Check to see if a web session is ready to be refreshed, and if so, disarm its trigger and update its refresh stamp.
 * @note	The caller needs to make immediate use of this function's return value because the refresh trigger will be cleared if it was previously set.
 * 			Any session longer than 2 minutes will be marked as ready for refresh.
 * @param	a pointer to the web session to be queried.
 * @return	true if the web session is ready to be refreshed, or false if it is not.
 */
bool_t sess_refresh_check(session_t *sess) {

	bool_t result = false;

	if (sess) {
		mutex_get_lock(&(sess->lock));

		if (sess->refresh.trigger || (time(NULL) - sess->refresh.stamp) > 120) {
			sess->refresh.trigger = false;
			sess->refresh.stamp = time(NULL);
			result = true;
		}

		mutex_unlock(&(sess->lock));
	}

	return result;
}
/**
 * @brief	Securely generate a unique zbase32-encoded token for a session.
 * @param	sess	a pointer to the input web session.
 * @return	a managed string containing the generated web session token.
 */
stringer_t * sess_token(session_t *sess) {

	scramble_t *encrypted = NULL;
	stringer_t *binary = NULL, *encoded = NULL;

	if (!(binary = st_merge("ssss", PLACER(&(sess->warden.host), sizeof(uint64_t)), PLACER(&(sess->warden.stamp), sizeof(uint64_t)),
		PLACER(&(sess->warden.number),	sizeof(uint64_t)), PLACER(&(sess->warden.key), sizeof(uint64_t)))) ||
		!(encrypted = scramble_encrypt(magma.secure.sessions, binary)) ||
		!(encoded = zbase32_encode(PLACER(encrypted, scramble_total_length(encrypted))))) {
		log_pedantic("An error occurred while trying to generate the session token.");
	}

	if (encrypted) {
		scramble_free(encrypted);
	}

	st_cleanup(binary);

	return encoded;
}

/**
 * @brief	Update a session's underlying data and its refresh timestamp.
 * @note	This function ensures the session's associated meta user data, mail folders and messages, and contact folders and contact entries are all current.
 * @param	sess	a pointer to the session object to be updated.
 * @return	This function returns no value.
 */
void sess_update(session_t *sess) {

	if (sess) {
		// Flush the update trackers to discourage unnecessary refreshes.
		sess_refresh_flush(sess);

		meta_user_update(sess->user, META_NEED_LOCK);
		meta_folders_update(sess->user, META_NEED_LOCK);
		meta_messages_update(sess->user, META_NEED_LOCK);
		meta_contacts_update(sess->user, META_NEED_LOCK);
	}

	return;
}

/**
 * @brief	Set a web session's trigger so it will be refreshed as soon as possible.
 * @param	sess	a pointer to the web session to be refreshed.
 * @return	This function returns no value.
 */
void sess_trigger(session_t *sess) {

	if (sess) {
		mutex_get_lock(&(sess->lock));
		sess->refresh.trigger = true;
		mutex_unlock(&(sess->lock));
	}

	return;
}

/**
 * @brief	Release a web session.
 * @note	If the web session should have been refreshed, it will be refreshed before the reference counter is decremented.
 * @param	sess	a pointer to the web session to be released.
 * @return	This function returns no value.
 */
void sess_release(session_t *sess) {

	// If its been more than two minutes since we last checked to see if things are up to date, queue a refresh ahead of the decrement.
	if (sess_refresh_check(sess)) {
		requeue(&sess_update, &sess_ref_dec, sess);
	}
	else {
		sess_ref_dec(sess);
	}

	return;
}

/**
 * @brief	Free an attachment object.
 * @note	This is an inx helper function.
 * @param	attachment	a pointer to the attachment object to be destroyed.
 * @return	This function returns no value.
 */
void sess_release_attachment(attachment_t *attachment) {

	if (attachment) {
		st_cleanup(attachment->filename);
		st_cleanup(attachment->filedata);
		mm_free(attachment);
	}

}

/**
 * @brief	Free a composition object.
 * @note	This is an inx helper function.
 * @param	comp	a pointer to the composition object to be destroyed.
 * @return	This function returns no value.
 */
void sess_release_composition(composition_t *comp) {

	if (comp) {
		inx_cleanup(comp->attachments);
		mm_free(comp);
	}

	return;
}

/**
 * @brief	Queue a web session refresh if there is stale data.
 * @param	sess	a pointer to the web session to be queried.
 * @param	object	the value of the object in the cache to be checked (can include OBJECT_CONTACTS and OBJECT_FOLDERS).
 */
void sess_serial_check(session_t *sess, uint64_t object) {

	if (sess && meta_user_serial_check(sess->user, object)) {
		sess_trigger(sess);
	}

	return;
}

/**
 * @brief	Create a new session for a given web connection.
 * @note	The session stores the following data points: remote IP address, request path, application name, the specified http hostname,
 * 			the remote client's user agent string, the server's host number, a unique session id, the server's current timestamp, a randomly-
 * 			generated session key for authentication, and an encrypted token for the session returned to the user as a cookie.
 * @param	con			a pointer to the connection underlying the web session.
 * @param	path		a pointer to a managed string containing the pathname of the generating request (should be "/portal/camel").
 * @param	application	a pointer to a managed string containing the name of the parent application of the session (should be "portal").
 * @return	NULL on failure or a pointer to a newly allocated session object for the specified connection.
 */
session_t *sess_create(connection_t *con, stringer_t *path, stringer_t *application) {

	session_t *output;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!(output = mm_alloc(sizeof(session_t)))) {
		log_pedantic("Unable to allocate %zu bytes for a session context.", sizeof(session_t));
		return NULL;
	}
	else if (pthread_mutex_init(&(output->lock), NULL) != 0) {
		log_pedantic("Unable to initialize reference lock for new user session.");
		mm_free(output);
		return NULL;
	} else if (!(output->compositions = inx_alloc(M_INX_LINKED, &sess_release_composition))) {
		log_pedantic("Unable to allocate space for user session's compositions.");
		mm_free(output);
		return NULL;
	}

	if (!(ip_copy(&(output->warden.ip), con_addr(con, MEMORYBUF(64)))) ||
		(path && !(output->request.path = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, path))) ||
		(application && !(output->request.application = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, application))) ||
		(con->http.host && !(output->request.host = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, con->http.host))) ||
		(con->http.agent && !(output->warden.agent = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, con->http.agent))) ||
		!(output->warden.host = magma.host.number) || !(key.val.u64 = output->warden.number = sess_number()) ||
		!(output->warden.stamp = time(NULL)) || !(output->warden.key = sess_key()) || !(output->warden.token = sess_token(output))) {
		log_pedantic("Unable to initialize the session warden context.");
		sess_destroy(output);
		return NULL;
	}

	output->request.httponly = true;
	output->request.secure = (con_secure(con) == 1 ? true : false);

	sess_ref_add(output);

	if (inx_insert(objects.sessions, key, output) != 1) {
		log_pedantic("Unable to insert the session into the global context.");
		sess_ref_dec(output);
		sess_destroy(output);
		return NULL;
	}

	return output;
}

/**
 * @brief	Try to retrieve the session associated with a client connection's supplied cookie.
 * @param	con				a pointer to the connection object sending the cookie.
 * @param	application		a managed string containing the application associated with the session.
 * @param	path			a managed string containing the path associated with the session.
 * @param	token			the encrypted user token retrieved from the supplied http cookie.
 * @return	1 if the cookie was found and valid, or one of the following values on failure:
 * 			 0 = Session not found.
 *			-1 = Server error.
 *			-2 = Invalid token.
 *			-3 = Security violation / incorrect user-agent.
 *			-4 = Security violation / incorrect session key.
 *			-5 = Security violation / incorrect source address.
 *			-6 = Session terminated by logout.
 *			-7 = Session timed out.
 */
int_t sess_get(connection_t *con, stringer_t *application, stringer_t *path, stringer_t *token) {

	uint64_t *numbers;
	scramble_t *scramble;
	stringer_t *binary, *encrypted;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	int_t result = 1;

	/// Most session attributes need simple equality comparison, except for timeout checking. Make sure not to validate against a stale session that should have already timed out (which will have to be determined dynamically).
	encrypted = zbase32_decode(token);
	scramble = scramble_import(encrypted);
	binary = scramble_decrypt(magma.secure.sessions, scramble);

	st_cleanup(encrypted);

	if (!binary) {
		return 0;
	}

	numbers = st_data_get(binary);

	// QUESTION: Is this necessary? doesn't inx_find() lock the inx?
	inx_lock_read(objects.sessions);

	key.val.u64 = *(numbers + 2);

	if ((con->http.session = inx_find(objects.sessions, key))) {
		sess_ref_add(con->http.session);
	}

	inx_unlock(objects.sessions);
	st_free(binary);

	// Return if we didn't find the session or user.
	if (!con->http.session || !con->http.session->user) {
		return 0;
	}

	// We need to do full validation against the cookie and associated session.
	// First, the cookie.
	if ((*numbers != con->http.session->warden.host) || (*(numbers + 1) != con->http.session->warden.stamp) ||
			(*(numbers + 2) != con->http.session->warden.number)) {
		log_error("Received mismatched cookie for authenticated session { user = %s }", st_char_get(con->http.session->user->username));
		result = -2;
	} else if (*(numbers + 3) != con->http.session->warden.key) {
		log_error("Cookie contained an incorrect session key { user = %s }", st_char_get(con->http.session->user->username));
		result = -4;
	} else if (st_cmp_cs_eq(application, con->http.session->request.application)) {
		log_error("Cookie did not match session's application { user = %s }", st_char_get(con->http.session->user->username));
		result = -2;
	} else if (st_cmp_cs_eq(path, con->http.session->request.path)) {
		log_error("Cookie did not match session's path { user = %s }", st_char_get(con->http.session->user->username));
		result = -2;
	} else if (st_cmp_cs_eq(con->http.agent, con->http.session->warden.agent)) {
		log_error("Cookie contained a mismatched user agent { user = %s }", st_char_get(con->http.session->user->username));
		result = -3;
	} else if (con->http.session->request.secure != (con_secure(con) ? 1 : 0)) {
		log_error("Cookie was submitted from a mismatched transport layer { user = %s }", st_char_get(con->http.session->user->username));
		result = -5;
	} else if (!ip_address_equal(&(con->http.session->warden.ip), (ip_t *)con_addr(con, MEMORYBUF(64)))) {
		log_error("Cookie was submitted from a mismatched IP address { user = %s }", st_char_get(con->http.session->user->username));
		result = -5;
	}

	// Finally, do comparisons to see that we haven't timed out.
	/* Did we expire? */
	if (magma.http.session_timeout <= (time(NULL) - con->http.session->warden.stamp)) {
		log_pedantic("User submitted expired or invalidated cookie; marking for deletion { user = %s }", st_char_get(con->http.session->user->username));
		result = -7;
	}

	// QUESTION: This destruction needs a second look.
	if (result < 0) {

		if (!inx_delete(objects.sessions, key)) {
			log_pedantic("Unexpected error occurred attempting to delete expired cookie { user = %s }", st_char_get(con->http.session->user->username));
		}

		sess_ref_dec(con->http.session);
		//sess_destroy(con->http.session);
		con->http.session = NULL;
	}
	// Otherwise, if the last session status update is more than 10 minutes ago, check now to see if things are current.
	// QUESTION: Why is it 600 here and 120 elsewhere?
	else if ((time(NULL) - sess_refresh_stamp(con->http.session)) > 600) {
		sess_update(con->http.session);
	}

	return result;
}
