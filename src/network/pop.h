
/**
 * @file /magma/network/pop.h
 *
 * @brief	The POP control structures.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_NETWORK_POP_H
#define MAGMA_NETWORK_POP_H

typedef struct {
	bool_t expunge;
	new_meta_user_t *user;
	int_t session_state;
	stringer_t *username;
	uint64_t usernum;
} __attribute__ ((__packed__)) pop_session_t;

#endif

