
/**
 * @file /magma/network/sessions.h
 *
 * @brief	Structures for handling web session data.
 */

#ifndef MAGMA_NETWORK_SESSIONS_H
#define MAGMA_NETWORK_SESSIONS_H

enum {
	SESSION_STATE_TERMINATED = -1,
	SESSION_STATE_NEUTRAL = 0,
	SESSION_STATE_AUTHENTICATED = 1
};

typedef struct __attribute__ ((packed)) {
	uint64_t attach_id; /* The unique attachment id. */
	stringer_t *filename; /* The local filename of the submitted attachment. */
	stringer_t *filedata; /* The raw data of the attached file. */
} attachment_t;

typedef struct __attribute__ ((packed)) {
	uint64_t comp_id; /* The composition ID, returned in response to a messages.compose request. */
	uint64_t attached; /* The attachment # counter. */
	inx_t *attachments; /* A list of the files that have been submitted for attachment to this composition. */
} composition_t;

typedef struct __attribute__ ((packed)) {

	int_t state;
	meta_user_t *user;
	inx_t *compositions;
	uint64_t composed;

	struct __attribute__ ((packed)) {
		bool_t secure;
		bool_t httponly;
		stringer_t *host;
		stringer_t *path;
		stringer_t *application;
	} request;

	// The warden is responsible for locating a session in memory and ensuring that only legitimate requests are allowed access.
	struct __attribute__ ((packed)) {
		ip_t ip; /* The owner's IP address. */
		uint64_t key; /* Randomly generated number used to authenticate a request. */
		uint64_t host; /* The cluster node that created and owns a particular session. */
		uint64_t stamp; /* The date of origin. */
		uint64_t number; /* The unique auto-incrementing number and the value used to locate a session in cache. */
		stringer_t *agent; /* The owner's user-agent string. */
		stringer_t *token; /* The encrypted token used to retrieve this session during future requests. */
	} warden;

	struct __attribute__ ((packed)) {
		time_t stamp;
		uint64_t web;
	} refs;

	struct __attribute__ ((packed)) {
		time_t stamp;
		bool_t trigger;
	} refresh;

	pthread_mutex_t lock;

} session_t;

#endif

