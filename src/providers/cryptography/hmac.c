
/**
 * @file /magma/providers/cryptography/hmac.c
 *
 * @brief   Functions used to create a Hashed Message Authentication Code (HMAC).
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief  Performs an HMAC using the specified digest and key.
 *
 * @param  digest   Digest to be used with the HMAC.
 * @param  s        Input data.
 * @param  key      Key used in HMAC.
 * @param  output   Stringer containing buffer for output.
 *
 * @return	The managed string containing the resulting HMAC or NULL if an error occurs.
 */
stringer_t * deprecated_hmac_digest(digest_t *digest, stringer_t *s, stringer_t *key, stringer_t *output) {

	int_t olen;
	uint_t rlen;
	uint32_t opts;
	HMAC_CTX hmac;
	stringer_t *result = NULL;

	// Ensure a digest pointer was passed in and that we can retrieve the output length.
	if (!digest || (olen = EVP_MD_size_d((const EVP_MD *)digest)) <= 0) {
		log_pedantic("The hash algorithm is missing or invalid.");
		return NULL;
	}
	else if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding a result.");
		return NULL;
	}
	else if (st_empty(s)) {
		log_pedantic("The input string does not appear to have any data ready for encoding. {%slen = %zu}", s ? "" : "s = NULL / ",	s ? st_length_get(s) : 0);
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	else if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < olen) || (!st_valid_avail(opts) && st_length_get(output) < olen))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %i}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), olen);
		return NULL;
	}
	else if (!output && !(result = st_alloc(olen))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %i}", olen);
		return NULL;
	}

	// Initialize the context.
	HMAC_CTX_init_d(&hmac);
	rlen = olen;

	// Setup the digest algorithm.
	if (HMAC_Init_ex_d(&hmac, st_data_get(key), st_length_get(key), (const EVP_MD *)digest, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize the hashed message authentication code context.");
		HMAC_CTX_cleanup_d(&hmac);
		if (!output) st_free(result);
		return NULL;
	}

	// Process the input data.
	else if (HMAC_Update_d(&hmac, st_data_get(s), st_length_get(s)) != 1) {
		log_pedantic("An error occurred while trying to process the input data.");
		HMAC_CTX_cleanup_d(&hmac);
		if (!output) st_free(result);
		return NULL;
	}

	// Retrieve the HMAC output.
	else if (HMAC_Final_d(&hmac, st_data_get(result), &rlen) != 1) {
		log_pedantic("An error occurred while trying to retrieve the hashed message authentication code result.");
		HMAC_CTX_cleanup_d(&hmac);
		if (!output) st_free(result);
		return NULL;
	}

	// Cleanup.
	HMAC_CTX_cleanup_d(&hmac);

	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, rlen);
	}
	return result;
}

stringer_t * deprecated_hmac_md4(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_md4_d(), s, key, output);
}

stringer_t * deprecated_hmac_md5(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_md5_d(), s, key, output);
}

stringer_t * deprecated_hmac_sha(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_sha_d(), s, key, output);
}

stringer_t * deprecated_hmac_sha1(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_sha1_d(), s, key, output);
}

stringer_t * deprecated_hmac_sha224(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_sha224_d(), s, key, output);
}

stringer_t * deprecated_hmac_sha256(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_sha256_d(), s, key, output);
}

stringer_t * deprecated_hmac_sha384(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_sha384_d(), s, key, output);
}

stringer_t * deprecated_hmac_sha512(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_sha512_d(), s, key, output);
}

stringer_t * deprecated_hmac_ripemd160(stringer_t *s, stringer_t *key, stringer_t *output) {
	return deprecated_hmac_digest((digest_t *)EVP_ripemd160_d(), s, key, output);
}


