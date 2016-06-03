
/**
 * @file /magma/providers/cryptography/ciphers.c
 *
 * @brief Functions used to handle cryptographic cipher primitives.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Look up a cipher type by name.
 * @param	name	a descriptive name of the cipher to be looked up.
 * @return	NULL on failure, or the relevant cipher data structure on success.
 */
cipher_t * cipher_name(stringer_t *name) {

	const EVP_CIPHER *result = NULL;
	if (!st_empty(name) && !(result = EVP_get_cipherbyname_d(st_char_get(name)))) {
		log_pedantic("The name provided did not match any of the available cipher methods. {name = %.*s}", st_length_int(name), st_char_get(name));
	}

	return (cipher_t *)result;
}

/**
 * @brief	Look up a cipher type by its nid.
 * @param	id	the numerical ID (nid) of the specified cipher.
 * @return	NULL on failure, or the relevant cipher data structure on success.
 */
cipher_t * cipher_id(int_t id) {

	const EVP_CIPHER *result = NULL;

	if (!(result = EVP_get_cipherbyname_d(OBJ_nid2sn_d(id)))) {
		log_pedantic("The id provided did not match any of the available cipher methods. {id = %i / name = %s}", id, OBJ_nid2sn_d(id));
	}

	return (cipher_t *)result;
}

/**
 * @brief	Return the numerical ID (nid) of a specified cipher.
 * @param	c	the input cipher type.
 * @return	NID_undef on failure, or the nid of the specified cipher.
 */
int_t cipher_numeric_id(cipher_t *c) {

	int_t result = NID_undef;

	if (c && (result = EVP_CIPHER_nid_d((const EVP_CIPHER *)c)) < 0) {
		log_pedantic("Unable to determine the encryption cipher's numerical identifier.");
	}

	return result;
}

/**
 * @brief	Determine the key length of a specified cipher.
 * @param	c	the input cipher type.
 * @return	-1 on failure, or the cipher's key length in bytes.
 */
int_t cipher_key_length(cipher_t *c) {

	int_t result = -1;

	if (c && (result = EVP_CIPHER_key_length_d((const EVP_CIPHER *)c)) < 0) {
		log_pedantic("Unable to determine the encryption cipher's key length. {cipher = %s}", OBJ_nid2sn_d(EVP_CIPHER_nid_d((const EVP_CIPHER *)c)));
	}

	return result;
}

/**
 * @brief	Determine the IV length of a specified cipher.
 * @param	c	the input cipher type.
 * @return	-1 on failure, or the cipher's IV length in bytes.
 */
int_t cipher_vector_length(cipher_t *c) {

	int_t result = -1;

	if (c && (result = EVP_CIPHER_iv_length_d((const EVP_CIPHER *)c)) < 0) {
		log_pedantic("Unable to determine the encryption cipher's vector length. {cipher = %s}", OBJ_nid2sn_d(EVP_CIPHER_nid_d((const EVP_CIPHER *)c)));
	}

	return result;
}

/**
 * @brief	Determine the block length of a specified cipher.
 * @param	c	the input cipher type.
 * @return	-1 on failure, or the cipher's block length in bytes.
 */
int_t cipher_block_length(cipher_t *c) {

	int_t result = -1;

	if (c && (result = EVP_CIPHER_block_size_d((const EVP_CIPHER *)c)) < 0) {
		log_pedantic("Unable to determine the encryption cipher's block length. {cipher = %s}", OBJ_nid2sn_d(EVP_CIPHER_nid_d((const EVP_CIPHER *)c)));
	}

	return result;
}

