
/**
 * @file /magma/objects/contacts/contacts.c
 *
 * @brief	The interface to managing user address book contacts.
 */

#include "magma.h"


/**
 * @brief	Free a contact entry object.
 * @param	contact		a pointer to the contact entry to be freed.
 * @return	This function returns no value.
 */
void contact_free(contact_t *contact) {

	if (contact) {
		inx_cleanup(contact->details);

		if (!st_opt_test(contact->name, FOREIGNDATA)) {
			st_free(contact->name);
		}

		mm_free(contact);
	}

	return;
}

/**
 * @brief	Free a contact entry detail.
 * @param	detail	a pointer to the contact entry detail to be freed.
 * @return	This function returns no value.
 */
void contact_detail_free(contact_detail_t *detail) {

	mm_cleanup(detail);
	return;
}

/**
 * @brief	Create a new contact entry object.
 * @param	contactnum	the numerical id of the new contact entry.
 * @param	name		a managed string containing the name of the contact.
 * @return	NULL on failure, or a pointer to the newly allocated and initialized contact entry object on success.
 */
contact_t * contact_alloc(uint64_t contactnum, stringer_t *name) {

	contact_t *result;

	if (!(result = mm_alloc(align(16, sizeof(contact_t) + sizeof(placer_t)) + align(8, st_length_get(name) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for a contact structure.", align(16, sizeof(contact_t) + sizeof(placer_t)) + align(8, st_length_get(name) + 1));
		return NULL;
	}
	else if (!(result->details = inx_alloc(M_INX_TREE, &contact_detail_free))) {
		log_pedantic("Unable to initialize the contact detail index.");
		mm_free(result);
		return NULL;
	}

	result->contactnum = contactnum;
	result->name = (placer_t *)((chr_t *)result + sizeof(contact_t));
	((placer_t *)result->name)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->name)->length = st_length_get(name);
	((placer_t *)result->name)->data = (chr_t *)result + align(16, sizeof(contact_t) + sizeof(placer_t));
	mm_copy(st_data_get(result->name), st_data_get(name), st_length_get(name));

	return result;
}

/**
 * @brief	Create a new contact detail from a key-value pair.
 * @param	key		a managed string containing the name of the detail.
 * @param	value	a managed string containing the value associated with the detail.
 * @param	flags	a bitmask of flags associated with the detail.
 * @return	NULL on failure, or a pointer to the newly allocated and initialized contact detail object on success.
 */
contact_detail_t * contact_detail_alloc(stringer_t *key, stringer_t *value, uint64_t flags) {

	contact_detail_t *result;

	if (!(result = mm_alloc(align(16, sizeof(contact_detail_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(key) + 1) + align(8, st_length_get(value) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for a contact detail structure.", align(16, sizeof(contact_detail_t) + sizeof(placer_t) + sizeof(placer_t)) +
			align(8, st_length_get(key) + 1) + align(8, st_length_get(value) + 1));
		return NULL;
	}

	result->flags = flags;

	result->key = (placer_t *)((chr_t *)result + sizeof(contact_detail_t));
	result->value = (placer_t *)((chr_t *)result + sizeof(contact_detail_t) + sizeof(placer_t));

	((placer_t *)result->key)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->key)->length = st_length_get(key);
	((placer_t *)result->key)->data = (chr_t *)result + align(16, sizeof(contact_detail_t) + sizeof(placer_t) + sizeof(placer_t));

	((placer_t *)result->value)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->value)->length = st_length_get(value);
	((placer_t *)result->value)->data = (chr_t *)result + align(16, sizeof(contact_detail_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(key) + 1);

	mm_copy(st_data_get(result->key), st_data_get(key), st_length_get(key));
	mm_copy(st_data_get(result->value), st_data_get(value), st_length_get(value));

	return result;
}

// Contact properties.

/**
 * @brief	Update the name (in memory) of a contact entry.
 * @param	contact		a pointer to the contact entry object to be updated.
 * @param	name		a managed string containing the new name of the specified contact entry.
 * @return	This function returns no value.
 */
void contact_name(contact_t *contact, stringer_t *name) {

	/// HIGH: A possible segfault is exposed here, related to the contacts.edit camelface call and
	/// 		having to do with the way foreign data is handled.

	size_t len;
	uchr_t *data;

	if (contact && !st_empty_out(name, &data, &len) && (data = mm_dupe(data, len + 1))) {

		// If the name isn't foreign data, then we should free it first.
		if (!st_opt_test(contact->name, FOREIGNDATA)) {
			mm_free(st_data_get(contact->name));
		}

		st_data_set(contact->name, data);
		st_length_set(contact->name, len);

		// Check whether the existing name is using a foreign buffer that needs to be freed?
		if (st_opt_test(contact->name, FOREIGNDATA)) {
			st_opt_set(contact->name, FOREIGNDATA, false);
		}
	}

	return;
}

/**
 * @brief	Retrieve all of a user's contacts (and contact folders) from the database.
 * @param	usernum		the numerical id of the user whose contact folders and contact entries should be fetched.
 * @return	NULL on failure, or a pointer to an inx holder containing all of the specified user's contact folders and entries on success.
 */
inx_t * contacts_update(uint64_t usernum) {

	inx_t *folders;
	inx_cursor_t *cursor;
	contact_folder_t *active;

	// If the fetch attempt fails, don't free the existing contacts index.
	if (!(folders = magma_folder_fetch(usernum, M_FOLDER_CONTACTS)) || !(cursor = inx_cursor_alloc(folders))) {
		inx_cleanup(folders);
		return NULL;
	}

	while ((active = inx_cursor_value_next(cursor))) {
		if (contacts_fetch(usernum, active) != 1) {
			log_pedantic("Unable to load the user contacts. { usernum = %lu / folder = %lu }", usernum, active->foldernum);
			inx_cursor_free(cursor);
			inx_free(folders);
			return NULL;
		}
	}

	inx_cursor_free(cursor);

	return folders;
}

/**
 * @brief	Create a new contact entry object and insert it into the database.
 * @param	usernum		the numerical id of the user to whom the new contact will belong.
 * @param	foldernum	the numerical id of the parent folder that will contain the contact entry.
 * @param	name		a pointer to a managed string containing the name of the new contact entry.
 * @return	NULL on failure, or a pointer to the newly created contact entry object on success.
 */
contact_t * contact_create(uint64_t usernum, uint64_t foldernum, stringer_t *name) {

	uint64_t contactnum;
	contact_t *result = NULL;

	if (!usernum || !foldernum || st_empty(name)) {
		log_pedantic("Invalid contact creation request.");
		return NULL;
	}
	else if (!(contactnum = contact_insert(usernum, foldernum, name)) || !(result = contact_alloc(contactnum, name))) {
		log_pedantic("Contact creation failed.");
		return NULL;
	}

	return result;
}

/**
 * @brief	Remove a contact entry and its associated details from the database.
 * @see		contact_delete()
 * @note	This function is not currently in use anywhere in the code.
 * @param	contact		a pointer to the contact entry to be deleted from the database.
 * @param	usernum		the numerical id of the user to whom the contact entry belongs.
 * @param	foldernum	the numerical id of the parent folder containing the specified contact entry.
 * @return	1 if the specified contact was deleted successfully, 0 if no matching contact was found in the database, or -1 on general failure.
 */
int_t contact_remove(contact_t *contact, uint64_t usernum, uint64_t foldernum) {

	if (!contact || !contact->contactnum || !usernum || !foldernum) {
		log_pedantic("Invalid contact removal request.");
		return -1;
	}

	return (contact_delete(contact->contactnum, usernum, foldernum));
}

/**
 * @brief	Move a contact entry from one contact to another.
 * @param	source	a pointer to the original parent contact folder object of the contact entry to be moved.
 * @param	target	a pointer to the contact folder object that will become the new parent of the specified contact entry.
 * @param	contact	a pointer to the contact object that is to be moved.
 * @param	usernum	the numerical id of the user to whom the specified contact entry belongs.
 * @return	-1 on error, or 1 if the contact move operation was successful.
 */
int_t contact_move(contact_folder_t *source, contact_folder_t *target, contact_t *contact, uint64_t usernum) {

	inx_t *swap;
	contact_t *dupe = NULL;
	multi_t multi = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!contact || !contact->contactnum || !usernum || !source || !source->foldernum || !target || !target->foldernum) {
		log_pedantic("Invalid contact move request.");
		return -1;
	}
	else if (contact_update(contact->contactnum, usernum, source->foldernum, target->foldernum, NULL) <= 0) {
		log_pedantic("Unable to modify the contact entry in the database.");
		return -1;
	}

	if ((multi.val.u64 = contact->contactnum) && (dupe = contact_alloc(contact->contactnum, contact->name))) {
		swap = dupe->details;
		dupe->details = contact->details;
		contact->details = swap;
	}

	// QUESTION: The comment below doesn't seem to make any sense.
	// Delete the contact record from the database. We do this before the insert to ensure things continue working properly even if the target and source folders are identical.
	inx_delete(source->records, multi);

	// At this point we return success even if we can't insert the record. Incrementing the serial number after an error ensures a full status refresh.
	if (dupe && !inx_insert(target->records, multi, dupe)) {
		serial_increment(OBJECT_CONTACTS, usernum);
		contact_free(dupe);
	}

	return 1;
}

/**
 * @brief	Change a detail of a contact entry.
 * @param	contact		a pointer to the target contact entry.
 * @param	usernum		the numerical id of the user to whom the specified contact entry belongs.
 * @param	foldernum
 * @param	key			a managed string containing the name of the contact entry detail to be changed.
 * 						If the special value "name" is passed as the key, the contact name will be changed instead.
 * @param	value		a managed string containing the new value of the specified contact entry detail;
 * 						if NULL is passed, the contact detail will be deleted rather than modified.
 * @return	-1 on failure or 1 on success.
 */
int_t contact_edit(contact_t *contact, uint64_t usernum, uint64_t foldernum, stringer_t *key, stringer_t *value) {

	contact_detail_t *detail = NULL;
	multi_t multi = { .type = M_TYPE_STRINGER, .val.st = NULL };

	if (!contact || !contact->contactnum || !contact->details || st_empty(key)) {
		log_pedantic("Invalid contact edit request.");
		return -1;
	}

	// Handle name edits with special logic.
	if (!st_cmp_ci_eq(key, PLACER("name", 4))) {

		if (contact_update(contact->contactnum, usernum, foldernum, 0, value) < 0) {
			log_pedantic("Contact name update failed.");
			return -1;
		}

		contact_name(contact, value);
	}

	// A NULL value is used to remove a detail key completely.
	else if (!value) {

		if (contact_detail_delete(contact->contactnum, key) < 0 || !(multi.val.st = key)) {
			log_pedantic("Contact creation failed.");
			return -1;
		}

		// If the delete was successful, delete any existing detail instances from the context. Deleting a contact detail key that doesn't exist isn't considered an
		// error so there is no need to check the result.
		inx_delete(contact->details, multi);


		/// LOW: It would be nice if we didn't call this function each time we updated a contact, but rather we called it when all of the updates have been completed.
		contact_update_stamp(contact->contactnum, usernum, foldernum);
	}
	// If the value isn't empty we need to update the database.
	else {

		if (contact_detail_upsert(contact->contactnum, key, value, 0) < 0 || !(detail = contact_detail_alloc(key, value, 0))) {
			log_pedantic("Contact creation failed.");
			return -1;
		}
		 // If the edit was successful, create a new detail record and update the contact record.
		else if (!(multi.val.st = detail->key) || !inx_replace(contact->details, multi, detail)) {
			log_pedantic("The contact was unable to accept the updated value.");
			contact_detail_free(detail);
			return -1;
		}

		/// LOW: It would be nice if we didn't call this function each time we updated a contact, but rather we called it when all of the updates have been completed.
		contact_update_stamp(contact->contactnum, usernum, foldernum);
	}

	return 1;
}

/**
 * @brief	Validate the name of a requested contact entry name.
 * @param	name	a pointer to a managed string containing the name of the contact to be validated.
 * @param	errmsg	if not NULL, the address of a string pointer that will receive a descriptive error message upon validation failure.
 * @return	true if the proposed contact name was valid, or false if it was not.
 */
bool_t contact_validate_name(stringer_t *name, chr_t **errmsg) {

	uchr_t *sbuf;
	size_t slen;

	if (st_empty_out(name, &sbuf, &slen)) {
		if (errmsg) *errmsg = "Contact name cannot be zero-length.";
		return false;
	}

	if (slen > 255) {
		if (errmsg) *errmsg = "Contact name must be 255 characters or less.";
		return false;
	}

	for (size_t i = 0; i < slen; i++) {

		if (!chr_printable(*sbuf)) {
			if (errmsg) *errmsg = "Contact name must contain printable characters only.";
			return false;
		}

		sbuf++;
	}

	return true;
}

/**
 * @brief	Validate the value of a requested contact entry detail.
 * @param	key		a pointer to a managed string containing the name of the contact detail key to be validated.
 * @param	value	a pointer to a managed string containing the value of the contact detail to be validated.
 * @param	errmsg	if not NULL, the address of a string pointer that will receive a descriptive error message upon validation failure.
 * @return	true if the proposed contact key/value pair was valid, or false if it was not.
 */
bool_t contact_validate_detail(stringer_t *key, stringer_t *value, chr_t **errmsg) {

	uchr_t *kbuf, *vbuf;
	size_t klen, vlen;

	if (st_empty_out(key, &kbuf, &klen)) {
		if (errmsg) *errmsg = "Contact detail key name cannot be zero-length.";
		return false;
	}

	if (klen > 255) {
		if (errmsg) *errmsg = "Contact detail key name must be 255 characters or less.";
		return false;
	}

	for (size_t i = 0; i < klen; i++) {

		if (!chr_printable(*kbuf)) {
			if (errmsg) *errmsg = "Contact detail key name must contain printable characters only.";
			return false;
		}

		kbuf++;
	}

	st_empty_out(value, &vbuf, &vlen);

	for (size_t i = 0; i < vlen; i++) {

		if (!chr_printable(*vbuf)) {
			if (errmsg) *errmsg = "Contact detail value must contain printable characters only.";
			return false;
		}

		vbuf++;
	}

	return true;
}
