
/**
 * @file /magma/providers/cryptography/ecies.c
 *
 * @brief	ECIES encryption/decryption functions.
 */

#include "magma.h"

EC_GROUP *ecies_curve_group = NULL;
const EVP_MD *ecies_hmac_evp = NULL;
const EVP_MD *ecies_envelope_evp = NULL;
const EVP_CIPHER *ecies_cipher_evp = NULL;

bool_t deprecated_ecies_start(void) {

	if (!(ecies_cipher_evp = EVP_get_cipherbyname_d(OBJ_nid2sn_d(ECIES_CIPHER))) || !(ecies_hmac_evp = EVP_get_digestbyname_d(OBJ_nid2sn_d(ECIES_HMAC))) ||
			!(ecies_envelope_evp = EVP_get_digestbyname_d(OBJ_nid2sn_d(ECIES_ENVELOPE))) || !(ecies_curve_group	= deprecated_ecies_group(ECIES_CURVE, true))) {
		log_error("Initialization of the default ECIES algorithms failed.");
		return false;
	}

	return true;
}

/**
 * @brief	Destroy all initialized ECIES curve group information.
 * @return	This function returns no value.
 */
void deprecated_ecies_stop(void) {

	EC_GROUP *group;

	if (ecies_curve_group) {
		group = ecies_curve_group;
		ecies_curve_group = NULL;
		EC_GROUP_free_d(group);
	}

	return;
}

/**
 * @brief	Free an ECIES key pair.
 * @return	This function returns no value.
 */
void deprecated_ecies_key_free(EC_KEY *key) {

	EC_KEY_free_d(key);
	return;
}

/**
 * @brief	Allocate a new ECIES key pair from the curve defined by ECIES_CURVE.
 * @see		NID_sect571k1
 * @return	NULL on failure, or a pointer to the newly allocated key pair.
 */
