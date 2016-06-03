
/**
 * @file /magma/objects/mail/remove_message.c
 *
 * @brief Functions used to delete messages.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// LOW: When the reliable job queue is working we can handle unlink errors by creating a job entry which will retry the
/// unlink operation at a later time.
/**
 * @brief	Remove a specified mail message from both the database and storage.
 * @param	usernum		the user id to whom the specified mail message belongs.
 * @param	messagenum	the target mail message id.
 * @param	size		the size of the message in bytes, to be assessed against the user quota.
 * @param	server		the name of the server on which the mail message resides.
 * @return	true if the message removal succeeds or false on failure.
 */
bool_t mail_remove_message(uint64_t usernum, uint64_t messagenum, uint32_t size, chr_t *server) {

	chr_t *path;
	int_t state;
	int64_t transaction;

	// Build the message path.
	if (!(path = mail_message_path(messagenum, server))) {
		return false;
	}

	// We want to delete the message as part of a transaction.
	if ((transaction = tran_start()) < 0) {
		ns_free(path);
		return false;
	}

	// Remove from the database.
	if (!mail_db_delete_message(usernum, messagenum, size, transaction)) {
		tran_rollback(transaction);
		ns_free(path);
		return false;
	}

	// Commit the transaction.
	if ((state = tran_commit(transaction))) {
		log_pedantic("Could not commit the transaction. {tran_commit = %i}", state);
		ns_free(path);
		return false;
	}

	// Unlink the file. We return success even if the unlink operation fails because the database record has already been removed. The result
	// is an orphaned file that will someday need to be cleaned.
	if ((state = unlink(path)) != 0) {
		log_pedantic("Could not unlink the message %s. {unlink = %i}", path, state);
	}

	ns_free(path);
	return true;
}
