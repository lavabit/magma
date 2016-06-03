
/**
 * @file /magma/providers/checkers/spf.c
 *
 * @brief	The functions used to validate SPF information.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

// #define MAGMA_SPF_DEBUG

pool_t *spf_pool = NULL;
chr_t spf_version[8];

/**
 * @brief	Return the version string of the spf library.
 * @return	a pointer to a character string containing the spf library version information.
 */
const chr_t * lib_version_spf(void) {
	return spf_version;
}

/**
 * @brief	Initialize the spf library and bind dynamically to the exported functions that are required.
 * @return	false on failure or true on success.
 */
bool_t lib_load_spf(void) {

	int32_t major, minor, patch;

	symbol_t spf[] = {
		M_BIND(SPF_get_lib_version), M_BIND(SPF_request_free), M_BIND(SPF_request_new),	M_BIND(SPF_request_query_mailfrom),
		M_BIND(SPF_request_set_env_from), M_BIND(SPF_request_set_helo_dom),	M_BIND(SPF_request_set_ipv4), M_BIND(SPF_request_set_ipv6),
		M_BIND(SPF_response_free), M_BIND(SPF_response_reason), M_BIND(SPF_response_result), M_BIND(SPF_server_free), M_BIND(SPF_server_new),
		M_BIND(SPF_strerror), M_BIND(SPF_strreason), M_BIND(SPF_strresult), M_BIND(SPF_dns_zone_new),  M_BIND(SPF_dns_zone_add_str)
	};

	if (lib_symbols(sizeof(spf) / sizeof(symbol_t), spf) != 1) {
		return false;
	}

	SPF_get_lib_version_d(&major, &minor, &patch);
	snprintf(spf_version, 8, "%i.%i.%i", major, minor, patch);

	return true;
}

/**
 * @brief	Initialize the pool of spf server connections.
 * @return	false on failure or true on success.
 */
bool_t spf_start(void) {

#ifdef MAGMA_SPF_DEBUG
	int_t spf_debug = 1;
#else
	int_t spf_debug = 0;
#endif

	SPF_server_t *object;

	// Create the object pool.
	if ((spf_pool = pool_alloc(magma.iface.spf.pool.connections, magma.iface.spf.pool.timeout)) == NULL) {
		log_pedantic("Could not initialize the SPF object pool.");
		return false;
	}

	// Initialize the objects in the pool.
	for (uint32_t i = 0; i < magma.iface.spf.pool.connections; i++) {
		if ((object = SPF_server_new_d(SPF_DNS_CACHE, spf_debug)) == NULL) {
			log_pedantic("Could not initialize SPF object number %i.", i + 1);
			return false;
		}
		pool_set_obj(spf_pool, i, object);
	}

	return true;
}

/**
 * @brief	Destroy the spf connection pool.
 * @return	This function returns no value.
 */
void spf_stop(void) {

	SPF_server_t *object;

	// Destroy the objects.
	for (uint32_t i = 0; i < magma.iface.spf.pool.connections; i++) {

		if ((object = pool_get_obj(spf_pool, i))) {
			SPF_server_free_d(object);
		}

	}

	// Destroy the pool.
	pool_free(spf_pool);
	spf_pool = NULL;

	return;
}

/**
 * @brief	Validate an smtp request via spf.
 * @param	ip			an ip address object containing the ip address of the connection to be checked.
 * @param	helo		a managed string containing the client supplied HELO request value.
 * @param	mailfrom	a managed string containing the client supplied MAIL FROM request value.
 * TODO: We really need to change these return values to account for 0
 * @return	-2 on spf check fail, -1 on other failure, and 1 on spf pass.
 */
