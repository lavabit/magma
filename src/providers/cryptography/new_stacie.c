/**
 * @file /magma/objects/users/stacie.c
 *
 * @brief Functions used to generate STACIE-specified tokens and keys.
 * @Author Ivan. Kent implemented coding standards and restructured code.
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief   clamp: range limit the input param 'x' between the values min
 *          and max.
 * @Author  Kent.
 * @param   x    number to be clamped
 * @param   min  minimum value for the clamp
 * @param   max  maximum value for the clamp
 * TODO Kent. Where does this clamp() helper function belong?
 * TODO Kent. Does st_xor() return a newly allocated stringer?  Or is that a
 * TODO Kent. Which if any of these routines are private?  Mark the private routines
 *          as constant and can only be calls from this scope.
 *          function of whether the last argument to st_xor is null or nonnull?
 *          Find out which is the case and cleanup all calls to st_xor() here.
 * TODO Kent. count: does uint24_put_no() allocate heap space? If so, comment, 
 *          add an error case for cleaning it up and verify it gets cleaned
 *          up in the success case.
 * TODO Kent. scan entire code base for uses of st_append() of the form: 
 *          a = st_append(a, b) and fix or leak memory on failure.
 * TODO Kent. verify and add a call to st_wipe to the st_free routine if
 *          the SECURE flag is set!
 * Fixed:   stacie_hashed_key_derive() failed to error out when it detected
 *          the hash_input stringer failed to allocate.
 * Fixed:   none of the calls to st_append were being checked for failure.  
 *          added checks for all.
 * Fixed:   there was basically no error checking in stacie_hashed_key_derive().
 *          Added error checking for all calls that could fail
 * Fixed:   stacie_hashed_token_derive failed to free result in the case of the 
 *          allocation call to create hash_input stringer.
 * Fixed:   All calls to st_append() of the form a = st_append(a, b) are unsafe
 *          and will result in leaked heap memory in the event of st_append() 
 *          returning an error.  NEVER use the code stucture:
 *          a = st_append(a, b) where the 'a' parameter will get overwritten!!!
 * Fixed:   stacie_seed_key_derive: first call to st_append() failed to free temp2
 *          on error.
 */
static
uint_t
uint_clamp (uint_t x, uint_t min, uint_t max) {
    return x > max? max : x < min? min: x;
}   // uint_clamp()

/**
 * @brief   Calculate total number of hash rounds for key derivation.
 * @author  Ivan
 * @param   password    User password.
 * @param   bonus       Number of bonus hash rounds.
 * @return  Total number of hash rounds, 0 on failure.
 */
uint_t
stacie_rounds_calculate (stringer_t *password, uint_t bonus) {
	uint_t pass_len;
	uint_t hash_rounds;

	/*
     * Between this call to st_empty() and the following call to
	 * utf_length_get() st_empty() is called 3 times.
	 */
	if (st_empty(password)) {
		log_pedantic("password is empty.");
		goto error:
	}

#if 0  // cleanup the utf code then re-enable this section
	if (pass_len = utf_length_get(password) == 0) {
		log_pedantic("password length is 0");
		return 0;
	}
#endif

	if (bonus >= (MAX_HASH_NUM - 2)) {
		hash_rounds = MAX_HASH_NUM;
	} else {
		if (pass_len >= 24) {
			hash_rounds = 2;
		} else {
			// There must be a clearer way to describe the following.  
			// As it's written, I couldn't tell whether it's right or 
			// not.
			hash_rounds = ((uint_t) 2) << (23 - pass_len);
		}
	}

	hash_rounds += bonus;

	// clamp the return hash_rounds between MIN and MAX
	return uint_clamp(hash_rounds, MIN_HASH_NUM, MAX_HASH_NUM);

error:
	return 0;
}   // stacie_rounds_calculate()

/*
 * @brief   Computer the key used to extract the entropy seed.
 * @author  Ivan
 * @param   salt  User specific salt.
 * @return  Key   used by the hmac function to extract seed.
 *
 * Note: the returned 'seed_key' is malloced from the heap and is
 * the responsibility of the caller to free.
 */
