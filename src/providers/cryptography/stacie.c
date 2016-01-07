/**
 * @file /magma/providers/cryptography/stacie.c
 *
 * @brief These functions implement the Safely Turning Authentication Credentials Into Entropy (STACIE)
 * 		specification. These functions implement the individual steps required for key and token derivation.
 * 		The inputs for these functions should already processed and normalized for a deterministic
 * 		output.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief   Calculate the number of hash rounds needed for the seed and key derivation stages.
 *
 * @param   password	A password which may contain any valid Unicode character (presumably encoded using UTF-8).
 * @param   bonus	The number of additional rounds which should be added beyond the number of base rounds calculated
 * 		using the password length.
 *
 * @return  Valid passwords will return a value between 8 (the prescribed minimum) and 16,777,216 (the maximum possible
 * 		values for an unsigned 24 bit integer, if you include 0). If an error occurs then 0 is returned.
 */
uint32_t stacie_rounds_calculate(stringer_t *password, uint32_t bonus) {

	ssize_t len = 0;
	uint64_t base = 0;
	uint32_t rounds = 0, exponent = 0;

	if (!(len = utf8_length_st(password))) {
		log_pedantic("The password appears to be empty, or is an invalid UTF8 string.");
		return 0;
	}

	// Calculate the number of base rounds based on length. Add 2^X rounds, where X equals the number
	// of characters fewer than 24. For passwords of 24 characters and higher, use an exponent value of 1.
	exponent = (24 - uint64_clamp(1, 23, len));

	// Each exponent shift operation is the equivalent of raising the value by power of 2.
	base = (0x00000001 << exponent);

	// Add the bonus rounds to our base rounds. If the value is below 8 this will raise the total to the minimum. If the
	// value is over the max, it will be reduced. The result is clamped to a value between 8 and 16,777,216.
	rounds = uint64_clamp(STACIE_ROUNDS_MIN, STACIE_ROUNDS_MAX, (base + bonus));
	return rounds;
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
 * @return	provides a managed string with the entropy seed value stored in a secure memory buffer, or NULL if an error occurs.
 */
stringer_t * stacie_entropy_seed_derive(uint32_t rounds, stringer_t *password, stringer_t *salt) {

	HMAC_CTX ctx;
	uint_t seed_len = 64;
	stringer_t *seed = NULL;
	size_t salt_len, password_len;
	uchr_t *salt_data, *password_data;
	const EVP_MD *digest = EVP_sha512_d();

	// Sanity check the rounds value.
	if (rounds < STACIE_ROUNDS_MIN || rounds > STACIE_ROUNDS_MAX) {
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
	// This implementation requires the salt value to be 128 bytes in length.
	else if (st_empty_out(salt, &salt_data, &salt_len) || salt_len != 128) {
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
			ERR_error_string_d(ERR_get_error_d(), NULL));
		HMAC_CTX_cleanup_d(&ctx);
		st_cleanup(seed);
		return NULL;
	}

	// Update the HMAC context using the password. The password is repeated successively according to the rounds variable.
	for (uint32_t count = 0; count < rounds; count++) {
		if (HMAC_Update_d(&ctx, password_data, password_len) != 1) {
			log_error("The STACIE seed derivation failed because the HMAC function context couldn't be updated. {%s}",
				ERR_error_string_d(ERR_get_error_d(), NULL));
			HMAC_CTX_cleanup_d(&ctx);
			st_cleanup(seed);
			return NULL;
		}
	}

	// Finalize the HMAC context and retrieve the digest result as our seed value.
	if (HMAC_Final_d(&ctx, st_data_get(seed), &seed_len) != 1 || seed_len != 64) {
		log_error("Failed HMAC_Final_d(). {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
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
 * @brief   Derive the hashed key from a seed and user credentials.
 *
 * @param	base	Entropy seed for master key derivation, and the master key for the password key derivation, both values
 * 		must be 64 bytes in length.
 * @param	username	The pre-processed username, encoded in UTF8. The rules for normalization and equivalence are different
 * 		for every domain/deployment. Depending on those rules, the username passed in may be just the local part,
 * 		or it may consist of the full email address, which would includes the separator	(usually the '@' symbol)
 * 		and a domain name (preferably a fully qualified domain).
 * @param	password	The pre-processed password, encoded in UTF8 and normalized using the canonical composition form (aka NFC).
 * @param	salt		The user specific salt, which provides protection against precomputed lookup tables. The salt value
 * 		must be 128 bytes in length.
 *
 * @return	provides a managed string with the derived key value stored in a secure memory buffer, or NULL if an error occurs.
 */
stringer_t * stacie_hashed_key_derive(stringer_t *base, uint32_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt) {

	EVP_MD_CTX ctx;
	uint_t key_len = 64;
	uint32_t count_data;
	stringer_t *key = NULL;
	size_t username_len, password_len, salt_len, base_len;
	uchr_t *key_data, *username_data, *password_data, *salt_data, *base_data;
	const EVP_MD *digest = EVP_sha512_d();

	// Sanity check the rounds value.
	if (rounds < STACIE_ROUNDS_MIN || rounds > STACIE_ROUNDS_MAX) {
		log_error("An invalid value for rounds was passed to the STACIE hashed key derivation function. { rounds = %u }", rounds);
		return NULL;
	}
	// Ensure the digest pointer was returned correctly. If this fails, odds are OpenSSL wasn't initialized properly, or
	// was compiled without support for the cryptographic hash primitive we require.
	else if (!digest) {
		log_error("The STACIE hashed key derivation failed because the hash function wasn't available.");
		return NULL;
	}
	// What's the point of going any further if the username or password passed in were empty?
	else if (st_empty_out(username, &username_data, &username_len) || st_empty_out(password, &password_data, &password_len)) {
		log_error("The STACIE hashed key derivation failed because the username or password was empty. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return NULL;
	}
	// This implementation requires the base value to be 64 bytes in length,
	else if (st_empty_out(base, &base_data, &base_len) || base_len != 64) {
		log_error("The STACIE hashed key derivation failed because the seed wasn't 64 bytes in length.");
		return NULL;
	}
	// And the salt value to be 128 bytes in length.
	else if (st_empty_out(salt, &salt_data, &salt_len) || salt_len != 128) {
		log_error("The STACIE hashed key derivation failed because the salt wasn't 128 bytes in length.");
		return NULL;
	}

	// Allocate a secure buffer to hold the HMAC output, aka the seed value this function returns.
	else if (!(key = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), 64))) {
		log_error("The STACIE hashed key derivation failed because a secure memory buffer could not be allocated to hold the result.");
		return NULL;
	}

	// We set the key length here since it's a fixed output size, and we're going to use the key an input value
	// after the first round.
	if (!(key_data = st_data_get(key)) || (key_len = st_length_set(key, 64)) != 64) {
		st_cleanup(key);
	}

	// Initialize the context. We only need to do this once.
	EVP_MD_CTX_init_d(&ctx);

	for (uint32_t count = 0; count < rounds; count++) {

		// Store the counter as a big endian number.
		count_data = htobe32(count);

		// Setup the digest algorithm.
		if (EVP_DigestInit_ex_d(&ctx, (const EVP_MD *)digest, NULL) != 1) {
			log_pedantic("The STACIE hashed key derivation failed because an error occurred while trying to initialize the hash context. {%s}",
			ERR_error_string_d(ERR_get_error_d(), NULL));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}

		// Process the input data.
		if ((count != 0 && EVP_DigestUpdate_d(&ctx, key_data, key_len) != 1) ||
			EVP_DigestUpdate_d(&ctx, base_data, base_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, username_data, username_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, salt_data, salt_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, password_data, password_len) != 1 ||
			EVP_DigestUpdate_d(&ctx, ((uchr_t *)&count_data) + 1, 3) != 1) {
			log_pedantic("The STACIE hashed key derivation failed because an error occurred while trying to process the input data. {%s}",
			ERR_error_string_d(ERR_get_error_d(), NULL));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}

		// Retrieve the hash output.
		else if (EVP_DigestFinal_d(&ctx, st_data_get(key), &key_len) != 1 || key_len != 64) {
			log_pedantic("The STACIE hashed key derivation failed because an error occurred while trying to retrieve the hash result. {%s}",
			ERR_error_string_d(ERR_get_error_d(), NULL));
			EVP_MD_CTX_cleanup_d(&ctx);
			st_cleanup(key);
			return NULL;
		}
	}

	// Cleanup.
	EVP_MD_CTX_cleanup_d(&ctx);

	return key;
}

/**
 * @brief   Derive the hashed token using derived values.
 *
 * @param   base        The base input that will be hashed into the token, either a password_key or verification token.
 * @param   username    Username stringer.
 * @param   salt        User-specific salt.
 * @param   nonce       Token-specific nonce.
 * @return  Stringer containing the hashed token.
 *
 * Note: both the salt and nonce parameters are allowed to be NULL or
 * zero length stringers here.  Test coverage exists for these cases
 * in the check tests.
 */
stringer_t * stacie_hashed_token_derive(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce) {

	stringer_t *hashed_token;
	stringer_t *hash_input;
	stringer_t *count;

	if (st_empty(base) || (st_length_get(base) != 64)) {
		log_pedantic("base is NULL, empty or length != 64");
		goto error;
	}

	if (st_empty(username)) {
		log_pedantic("username is NULL or empty");
		goto error;
	}

	// if non-null, salt len must be >= 64
	size_t salt_len = 0;
	if (!st_empty(salt)) {
		if ((salt_len = st_length_get(salt)) < 64) {
			log_pedantic("salt is NULL, empty or length != 64");
			goto error;
		}
	}

	size_t nonce_len = 0;
	if (!st_empty(nonce)) {
		if ((nonce_len = st_length_get(nonce)) < 64) {
			log_pedantic("nonce is NULL, empty or length != 64");
			goto error;
		}
	}

	hashed_token = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (hashed_token == NULL) {
		log_error("st_alloc_opts() failed");
		goto error;
	}

	size_t input_len;
	input_len = 64;
	input_len += st_length_get(base);
	input_len += st_length_get(username);
	input_len += salt_len;
	input_len += nonce_len;
	input_len += 3;

	hash_input = st_alloc_opts((MANAGED_T | JOINTED | SECURE), input_len);
	if (hash_input == NULL) {
		log_error("st_alloc_opts() failed");
		goto cleanup_hashed_token;
	}

	for (uint_t i = 0; i < STACIE_ROUNDS_MIN; i++) {
		st_wipe(hash_input); // reset the temp hash_input

		// Build hash_input by starting with the value for hashed_token
		st_append(hash_input, hashed_token);
		st_wipe(hashed_token); // why?

		st_append(hash_input, base);
		st_append(hash_input, username);

		if (salt_len != 0) {
			st_append(hash_input, salt);
		}

		if (nonce_len != 0) {
			st_append(hash_input, nonce);
		}

		count = uint24_put_no(i);
		if (count == NULL) {
			log_error("uint24_put_no() failed");
			goto cleanup_hash_input;
		}

		if (st_append(hash_input, count) == NULL) {
			log_error("st_append() failed");
			goto cleanup_count;
		}
		st_free(count);

		// perform digest and store the result into hashed_token
		if (hash_sha512(hash_input, hashed_token) == NULL) {
			log_error("hash_sha512() failed");
			goto cleanup_hash_input;
		}
	}

	st_free(hash_input);

	return hashed_token;

	cleanup_count: st_free(count);
	cleanup_hash_input: st_free(hash_input);
	cleanup_hashed_token: st_free(hashed_token);
	error: return NULL;
}

/**
 * @brief   Derive the realm key used to decrypt keys for realm-specific user information.
 *
 * @param   master_key  Stringer containing master key derived from user password.
 * @param   realm       Realm name.
 * @param   shard       the shard serves as a realm-specific salt.
 * @return  Stringer containing the realm key.
 */
stringer_t * stacie_realm_key_derive(stringer_t *master_key, stringer_t *realm, stringer_t *shard) {

	stringer_t *hash_input = NULL;
	stringer_t *hash_output = NULL;
	stringer_t *realm_key = NULL;

	if (st_empty(master_key) || (st_length_get(master_key) != 64)) {
		log_pedantic("An empty or invalid master key was passed in.");
		goto error;
	}

	if (st_empty(realm)) {
		log_pedantic("An empty realm was passed in.");
		goto error;
	}

	if (st_empty(shard) || (st_length_get(shard) != 64)) {
		log_pedantic("shard is NULL, empty or length != 64");
		goto error;
	}

	size_t input_len = 0;
	input_len += st_length_get(master_key);
	input_len += st_length_get(realm);
	input_len += st_length_get(shard);

	hash_input = st_alloc_opts((MANAGED_T | JOINTED | SECURE), input_len);
	if (hash_input == NULL) {
		log_error("st_alloc_opts() failed");
		goto error;
	}

	hash_output = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (hash_output == NULL) {
		log_error("st_alloc_opts() failed");
		goto cleanup_hash_input;
	}

	realm_key = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (realm_key == NULL) {
		log_error("st_alloc_opts() failed");
		goto cleanup_hash_output;
	}

	st_append(hash_input, master_key);
	st_append(hash_input, realm);

	if (st_append(hash_input, shard) == NULL) {
		log_error("st_append() failed");
		goto cleanup_realm_key;
	}

	if (hash_sha512(hash_input, hash_output) == NULL) {
		log_error("hash_sha512() failed");
		goto cleanup_realm_key;
	}

	if (st_xor(hash_output, shard, realm_key) == NULL) {
		log_error("st_xor() failed");
		goto cleanup_realm_key;
	}

	st_free(hash_input);
	st_free(hash_output);

	return realm_key;

	cleanup_realm_key: st_free(realm_key);
	cleanup_hash_output: st_free(hash_output);
	cleanup_hash_input: st_free(hash_input);
	error: return NULL;
}

/**
 * @brief   Derive the encryption key used to decrypt realm-specific key.
 *
 * @param   realm_key  Stringer containing the realm key.
 * @return  Encryption key.
 */
stringer_t * stacie_realm_cipher_key_derive(stringer_t *realm_key) {

	stringer_t *pl;
	stringer_t *realm_cipher_key;

	if (st_empty(realm_key) || (st_length_get(realm_key) != 64)) {
		log_pedantic("Realm key is zero, NULL or length != 64");
		goto error;
	}

	pl = PLACER(st_data_get(realm_key) + 32, 32);
	if (pl == NULL) {
		log_error("PLACER set in realm_key failed");
		goto error;
	}

	realm_cipher_key = st_dupe_opts((MANAGED_T | CONTIGUOUS | SECURE), pl);
	if (realm_cipher_key == NULL) {
		log_error("st_dupe_opts() failed");
		goto error;
	}

	return realm_cipher_key;

	error: return NULL;
}

/**
 * @brief   Derive the initialization vector used to decrypt realm-specific key.
 *
 * @param   realm_key  Stringer containing the realm key.
 * @return  Initialization vector.
 */
stringer_t * stacie_realm_init_vector_derive(stringer_t *realm_key) {

	stringer_t *pl1;
	stringer_t *pl2;
	stringer_t *init_vector;

	if (st_empty(realm_key) || (st_length_get(realm_key) != 64)) {
		log_pedantic("Realm key is zero, NULL or length != 64");
		goto error;
	}

	pl1 = PLACER(st_data_get(realm_key), 16);
	pl2 = PLACER(st_data_get(realm_key) + 16, 16);
	if (pl1 == NULL || pl2 == NULL) {
		log_error("PLACER sets in realm_key failed");
		goto error;
	}

	init_vector = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), 16);
	if (init_vector == NULL) {
		log_error("st_alloc_opts() failed");
		goto error;
	}

	if (st_xor(pl1, pl2, init_vector) == NULL) {
		log_error("st_xor() failed");
		goto cleanup_init_vector;
	}

	return init_vector;

	cleanup_init_vector: st_free(init_vector);
	error: return NULL;
}
