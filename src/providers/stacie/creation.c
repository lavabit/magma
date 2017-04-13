
/**
 * @file /magma/src/providers/stacie/creation.c
 *
 * @brief Create cryptographically strong random STACIE salt, nonce, and shard values.
 */

#include "magma.h"

/**
 * @brief	Generates a random shard of precisely 64 bytes, suitable for use with a STACIE realm.
 *
 * @note	The master key is combined with the realm label and a random shard value to create the realm key.
 *
 * @param	output	a managed string to receive the output; if passed as NULL, an output buffer will be allocated.
 *
 * @return	A managed string holding the freshly generated shard in binary form. If NULL was passed in then the result must be freed by the caller.
 */
stringer_t * stacie_create_shard(stringer_t *output) {

	stringer_t *result = NULL;

	// If an output buffer was passed in make sure it's a valid destination with length tracking.
	if (output && (!st_valid_destination(st_opt_get(output)) || !st_valid_tracked(st_opt_get(output)) || st_avail_get(output) < STACIE_SHARD_LENGTH)) {
		log_pedantic("An output string was supplied but is not a valid destination for the shard value.");
		return NULL;
	}
	// Otherwise allocate an output buffer.
	else if (!output && !(output = result = st_alloc(STACIE_SHARD_LENGTH))) {
		log_pedantic("Failed to allocate an output buffer for the shard value.");
		return NULL;
	}

	// Attempt to write the random bytes into the output buffer.
	if (rand_write(PLACER(st_data_get(output), STACIE_SHARD_LENGTH)) != STACIE_SHARD_LENGTH) {
		log_pedantic("Failed to create a random shard value.");
		st_cleanup(result);
		return NULL;
	}

	st_length_set(output, STACIE_SHARD_LENGTH);
	return output;
}

/**
 * @brief	Generates a random salt of precisely 128 bytes, suitable for use with STACIE authentication scheme.
 *
 * @note	While the salt and nonce creation functions are nearly identical, they remain seperated so they can use distinct
 * 			preprocessor definitions for the result length.
 *
 * @param	output	a managed string to receive the output; if passed as NULL, an output buffer will be allocated.
 *
 * @return	A managed string holding the freshly generated salt in binary form. If NULL was passed in then the result must be freed by the caller.
 */
stringer_t * stacie_create_salt(stringer_t *output) {

	stringer_t *result = NULL;

	// If an output buffer was passed in make sure it's a valid destination with length tracking.
	if (output && (!st_valid_destination(st_opt_get(output)) || !st_valid_tracked(st_opt_get(output)) || st_avail_get(output) < STACIE_SALT_LENGTH)) {
		log_pedantic("An output string was supplied but is not a valid destination for the salt value.");
		return NULL;
	}
	// Otherwise allocate an output buffer.
	else if (!output && !(output = result = st_alloc(STACIE_SALT_LENGTH))) {
		log_pedantic("Failed to allocate an output buffer for the salt value.");
		return NULL;
	}

	// Attempt to write the random bytes into the output buffer.
	if (rand_write(PLACER(st_data_get(output), STACIE_SALT_LENGTH)) != STACIE_SALT_LENGTH) {
		log_pedantic("Failed to create a random salt value.");
		st_cleanup(result);
		return NULL;
	}

	st_length_set(output, STACIE_SALT_LENGTH);
	return output;
}

/**
 * @brief	Generates a random nonce of precisely 128 bytes, suitable for use with the STACIE authentication scheme.
 *
 * @note	While the salt and nonce creation functions are nearly identical, they remain seperated so they can use distinct
 * 			preprocessor definitions for the result length.
 *
 * @param	output	a managed string to receive the output; if passed as NULL, an output buffer will be allocated.
 *
 * @return	A managed string holding the freshly generated nonce in binary form. If NULL was passed in then the result must be freed by the caller.
 */
stringer_t * stacie_create_nonce(stringer_t *output) {

	stringer_t *result = NULL;

	// If an output buffer was passed in make sure it's a valid destination with length tracking.
	if (output && (!st_valid_destination(st_opt_get(output)) || !st_valid_tracked(st_opt_get(output)) || st_avail_get(output) < STACIE_NONCE_LENGTH)) {
		log_pedantic("An output string was supplied but is not a valid destination for the nonce value.");
		return NULL;
	}
	// Otherwise allocate an output buffer.
	else if (!output && !(output = result = st_alloc(STACIE_NONCE_LENGTH))) {
		log_pedantic("Failed to allocate an output buffer for the nonce value.");
		return NULL;
	}

	// Attempt to write the random bytes into the output buffer.
	if (rand_write(PLACER(st_data_get(output), STACIE_NONCE_LENGTH)) != STACIE_NONCE_LENGTH) {
		log_pedantic("Failed to create a random nonce value.");
		st_cleanup(result);
		return NULL;
	}

	st_length_set(output, STACIE_NONCE_LENGTH);
	return output;
}
