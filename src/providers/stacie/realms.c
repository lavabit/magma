
/**
 * @file /magma/src/providers/stacie/realms.c
 *
 * @brief Dervice STACIE realm key and vector shard values.
 */

#include "magma.h"

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
stringer_t * stacie_realm_key(stringer_t *master_key, stringer_t *realm, stringer_t *shard) {

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
stringer_t * stacie_realm_cipher(stringer_t *realm_key) {

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
stringer_t * stacie_realm_vector(stringer_t *realm_key) {

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
stringer_t * stacie_realm_tag(stringer_t *realm_key) {

	stringer_t *tag_key = NULL;

	if (st_empty(realm_key) || st_length_get(realm_key) != 64) {
		log_error("The realm tag key extraction failed because the realm key passed in wasn't valid.");
	}
	else if (!(tag_key = st_dupe_opts(MANAGED_T | CONTIGUOUS | SECURE, PLACER(st_data_get(realm_key) + 16, 16)))) {
		log_error("The realm tag key extraction failed because a secure memory buffer could not be allocated to hold the result.");
	}

	return tag_key;
}