stringer_t *
stacie_seed_key_derive (stringer_t *salt) {
	size_t salt_len;
	stringer_t *seed_key;
	stringer_t *temp1;
	stringer_t *temp2;
	unsigned char *piece;

	if (st_empty(salt)) {
		log_pedantic("salt is empty");
		goto error;
	}

	salt_len = st_length_get(salt);

	if (salt_len % 32 != 0) {
		log_info("Salt should be aligned to 32 octet boundary");
	}

	if (salt_len > 1024) {
		log_info("Salt should not exceed 1024 octets");
	}

	if (salt_len != 128) {
		seed_key = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 128);
		if (seed_key == NULL) {
			log_error("st_alloc_opts() failed");
			goto error;
		}   // seed_key is allocated

		// TODO: Add constant and a comment for that nekid '3'
		piece = mm_alloc(salt_len + 3);
		if (piece == NULL) {
			log_error("mm_alloc() failed");
			goto cleanup_seed_key;
		}   // piece is allocated

		mm_copy(piece, st_data_get(salt), salt_len);

		// TODO: Replace '3' with constant and a comment here
		mm_set(piece + salt_len, 0, 3);
		temp1 = PLACER(piece, salt_len + 3);
		temp2 = hash_sha512(temp1, NULL);
		if (temp2 == NULL) {
			log_error("hash_sha512() failed to hash salt string");
			goto cleanup_piece;
		}   // temp2 is allocated

		if (st_append(seed_key, temp2) == NULL) {
			log_error("st_append() failed");
		    goto cleanup_temp2;
		}

		st_free(temp2);

		// Why an assignment of the +2?
		piece[salt_len + 2] = (unsigned char) 1;

		temp2 = hash_sha512(temp1, NULL);
		if (temp2 == NULL) {
			log_error("hash_sha512() failed to hash salt string");
			goto cleanup_piece;
		}   // temp2 is allocated

		// '64' snd '128' hould be a properly named constants
		mm_copy(st_data_get(seed_key) + 64, st_data_get(temp2), 64);
		st_length_set(seed_key, 128);
		st_free(temp2);
		mm_free(piece);
	} else {
		// salt_len == 128
		seed_key = st_dupe_opts((MANAGED_T | JOINTED | SECURE), salt);
		if (seed_key == NULL) {
			log_error("st_dupe_ops() failed");
			goto error;
		}
	}

	return seed_key;

cleanup_temp2:
	st_free(temp2);
cleanup_piece:
    mm_free(piece);
cleanup_seed_key:
	st_free(seed_key);
error:
	return NULL;
}   // stacie_seed_key_derive()

/*
 * @brief   Extract the seed from user password.
 * @author  Ivan
 * @param   rounds      Number of hashing rounds.
 * @param   username    User username.
 * @param   password    User password.
 * @salt    salt        User specific salt (optional).
 * @return  Stringer with user's entropy seed.
 */
stringer_t *
stacie_seed_extract (
	uint_t rounds,
	stringer_t *username,
	stringer_t *password,
	stringer_t *salt)
{
	stringer_t *seed = NULL;
	stringer_t *temp;
	stringer_t  *key;
	size_t salt_len;

	if (rounds < MIN_HASH_NUM || rounds > MAX_HASH_NUM) {
		log_pedantic("hash rounds invalid");
		goto error;
	}

	if (st_empty(username) || st_empty(password)) {
		log_pedantic("username or password is NULL or empty");
		goto error;
	}

	if (salt == NULL)  {
		// Validate this form of the call returns new salt
		salt = hash_sha512(username, NULL);
		if (salt == NULL) {
			log_error("hash_sha512() failed");
			goto error;
		}   // salt is allocated
		
		// recurse, free the temp salt stringer and return
		seed = stacie_seed_extract(rounds, username, password, salt);
		st_free(salt);
		goto out;
	}

	salt_len = st_length_get(salt);
	if (salt_len < 64) {
		log_pedantic("Salt length < 64 octets.)");
		goto error;
	}

	key = stacie_seed_key_derive(salt);
	if (key == NULL) {
		log_error("stacie_seed_key_derive() failed");
		goto error;
	}   // key is allocated

	seed = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (seed == NULL) {
		log_error("st_alloc_opts() failed");
		goto cleanup_key;
	}   // seed is allocated

	seed = hmac_multi_sha512(rounds, password, key, seed);
	if (seed == NULL) {
		log_error("hmac_multi_sha512() failed");
		goto cleanup_seed;
	}

	st_free(key);

out:
	return seed;

cleanup_seed:
	st_free(seed);
cleanup_key:
	st_free(key);
error:
	return NULL;
}   // stacie_seed_extract()

