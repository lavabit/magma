
/**
 * @file /magma/servers/imap/fetch_response.c
 *
 * @brief	Functions used to handle IMAP commands/actions.
 */

#include "magma.h"

void imap_fetch_response_free(imap_fetch_response_t *response) {

	imap_fetch_response_t *holder;

	while (response) {
		st_cleanup(response->key);
		st_cleanup(response->value);
		holder = response;
		response = (imap_fetch_response_t *)response->next;
		mm_free(holder);
	}

	return;
}

imap_fetch_response_t * imap_fetch_response_add(imap_fetch_response_t *response, stringer_t *key, stringer_t *value) {

	imap_fetch_response_t *output, *holder;

	// Sanity
	if (!key || !value) {
		log_error("Passed a NULL value.");
		return response;
	}

	// Allocate structure.
	if (!(output = mm_alloc(sizeof(imap_fetch_response_t)))) {
		st_free(value);
		return response;
	}

	// Setup structure.
	if (!(output->key = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, key))) {
		mm_free(output);
		st_free(value);
		return response;
	}

	output->value = value;

	// If this is the first element.
	if (!response) {
		return output;
	}

	// Otherwise iterate to the end and append.
	holder = response;

	while (holder->next) {
		holder = (imap_fetch_response_t *)holder->next;
	}

	holder->next = (struct imap_fetch_response_t *)output;

	return response;
}
