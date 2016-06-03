
/**
 * @file /magma/providers/checkers/dkim.c
 *
 * @brief	Functions used to generate and verify Domain Keys Identified Mail (DKIM).
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

chr_t dkim_version[8];
DKIM_LIB *dkim_engine = NULL;

#define DKIM_PROCESS_ALL -1L

/**
 * @brief	Return the version string of the dkim library.
 * @return	a pointer to a character string containing the dkim library version information.
 */
const chr_t * lib_version_dkim(void) {
	return dkim_version;
}

/**
 * @brief	Initialize the dkim library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_dkim(void) {

	uint32_t version;

	symbol_t dkim[] = {
		M_BIND(dkim_body), M_BIND(dkim_chunk), M_BIND(dkim_close), M_BIND(dkim_eoh), M_BIND(dkim_eom), M_BIND(dkim_free),
		M_BIND(dkim_getresultstr), M_BIND(dkim_header),	M_BIND(dkim_init),	M_BIND(dkim_libversion),
		M_BIND(dkim_sign), M_BIND(dkim_verify), // yes there is one anomaly because of a name clash:
		{ .name = "dkim_getsighdr", .pointer = (void *)&dkim_getsighdrx_d },
	};

	if (lib_symbols(sizeof(dkim) / sizeof(symbol_t), dkim) != 1) {
		return false;
	}

	if (!(version = dkim_libversion_d()) || snprintf(dkim_version, 8, "%lu.%lu.%lu", (uint64_t)((0xff000000 & version) >> 24),
		(uint64_t)((0x00ff0000 & version) >> 16), (uint64_t)((0x0000ff00 & version) >> 8)) < 5) {
		return false;
	}

	return true;
}

/**
 * @brief	A memory allocation routine stub for dkim.
 * @note	This function is passed to dkim_init().
 * @param	closure		a void pointer to a memory closure passed to dkim_sign() and dkim_verify().
 * @param	nbytes		the size, in bytes, of the memory block to be allocated.
 * @return	a pointer to a newly allocated block of memory of specified size.
 */
void * dkim_memory_alloc(void *closure, size_t nbytes) {

	return mm_alloc(nbytes);
}

/**
 * @brief	A memory free routine stub for dkim.
 * @note	This function is passed to dkim_init().
 * @param	closure		a void pointer to a memory closure passed to dkim_sign() and dkim_verify().
 * @param	ptr			a pointer to the block of memory to be released.
 * @return	This function returns no value.
 */
void dkim_memory_free(void *closure, void *ptr) {

	mm_free(ptr);
	return;
}

/**
 * @brief	Start the dkim engine.
 * @return	false on failure or true on success.
 */
bool_t dkim_start(void) {

	if (!(dkim_engine = dkim_init_d(dkim_memory_alloc, dkim_memory_free))) {
		log_pedantic("DKIM engine failed to start. {dkim_init = NULL}");
		return false;
	}

	return true;
}

/**
 * @brief	Stop the dkim engine.
 * @return	This function returns no value.
 */
void dkim_stop(void) {

	if (dkim_engine) {
		dkim_close_d(dkim_engine);
		dkim_engine = NULL;
	}

	return;
}


/**
 * @brief	Generate a DKIM signature for a message.
 * @note	For this function to work, the magma.dkim.enabled configuration option must be set.
 *			This function also updates the provider.dkim.signed statistic.
 * @param	id			a managed string containing a printable string id for this message.
 * @param	message		a managed string containing the mail message data.
 * @return	NULL on failure or if magma.dkim.enabled is false; otherwise, a managed string containing a DKIM-Signature header
 * 			built using the dkim signature that was generated for the input message.
 */
