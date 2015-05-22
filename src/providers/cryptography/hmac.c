
/**
 * @file /magma/providers/cryptography/hmac.c
 *
 * @brief Functions used to create hmac's using specified digest functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Perform an HMAC using the specified digest and key.
 * @param 	digest	Digest to be used with the HMAC.
 * @param	s		Input data.
 * @param 	key		Key used in HMAC.
 * @param 	output	Stringer containing buffer for output.
 * @return	Pointer to stringer with buffer containing HMAC. NULL on failure.
 */
stringer_t * hmac_digest(digest_t *digest, stringer_t *s, stringer_t *key, stringer_t *output) {

	HMAC_CTX ctx;
	int_t olen;
	stringer_t *result;
	uint32_t opts;
	uint_t outlen;

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
	else if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < olen) ||
			(!st_valid_avail(opts) && st_length_get(output) < olen))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %i}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), olen);
		return NULL;
	}
	else if (!output && !(result = st_alloc(olen))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %i}", olen);
		return NULL;
	}

	HMAC_CTX_init_d(&ctx);

	if (HMAC_Init_ex_d(&ctx, st_data_get(key), st_length_get(key), (const EVP_MD *)digest, NULL) != 1) {
		log_info("Unable to generate a data authentication code. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		HMAC_CTX_cleanup_d(&ctx);

		if(!output) {
			st_free(result);
		}

		return NULL;
	}

	if (HMAC_Update_d(&ctx, st_data_get(s), st_length_get(s)) != 1) {
		log_info("Unable to generate a data authentication code. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		HMAC_CTX_cleanup_d(&ctx);

		if(!output) {
			st_free(result);
		}

		return NULL;
	}

	if (HMAC_Final_d(&ctx, st_data_get(output), &outlen) != 1) {
		log_info("Unable to generate a data authentication code. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		HMAC_CTX_cleanup_d(&ctx);

		if(!output) {
			st_free(result);
		}

		return NULL;
	}

	HMAC_CTX_cleanup_d(&ctx);

	return result;
}

/**
 * @brief	HMAC-MD4
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_md4(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_md4_d(), s, key, output);
}

/**
 * @brief	HMAC-MD5
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_md5(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_md5_d(), s, key, output);
}

/**
 * @brief	HMAC-SHA
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_sha(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha_d(), s, key, output);
}

/**
 * @brief	HMAC-SHA1
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_sha1(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha1_d(), s, key, output);
}

/**
 * @brief	HMAC-SHA224
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_sha224(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha224_d(), s, key, output);
}

/**
 * @brief	HMAC-SHA256
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_sha256(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha256_d(), s, key, output);
}

/**
 * @brief	HMAC-SHA384
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_sha384(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha384_d(), s, key, output);
}

/**
 * @brief	HMAC-SHA512
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_sha512(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha512_d(), s, key, output);
}

/**
 * @brief	HMAC-RIPEMD160
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_ripemd160(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_ripemd160_d(), s, key, output);
}




