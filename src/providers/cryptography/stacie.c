
/**
 * @file /magma/providers/cryptography/stacie.c
 *
 * @brief These functions implement the Safely Turning Authentication Credentials Into Entropy (STACIE)
 * 		specification. These functions implement the individual steps required for key and token derivation.
 * 		The inputs for these functions should already processed and normalized for a deterministic
 * 		output.
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
stringer_t * stacie_shard_create(stringer_t *output) {

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
stringer_t * stacie_salt_create(stringer_t *output) {

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
stringer_t * stacie_nonce_create(stringer_t *output) {

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

/**
 * @brief   Calculate the number of hash rounds needed for the seed and key derivation stages.
 *
 * @note	This function has an effective maximum of 16,777,216 because STACIE requires the current round be appended
 *		to the string being hashed. This value is supplied as an unsigned 24 bit big endian integer. THe prescribed maximum
 *		is derived from the fact that the total number of possible values for an unsigned 24 bit integer is 16,777,216.
 *		This includes 0, since the values being appended by the derivation functions start at 0.
 *
 * @remarks	While on the subject of a maximum value, if 16,777,216 hash rounds doesn't provide sufficient protection then the
 *		the problem is likely the password, or the hash function, and not the number of rounds.
 *
 * @param   password	A password which may contain any valid Unicode character (presumably encoded using UTF-8).
 * @param   bonus	The number of additional rounds which should be added beyond the number of dynamic rounds calculated
 * 		using the password length.
 *
 * @return  Valid passwords will return a value between 8 (the prescribed minimum) and 16,777,216 (the prescribed
 * 		maximum). If an error occurs, then 0 will be returned.
 */
uint32_t stacie_rounds_calculate(stringer_t *password, uint32_t bonus) {

	ssize_t len = 0;
	uint64_t dynamic = 0;
	uint32_t exponent = 0;

	// The number of bytes won't help us here. Many foreign characters require 2 or 3 bytes, so using the length
	// in bytes would mean non-English passwords wouldn't receive the appropriate level of protection. What we really
	// is the number of Unicode characters (or codepoints), which is what this function is supposed to return.
	if (!(len = utf8_length_st(password))) {
		log_pedantic("The password appears to be empty, or is an invalid UTF8 string.");
		return 0;
	}

	// Calculate the number of rounds based on password length. These make up the "dynamic" portion of the total, since
	// the number will depend on the password. The dynamic number of rounds is 2^X, and X is derived by subtracting
	// the number of characters in the password from constant 24. The exponent value has a floor of 1, so passwords with
	// 23 characters and above will all result in an exponent value of 1, which will result in 2 dynamic rounds. This next
	// line ensures the exponent follows these rules.
	exponent = (24 - uint64_clamp(1, 23, len));

	// The more traditional method of writing this would be pow(2, exponent), but to avoid the inclusion of a math
	// library, we use this simple bit shift operation. Each shift operation will double the result, assuming this is a
	// little endian system.
	dynamic = (0x00000001 << exponent);

	// Once we have the number of dynamic rounds, we add the static number of bonus rounds. If the sum is below 8, because
	// the password was sufficiently long, and the number of bonus rounds sufficiently low, then this next line will set the
	// number of rounds to our prescribed minimum value of 8. If the sum is greater than the prescribed maximum, we will
	// reduce the value to 16,777,216. Since there is no sanity check above on the bonus value passed in, the 64 bit
	// variation of the clamp function is used to avoid being tricked into an overflow situation. Magma should never pass
	// in an illegal value, but we protect against overflows just in case somebody decides to copy this code without
	// realizing they should be checking their values higher up the stack.
	return (uint32_t)uint64_clamp(STACIE_KEY_ROUNDS_MIN, STACIE_KEY_ROUNDS_MAX, (dynamic + bonus));
}