/*
 * @brief   Derive the hashed key from a seed and user credentials.
 * @author  Ivan
 * @param   base       Entropy seed for master key derivation, master key
 *                     for password key derivation.
 * @param   username   Username.
 * @param   password   User password.
 * @param   salt       Optional random salt value at least 64 bytes.
 * @return  Pointer to derived hashed key.
 *
 * Note: when there's a cascade of st_append calls on the same stringer it
 * is only necessary to check the error of the last call in the cascade.  
 * Reference the notes below for the explanation of this structure.
*/
stringer_t *
stacie_hashed_key_derive (
	stringer_t *base,
	uint_t rounds,
	stringer_t *username,
	stringer_t *password,
	stringer_t *salt)
{
	void *opt1;
	void *opt2;
	size_t input_len;
	stringer_t *hashed_key;
	stringer_t *hash_input;
	stringer_t *count;

	if (st_empty(base)) {
		log_pedantic("base is NULL or empty")
		goto error;
	}

	if (st_length_get(base) != 64) {
		log_pedantic("base length != 64");
		goto error;
	}

	if (rounds < MIN_HASH_NUM || rounds > MAX_HASH_NUM) {
		log_pedantic("hash rounds invalid");
		goto error;
	}

	if (st_empty(username) || st_empty(password)) {
		log_pedantic("username or password is NULL or empty");
		goto error;
	}

	// The salt param is allowed to be NULL.  If it's not NULL, it must be
	// less than 64 bytes long.
	size_t salt_len;
	if (st_empty(salt)) {
		salt_len = 0;
	} else {
		salt_len = st_length_get(salt);

		if (salt_len < 64) {
		log_pedantic("Non null salt length is < 64 bytes");
		goto error;
	}


	hashed_key = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (hashed_key == NULL) {
		log_error("st_alloc_opts() failed");
		goto error;
	}   // hashed_key is allocated

	input_len = 64;
	input_len += st_length_get(base);
	input_len += st_length_get(username);
	input_len += salt_len;
	input_len +=  st_length_get(password);
	input_len += 3;     // Replace this with a named constant

	hash_input = st_alloc_opts((MANAGED_T | JOINTED | SECURE), input_len);
	if (hash_input == NULL) {
		log_error("st_alloc_opts() failed");
		goto cleanup_hashed_key;
	}   // hash_input is allocated

	// Do not add checks for each call to the st_append cascade.  
	st_append(hash_input, base);
	st_append(hash_input, username);

	if (salt_len != 0) {
		st_append(hash_input, salt);
	}

	st_append(hash_input, password);

	count = uint24_put_no(0);  // TODO: did something just get allocated on the heap?

	/*
	 * NOTE: it is sufficient to check for an error condition at the 
     * end of a cascade of st_append() calls since any error that occurs
     * due to running out of heap space will not adversely affect 
     * additional calls to st_append(). 
     *
     */
	
	if (st_append(hash_input, count) == NULL) {
		log_error("st_append() failed");
		goto cleanup_hash_input;
	}

	// Note: hashed_key gets updated in the hash_sha512() call
	if (hash_sha512(hash_input, hashed_key) == NULL) {
		log_error("hash_sha512() failed");
		goto cleanup_hash_input;
	}

	// TODO: verify and add a call to st_wipe to the st_free routine if
	// the SECURE flag is set!
	st_wipe(hash_input);

	// Note: all the following st_append() calls are updating the 
	// first argument 'hash_input'.  Use same trick for only error checking
	// the last call to st_append() in the following cascade of calls.

	st_append(hash_input, hashed_key);
	st_append(hash_input, base);
	st_append(hash_input, username);

	if (salt_len != 0) {
		st_append(hash_input, salt);
	}

	st_append(hash_input, password);

	count = uint24_put_no(1);  // TODO: allocated on the heap?

	// last st_append in this cascade
	if (st_append(hash_input, count) == NULL) {
		log_error("st_append() failed");
		goto cleanup_hash_input;
	}

	if (hash_sha512(hash_input, hashed_key) == NULL) {
		log_error("hash_sha512() failed");
		goto cleanup_hash_input;
	}

	opt1 = st_data_get(hashed_key);
	opt2 = st_data_get(hash_input);

	if (opt1 == NULL || opt2 == NULL) {
		log_error("st_data_get() failed");
		goto cleanup_hash_input;
	}

	// initialization for this for loop is confusing.  why 
	// start at 2?
	for(uint_t i = 2; i < rounds; i++) {
		mm_copy(opt2, opt1, 64);
		if (st_length_set(hash_input, input_len - 3) == 0) {
			log_error("st_length_set() failed");
			goto cleanup_hash_input;
		}
		count = uint24_put_no(i);  // TODO: allocated on the heap?

		if (st_append(hash_input, count) == NULL) {
			log_error("st_data_get() failed");
			goto cleanup_hash_input;
		}
		if (hash_sha512(hash_input, hashed_key) == NULL) {
			log_error("hash_sha512() failed");
			goto cleanup_hash_input;
		}
	}

	/*
	 * TODO: add st_wipe() to the st_free routine if the SECURE memory
	 * field is set and remove this call.
	 */
	st_wipe(hash_input);
	st_free(hash_input);

	return hashed_key;

cleanup_hash_input:
	st_free(hash_input);
cleanup_hashed_key:
	st_free(hashed_key);
error:
	return NULL;
}   // stacie_hashed_key_derive()

/*
 * @brief   Derive hashed token as per STACIE authentication protocol.
 * @author  Ivan
 * @param   base        The base input that will be hashed into the token, 
 *                      either a password_key or verification token
 * @param   username    Username stringer.
 * @param   salt        User-specific salt.
 * @param   nonce       Token-specific nonce.
 * @return  Stringer containing the hashed token.
*/
stringer_t *
stacie_hashed_token_derive (
	stringer_t *base,
	stringer_t *username,
	stringer_t *salt,
	stringer_t *nonce)
{
	size_t salt_len = 0
	size_t nonce_len = 0;
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

	if (!st_empty(salt) && ((salt_len = st_length_get(salt)) < 64)) {
		log_pedantic("salt is NULL, empty or length != 64");
		goto error;
	}

	if (!st_empty(nonce) && ((nonce_len = st_length_get(nonce)) < 64)) {
		log_pedantic("nonce is NULL, empty or length != 64");
		goto error;
	}

	if (!(hashed_token = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64))) {
		log_error("st_alloc_opts() failed");
		goto error;
	}   // hashed_token is allocated

	if (!(hash_input = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64 +
		+ st_length_get(base) + st_length_get(username) +
		+ salt_len + nonce_len + 3))) {
		log_error("st_alloc_opts() failed");
		goto cleanup_hashed_token;
	}   // hash_input is allocated

	for(uint_t i = 0; i < MIN_HASH_NUM; i++) {
		hash_input = st_append(hash_input, hashed_token);
		st_wipe(hashed_token);
		hash_input = st_append(hash_input, base);
		hash_input = st_append(hash_input, username);
		if (salt_len) {
			hash_input = st_append(hash_input, salt);
		}
		if (nonce_len) {
			hash_input = st_append(hash_input, nonce);
		}
		count = uint24_put_no(i);
		hash_input = st_append(hash_input, count);
		hashed_token = hash_sha512(hash_input, hashed_token);
		st_wipe(hash_input);
	}