EC_KEY * deprecated_ecies_key_alloc(void) {

	EC_KEY *key = NULL;

	// Create a key and assign the group.
	if (ecies_curve_group) {

		if (!(key = EC_KEY_new_d())) {
			log_info("An error occurred while initializing an empty ECIES key context. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		} else if (EC_KEY_set_group_d(key, ecies_curve_group) != 1) {
			log_info("Unable to assign the default group to our empty key context.. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			EC_KEY_free_d(key);
			key = NULL;
		}

	}

	// If an error occurs above, attempt key creation from scratch.
	if (!key) {

		if (!(key = EC_KEY_new_by_curve_name_d(ECIES_CURVE))) {
			log_info("An error occurred while trying to create a new ECIES key using the default curve. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			return NULL;
		}

		EC_KEY_set_conv_form_d(key, POINT_CONVERSION_COMPRESSED);
	}

	return key;
}

EC_GROUP * deprecated_ecies_group(uint64_t curve, bool_t precompute) {

	EC_GROUP *group;

	if (!(group = EC_GROUP_new_by_curve_name_d(curve))) {
		log_error("An error occurred while trying to create the elliptical group. {%s}",
				ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	} else if (precompute && EC_GROUP_precompute_mult_d(group, NULL) != 1) {
		log_error("Unable to precompute the required elliptical curve point data. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_GROUP_free_d(group);
		return NULL;
	}

	EC_GROUP_set_point_conversion_form_d(group, POINT_CONVERSION_COMPRESSED);

	return group;
}

/**
 * @brief	Generate a random ECIES key pair.
 * @return	NULL on failure, or a new random ECIES key pair on success.
 */
EC_KEY * deprecated_ecies_key_create(void) {

	EC_KEY *key = NULL;

	if (!(key = deprecated_ecies_key_alloc())) {
		log_info("Unable to allocate an empty key context.");
		return NULL;
	}

	// This should generate a random key pair.
	if (EC_KEY_generate_key_d(key) != 1) {
		log_info("An error occurred while trying to generate a random ECIES key pair. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_KEY_free_d(key);
		return NULL;
	}

	return key;
}

EC_KEY * deprecated_ecies_key_public(uint64_t format, placer_t data) {

	EC_KEY *key = NULL;
	EC_POINT *point = NULL;

	if (!(key = deprecated_ecies_key_alloc())) {
		log_info("Unable to allocate an empty key context.");
		return NULL;
	}

	// Process a key in binary format.
	if (format & ECIES_PUBLIC_BINARY) {

		// Generate the empty point context we'll be assigning to below.
		if (!(point = EC_POINT_new_d(EC_KEY_get0_group_d(key)))) {
			log_info("An error occurred while allocate the elliptical curve point. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			EC_KEY_free_d(key);
			return NULL;
		}
		else if (EC_POINT_oct2point_d(EC_KEY_get0_group_d(key), point, pl_data_get(data), pl_length_get(data), NULL) != 1) {
			log_info("An error occurred while parsing the binary elliptical curve point data used to represent the public key. {%s}",
					ssl_error_string(MEMORYBUF(256), 256));
			EC_POINT_free_d(point);
			EC_KEY_free_d(key);
			return NULL;
		}

	}
	// Process a key in hex.
	else if (format & ECIES_PUBLIC_HEX) {

		if (!(point = EC_POINT_hex2point_d(EC_KEY_get0_group_d(key), pl_char_get(data), NULL, NULL))) {
			log_info("An error occurred while parsing the binary elliptical curve point data used to represent the public key. {%s}",
					ssl_error_string(MEMORYBUF(256), 256));
			EC_KEY_free_d(key);
			return NULL;
		}

	}
	// Invalid format!
	else {
		log_info("The public key data is using an unrecognized format.");
		EC_POINT_free_d(point);
		EC_KEY_free_d(key);
		return NULL;
	}

	// Assign the point to our empty key instance.
	if (EC_KEY_set_public_key_d(key, point) != 1) {
		log_info("The provided point data does not represent a valid public key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_POINT_free_d(point);
		EC_KEY_free_d(key);
		return NULL;
	}

	// The above function call duplicates the point so the local copy is no longer needed.
	EC_POINT_free_d(point);

	// Ensures the provided point is along the active elliptical curve and therefore represents a valid public key.
	if (EC_KEY_check_key_d(key) != 1) {
		log_info("The provided point data does not represent a valid public key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_KEY_free_d(key);
		return NULL;
	}

	return key;
}

EC_KEY * deprecated_ecies_key_private(uint64_t format, placer_t data) {

	EC_KEY *key = NULL;
	BIGNUM *number = NULL;

	if (!(key = deprecated_ecies_key_alloc())) {
		log_info("Unable to allocate an empty key context.");
		return NULL;
	}

	// Process a key in binary format.
	if (format & ECIES_PRIVATE_BINARY) {

		if (!(number = BN_bin2bn_d(pl_data_get(data), pl_length_get(data), NULL))) {
			log_info("An error occurred while parsing the binary elliptical curve point data used to represent the private key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			EC_KEY_free_d(key);
			return NULL;
		}

	}
	// Process a key in hex.
	else if (format & ECIES_PRIVATE_HEX) {

		if (!(BN_hex2bn_d(&number, pl_char_get(data)))) {
			log_info("An error occurred while parsing the binary elliptical curve point data used to represent the private key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			EC_KEY_free_d(key);
			return NULL;
		}

	}
	// Invalid format!
	else {
		log_info("The private key data is using an unrecognized format.");
		EC_KEY_free_d(key);
		return NULL;
	}

	// Assign the point to our empty key instance.
	if (EC_KEY_set_private_key_d(key, number) != 1) {
		log_info("The provided point data does not represent a valid public key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_KEY_free_d(key);
		BN_free_d(number);
		return NULL;
	}

	// The above function call duplicates the point so the local copy is no longer needed.
	BN_free_d(number);

	return key;
}

/**
 * @brief	Return an ECIES public key as a null-terminated hex string.
 * @param	key	the input ECIES key pair.
 * @return	NULL on failure, or the hex-formatted public key as a null-terminated string.
 */
stringer_t * deprecated_ecies_key_public_hex(EC_KEY *key) {

	char *hex;
	const EC_POINT *point;
	const EC_GROUP *group;
	stringer_t *result = NULL;

	if (!(point = EC_KEY_get0_public_key_d(key))) {
		log_info("No public key available. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	} else if (!(group = EC_KEY_get0_group_d(key))) {
		log_info("No group available. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	} else if (!(hex = EC_POINT_point2hex_d(group, point, POINT_CONVERSION_COMPRESSED, NULL))) {
		log_info("Unable to serialize the public key into hex. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	}

	if (!(result = st_import(hex, ns_length_get(hex) + 1))) {
		log_info("Unable to make copy of ECIES public key.");
	}

	OPENSSL_free_d(hex);

	return result;
}

/**
 * @brief	Return an ECIES public key as binary data.
 * @param	key		the input ECIES key pair.
 * @param	olen	a pointer to store the length of the returned key.
 * @return	NULL on failure, or a pointer to the raw public key.
 */
uchr_t * deprecated_ecies_key_public_bin(EC_KEY *key, size_t *olen) {

	uchr_t *result;
	size_t rlen, blen = 512;
	const EC_POINT *point;
	const EC_GROUP *group;

	if (!(point = EC_KEY_get0_public_key_d(key))) {
		log_info("No public key available. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	} else if (!(group = EC_KEY_get0_group_d(key))) {
		log_info("No group available. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	} else if (!(result = mm_alloc(blen))) {
		log_info("Error allocating space for ECIES public key.");
		return NULL;
	} else if ((rlen = EC_POINT_point2oct_d(group, point, POINT_CONVERSION_COMPRESSED, result, blen, NULL)) <= 0) {
		log_info("Unable to extract the public key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		mm_free(result);
		return NULL;
	}

	if (olen) {
		*olen = rlen;
	}

	//	char * (*EC_POINT_point2hex_d)(const EC_GROUP *, const EC_POINT *, point_conversion_form_t form, BN_CTX *) __attribute__ ((common)) = NULL;
	//size_t (*EC_POINT_point2oct_d)(const EC_GROUP *, const EC_POINT *, point_conversion_form_t form, unsigned char *buf, size_t len, BN_CTX *ctx) __attribute__ ((common)) = NULL;

	return result;
}

/**
 * @brief	Return an ECIES private key as a hex string.
 * @param	key	the input ECIES key pair.
 * @return	NULL on failure, or the hex-formatted private key as a managed string.
 */
stringer_t * deprecated_ecies_key_private_hex(EC_KEY *key) {

	chr_t *hex;
	const BIGNUM *bn;
	stringer_t *result = NULL;

	if (!(bn = EC_KEY_get0_private_key_d(key))) {
		log_pedantic("No private key available. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	}
	else if (!(hex = BN_bn2hex_d(bn))) {
		log_pedantic("Unable to serialize the private key into hex. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	}

	else if (!(result = st_import_opts(MANAGED_T | CONTIGUOUS | SECURE, hex, ns_length_get(hex) + 1))) {
		log_pedantic("Unable to allocate secure buffer for hex string.");
		ns_wipe(hex, ns_length_get(hex));
		OPENSSL_free_d(hex);
		return NULL;
	}

	ns_wipe(hex, ns_length_get(hex));
	OPENSSL_free_d(hex);

	return result;
}

/**
 * @brief	Return an ECIES private key as binary data.
 * @param	key		the input ECIES key pair.
 * @param	olen	a pointer to store the length of the returned key.
 * @return	NULL on failure, or a pointer to the raw private key.
 */
uchr_t * deprecated_ecies_key_private_bin(EC_KEY *key, size_t *olen) {

	const BIGNUM *bn;
	int bn_len;
	uchr_t *result;

	if (!(bn = EC_KEY_get0_private_key_d(key))) {
		log_info("No private key available. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return NULL;
	} else if (!(bn_len = BN_num_bytes_d(bn))) {
		log_info("Error determining size of ECIES private key.");
		return NULL;
	}

	if (!(result = mm_sec_alloc(bn_len))) {
		log_info("Error allocating space for ECIES private key.");
		return NULL;
	}
	else if (!BN_bn2bin_d(bn, (unsigned char *)result)) {
		log_info("Error retrieving ECIES private key.");
		mm_sec_free(result);
		return NULL;
	}

	if (olen) {
		*olen = bn_len;
	}

	return result;
}

void * deprecated_ecies_envelope_derivation(const void *input, size_t ilen, void *output, size_t *olen) {

	if (EVP_Digest_d(input, ilen, output, (unsigned int *)olen, ecies_envelope_evp, NULL) != 1) {
		return NULL;
	}

	return output;
}

/**
 * @brief	Encrypt a block of data using an ECIES public key.
 * @param	key			the ECIES public key in the specified format.
 * @param	key_type	the encoding type of the ECIES public key (ECIES_PUBLIC_BINARY or ECIES_PUBLIC_HEX).
 * @param	data		a pointer to the block of data to be encrypted.
 * @param	length		the length, in bytes, of the data to be encrypted.
 * @return	NULL on failure, or a pointer to the header of the cryptex object containing the encrypted data on success..
 */
cryptex_t * deprecated_ecies_encrypt(stringer_t *key, ECIES_KEY_TYPE key_type, unsigned char *data, size_t length) {

	void *body;
	HMAC_CTX hmac;
	int body_length;
	cryptex_t *cryptex;
	EVP_CIPHER_CTX cipher;
	unsigned int mac_length;
	EC_KEY *user, *ephemeral;
	size_t envelope_length, block_length, key_length, hexkey_length;
	uchr_t *kbuf;
	unsigned char envelope_key[SHA512_DIGEST_LENGTH], iv[EVP_MAX_IV_LENGTH], block[EVP_MAX_BLOCK_LENGTH];

	// Simple sanity check.
	if (!key || !data || !length) {
		log_info("Invalid parameters passed in.");
		return NULL;
	}
	else if ((key_type != ECIES_PUBLIC_HEX) && (key_type != ECIES_PUBLIC_BINARY)) {
		log_info("Invalid ecies private key type specified!");
		return NULL;
	}
	else if (st_empty_out(key,&kbuf,&hexkey_length)) {
		log_info("Could not read key data.");
		return NULL;
	}

	// Make sure we are generating enough key material for the symmetric ciphers.
	if ((key_length = EVP_CIPHER_key_length_d(EVP_get_cipherbyname_d(OBJ_nid2sn_d(ECIES_CIPHER)))) * 2 > SHA512_DIGEST_LENGTH) {
		log_info("The key derivation method will not produce enough envelope key material for the chosen ciphers. {envelope = %i / required = %zu}", SHA512_DIGEST_LENGTH / 8, (key_length * 2) / 8);
		return NULL;
	}
	// Convert the user's public key from hex into a full EC_KEY structure.
	if (!(user = deprecated_ecies_key_public(key_type, pl_init(kbuf, hexkey_length)))) {
		log_info("Invalid public key provided.");
		return NULL;
	}
	// Create the ephemeral key used specifically for this block of data.
	else if (!(ephemeral = deprecated_ecies_key_create())) {
		log_info("An error occurred while trying to generate the ephemeral key.");
		EC_KEY_free_d(user);
		return NULL;
	}
	// Use the intersection of the provided keys to generate the envelope data used by the ciphers below. The ecies_key_derivation() function uses
	// SHA 512 to ensure we have a sufficient amount of envelope key material and that the material created is sufficiently secure.
	else if (ECDH_compute_key_d(envelope_key, SHA512_DIGEST_LENGTH, EC_KEY_get0_public_key_d(user), ephemeral, deprecated_ecies_envelope_derivation) != SHA512_DIGEST_LENGTH) {
		log_info("An error occurred while trying to compute the envelope key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_KEY_free_d(ephemeral);
		EC_KEY_free_d(user);
		return NULL;
	}
	// Determine the envelope and block lengths so we can allocate a buffer for the result.
	else if ((block_length = EVP_CIPHER_block_size_d(EVP_get_cipherbyname_d(OBJ_nid2sn_d(ECIES_CIPHER)))) == 0 || block_length > EVP_MAX_BLOCK_LENGTH || (envelope_length = EC_POINT_point2oct_d(EC_KEY_get0_group_d(ephemeral), EC_KEY_get0_public_key_d(ephemeral),
			POINT_CONVERSION_COMPRESSED, NULL, 0, NULL)) == 0) {
		log_info("Invalid block or envelope length. {block = %zu / envelope = %zu}", block_length, envelope_length);
		EC_KEY_free_d(ephemeral);
		EC_KEY_free_d(user);
		return NULL;
	}
	// We use a conditional to pad the length if the input buffer is not evenly divisible by the block size.
	else if (!(cryptex = deprecated_cryptex_alloc(envelope_length, EVP_MD_size_d(EVP_get_digestbyname_d(OBJ_nid2sn_d(ECIES_HMAC))), length, length + (length % block_length ? (block_length - (length % block_length)) : 0)))) {
		log_info("Unable to allocate a secure_t buffer to hold the encrypted result.");
		EC_KEY_free_d(ephemeral);
		EC_KEY_free_d(user);
		return NULL;
	}
	// Store the public key portion of the ephemeral key.
	else if (EC_POINT_point2oct_d(EC_KEY_get0_group_d(ephemeral), EC_KEY_get0_public_key_d(ephemeral), POINT_CONVERSION_COMPRESSED, deprecated_cryptex_envelope_data(cryptex), envelope_length, NULL) != envelope_length) {
		log_info("An error occurred while trying to record the public portion of the envelope key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_KEY_free_d(ephemeral);
		EC_KEY_free_d(user);
		deprecated_cryptex_free(cryptex);
		return NULL;
	}
	// The envelope key has been stored so we no longer need to keep the keys around.
	EC_KEY_free_d(ephemeral);
	EC_KEY_free_d(user);

	// For now we use an empty initialization vector.
	// LOW: Develop a more secure method for selecting and storing the initialization vector.
	memset(iv, 0, EVP_MAX_IV_LENGTH);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&cipher);
	body = deprecated_cryptex_body_data(cryptex);
	body_length = deprecated_cryptex_body_length(cryptex);

	// Initialize the cipher with the envelope key.
	if (EVP_EncryptInit_ex_d(&cipher, EVP_get_cipherbyname_d(OBJ_nid2sn_d(ECIES_CIPHER)), NULL, envelope_key, iv) != 1 || EVP_CIPHER_CTX_set_padding_d(&cipher, 0) != 1 ||
			EVP_EncryptUpdate_d(&cipher, body, &body_length, data, length - (length % block_length)) != 1) {
		log_info("An error occurred while trying to secure the data using the chosen symmetric cipher. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&cipher);
		deprecated_cryptex_free(cryptex);
		return NULL;
	}
	// Check whether all of the data was encrypted. If they don't match up, we either have a partial block remaining, or an error occurred.
	else if (body_length != length) {

		// Make sure all that remains is a partial block, and their wasn't an error.
		if (length - body_length >= block_length) {
			log_info("Unable to secure the data using the chosen symmetric cipher. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			EVP_CIPHER_CTX_cleanup_d(&cipher);
			deprecated_cryptex_free(cryptex);
			return NULL;
		}

		// Copy the remaining data into our partial block buffer. The memset() call ensures any extra bytes will be zero'ed out.
		memset(block, 0, EVP_MAX_BLOCK_LENGTH);
		memcpy(block, data + body_length, length - body_length);
		// Advance the body pointer to the location of the remaining space, and calculate just how much room is still available.
		body += body_length;

		if ((body_length = deprecated_cryptex_body_length(cryptex) - body_length) < 0) {
			log_info("The symmetric cipher overflowed!");
			EVP_CIPHER_CTX_cleanup_d(&cipher);
			deprecated_cryptex_free(cryptex);
			return NULL;
		}
		// Pass the final partially filled data block into the cipher as a complete block. The padding will be removed during the decryption process.
		else if (EVP_EncryptUpdate_d(&cipher, body, &body_length, block, block_length) != 1) {
			log_info("Unable to secure the data using the chosen symmetric cipher. {%s}", ssl_error_string(MEMORYBUF(256), 256));
			EVP_CIPHER_CTX_cleanup_d(&cipher);
			deprecated_cryptex_free(cryptex);
			return NULL;
		}

	}
	// Advance the pointer, then use pointer arithmetic to calculate how much of the body buffer has been used. The complex logic is needed so that we get
	// the correct status regardless of whether there was a partial data block.
	body += body_length;

	if ((body_length = deprecated_cryptex_body_length(cryptex) - (body - deprecated_cryptex_body_data(cryptex))) < 0) {
		log_info("The symmetric cipher overflowed!");
		EVP_CIPHER_CTX_cleanup_d(&cipher);
		deprecated_cryptex_free(cryptex);
		return NULL;
	} else if (EVP_EncryptFinal_ex_d(&cipher, body, &body_length) != 1) {
		log_info("Unable to secure the data using the chosen symmetric cipher. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&cipher);
		deprecated_cryptex_free(cryptex);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&cipher);

	// Generate an authenticated hash which can be used to validate the data during decryption.
	HMAC_CTX_init_d(&hmac);
	mac_length = deprecated_cryptex_hmac_length(cryptex);

	// At the moment we are generating the hash using encrypted data. At some point we may want to validate the original text instead.
	if (HMAC_Init_ex_d(&hmac, envelope_key + key_length, key_length, EVP_get_digestbyname_d(OBJ_nid2sn_d(ECIES_HMAC)), NULL) != 1 || HMAC_Update_d(&hmac, deprecated_cryptex_body_data(cryptex), deprecated_cryptex_body_length(cryptex)) != 1 || HMAC_Final_d(&hmac, deprecated_cryptex_hmac_data(
			cryptex), &mac_length) != 1) {
		log_info("Unable to generate a data authentication code. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		HMAC_CTX_cleanup_d(&hmac);
		deprecated_cryptex_free(cryptex);
		return NULL;
	}

	HMAC_CTX_cleanup_d(&hmac);

	return cryptex;
}

/**
 * @brief	Decrypt a block of data using an ECIES private key.
 * @param	key			the ECIES private key in the specified format.
 * @param	key_type	the encoding type of the ECIES private key (ECIES_PRIVATE_BINARY or ECIES_PRIVATE_HEX).
 * @param	cryptex		a pointer to the head of the cryptex object with the encrypted data.
 * @param	length		a pointer to a size_t variable which will receive the final length of the unencrypted data.
 * @return	NULL on failure, or a pointer to a memory address containing the decrypted data on success..
 */
uchr_t * deprecated_ecies_decrypt(stringer_t *key, ECIES_KEY_TYPE key_type, cryptex_t *cryptex, size_t *length) {

	HMAC_CTX hmac;
	size_t key_length;
	int output_length;
	EVP_CIPHER_CTX cipher;
	EC_KEY *user, *ephemeral;
	uchr_t *kbuf;
	size_t hexkey_length;
	unsigned int mac_length = EVP_MAX_MD_SIZE;
	unsigned char envelope_key[SHA512_DIGEST_LENGTH], iv[EVP_MAX_IV_LENGTH], md[EVP_MAX_MD_SIZE], *block, *output;

	// Simple sanity check.
	if (!key || !cryptex || !length) {
		log_info("Invalid parameters passed in.");
		return NULL;
	}
	else if ((key_type != ECIES_PRIVATE_HEX) && (key_type != ECIES_PRIVATE_BINARY)) {
		log_info("Invalid ecies private key type specified!");
		return NULL;
	}
	else if (st_empty_out(key,&kbuf,&hexkey_length)) {
		log_info("Could not read key data.");
		return NULL;
	}
	// Make sure we are generating enough key material for the symmetric ciphers.
	else if ((key_length = EVP_CIPHER_key_length_d(EVP_get_cipherbyname_d(OBJ_nid2sn_d(ECIES_CIPHER)))) * 2 > SHA512_DIGEST_LENGTH) {
		log_info("The key derivation method will not produce enough envelope key material for the chosen ciphers. {envelope = %i / required = %zu}", SHA512_DIGEST_LENGTH / 8, (key_length * 2) / 8);
		return NULL;
	}
	// Convert the user's public key from hex into a full EC_KEY structure.
	else if (!(user = deprecated_ecies_key_private(key_type, pl_init(kbuf, hexkey_length)))) {
		log_info("Invalid private key provided.");
		return NULL;
	}
	// Create the ephemeral key used specifically for this block of data.
	else if (!(ephemeral = deprecated_ecies_key_public(ECIES_PUBLIC_BINARY, pl_init(deprecated_cryptex_envelope_data(cryptex), deprecated_cryptex_envelope_length(cryptex))))) {
		log_info("An error occurred while trying to recreate the ephemeral key.");
		EC_KEY_free_d(user);
		return NULL;
	}
	// Use the intersection of the provided keys to generate the envelope data used by the ciphers below. The ecies_key_derivation() function uses
	// SHA 512 to ensure we have a sufficient amount of envelope key material and that the material created is sufficiently secure.
	else if (ECDH_compute_key_d(envelope_key, SHA512_DIGEST_LENGTH, EC_KEY_get0_public_key_d(ephemeral), user, deprecated_ecies_envelope_derivation) != SHA512_DIGEST_LENGTH) {
		log_info("An error occurred while trying to compute the envelope key. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_KEY_free_d(ephemeral);
		EC_KEY_free_d(user);
		return NULL;
	}

	// The envelope key material has been extracted, so we no longer need the user and ephemeral keys.
	EC_KEY_free_d(ephemeral);
	EC_KEY_free_d(user);

	// Use the authenticated hash of the ciphered data to ensure it was not modified after being encrypted.
	HMAC_CTX_init_d(&hmac);

	// At the moment we are generating the hash using encrypted data. At some point we may want to validate the original text instead.
	if (HMAC_Init_ex_d(&hmac, envelope_key + key_length, key_length, EVP_get_digestbyname_d(OBJ_nid2sn_d(ECIES_HMAC)), NULL) != 1 || HMAC_Update_d(&hmac, deprecated_cryptex_body_data(cryptex), deprecated_cryptex_body_length(cryptex)) != 1 || HMAC_Final_d(&hmac, md, &mac_length) != 1) {
		log_info("Unable to generate the authentication code needed for validation. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		HMAC_CTX_cleanup_d(&hmac);
		return NULL;
	}

	HMAC_CTX_cleanup_d(&hmac);

	// We can use the generated hash to ensure the encrypted data was not altered after being encrypted.
	if (mac_length != deprecated_cryptex_hmac_length(cryptex) || memcmp(md, deprecated_cryptex_hmac_data(cryptex), mac_length)) {
		log_info("The authentication code was invalid! The ciphered data has been corrupted!");
		return NULL;
	}

	// Create a buffer to hold the result.
	output_length = deprecated_cryptex_body_length(cryptex);

	if (!(block = output = mm_alloc(output_length + 1))) {
		log_info("An error occurred while trying to allocate memory for the decrypted data.");
		return NULL;
	}

	// For now we use an empty initialization vector. We also clear out the result buffer just to be on the safe side.
	memset(iv, 0, EVP_MAX_IV_LENGTH);
	memset(output, 0, output_length + 1);

	EVP_CIPHER_CTX_init_d(&cipher);

	// Decrypt the data using the chosen symmetric cipher.
	if (EVP_DecryptInit_ex_d(&cipher, EVP_get_cipherbyname_d(OBJ_nid2sn_d(ECIES_CIPHER)), NULL, envelope_key, iv) != 1 || EVP_CIPHER_CTX_set_padding_d(&cipher, 0) != 1 || EVP_DecryptUpdate_d(&cipher, block, &output_length, deprecated_cryptex_body_data(cryptex),
			deprecated_cryptex_body_length(cryptex)) != 1) {
		log_info("Unable to decrypt the data using the chosen symmetric cipher. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&cipher);
		free(output);
		return NULL;
	}

	block += output_length;

	if ((output_length = deprecated_cryptex_body_length(cryptex) - output_length) != 0) {
		log_info("The symmetric cipher failed to properly decrypt the correct amount of data!");
		EVP_CIPHER_CTX_cleanup_d(&cipher);
		free(output);
		return NULL;
	}

	if (EVP_DecryptFinal_ex_d(&cipher, block, &output_length) != 1) {
		log_info("Unable to decrypt the data using the chosen symmetric cipher. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&cipher);
		free(output);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&cipher);

	*length = deprecated_cryptex_original_length(cryptex);

	return output;
}

