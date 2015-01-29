
/**
 * @file /magma/web/teacher/datatier.c
 *
 * @brief	Allow users to train their statistical mail filter.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free a spam signature object.
 * @param	teach	the spam signature object to be freed.
 * @return	This function returns no value.
 */
void teacher_data_free(teacher_data_t *teach) {

	if (teach) {
		st_cleanup(teach->signature);
		st_cleanup(teach->username);
		st_cleanup(teach->password);
		mm_free(teach);
	}

	return;
}

/// HIGH: After training a signature, we should search the messages table for references to the signature being trained and update the message status flags.
/// The UPDATE_SIGNATURE_FLAGS_ADD/UPDATE_SIGNATURE_FLAGS_REMOVE queries were created for that purpose but aren't being used right now.
/// Note to self: retroactively brand messages as junk accordingly.

/**
 * @brief	Delete spam signature information from the database.
 * @note	If the signature matched junk, all matching messages in the database belonging to the user will have their junk flag cleared.
 * 			But if the signature didn't match junk, all matching messages in the database belonging to the user will have the junk flag added.
 * @param	teach	the spam signature to be removed from the database.
 * @return	This function returns no value.
 */
void teacher_data_delete(teacher_data_t *teach) {

	MYSQL_BIND parameters[4];
	uint32_t flag = MAIL_MARK_JUNK;

	if (!teach) {
		return;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Message was classified as junk, so we were removing that junk flag.
	if (teach->disposition) {

		// Status
		parameters[0].buffer_type = MYSQL_TYPE_LONG;
		parameters[0].buffer_length = sizeof(uint32_t);
		parameters[0].buffer = &flag;
		parameters[0].is_unsigned = true;

		// Status
		parameters[1].buffer_type = MYSQL_TYPE_LONG;
		parameters[1].buffer_length = sizeof(uint32_t);
		parameters[1].buffer = &flag;
		parameters[1].is_unsigned = true;

		// Usernum
		parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[2].buffer_length = sizeof(uint64_t);
		parameters[2].buffer = &(teach->usernum);
		parameters[2].is_unsigned = true;

		// Signature
		parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[3].buffer_length = sizeof(uint64_t);
		parameters[3].buffer = &(teach->signum);
		parameters[3].is_unsigned = true;

		// If the message was already deleted from the server, this won't do anything. Either way we can afford to ignore the return code.
		stmt_exec(stmts.update_signature_flags_remove, parameters);
	}
	// Message was classified as innocent, and is now being trained as junk we'll add the junk flag to any messages still in the database.
	else {

		// Status
		parameters[0].buffer_type = MYSQL_TYPE_LONG;
		parameters[0].buffer_length = sizeof(uint32_t);
		parameters[0].buffer = &flag;
		parameters[0].is_unsigned = true;

		// Usernum
		parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[1].buffer_length = sizeof(uint64_t);
		parameters[1].buffer = &(teach->usernum);
		parameters[1].is_unsigned = true;

		// Signature
		parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[2].buffer_length = sizeof(uint64_t);
		parameters[2].buffer = &(teach->signum);
		parameters[2].is_unsigned = true;

		// If the message was already deleted from the server, this won't do anything. Either way we can afford to ignore the return code.
		stmt_exec(stmts.update_signature_flags_add, parameters);
	}

	// Reset the statement parameters.
	mm_wipe(parameters, sizeof(parameters));

	// Signum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(teach->signum);
	parameters[0].is_unsigned = true;

	if (stmt_exec(stmts.delete_signature, parameters)) {
		log_pedantic("Unable to delete the message signature. {signature = %lu}", teach->signum);
		return;
	}

	// Update the structure so we know the signature has been recently trained.
	teach->completed = 1;

	st_cleanup(teach->signature);
	st_cleanup(teach->username);
	st_cleanup(teach->password);

	teach->signature = NULL;
	teach->username = NULL;
	teach->password = NULL;

	return;
}

/**
 * @brief	Fetch information about a spam signature from the database.
 * @param	signum	the numerical id of the spam signature to be retrieved.
 * @param	NULL on failure, or a pointer to a newly allocated signature teacher object on success.
 */
teacher_data_t * teacher_data_fetch(uint64_t signum) {

	row_t *row;
	table_t *result;
	teacher_data_t *teach;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Signum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &signum;
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.fetch_signature, parameters))) {
		return NULL;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return NULL;
	}
	else if (!(teach = mm_alloc(sizeof(teacher_data_t)))) {
		res_table_free(result);
		return NULL;
	}

	// Store the data.
	teach->username = res_field_string(row, 0);
	teach->password = res_field_string(row, 1);
	teach->usernum = res_field_uint64(row, 2);
	teach->disposition = res_field_int8(row, 3);
	teach->keynum = res_field_uint64(row, 4);
	teach->signature = res_field_string(row, 5);
	teach->signum = signum;

	if (!teach->username || !teach->password || !teach->signature || !teach->usernum || !teach->keynum) {
		teacher_data_free(teach);
		return NULL;
	}

	res_table_free(result);

	return teach;
}

