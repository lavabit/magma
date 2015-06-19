/**
 * @file    /magma/providers/cryptography/hmac.c
 *
 * @brief   Functions used to create hmac's using specified digest functions.
 *
 * @Author	Ivan
 * $Date$
 * $Revision$
 * 6/4/2015:(Kent) Simplified hmac_digest routine by unloading multiple use of 
 *          the param "output".  Callers that pass in an "output" stringer call 
 *          the hmac_digest() routine directly as before.  Callers that pass in 
 *          a NULL "output" stringer are routed to a new routine called 
 *          hmac_digest_null_output() that grows an output buffer on the fly, 
 *          then calls the hmac_digest() routine to complete the work.
 */

#include "magma.h"

/**
 * @brief   Perform an HMAC_digest using the specified digest and key.  Lazy 
 *          implies this routine will allocate it's own output buffer.  Note, 
 *          the caller is required to free the resulting returned object if 
 *          the call is successful.
 * @Author	Kent
 * @see     hmac_digest()
 * @note    hmac_digest_null_output() wraps the call to hmac_digest allowing an 
 *          interface to the hmac_digest() call that mallocs an 'output' stringer
 *          on the fly instead of requiring the caller to pass one in to 
 *          hmac_digest().
 * @param   rounds  The amount of times that input s should be self-concatenated
 * @param   digest  Digest to be used with the HMAC.
 * @param   s       Input data.
 * @param   key     Key used in HMAC.
 * @return  Pointer to stringer with buffer containing HMAC. NULL on failure.
 */

//       1         2         3         4         5         6         7         8
//345678911234567892123456789312345678941234567895123456789612345678971234567890

stringer_t *
hmac_multi_digest_null_output (uint_t rounds, digest_t *digest, stringer_t *s, stringer_t *key)
{
	int_t olen;
	stringer_t *output, *result;

	// Sanity check *digest because we need to access the function for olen
    if (digest == NULL) {
        log_pedantic("Digest algorithm is NULL");
        return NULL;
    }

    if ((olen = EVP_MD_size_d((const EVP_MD *)digest)) < 1) {
        log_error("EVP_MD_size_d() returned invalid size for digest algorithm");
        return NULL;
    }

	// alloc the output stringer.
	if ((output = st_alloc (olen)) == NULL) {
		log_error("st_alloc() failed for 'output'. {requested = %i}", olen);
		return NULL;
	}

	if ((result = hmac_multi_digest(rounds, digest, s, key, output)) == NULL) {
		log_error("hmac_digest() failed");
		st_free(output);
	}

	/**
     * TODO (kent) we allocated a managed buffer and defined the olen in the 
	 * alloc call.  Is is necessary to set the length again?
	 *   if (result)
	 *   st_length_set(result, outlen);
	 */
	return result;
}   // hmac_digest_null_output()

/**
 * @brief	Perform an HMAC using the specified digest and key.
 * @param 	digest	Digest to be used with the HMAC.
 * @param	s		Input data.
 * @param 	key		Key used in HMAC.
 * @param 	output	Stringer containing buffer for output.  
 * @return	Pointer to stringer with buffer containing HMAC. NULL on failure.
 */
stringer_t *
hmac_digest (digest_t *digest, stringer_t *s, stringer_t *key, stringer_t *output)
{
	return hmac_multi_digest (1, digest, s, key, output);
}	// hmac_digest()

/**
 * @brief   Perform an HMAC on a multi-concatenated input using the specified 
			digest and key.
 * @param	rounds	The amount of times that input s should be self-concatenated to 
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
	stringer_t *result;
    int_t retval = 1;

	if (rounds < 1) {
		log_pedantic("rounds must be > 0");
		return NULL;
	}

	if (digest == NULL) {
		log_pedantic("Digest algorithm is NULL");
        return NULL;
    }

    olen = EVP_MD_size_d((const EVP_MD *)digest);
	if (olen < 1) {
		log_pedantic("EVP_MD_size_d() returned invalid size for digest algorithm");
		return NULL;
	}

	if (st_empty(s)) {
		log_pedantic("Input stringer 's' is NULL or empty");
		return NULL;
	}

	if (st_empty(key)) {
		log_pedantic("Key stringer 'key' is NULL or empty");
		return NULL;
	}

	if (output == NULL)  {
		return hmac_multi_digest_null_output(rounds, digest, s, key);
	}

	opts = *((uint32_t *)output);
	if (!st_valid_destination(opts)) {
		log_error("Output Stringer 'output' failed st_valid_destination()");
		return NULL;
	}

	if (st_valid_avail(opts) && st_avail_get(output) < olen) {
		log_error("'output' stringer is too small. {avail = %zu / required = %i}",
		          st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), olen);
		return NULL;
	}

	if (!st_valid_avail(opts) && st_length_get(output) < olen) {
		log_error("'output' stringer is too small. {avail = %zu / required = %i}",
		          st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), olen);
		return NULL;
	}


	HMAC_CTX_init_d(&ctx);

	if ((retval = HMAC_Init_ex_d(&ctx, st_data_get(key), st_length_get(key), 
                                 (const EVP_MD *)digest, NULL)) != 1) {
		log_error ("Failed HMAC_Init_ex_d(). {%s}",
                   ERR_error_string_d(ERR_get_error_d(), 
                   NULL));
		goto hmac_multi_digest_out_1;
	} 

	for (uint_t i = 0; i < rounds; i++) {
		if ((retval = HMAC_Update_d(&ctx, st_data_get(s), st_length_get(s))) != 1) {
			log_error ("Failed HMAC_Update_d(). {%s}", 
                       ERR_error_string_d(ERR_get_error_d(), NULL));
			goto hmac_multi_digest_out_1;
		}
	} 

	if ((retval = HMAC_Final_d(&ctx, st_data_get(result), &outlen)) != 1) {
		log_error ("Failed HMAC_Final_d(). {%s}", 
                   ERR_error_string_d(ERR_get_error_d(), NULL));
		goto hmac_multi_digest_out_1;
    }

hmac_multi_digest_out_1:
	HMAC_CTX_cleanup_d(&ctx);

	if (retval != 1) {
		return NULL;		// Error return
	}

	if (st_valid_tracked(opts)) {
		st_length_set (output, outlen);
	}

	return output;
}	// hmac_multi_digest()

/**
 * @brief	Helper functions
 * @param	s		Input data.
 * @param	key		HMAC key.
 * @param 	output	Stringer with HMAC
 * @return	Pointer to stringer with HMAC buffer. NULL on failure.
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
 * @brief	Multi-concatenated HMAC-SHA512
 * @param	rounds		Number of times input is self-concantenated.
 * @param	s		Input.
 * @param	key		HMAC key.
 * @param	output		Stringer with HMAC.
 * @return	Pointer to stringer with buffer containing HMAC, NULL on failure.
*/
stringer_t * hmac_multi_sha512(uint_t rounds, stringer_t *s, stringer_t *key, stringer_t *output) {
	return hmac_multi_digest(rounds, (digest_t *)EVP_sha512_d(), s, key, output);
}


