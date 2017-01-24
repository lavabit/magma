
/**
 * @file /magma/network/imap.h
 *
 * @brief	The IMAP server control structures.
 */

#ifndef MAGMA_NETWORK_IMAP_H
#define MAGMA_NETWORK_IMAP_H

typedef array_t imap_arguments_t;

// A structure containing the folder status information.
typedef struct {
	uint64_t foldernum, recent, unseen, uidnext, messages, first;
} imap_folder_status_t;

typedef struct {
	int_t uid, flags, internaldate, envelope, bodystructure, rfc822, rfc822_header, rfc822_size, rfc822_text, body;
	array_t *peek, *peek_partial, *normal, *normal_partial;
} imap_fetch_dataitems_t;

typedef struct {
	stringer_t *key, *value;
	struct imap_fetch_response_t *next;
} imap_fetch_response_t;

typedef struct {
	meta_user_t *user;
	imap_arguments_t *arguments;
	stringer_t *tag, *command, *username;
	int_t read_only, uid, session_state;
	uint64_t usernum, selected, user_checkpoint, messages_checkpoint, folders_checkpoint, messages_recent, messages_total;
} __attribute__((__packed__)) imap_session_t;

#endif

