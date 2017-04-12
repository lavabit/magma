
/**
 * @file /magma/src/providers/stacie/tokens.c
 *
 * @brief Derive STACIE token values.
 */

#include "magma.h"

/**
 * @brief   Derive the token values using the supplied input values. The base value supplied depends on which token is
 * 		being derived.
 *
 * @note The token values are used for authentication with an untrusted server, which needs to authenticate a user,
 * 		but does not know anything about the plain text password. Untrusted servers store the static verification token,
 * 		and using this function, can verify ephemeral login tokens. Servers can also authenticate a password change request
 * 		by checking whether the supplied password key, if passed to this the token derivation function, matches the stored
 * 		verification token. The base value passed in is dependent upon which token is being derived. The result is obtained
 * 		by applying the designated hash function over the input values a fixed number of times.
 *
 * @param   base	The initial entropy being applied to the token derivation process. For the static verification
 * 		token, this will be the password key value. For an ephemeral login token, this will be the verification token.
 * 		Because the base value is a created using the designated hash function, its length must match output size.
 * 		For Magma, which is using SHA-512, that means the base value must always be 64 bytes.
 * @param	username	The pre-processed username, encoded in UTF8. The rules for normalization and equivalence are different
 * 		for every domain/deployment. Depending on those rules, the username passed in may be just the local part, or it
 * 		may consist of the full email address, which would includes the separator	(usually the '@' symbol) and a domain
 * 		name (preferably a fully qualified domain).
 * @param	salt	The user specific salt, which provides protection against precomputed lookup tables. For Magma,
 * 		the salt value must always be 128 bytes in length; @see stacie_derive_seed() for more on the salt length.
 * @param	nonce	When deriving an ephemeral login token, an additional single use, randomly generated value is supplied
 * 		as the nonce parameter. For Magma, this value must always be 128 bytes in length.
 *
 * @return	provides a managed string with the derived token stored in a secure memory buffer, or NULL if an error
 * 		occurs. The	length of the output depends on the hash function being used. Magma currently uses SHA-512, which will result in
 * 		the output being exactly 64 bytes, with a level of reliability that rivals death and taxes.
 */
stringer_t * stacie_derive_token(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce) {

	EVP_MD_CTX ctx;
	uint_t token_len = 64;
	uint32_t count_data;
	stringer_t *token = NULL;
	size_t base_len, username_len, salt_len, nonce_len = 0;
	uchr_t *token_data, *base_data, *username_data, *salt_data, *nonce_data = NULL;
	const EVP_MD *digest = EVP_sha512_d();

	// Ensure the digest pointer was returned correctly. If this fails, odds are OpenSSL wasn't initialized properly, or
	// was compiled without support for the cryptographic hash primitive we require.
	if (!digest) {
		log_error("The STACIE token derivation failed because the hash function wasn't available.");
		return NULL;
	}
	// What's the point of going any further if the username passed in were empty?
	else if (st_empty_out(username, &username_data, &username_len)) {
		log_error("The STACIE token derivation failed because the username was empty. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}
	// This implementation requires the base value to be 64 bytes in length,
	else if (st_empty_out(base, &base_data, &base_len) || base_len != 64) {
		log_error("The STACIE token derivation failed because the base value wasn't 64 bytes in length. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}
	// And the salt value must be 128 bytes in length.
	else if (st_empty_out(salt, &salt_data, &salt_len) || salt_len != STACIE_SALT_LENGTH) {
		log_error("The STACIE token derivation failed because the salt value wasn't 128 bytes in length. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}
	// Finally, we retrieve the length of the nonce value. The nonce value is allowed to be NULL, but if a value is supplied,
	// it must be 128 bytes in length.
	else if (!st_empty_out(nonce, &nonce_data, &nonce_len) && nonce_len != STACIE_NONCE_LENGTH) {
		log_error("The STACIE token derivation failed because a nonce value was provided that wasn't 128 bytes in length. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}

	// Allocate a secure buffer to hold the HMAC output, aka the seed value this function returns.
	else if (!(token = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), 64))) {
		log_error("The STACIE token derivation failed because a secure memory buffer could not be allocated to hold the result.");
		return NULL;
	}

	// We set the key length here since it's a fixed output size, and we're going to use the key an input value
	// after the first round.
	if (!(token_data = st_data_get(token)) || (token_len = st_length_set(token, 64)) != 64) {
		st_cleanup(token);
	}

	// Initialize the context. We only need to do this once.
	EVP_MD_CTX_init_d(&ctx);

	for (uint32_t count = 0; count < STACIE_TOKEN_ROUNDS; count++) {

		// Store the counter as a big endian number.
		count_data = htobe32(count);

		// Setup the digest algorithm.
		if (EVP_DigestInit_ex_d(&ctx, (const EVP_MD *)digest, NULL) != 1) {
			log_pedantic("The STACIE token derivation failed because an error occurred while trying to initialize the hash context. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(token);
			return NULL;
		}

		// Process the input data. Note that the token value, which holds the output from the previous round isn't used
		// during the first iteration, and the nonce value won't always be available.
		if ((count != 0 && EVP_DigestUpdate_d(&ctx, token_data, token_len) != 1) ||
			EVP_DigestUpdate_d(&ctx, base_data, base_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, username_data, username_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, salt_data, salt_len) != 1 ||
			(nonce_data && EVP_DigestUpdate_d(&ctx, nonce_data, nonce_len) != 1) ||
			EVP_DigestUpdate_d(&ctx, ((uchr_t *)&count_data) + 1, 3) != 1) {
			log_pedantic("The STACIE token derivation failed because an error occurred while trying to process the input data. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(token);
			return NULL;
		}

		// Retrieve the hash output.
		else if (EVP_DigestFinal_d(&ctx, token_data, &token_len) != 1 || token_len != 64) {
			log_pedantic("The STACIE token derivation failed because an error occurred while trying to retrieve the hash result. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(token);
			return NULL;
		}
	}

	// Cleanup.
	EVP_MD_CTX_cleanup_d(&ctx);

	return token;
}