/**
 * @brief	Concentrates and then extracts the random entropy provided by the password into a seed value for the first hash stage.
 *
 * @note	This implementation requires a seed value of 128 bytes because it was written specifically for Magma. However,
 *		the STACIE specification does technically allow the salt value to be empty, or of a different length, and provides
 *		additional logic in those circumstances. Those additional rules have not been implemented, so if this implementation
 *		does not receive a salt value that is exactly 128 bytes in length, it will return NULL to indicate the error.
 *
 * @param	rounds      The derived number of rounds, based on the password length, bonus rounds, and applicable limits.
 * @param	password    The pre-processed password, encoded in UTF8 and normalized using the canonical composition form (aka NFC).
 * @param	salt        The user specific salt, which provides protection against precomputed lookup tables. The salt
 * 		must be 128 bytes in length.
 *
 * @return	provides a managed string with the entropy seed value stored in a secure memory buffer, or NULL if an error occurs. The
 * 		length of the return value depends on the HMAC function being used. Magma currently uses an HMAC function based around
 * 		SHA-512, which results in the output being exactly 64 bytes, at least it was 64 bytes every time I bothered to look.
 */
stringer_t * stacie_entropy_seed_derive(uint32_t rounds, stringer_t *password, stringer_t *salt) {

	HMAC_CTX ctx;
	uint_t seed_len = 64;
	stringer_t *seed = NULL;
	size_t salt_len, password_len;
	uchr_t *salt_data, *password_data;
	const EVP_MD *digest = EVP_sha512_d();

	// Sanity check the rounds value.
	if (rounds < STACIE_KEY_ROUNDS_MIN || rounds > STACIE_KEY_ROUNDS_MAX) {
		log_error("An invalid value for rounds was passed to the STACIE seed derivation function. { rounds = %u }", rounds);
		return NULL;
	}
	// Ensure the digest pointer was returned correctly. If this fails, odds are OpenSSL wasn't initialized properly, or
	// was compiled without support for the cryptographic hash primitive we require.
	else if (!digest) {
		log_error("The STACIE seed derivation failed because the HMAC message digest function wasn't available.");
		return NULL;
	}
	// What's the point of going any further if the password is empty?
	else if (st_empty_out(password, &password_data, &password_len)) {
		log_pedantic("The STACIE seed derivation failed because the password was empty.");
		return NULL;
	}
	// This implementation requires the salt value to be  bytes in length.
	else if (st_empty_out(salt, &salt_data, &salt_len) || salt_len != STACIE_SALT_LENGTH) {
		log_pedantic("The STACIE seed derivation failed because the salt wasn't 128 bytes in length.");
		return NULL;
	}

	// Allocate a secure buffer to hold the HMAC output, aka the seed value this function returns.
	if (!(seed = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), 64))) {
		log_error("The STACIE seed derivation failed because a secure memory buffer could not be allocated to hold the result.");
		return NULL;
	}

	// Initialize the HMAC context, then initialize the key using the salt.
	HMAC_CTX_init_d(&ctx);

	if (HMAC_Init_ex_d(&ctx, salt_data, salt_len, digest, NULL) != 1) {
		log_error("The STACIE seed derivation failed because the HMAC function didn't initialize properly. {%s}",
			ssl_error_string(MEMORYBUF(256), 256));
		HMAC_CTX_cleanup_d(&ctx);
		st_cleanup(seed);
		return NULL;
	}

	// Update the HMAC context using the password. The password is repeated successively according to the rounds variable.
	for (uint32_t count = 0; count < rounds; count++) {
		if (HMAC_Update_d(&ctx, password_data, password_len) != 1) {
			log_error("The STACIE seed derivation failed because the HMAC function context couldn't be updated. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			HMAC_CTX_cleanup_d(&ctx);
			st_cleanup(seed);
			return NULL;
		}
	}

	// Finalize the HMAC context and retrieve the digest result as our seed value.
	if (HMAC_Final_d(&ctx, st_data_get(seed), &seed_len) != 1 || seed_len != 64) {
		log_error("Failed HMAC_Final_d(). {%s}", ssl_error_string(MEMORYBUF(256), 256));
		HMAC_CTX_cleanup_d(&ctx);
		st_cleanup(seed);
		return NULL;
	}

	// Set the stringer length.
	HMAC_CTX_cleanup_d(&ctx);
	st_length_set(seed, 64);

	return seed;
}

