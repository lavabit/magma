
/**
 * @file /magma/src/network/new_meta.h
 *
 * @brief Meta information structures/types for users, folders, messages, etc.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_NETWORK_NEW_META_H
#define MAGMA_NETWORK_NEW_META_H

// All of a user's information is stored using this structure.
typedef struct {

	uint64_t usernum;
	pthread_rwlock_t lock;
	META_USER_FLAGS flags;
	stringer_t *username, *verification;
	inx_t *aliases, *messages, *message_folders, *folders, *contacts;

	struct {
		stringer_t *public, *private;
	} keys;

	struct {
		uint64_t user, messages, folders, contacts, aliases;
	} serials;

	struct {
		time_t stamp;
		uint64_t smtp, pop, imap, web, generic;
		pthread_mutex_t lock;
	} refs;

} new_meta_user_t;

#endif