stringer_t * dkim_create(stringer_t *id, stringer_t *message) {

	DKIM *context;
	DKIM_STAT status;
	dkim_sigkey_t key;
	stringer_t *output = NULL, *signature = NULL;

	if (!magma.dkim.enabled) {
		return NULL;
	}

	key = (uchr_t *)magma.dkim.privkey;

	// Create a new handle to sign the message.
	if (!(context = dkim_sign_d(dkim_engine, st_data_get(id), NULL, key, (uchr_t *)magma.dkim.selector, (uchr_t *)magma.dkim.domain, DKIM_CANON_RELAXED,
			DKIM_CANON_RELAXED, DKIM_SIGN_RSASHA256, DKIM_PROCESS_ALL, &status)) ||	status != DKIM_STAT_OK) {
		log_pedantic("Allocation of the DKIM signature context failed. {%sstatus = %s}", context ? "" : "dkim_sign = NULL / ", dkim_getresultstr_d(status));

		if (context) {
			dkim_free_d(context);
		}

		return NULL;
	}

	// Handle the message as a chunk, then finalize input by processing a NULL chunk and signal end-of-message.
	if ((status = dkim_chunk_d(context, st_data_get(message), st_length_get(message))) == DKIM_STAT_OK &&
		(status = dkim_chunk_d(context, NULL, 0)) == DKIM_STAT_OK &&
		(status = dkim_eom_d(context, NULL)) == DKIM_STAT_OK && (signature = st_alloc_opts(NULLER_T | CONTIGUOUS | HEAP, DKIM_MAXHEADER + 1))) {
		// Then store the signature into our buffer.
		status = dkim_getsighdrx_d(context, st_data_get(signature), DKIM_MAXHEADER + 1, 0);
	}

	dkim_free_d(context);

	if (status == DKIM_STAT_OK && signature) {
		output = st_merge("nns", DKIM_SIGNHEADER, ": ", signature);
		stats_adjust_by_name("provider.dkim.signed", 1);
	}
	else if (status != DKIM_STAT_OK) {
		log_pedantic("An error occurred while trying to generate the DKIM signature. {result = %s}", dkim_getresultstr_d(status));
		/// LOW: Should we track signing errors too?
	}

	st_cleanup(signature);

	return output;
}

/**
 * @brief	Perform dkim verification of a signed message.
 * @note	This function also updates the provider.dkim.* statistics.
 * @param	id			a managed string containing a printable string id for this message.
 * @param	message		a managed string containing the mail message data.
 * @return	1 if the dkim verification was successful, or < 0 otherwise.
 *         -1:	General internal or dkim-related failure occurred, or no signature was present.
 *         -2:	The signature was bad or the signing key was revoked or key retrieval failed (try again later).
 */
int_t dkim_check(stringer_t *id, stringer_t *message) {

	DKIM *context;
	DKIM_STAT status;

	stats_adjust_by_name("provider.dkim.checked", 1);

	// Create a new handle to verify the signed message.
	if (!(context = dkim_verify_d(dkim_engine, st_data_get(id), NULL, &status)) || status != DKIM_STAT_OK) {
		log_pedantic("Allocation of the DKIM verification context failed. {%status = %s}", context ? "" : "dkim_verify = NULL / ", dkim_getresultstr_d(status));
		stats_adjust_by_name("provider.dkim.errors", 1);

		if (context) {
			dkim_free_d(context);
		}

		return -1;
	}

	// Handle the message as a chunk, then finalize input by processing a NULL chunk and signal end-of-message.
	if ((status = dkim_chunk_d(context, st_data_get(message), st_length_get(message))) == DKIM_STAT_OK && (status = dkim_chunk_d(context, NULL, 0)) == DKIM_STAT_OK) {
		status = dkim_eom_d(context, NULL);
	}

	dkim_free_d(context);

	if (status == DKIM_STAT_BADSIG || status == DKIM_STAT_REVOKED || status == DKIM_STAT_KEYFAIL) {
		//log_pedantic("The DKIM signature found but it does not validate.");
		stats_adjust_by_name("provider.dkim.fail", 1);
		return -2;
	}
	else if (status == DKIM_STAT_NOSIG) {
		//log_pedantic("The message doesn't appear to contain a DKIM signature.");
		stats_adjust_by_name("provider.dkim.missing", 1);
		return -1;
	}
	else if (status != DKIM_STAT_OK) {
		//log_pedantic("The DKIM signature could not be validated. {result = %s}", dkim_getresultstr_d(status));
		stats_adjust_by_name("provider.dkim.neutral", 1);
		return -1;
	}

	//log_pedantic("DKIM signature found and it validated!");
	stats_adjust_by_name("provider.dkim.pass", 1);

	return 1;
}
