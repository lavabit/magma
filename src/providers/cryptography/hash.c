
/**
 * @file /magma/providers/cryptography/hash.c
 *
 * @brief Functions used to create a secure one-way hash of an arbitrary input buffer.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

stringer_t * hash_digest(digest_t *digest, stringer_t *s, stringer_t *output) {

	int_t olen;
	uint_t rlen;
	uint32_t opts;
	EVP_MD_CTX ctx;
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
	EVP_MD_CTX_init_d(&ctx);
	rlen = olen;

	// Setup the digest algorithm.
	if (EVP_DigestInit_ex_d(&ctx, (const EVP_MD *)digest, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize the hash context. {%s}",	ERR_error_string_d(ERR_get_error_d(), NULL));
		EVP_MD_CTX_cleanup_d(&ctx);
		if (!output) {
			st_free(result);
		}
		return NULL;
	}

	// Process the input data.
	else if (EVP_DigestUpdate_d(&ctx, st_data_get(s), st_length_get(s)) != 1) {
		log_pedantic("An error occurred while trying to process the input data. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		EVP_MD_CTX_cleanup_d(&ctx);
		if (!output) {
			st_free(result);
		}
		return NULL;
	}

	// Retrieve the hash output.
	else if (EVP_DigestFinal_d(&ctx, st_data_get(result), &rlen) != 1) {
		log_pedantic("An error occurred while trying to retrieve the hash result. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		EVP_MD_CTX_cleanup_d(&ctx);
		if (!output) {
			st_free(result);
		}
		return NULL;
	}

	// Cleanup.
	EVP_MD_CTX_cleanup_d(&ctx);

	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, rlen);
	}
	return result;
}

stringer_t * hash_md4(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_md4_d(), s, output);
}

stringer_t * hash_md5(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_md5_d(), s, output);
}

stringer_t * hash_sha(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_sha_d(), s, output);
}

stringer_t * hash_sha1(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_sha1_d(), s, output);
}

stringer_t * hash_sha224(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_sha224_d(), s, output);
}

stringer_t * hash_sha256(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_sha256_d(), s, output);
}

stringer_t * hash_sha384(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_sha384_d(), s, output);
}

stringer_t * hash_sha512(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_sha512_d(), s, output);
}

stringer_t * hash_ripemd160(stringer_t *s, stringer_t *output) {
	return hash_digest((digest_t *)EVP_ripemd160_d(), s, output);
}



