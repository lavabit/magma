
/**
 * @file /magma/objects/warehouse/domains.c
 *
 * @brief	Functions for managing the list of system domain names.
 */

#include "magma.h"

inx_t *domains = NULL;

/**
 * @brief	Free the global list of domains.
 * @return	This function returns no value.
 */
void domain_stop(void) {

	inx_cleanup(domains);
	domains = NULL;

	return;
}

/**
 * @brief	Fetch and store the list of configured domains from the database.
 * @return	true if the domain retrieval was successful, or false on error.
 */
bool_t domain_start(void) {

	if (!(domains = warehouse_fetch_domains())) {
		return false;
	}

	return true;
}

/**
 * @brief	Allocate and initialize a new domain object.
 * @param	domain		a pointer to a managed string containing the name of the specified domain.
 * @param	restricted	if set, indicate that the domain is restricted to authenticated users only.
 * @param	mailboxes	if set, indicate that mailboxes are hosted locally for the domain.
 * @param	wildcard	if set, indicate that wildcards are enabled for the domain.
 * @param	dkim		if set, indicate that outbound messages for the domain should be signed via DKIM.
 * @param	spf			if set, indicate that SPF is configured for the domaini.
 * @return	NULL on failure or a pointer to the newly initialized domain object on success.
 */
domain_t * domain_alloc(stringer_t *domain, int_t restricted, int_t mailboxes, int_t wildcard, int_t dkim, int_t spf) {

	domain_t *result;

	if (!(result = mm_alloc(align(16, sizeof(domain_t) + sizeof(placer_t) + st_length_get(domain))))) {
		log_pedantic("Unable to allocate %zu bytes for a domain structure.", align(16, sizeof(domain_t) + sizeof(placer_t) + st_length_get(domain)));
		return NULL;
	}

	result->restricted = restricted;
	result->mailboxes = mailboxes;
	result->wildcard = wildcard;
	result->dkim = dkim;
	result->spf = spf;
	result->domain = (placer_t *)((chr_t *)result + sizeof(domain_t));
	// QUESTION: How is this on the stack?
	((placer_t *)result->domain)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->domain)->length = st_length_get(domain);
	((placer_t *)result->domain)->data = (chr_t *)result + sizeof(domain_t) + sizeof(placer_t);
	mm_copy(st_data_get(result->domain), st_data_get(domain), st_length_get(domain));

	return result;
}

/**
 * @brief	Determine whether mailboxes are hosted locally for a specified domain.
 * @param	domain		a pointer to a managed string containing the name of the domain to be queried.
 * @return	-1 on failure or if the domain was not found, 0 if the domain is foreign, or 1 if the mailbox is hosted locally for the domain.
 */
int_t domain_mailboxes(stringer_t *domain) {

	domain_t *record;
	int_t result = -1;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = domain };

	if ((record = inx_find(domains, key))) {
		result = record->mailboxes;
	}

	return result;
}

/**
 * @brief	Determine whether a specified domain is restricted to authenticated users.
 * @param	domain		a pointer to a managed string containing the name of the domain to be queried.
 * @return	-1 on failure or if the domain was not found, 0 if restricted relay is disabled, or 1 if the domain is restricted to authenticated users only.
 */
int_t domain_restricted(stringer_t *domain) {

	domain_t *record;
	int_t result = -1;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = domain };

	if ((record = inx_find(domains, key))) {
		result = record->restricted;
	}

	return result;
}

/**
 * @brief	Determine whether a specified domain has wildcards enabled.
 * @param	domain		a pointer to a managed string containing the name of the domain to be queried.
 * @return	-1 on failure or if the domain wasn't found, 0 if wildcards are disabled, or 1 if the domain has wildcards enabled.
 */
int_t domain_wildcard(stringer_t *domain) {

	domain_t *record;
	int_t result = -1;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = domain };

	if ((record = inx_find(domains, key))) {
		result = record->wildcard;
	}

	return result;
}

/// TODO: Eliminate dkim+spf and replace them with a `sign` flag.
/**
 * @brief	Determine whether outbound messages should be signed via DKIM for a specified domain.
 * @param	domain		a pointer to a managed string containing the name of the domain to be queried.
 * @return	-1 on failure or if the domain wasn't found, 0 if DKIM signing is disabled, or 1 if outbound messages should be signed via DKIM for the domain.
 */
int_t domain_dkim(stringer_t *domain) {

	domain_t *record;
	int_t result = -1;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = domain };

	if ((record = inx_find(domains, key))) {
		result = record->dkim;
	}

	return result;
}

/**
 * @brief	Determine whether SPF has been configured for a specified domain.
 * @param	domain		a pointer to a managed string containing the name of the domain to be queried.
 * @return	-1 on failure or if the domain wasn't found, 0 if SPF is disabled, or 1 if SPF is actively configured for the domain.
 */
int_t domain_spf(stringer_t *domain) {

	domain_t *record;
	int_t result = -1;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = domain };

	if ((record = inx_find(domains, key))) {
		result = record->spf;
	}

	return result;
}
