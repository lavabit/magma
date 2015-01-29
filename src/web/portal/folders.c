
/**
 * @file /magma/web/portal/folders.c
 *
 * @brief	Folder manipulation functions for use by the portal.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Add a new mail folder for a user.
 * @note	If parent is 0, then the new folder is assumed to be a root folder.
 * 			This function returns no value, but returns the appropriate portal json-rpc response directly.
 * @param	con		a pointer to the connection object across which the add folder response will be sent.
 * @param	name	a managed string containing the name of the new folder.
 * @param	parent	the numerical id of the folder that will be the parent of the new folder (or 0 to specify a root folder).
 * @return	This function returns no value.
 */
void portal_folder_mail_add(connection_t *con, stringer_t *name, uint64_t parent) {

	int_t state;
	meta_folder_t *active;
	stringer_t *folder = NULL, *complete = NULL;

	// If the folder has a parent.
	if (parent && !(active = meta_folders_by_number(con->http.session->user->folders, parent))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_ADD, "Invalid parent folder.");
		return;
	}
	else if (parent && (!(folder = meta_folders_name(con->http.session->user->folders, active)) || !(complete = st_merge("sns", folder, ".", name)))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Create the folder.
	meta_user_wlock(con->http.session->user);

	if ((state = imap_folder_create(con->http.session->user->usernum, con->http.session->user->folders, complete ? complete : name)) == 1) {
		sess_serial_check(con->http.session, OBJECT_FOLDERS);
	}

	meta_user_unlock(con->http.session->user);

	// Let the user know what happened.
	if (state == -1) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder name is invalid.",	"id", con->http.portal.id);
	}
	else if (state == -2) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder would exceed the limit on folder levels.",	"id", con->http.portal.id);
	}
	else if (state == -3) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "Your Inbox is not allowed to have sub folders.",	"id", con->http.portal.id);
	}
	else if (state == -4) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder name is too long.",	"id", con->http.portal.id);
	}
	else if (state == -5) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The folder name requested already exists.",	"id", con->http.portal.id);
	}
	else if (state != 1 || !(active = meta_folders_by_name(con->http.session->user->folders, complete ? complete : name))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}
	else {
		portal_endpoint_response(con, "{s:s, s:{s,I}, s:I}", "jsonrpc", "2.0", "result", "folderID", active->foldernum, "id", con->http.portal.id);
	}

	st_cleanup(complete);
	st_cleanup(folder);
	return;
}
/**
 * @brief	Add a new contact folder for a user.
 * @note	If parent is 0, then the new folder is assumed to be a root folder.
 * 			This function returns no value, but returns the appropriate portal json-rpc response directly.
 * @param	con		a pointer to the connection object across which the add folder response will be sent.
 * @param	name	a managed string containing the name of the new folder.
 * @param	parent	the numerical id of the folder that will be the parent of the new folder (or 0 to specify a root folder).
 * @return	This function returns no value.
 */
void portal_folder_contacts_add(connection_t *con, stringer_t *name, uint64_t parent) {

	int_t state;
	contact_folder_t *active;
	stringer_t *folder = NULL, *complete = NULL;

	// If a parent is specified, make sure it actually exists.
	if (parent && !(active = magma_folder_find_number(con->http.session->user->contacts, parent))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_ADD, "Invalid parent folder.");
		return;
	}
	/*else if (parent && (!(folder = magma_folder_name(con->http.session->user->contacts, active)) || !(complete = st_merge("sns", folder, ".", name)))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	} */

	// Create the folder.
	meta_user_wlock(con->http.session->user);

	if ((state = contact_folder_create(con->http.session->user->usernum, parent, name, con->http.session->user->contacts)) == 1) {
		sess_serial_check(con->http.session, OBJECT_CONTACTS);
	}

	meta_user_unlock(con->http.session->user);

	switch(state) {
		case 0:
			portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
			break;
		case -1:
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder name is invalid.",	"id", con->http.portal.id);
			break;
		case -2:
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder name is too long.",	"id", con->http.portal.id);
			break;
		case -3:
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The folder name requested already exists.",	"id", con->http.portal.id);
			break;
		case -4:
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "An invalid parent folder was specified.",	"id", con->http.portal.id);
			break;
		case -5:
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder would exceed the limit on folder levels.",	"id", con->http.portal.id);
		default:
			break;
	}

	// Let the user know what happened.
	if (state == 1 && (active = magma_folder_find_name(con->http.session->user->contacts, name, parent, false))) {
		portal_endpoint_response(con, "{s:s, s:{s,I}, s:I}", "jsonrpc", "2.0", "result", "folderID", active->foldernum, "id", con->http.portal.id);
	}
	else if (state == 1) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "folders.add", "failed", "message", "The specified folder could not be created.",	"id", con->http.portal.id);
	}

	st_cleanup(complete);
	st_cleanup(folder);
	return;
}

/**
 * @brief	Remove a user's mail folder.
 * @param	con			a pointer to the connection object across which the remove folder response will be sent.
 * @param	foldernum	the numerical id of the mail folder that will be removed.
 * @return	This function returns no value.
 */
void portal_folder_mail_remove(connection_t *con, uint64_t foldernum) {

	int_t state;
	meta_folder_t *active;
	stringer_t *folder = NULL;

	// See if the folder exists.
	if (!(active = meta_folders_by_number(con->http.session->user->folders, foldernum))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,	"The folder provided for deletion is invalid.");
		return;
	}
	else if (!(folder = meta_folders_name(con->http.session->user->folders, active))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	meta_user_wlock(con->http.session->user);

	if ((state = imap_folder_remove(con->http.session->user->usernum, con->http.session->user->folders, con->http.session->user->messages, folder)) == 1) {
		sess_serial_check(con->http.session, OBJECT_FOLDERS);
	}

	meta_user_unlock(con->http.session->user);

	// Let the user know what happened.
	if (state == 1) {
		portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "folder.remove", "success", "id", con->http.portal.id);
	}
	else if (state == -1) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,
			"The folder provided for deletion is invalid.");
	}
	else if (state == -2) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,
			"You are not allowed to delete the Inbox folder.");
	}
	else if (state == -3) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,
			"The folder you are trying to delete does not exist.");
	}
	else {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}

	st_cleanup(folder);
}

/**
 * @brief	Remove a user's contacts folder.
 * @param	con			a pointer to the connection object across which the remove folder response will be sent.
 * @param	foldernum	the numerical id of the contacts folder that will be removed.
 * @return	This function returns no value.
 */
void portal_folder_contacts_remove(connection_t *con, uint64_t foldernum) {

	int_t state;
	contact_folder_t *active;

	// See if the folder exists.
	if (!(active = magma_folder_find_number(con->http.session->user->contacts, foldernum))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,	"The folder provided for deletion is invalid.");
		return;
	}

	meta_user_wlock(con->http.session->user);

	// TODO: Double check this comparison
	if (!(state = contact_folder_remove(con->http.session->user->usernum, foldernum, con->http.session->user->contacts))) {
		sess_serial_check(con->http.session, OBJECT_CONTACTS);
	}

	meta_user_unlock(con->http.session->user);

	// Let the user know what happened.
	switch (state) {
		case 0:
			portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "folder.remove", "success", "id", con->http.portal.id);
			break;
		case -1:
			portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
			break;
		case -2:
			portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,
				"The folder provided for deletion is invalid.");
			break;
		case -3:
			portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,
				"Contact folders may not be deleted unless empty.");
			break;
	}

	return;
}