/**
 * @brief	Save spam signature information to the cache.
 * @note	The information will be cached for 2 hours.
 * @param	teach	the spam signature to be cached.
 * @return	This function returns no value.
 */
void teacher_data_save(teacher_data_t *teach) {

	chr_t key[256];
	size_t keylen;
	stringer_t *data = NULL;

	// Build the key.
	if ((keylen = snprintf(key, 256, "magma.web.teacher.%lu", teach->signum)) <= 0) {
		return;
	}

	// Serialize the teacher data.
	if (!serialize_int32(data, teach->completed) || !serialize_int32(&data, teach->disposition) || !serialize_uint64(&data, teach->usernum) ||
		!serialize_uint64(&data, teach->signum) || !serialize_uint64(&data, teach->keynum) || !serialize_st(&data, teach->username) ||
		!serialize_st(&data, teach->password) || !serialize_st(&data, teach->signature)  || !data) {
		log_pedantic("Unable to serialize a teacher data.");

		if (!data) {
			st_free(data);
		}

		return;
	}

	// Store it for two hours.
	if (!cache_set(PLACER(key, keylen), data, 7200)) {
		st_free(data);
		return;
	}

	st_free(data);

	return;
}

/**
 * @brief	Get information about a spam signature from the cache, or fall back to the database.
 * @param	signum	the numerical id of the spam signature to be retrieved.
 * @param	NULL on failure, or a pointer to a newly allocated signature teacher object on success.
 */
teacher_data_t * teacher_data_get(uint64_t signum) {

	chr_t key[256];
	size_t keylen;
	stringer_t *data;
	teacher_data_t *teach;
	serialization_t serial;

	// Build the key.
	if ((keylen = snprintf(key, 256, "magma.web.teacher.%lu", signum)) <= 0) {
		return teacher_data_fetch(signum);
	}

	// Pull it. If no teacher data is found we'll need to check the database.
	if (!(data = cache_get(PLACER(key, keylen)))) {
		return teacher_data_fetch(signum);
	}

	// Setup the deserialization structure.
	mm_wipe(&serial, sizeof(serialization_t));
	serial.data = data;

	// Deserialize the teacher data.
	if (!(teach = mm_alloc(sizeof(teacher_data_t))) || !deserialize_int32(&serial, &(teach->completed)) || !deserialize_int32(&serial, &(teach->disposition)) ||
		!deserialize_uint64(&serial, &(teach->usernum)) || !deserialize_uint64(&serial, &(teach->signum)) || !deserialize_uint64(&serial, &(teach->keynum)) ||
		!deserialize_st(&serial, &(teach->username)) || !deserialize_st(&serial, &(teach->password)) || !deserialize_st(&serial, &(teach->signature))) {
		log_pedantic("Unable to deserialize the teacher data. {key = %s}", key);
		teacher_data_free(teach);
		st_free(data);
		return teacher_data_fetch(signum);
	}

	st_free(data);

	// Error check the returned structure.
	if (!teach->completed && (!teach->username || !teach->password || !teach->signature || !teach->usernum || !teach->keynum)) {
		teacher_data_free(teach);
		return teacher_data_fetch(signum);
	}

	return teach;
}