// where is hash_input being freed?  KDH

	return hashed_token;

error:
	return NULL;
} stacie_hashed_token_derive()

/*
 * @brief   Derive the realm key used to decrypt keys for realm-specific user information.
 * @author  Ivan
 * @param   master_key  Stringer containing master key derived from user password.
 * @param   realm       Realm name.
 * @param   shard       Shard serves as a realm-specific salt.
 * @return  Stringer containing the realm key.
*/
stringer_t *
stacie_realm_key_derive (
	stringer_t *master_key,
	stringer_t *realm,
	stringer_t *shard)
{
	bool_t error = false;
	stringer_t *hash_input = NULL;
	stringer_t *realm_key= NULL;
	stringer_t *hash_output = NULL;

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

	hash_input = st_alloc_opts(
		(MANAGED_T | JOINTED | SECURE),
		st_length_get(master_key) + st_length_get(realm) + st_length_get(shard)
	);
	if (hash_input == NULL) {
		log_error("Failed to allocate secure stringer for hash input.");
		goto error;
	}

	realm_key = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (realm_key == NULL) {
		log_error("Failed to allocate secure stringer for realm key.");
		goto cleanup_hash_input;
	}

	hash_output = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 64);
	if (hash_output == NULL) {
		log_error("Failed to allocate secure stringer for hash output.");
		goto cleanup_realm_key;
	}

	hash_input = st_append(hash_input, master_key);
	if (hash_input == NULL) {
		log_error("Failed to append hash input stringer with master key.");
		goto cleanup_hash_output;
	}

	hash_input = st_append(hash_input, realm);
	if (hash_input == NULL) {
		log_error("Failed to append hash input stringer with realm key.");
		goto cleanup_hash_output;
	}

	hash_input = st_append(hash_input, shard);
	if (hash_input == NULL) {
		log_error("Failed to append hash input stringer with shard.");
		goto cleanup_hash_output;
	}

	hash_output = hash_sha512(hash_input, hash_output);
	if (hash_output == NULL) {
		log_error("Failed to hash input stringer.");
		goto cleanup_hash_output;
	}

	realm_key = st_xor(hash_output, shard, realm_key);
	if (realm_key == NULL) {
		log_error("Failed to xor input stringer with shard.");
		goto cleanup_hash_output;
	}

    st_free(hash_input);
    st_free(hash_output);

	return realm_key;


