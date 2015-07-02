/**
 * @file    /magma/providers/cryptography/hmac.c
 *
 * @brief   Functions used to create hmac's using specified digest functions.
 *
 * @Author  Ivan
 * $Date$
 * $Revision$
 * 7/1/2015: Kent. Refactored hmac_digest to have a single routine that does
 *          most of the work: hmac_multi_digest().  If called with a null
 *          output param, the output param is allocated and a recursive call
 *          back to hmac_multi_digest() is made.  The simpler routine:
 *          hmac_digest() is replaced with a call to hmac_multi_digest() with
 *          a rounds count of 1.
 */

#include "magma.h"

/**
 * @brief   hmac_multi_digest: perform an HMAC on a multi-concatenated input
			using the specified digest and key.
 * @param   rounds  The amount of times that input s should be self-concatenated to
					serve as the input of the digest.
 * @param   digest  Digest to be used with the HMAC.
 * @param   s       Input data.
 * @param   key     Key used in HMAC.
 * @param   output  Stringer containing buffer for output.
 * @return  Pointer to stringer with buffer containing HMAC. NULL on failure.
 */
stringer_t *
hmac_multi_digest (
	uint_t rounds,
	digest_t *digest,
	stringer_t *s,
	stringer_t *key,
	stringer_t *output)
{
	int_t olen;
	HMAC_CTX ctx;
	uint32_t opts;
	uint_t outlen;
	stringer_t *result;     // the return value
	int_t retval = 0;       // retval must be == 1 for successful return

	if (rounds < 1) {
		log_pedantic("rounds must be > 0");
		goto out;
	}

	if (digest == NULL) {
		log_pedantic("Digest algorithm is NULL");
		goto out;
	}

	olen = EVP_MD_size_d((const EVP_MD *)digest);
	if (olen < 1) {
		log_pedantic("EVP_MD_size_d() returned invalid size for digest algorithm");
		goto out;
	}

	if (st_empty(s)) {
		log_pedantic("Input stringer 's' is NULL or empty");
		goto out;
	}

	if (st_empty(key)) {
		log_pedantic("Key stringer 'key' is NULL or empty");
		goto out;
	}

	if (output == NULL)  {
		// allocate on the fly and reenter the routine
		output = st_alloc (olen);

		if (output == NULL) {
			log_error("st_alloc() failed for 'output'. {requested = %i}", olen);
			goto out;
		}

		/*
		 * Note: you can't return on the recursive call to hmac_multi_digest()
		 * because you've allocated the output stringer on the fly.  If
		 * you fail, you have to cleanup.
		 */
		result = hmac_multi_digest(rounds, digest, s, key, output);

		if (result == NULL) {
			log_error("hmac_multi_digest() failed");
			st_free(output);
			goto out;
		} else {
			retval = 1;
			goto out;
		}

#if 0
		/*
		 * Todo: IVAN! if olen is the size of the digest, and we've allocated a
		 * new buffer of the same size, why would we need to reset the length as
		 * is shown in your version of hmac, line 187?
		 */
		if (st_length_set(result, outlen) == 0)  {
			log_error("Failed st_length_set() on new output buffer");
			st_free(output);
			result = NULL;
			goto out;
		}
#endif
	}

    result = output;
	opts = *((uint32_t *)result);
	if (st_valid_destination(opts) == NULL) {
		log_error("failed st_valid_destination() on Stringer 'result'");
		goto out;
	}

	if ((st_valid_avail(opts) != NULL) && (st_avail_get(result) < olen)) {
		log_error("'result' stringer is too small. {avail = %zu / required = %i}",
                  st_valid_avail(opts) ? st_avail_get(result) : st_length_get(result), olen);
		goto out;
	}

	if ((st_valid_avail(opts) == NULL) && (st_length_get(result) < olen)) {
		log_error("'result' stringer is too small. {avail = %zu / required = %i}",
		          st_valid_avail(opts) ? st_avail_get(result) : st_length_get(result), olen);
		goto out;
	}


	HMAC_CTX_init_d(&ctx);

	retval = HMAC_Init_ex_d(&ctx, st_data_get(key), st_length_get(key), (const EVP_MD *)digest, NULL);
	if (retval != 1)  {
		log_error ("Failed HMAC_Init_ex_d(). {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		goto out_cleanup_ctx;
	}

	for (uint_t i = 0; i < rounds; i++) {
		retval = HMAC_Update_d(&ctx, st_data_get(s), st_length_get(s));
		if (retval != 1) {
			log_error ("Failed HMAC_Update_d(). {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
			goto out_cleanup_ctx;
		}
	}

	retval = HMAC_Final_d(&ctx, st_data_get(result), &outlen);
	if (retval != 1) {
		log_error ("Failed HMAC_Final_d(). {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		goto out_cleanup_ctx;
	}

	// IVAN, is this necessary??
	if (st_valid_tracked(opts)) {
        st_length_set (result, outlen);
    }

out_cleanup_ctx:
	HMAC_CTX_cleanup_d(&ctx);

out:                            // Error return
	if (retval != 1) {
		return NULL;
	}

	return result;
}   // hmac_multi_digest()

/**
 * @brief  hmac_digest: perform an HMAC using the specified digest and key.
 * @param  digest   Digest to be used with the HMAC.
 * @param  s        Input data.
 * @param  key      Key used in HMAC.
 * @param  output   Stringer containing buffer for output.
 * @return Pointer to stringer with buffer containing HMAC. NULL on failure.
 */
stringer_t *
hmac_digest (digest_t *digest, stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_multi_digest (1, digest, s, key, output);
}   // hmac_digest()

/**
 * @brief   Helper functions
 * @param   s       Input data.
 * @param   key     HMAC key.
 * @param   output  Stringer with HMAC
 * @return  Pointer to stringer with HMAC buffer. NULL on failure.
 */
stringer_t * hmac_md4(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_md4_d(), s, key, output);
}

stringer_t * hmac_md5(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_md5_d(), s, key, output);
}

stringer_t * hmac_sha(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha_d(), s, key, output);
}

stringer_t * hmac_sha1(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha1_d(), s, key, output);
}

stringer_t * hmac_sha224(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha224_d(), s, key, output);
}

stringer_t * hmac_sha256(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha256_d(), s, key, output);
}

stringer_t * hmac_sha384(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha384_d(), s, key, output);
}

stringer_t * hmac_sha512(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_sha512_d(), s, key, output);
}

stringer_t * hmac_ripemd160(stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_digest((digest_t *)EVP_ripemd160_d(), s, key, output);
}

/**
 * @brief   Multi-concatenated HMAC-SHA512
 * @param   rounds  Number of times input is self-concantenated.
 * @param   s       Input.
 * @param   key     HMAC key.
 * @param   output  Stringer with HMAC.
 * @return  Pointer to stringer with buffer containing HMAC, NULL on failure.
*/
stringer_t * hmac_multi_sha512(uint_t rounds, stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_multi_digest(rounds, (digest_t *)EVP_sha512_d(), s, key, output);
}