int_t spf_check(ip_t *ip, stringer_t *helo, stringer_t *mailfrom) {

	uint32_t item;
	placer_t domain;
	SPF_reason_t reason;
	SPF_errcode_t error;
	SPF_result_t response;
	SPF_request_t *spf_request = NULL;
	SPF_response_t *spf_response = NULL;

	mail_domain_get(mailfrom, &domain);
	stats_adjust_by_name("provider.spf.checked", 1);

	if (pool_pull(spf_pool, &item) != PL_RESERVED) {
		stats_adjust_by_name("provider.spf.error", 1);
		return -1;
	}

	// Create the request context.
	else if (!(spf_request = SPF_request_new_d(pool_get_obj(spf_pool, item)))) {
		log_pedantic("SPF request allocation error. {SPF_request_new = NULL}");
		pool_release(spf_pool, item);
		stats_adjust_by_name("provider.spf.error", 1);
		return -1;
	}

	// Set the IP address for the request.
	else if ((ip->family == AF_INET && (error = SPF_request_set_ipv4_d(spf_request, ip->ip4)) != SPF_E_SUCCESS) ||
		(ip->family == AF_INET6 && (error = SPF_request_set_ipv6_d(spf_request, ip->ip6)) != SPF_E_SUCCESS) ||
		(ip->family != AF_INET && ip->family != AF_INET6)) {
		SPF_request_free_d(spf_request);
		pool_release(spf_pool, item);
		log_pedantic("SPF context configuration error. {error = %s}", SPF_strerror_d(error));
		stats_adjust_by_name("provider.spf.error", 1);
		return -1;
	}

	// Set the HELO/EHLO domain and the MAIL FROM address.
	else if ((error = SPF_request_set_helo_dom_d(spf_request, st_char_get(helo))) != SPF_E_SUCCESS ||
		(error = SPF_request_set_env_from_d(spf_request, st_char_get(mailfrom))) != SPF_E_SUCCESS) {
		SPF_request_free_d(spf_request);
		pool_release(spf_pool, item);
		stats_adjust_by_name("provider.spf.error", 1);
		return -1;
	}

	// Query the domain for its SPF record.
	else if ((error = SPF_request_query_mailfrom_d(spf_request, &spf_response)) != SPF_E_SUCCESS) {
		if (spf_response) SPF_response_free_d(spf_response);
		SPF_request_free_d(spf_request);
		pool_release(spf_pool, item);

		// Indicates the domain being queried did not publish an SPF record.
		if (error == SPF_E_NOT_SPF) {
			stats_adjust_by_name("provider.spf.missing", 1);
		}
		else {
			log_pedantic("SPF query error. {domain = %.*s / error = %s}", st_length_int(&domain), st_char_get(&domain), SPF_strerror_d(error));
			stats_adjust_by_name("provider.spf.error", 1);
		}
		return -1;
	}


	// Check the result.
	response = SPF_response_result_d(spf_response);
	reason = SPF_response_reason_d(spf_response);
	SPF_response_free_d(spf_response);
	SPF_request_free_d(spf_request);
	pool_release(spf_pool, item);

	if (response == SPF_RESULT_PASS) {
#ifdef MAGMA_SPF_DEBUG
		log_pedantic("SPF check passed. {result = PASS / reason = %s}", SPF_strreason_d(reason));
#endif
		stats_adjust_by_name("provider.spf.pass", 1);
		return 1;
	}
	else if (response == SPF_RESULT_NEUTRAL) {
#ifdef MAGMA_SPF_DEBUG
		log_pedantic("SPF check neutral. {result = NEUTRAL / reason = %s}", SPF_strreason_d(reason));
#endif
		stats_adjust_by_name("provider.spf.neutral", 1);
		return -1;
	}
	else if (response == SPF_RESULT_FAIL) {
#ifdef MAGMA_SPF_DEBUG
		log_pedantic("SPF check failed. {result = FAILED / reason = %s}", SPF_strreason_d(reason));
#endif
		stats_adjust_by_name("provider.spf.fail", 1);
		return -2;
	}

#ifdef MAGMA_SPF_DEBUG
	log_pedantic("SPF check error. {result = %s / reason = %s}", SPF_strresult_d(response), SPF_strreason_d(reason));
#endif
	stats_adjust_by_name("provider.spf.error", 1);
	return -1;
}