cleanup_hash_output:
	st_free(hash_output);
cleanup_realm_key:
	st_free(realm_key);
cleanup_hash_input:
	st_free(hash_input);
error:
	return NULL;
}   // stacie_realm_key_derive()

/*
 * @brief   Derive the encryption key used to decrypt realm-specific key.
 * @author  Ivan
 * @param   realm_key  Stringer containing the realm key.
 * @return  Encryption key.
*/
stringer_t *
stacie_realm_cipher_key_derive (stringer_t *realm_key) {
	stringer_t *realm_cipher_key;
	stringer_t *pl;

	if (st_empty(realm_key) || (st_length_get(realm_key) != 64)) {
		log_pedantic("Realm key is zero, NULL or length != 64");
		goto error;
	}

	pl = PLACER(st_data_get(realm_key) + 32, 32);
	if (pl == NULL) {
		log_error("Stting PLACER in realm_key failed");
		goto error;
	}

	realm_cipher_key = st_dupe_opts((MANAGED_T | JOINTED | SECURE), pl);
	if (realm_cipher_key == NULL) {
		log_error("st_dupe_opts() failed");
		goto error;
	}

	return realm_cipher_key;

error:
	return NULL;
}   // stacie_realm_cipher_key_derive()

/*
 * @brief   Derive the initialization vector used to decrypt realm-specific key.
 * @author  Ivan
 * @param   realm_key  Stringer containing the realm key.
 * @return  Initialization vector.
*/
stringer_t *
stacie_realm_init_vector_derive (stringer_t *realm_key) {
	stringer_t *init_vector;
	stringer_t *pl1;
	stringer_t *pl2;
	stringer_t *temp;

	if (st_empty(realm_key) || (st_length_get(realm_key) != 64)) {
		log_pedantic("Realm key is zero, NULL or length != 64");
		goto error;
	}

	pl1 = PLACER(st_data_get(realm_key), 16);
	pl2 = PLACER(st_data_get(realm_key) + 16, 16);
    if (pl1 == NULL || pl2 == NULL) {
		log_error("Setting PLACERs in realm_key failed");
		goto error;
	}

	temp = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 16);
	if (temp == NULL) {
		log_error("st_alloc_opts() failed");
		goto error;
	}   // temp is allocated

	init_vector = st_xor(pl1, pl2, temp);
	if (init_vector == NULL) {
		log_error("st_xor() failed");
		goto cleanup_temp;
	}

	return init_vector;

cleanup_temp:
	st_free(temp);
error:
	return NULL;
}   // stacie_realm_init_vector_derive()