/**
 * @brief   Derive the master key and password key values using the user credentials. The base value provided is dependent
 * 		upon which of the two keys is being derived. The result is obtained by applying the designated hash function
 * 		over the input values the appropriate number of times.
 *
 * @note	The "master key" and the "password key" are referred to as "keys" because deriving them values requires knowing
 *		"secret" information, or more specifically, the plain text password and the number of hash rounds being applied.
 *		The number of hash rounds is considered a "secret" because it requires knowledge of the plain text password to derive.
 *		If an attacker were able to discover the number of rounds they could, at least in theory, use that information to
 *		derive the length of a password. If you don't understand why an attacker knowing the length of a password might
 *		be considered a bad thing, then I pity you.
 *
 * @param	base	The initial entropy being applied to the key derivation process. For the master key derivation,
 * 		this should be the entropy seed value. For the password key derivation, this should be the master key value.
 * 		Because the base value is a created using the designated hash function, its length must match output size.
 * 		For Magma, which is using SHA-512, that means the base value must always be 64 bytes.
 * @param	username	The pre-processed username, encoded in UTF8. The rules for normalization and equivalence are different
 * 		for every domain/deployment. Depending on those rules, the username passed in may be just the local part, or it
 * 		may consist of the full email address, which would includes the separator	(usually the '@' symbol) and a domain
 * 		name (preferably a fully qualified domain).
 * @param	password	The pre-processed password, encoded in UTF8 and normalized using the canonical composition form (aka NFC).
 * @param	salt		The user specific salt, which provides protection against precomputed lookup tables. For Magma,
 * 		the salt value must always be 128 bytes in length; @see stacie_entropy_seed_derive() for more on the salt length.
 *
 * @return	provides a managed string with the derived key stored in a secure memory buffer, or NULL if an error occurs. The
 * 		length of the output depends on the hash function being used. Magma currently uses SHA-512, which will result in
 * 		the output being exactly 64 bytes, every single time.
 */
stringer_t * stacie_hashed_key_derive(stringer_t *base, uint32_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt) {

	EVP_MD_CTX ctx;
	uint_t key_len = 64;
	uint32_t count_data;
//	useconds_t delay = 0;
	stringer_t *key = NULL;
	size_t base_len, username_len, password_len, salt_len;
	uchr_t *key_data, *base_data, *username_data, *password_data, *salt_data;
	const EVP_MD *digest = EVP_sha512_d();

	// Sanity check the rounds value.
	if (rounds < STACIE_KEY_ROUNDS_MIN || rounds > STACIE_KEY_ROUNDS_MAX) {
		log_error("An invalid value for rounds was passed to The STACIE key derivation function. { rounds = %u }", rounds);
		return NULL;
	}
	// Ensure the digest pointer was returned correctly. If this fails, odds are OpenSSL wasn't initialized properly, or
	// was compiled without support for the cryptographic hash primitive we require.
	else if (!digest) {
		log_error("The STACIE key derivation failed because the hash function wasn't available.");
		return NULL;
	}
	// What's the point of going any further if the username or password passed in were empty?
	else if (st_empty_out(username, &username_data, &username_len) || st_empty_out(password, &password_data, &password_len)) {
		log_error("The STACIE key derivation failed because the username or password was empty. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}
	// This implementation requires the base value to be 64 bytes in length,
	else if (st_empty_out(base, &base_data, &base_len) || base_len != 64) {
		log_error("The STACIE key derivation failed because the base value wasn't 64 bytes in length. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}
	// And the salt value must be 128 bytes in length.
	else if (st_empty_out(salt, &salt_data, &salt_len) || salt_len != STACIE_SALT_LENGTH) {
		log_error("The STACIE key derivation failed because the salt value wasn't 128 bytes in length. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}

	// Allocate a secure buffer to hold the HMAC output, aka the seed value this function returns.
	else if (!(key = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), 64))) {
		log_error("The STACIE key derivation failed because a secure memory buffer could not be allocated to hold the result.");
		return NULL;
	}

	// We set the key length here since it's a fixed output size, and we're going to use the key as an input value
	// after the first round.
	if (!(key_data = st_data_get(key)) || (key_len = st_length_set(key, 64)) != 64) {
		st_cleanup(key);
		return NULL;
	}

	// Initialize the context. We only need to do this once.
	EVP_MD_CTX_init_d(&ctx);

	for (uint32_t count = 0; count < rounds; count++) {

		// Store the counter as a big endian number.
		count_data = htobe32(count);

		// Setup the digest algorithm.
		if (EVP_DigestInit_ex_d(&ctx, (const EVP_MD *)digest, NULL) != 1) {
			log_pedantic("The STACIE key derivation failed because an error occurred while trying to initialize the hash context. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}

		// Process the input data. Note that the key value, which holds the output from the previous round isn't used
		// during the first iteration.
		if ((count != 0 && EVP_DigestUpdate_d(&ctx, key_data, key_len) != 1) ||
			EVP_DigestUpdate_d(&ctx, base_data, base_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, username_data, username_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, salt_data, salt_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, password_data, password_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, ((uchr_t *)&count_data) + 1, 3) != 1) {
			log_pedantic("The STACIE key derivation failed because an error occurred while trying to process the input data. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}

		// Retrieve the hash output.
		else if (EVP_DigestFinal_d(&ctx, key_data, &key_len) != 1 || key_len != 64) {
			log_pedantic("The STACIE key derivation failed because an error occurred while trying to retrieve the hash result. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}

		// This logic is wrapped in an ifdef conditional to allow for compilation as a standalone module.
#ifdef MAGMA_ENGINE_STATUS_H

		// Every 1000 rounds we check the magma status.
		else if (count % 1000 == 0 && !status()) {
			log_pedantic("The STACIE key derivation has been aborted by a system shutdown.");
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}

		// To help mitigate the load generated by processing very short passwords, we sleep for a microsecond, with the number
		// gradually increasing, as the number of rounds increases. Unfortunately this means a password with a very large number
		// rounds may not finish in time to satisfy the user.
//		if (0xFFFF0000 & count) {
//			usleep(delay++);
//		}
#endif

	}

	// Cleanup.
	EVP_MD_CTX_cleanup_d(&ctx);

	return key;
}

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
 * 		the salt value must always be 128 bytes in length; @see stacie_entropy_seed_derive() for more on the salt length.
 * @param	nonce	When deriving an ephemeral login token, an additional single use, randomly generated value is supplied
 * 		as the nonce parameter. For Magma, this value must always be 128 bytes in length.
 *
 * @return	provides a managed string with the derived token stored in a secure memory buffer, or NULL if an error
 * 		occurs. The	length of the output depends on the hash function being used. Magma currently uses SHA-512, which will result in
 * 		the output being exactly 64 bytes, with a level of reliability that rivals death and taxes.
 */
stringer_t * stacie_hashed_token_derive(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce) {

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

/**
 * @brief   Derive the symmetric encryption keys applicable to a given realm.
 *
 * @note	It is important that implementations use unique shard's for every realm. This ensures that if a realm key
 * 		is ever compromised, the data in alternate realms will remain secure. Also, depending on the implementation,
 * 		the shard might only be stored on the server. This means if the client device is ever lost, a user can protect
 * 		the information on the device by changing their password. This ensures that any data residing on the lost device
 * 		will remain protected, even if the lost device holds the plain text of the former password or the stale master key.
 * 		This operational methodology won't work for everyone, as it also prevents data from being accessible offline.
 *
 * @param	master_key	The master key, which was derived using the user's password.
 * @param	realm	Realm human readable name for the category of data protected using the given symmetric key.
 * @param   shard	The shard is the realm specific portion of the key. The shard must be exactly 64 bytes in length.
 *
 * @return	provides a managed string with the realm specific key stored in a secure memory buffer, or NULL if an error
 * 		occurs. The	length of the output depends on the hash function being used. Magma currently uses SHA-512, which will
 * 		result in the output being exactly 64 bytes. The output will contain the cipher key, and the initialization vector,
 * 		which will need to parsed out.
 */
stringer_t * stacie_realm_key_derive(stringer_t *master_key, stringer_t *realm, stringer_t *shard) {

	EVP_MD_CTX ctx;
	uint_t hash_len = 64;
	stringer_t *result = NULL;
	size_t key_len, realm_len, shard_len;
	uchr_t *key_data, *realm_data, *shard_data, *hash_data = MEMORYBUF(64);
	const EVP_MD *digest = EVP_sha512_d();

	// Ensure the digest pointer was returned correctly. If this fails, odds are OpenSSL wasn't initialized properly, or
	// was compiled without support for the cryptographic hash primitive we require.
	if (!digest) {
		log_error("The STACIE realm key derivation failed because the hash function wasn't available.");
		return NULL;
	}

	// What's the point of going any further if the master key is empty?
	else if (st_empty_out(master_key, &key_data, &key_len) || key_len != STACIE_KEY_LENGTH) {
		log_error("The STACIE realm key derivation failed because the master key was empty.");
		return NULL;
	}

	// This implementation requires the realm label to be less than 16
	else if (st_empty_out(realm, &realm_data, &realm_len)) {
		log_error("The STACIE realm key derivation failed because the realm label was empty.");
		return NULL;
	}

	// And the shard value, which must also be 64 bytes in length.
	else if (st_empty_out(shard, &shard_data, &shard_len) || shard_len != 64) {
		log_error("The STACIE realm key derivation failed because the shard value was empty.");
		return NULL;
	}

	// Allocate a secure buffer to hold the HMAC output, aka the seed value this function returns.
	else if (!(result = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), 64))) {
		log_error("The STACIE realm key derivation failed because a secure memory buffer could not be allocated to hold the result.");
		return NULL;
	}

	// Initialize the context. We only need to do this once.
	EVP_MD_CTX_init_d(&ctx);

	// Setup the digest algorithm.
	if (EVP_DigestInit_ex_d(&ctx, (const EVP_MD *)digest, NULL) != 1) {
		log_pedantic("The STACIE realm key derivation failed because an error occurred while trying to initialize the hash context. {%s}",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_MD_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Process the input data.
	if (EVP_DigestUpdate_d(&ctx, key_data, key_len) != 1 ||
		EVP_DigestUpdate_d(&ctx, realm_data, realm_len) != 1 ||
		EVP_DigestUpdate_d(&ctx, shard_data, shard_len) != 1) {
		log_pedantic("The STACIE realm key derivation failed because an error occurred while trying to process the input data. {%s}",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_MD_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Retrieve the hash output.
	else if (EVP_DigestFinal_d(&ctx, hash_data, &hash_len) != 1 || hash_len != 64) {
		log_pedantic("The STACIE realm key derivation failed because an error occurred while trying to retrieve the hash result. {%s}",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_MD_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	EVP_MD_CTX_cleanup_d(&ctx);

	if (!st_xor(PLACER(hash_data, hash_len), shard, result)) {
		log_error("The STACIE realm key derivation failed because an error occurred while trying to perform the XOR operation.");
		st_cleanup(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Extract the symmetric encryption key from the realm key.
 *
 * @param	realm_key	The complete realm key, which holds the vector, tag and cipher key values.
 *
 * @return	provides a managed string with the symmetric encryption key stored in a secure memory buffer, or NULL if an error
 * 		occurs.
 */
stringer_t * stacie_realm_cipher_key(stringer_t *realm_key) {

	stringer_t *cipher_key = NULL;

	if (st_empty(realm_key) || st_length_get(realm_key) != 64) {
		log_error("The realm cipher key extraction failed because the realm key passed in wasn't valid.");
	}
	else if (!(cipher_key = st_dupe_opts(MANAGED_T | CONTIGUOUS | SECURE, PLACER(st_data_get(realm_key) + 32, 32)))) {
		log_error("The realm cipher key extraction failed because a secure memory buffer could not be allocated to hold the result.");
	}

	return cipher_key;
}

/**
 * @brief   Extract the vector key from the realm key.
 *
 * @param	realm_key	The complete realm key, which holds the vector, tag and cipher key values.
 *
 * @return  provides a managed string with the vector key stored in a secure memory buffer, or NULL if an error
 * 		occurs.
 */
stringer_t * stacie_realm_vector_key(stringer_t *realm_key) {

	stringer_t *vector_key = NULL;

	if (st_empty(realm_key) || st_length_get(realm_key) != 64) {
		log_error("The realm vector key extraction failed because the realm key passed in wasn't valid.");
	}
	else if (!(vector_key = st_dupe_opts(MANAGED_T | CONTIGUOUS | SECURE, PLACER(st_data_get(realm_key), 16)))) {
		log_error("The realm vector key extraction failed because a secure memory buffer could not be allocated to hold the result.");
	}

	return vector_key;
}

/**
 * @brief   Extract the tag key from the realm key.
 *
 * @param	realm_key	The complete realm key, which holds the vector, tag and cipher key values.
 *
 * @return  provides a managed string with the tag key stored in a secure memory buffer, or NULL if an error
 * 		occurs.
 */
stringer_t * stacie_realm_tag_key(stringer_t *realm_key) {

	stringer_t *tag_key = NULL;

	if (st_empty(realm_key) || st_length_get(realm_key) != 64) {
		log_error("The realm tag key extraction failed because the realm key passed in wasn't valid.");
	}
	else if (!(tag_key = st_dupe_opts(MANAGED_T | CONTIGUOUS | SECURE, PLACER(st_data_get(realm_key) + 16, 16)))) {
		log_error("The realm tag key extraction failed because a secure memory buffer could not be allocated to hold the result.");
	}

	return tag_key;
}

/**
 *
 * @brief
 *
 * @param serial
 * @param vector_key
 * @param tag_key
 * @param cipher_key
 * @param buffer
 *
 * @return
 */
stringer_t * stacie_realm_encrypt(uint16_t serial, stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer) {

	uint8_t pad = 0;
	uint16_t serial_be;
	size_t buffer_len;
	EVP_CIPHER_CTX ctx;
	uchr_t *buffer_data, tag[16];
	uint32_t buffer_len_be;
	int_t  out_len, avail_len, used_len = 0;
	stringer_t *vector = MANAGEDBUF(16), *vector_shard = MANAGEDBUF(16), *tag_shard = MANAGEDBUF(16), *output = NULL;

	if (st_length_get(vector_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte vector key. { vector_key_len = %zu }", st_length_get(vector_key));
		return NULL;
	}
	else if (st_length_get(tag_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte tag key. { tag_key_len = %zu }", st_length_get(tag_key));
		return NULL;
	}
	else if (st_length_get(cipher_key) != 32) {
		log_error("STACIE realm encryption requires a 32 byte cipher key. { cipher_key_len = %zu }", st_length_get(cipher_key));
		return NULL;
	}
	else if (st_empty_out(buffer, &buffer_data, &buffer_len) || buffer_len < STACIE_ENCRYPT_MIN || buffer_len > STACIE_ENCRYPT_MAX) {
		log_error("STACIE realm encryption requires an input buffer with a minimum of 1 byte and a maximum of 16,777,215 bytes. { buffer_len = %zu }",
			buffer_len);
		return NULL;
	}

	// Generate a random initialization vector shard which will be combined with the vector key.
	else if (rand_write(vector_shard) != 16 || !st_xor(vector_key, vector_shard, vector)) {
		log_error("The STACIE realm encryption failed because a random vector could not be generated.");
		return NULL;
	}

	// Calculate the number of padding bytes required.
	pad = STACIE_BLOCK_LENGTH - ((buffer_len + 4) % STACIE_BLOCK_LENGTH);
	avail_len = buffer_len + pad + 4;
	out_len = STACIE_ENVELOPE_LENGTH + avail_len;

	// Create the big endian representations of the serial number and the plain text length.
	serial_be = htobe16(serial);
	buffer_len_be = htobe32(buffer_len);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_EncryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Set the vector length.
	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to 16 bytes.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != 32) {
		log_pedantic("The encryption key length does not match the selected cipher key length. { key = %i / expected = %i }", 32, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Add the vector and key data.
	if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, st_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Allocate the output buffer.
	if (!(output = st_alloc(out_len))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the output buffer.", avail_len);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Write the plain text length into the cipher text as a 24 bit big endian integer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH, &avail_len, ((uchr_t *)&buffer_len_be) + 1, 3) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;

	// Write the pad length into the cipher text as an 8 bit integer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len, ((uchr_t *)&pad), 1) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;

	// Encrypt the plain text buffer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len, buffer_data, buffer_len) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;

	// Encrypt the padding bytes (if necessary).
	for (int_t i = 0; i < pad; i++) {
		if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len, ((uchr_t *)&pad), 1) != 1) {
			log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
			EVP_CIPHER_CTX_cleanup_d(&ctx);
			st_free(output);
			return NULL;
		}

		used_len += avail_len;
		avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;
	}

	if (EVP_EncryptFinal_ex_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len) != 1) {
		log_pedantic("An error occurred while trying to complete encryption process.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;

	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_GET_TAG, 16, &tag[0]) != 1 || st_xor(tag_key, PLACER(&tag[0], 16), tag_shard) != tag_shard) {
		log_pedantic("An error occurred while trying to retrieve the tag value.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	mm_move(st_data_get(output), ((uchr_t *)&serial_be), 2);
	mm_move(st_data_get(output) + 2, st_data_get(vector_shard), 16);
	mm_move(st_data_get(output) + 18, st_data_get(tag_shard), 16);
	st_length_set(output, STACIE_ENVELOPE_LENGTH + used_len);

	return output;
}

/**
 *
 * @brief
 *
 * @param vector_key
 * @param tag_key
 * @param cipher_key
 * @param buffer
 *
 * @return
 */
stringer_t * stacie_realm_decrypt(stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer) {

	uint8_t pad = 0;
	size_t buffer_len;
	EVP_CIPHER_CTX ctx;
	uchr_t *buffer_data;
	uint32_t buffer_len_be = 0;
	int_t  out_len, avail_len, used_len = 0;
	stringer_t *vector = MANAGEDBUF(16), *tag = MANAGEDBUF(16), *output = NULL;

	if (st_length_get(vector_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte vector key. { vector_key_len = %zu }", st_length_get(vector_key));
		return NULL;
	}
	else if (st_length_get(tag_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte tag key. { tag_key_len = %zu }", st_length_get(tag_key));
		return NULL;
	}
	else if (st_length_get(cipher_key) != 32) {
		log_error("STACIE realm encryption requires a 32 byte cipher key. { cipher_key_len = %zu }", st_length_get(cipher_key));
		return NULL;
	}
	else if (st_empty_out(buffer, &buffer_data, &buffer_len) || buffer_len < 54) {
		log_error("STACIE realm encryption generates a minimum of 54 bytes. { buffer_len = %zu }",
			buffer_len);
		return NULL;
	}

	st_xor(PLACER(buffer_data + 2, 16), vector_key, vector);
	st_xor(PLACER(buffer_data + 18, 16), tag_key, tag);

	out_len = avail_len = (buffer_len - STACIE_ENVELOPE_LENGTH);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_DecryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to 16 bytes.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != 32) {
		log_pedantic("The encryption key length does not match the selected cipher key length. { key = %i / expected = %i }", 32, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Add the vector and key data.
	if (EVP_DecryptInit_ex_d(&ctx, NULL, NULL, st_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	if (!(output = st_alloc(out_len))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the output buffer.", avail_len);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	if (EVP_DecryptUpdate_d(&ctx, st_data_get(output), &avail_len, buffer_data + STACIE_ENVELOPE_LENGTH, buffer_len - STACIE_ENVELOPE_LENGTH) != 1) {
		log_pedantic("An error occurred while trying to decrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - used_len;

	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_TAG, 16, st_data_get(tag)) != 1) {
		log_pedantic("An error occurred while trying to set the decryption tag value.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	if (EVP_DecryptFinal_ex_d(&ctx, st_data_get(output) + used_len, &avail_len) != 1) {
		log_pedantic("An error occurred while trying to complete decryption process. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	mm_move(((uchr_t *)&buffer_len_be) + 1, st_data_get(output), 3);
	mm_move((uchr_t *)&pad, st_data_get(output) + 3, 1);
	buffer_len = be32toh(buffer_len_be);

	if (((buffer_len + pad + 4) % 16) != 0 || (buffer_len + pad + 4) != (size_t)used_len) {
		log_pedantic("An invalid length or pad value was found inside the decrypted buffer.");
		st_free(output);
		return NULL;
	}

	buffer_data = st_data_get(output) + buffer_len + 4;
	for (int_t i = 0; i < pad; i++) {
		if (*buffer_data++ != pad) {
			log_pedantic("An invalid padding byte was found inside the decrypted buffer.");
			st_free(output);
			return NULL;
		}
	}

	mm_move(st_data_get(output), st_data_get(output) + 4, buffer_len);
	st_length_set(output, buffer_len);

	return output;
}

