
/**
 * @file /magma/providers/cryptography/cryptex.c
 *
 * @brief	Functions for handling the secure data type.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the length of a cryptex envelope.
 * @param	cryptex		a pointer to the head of the cryptex object to be examined.
 * @return	the length, in bytes, of the cryptex envelope.
 */
uint64_t cryptex_envelope_length(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return head->length.envelope;
}

/**
 * @brief	Get the length of a cryptex HMAC.
 * @param	cryptex		a pointer to the head of the cryptex object to be examined.
 * @return	the length, in bytes, of the cryptex HMAC.
 */
uint64_t cryptex_hmac_length(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return head->length.hmac;
}

/**
 * @brief	Get the length of a cryptex object's encrypted data body.
 * @param	cryptex		a pointer to the head of the cryptex object to be examined.
 * @return	the length, in bytes, of the cryptex encrypted data body.
 */
uint64_t cryptex_body_length(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return head->length.body;
}

/**
 * @brief	Get the original length of a cryptex object's data buffer.
 * @param	cryptex		a pointer to the head of the cryptex object to be examined.
 * @return	the length, in bytes, of the cryptex object's original data.
 */
uint64_t cryptex_original_length(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return head->length.original;
}

/**
 * @brief	Determine the total length of the data underlying a cryptex object.
 * @param	cryptex		the input cryptex object.
 * @return	the total length of the associated cryptex data in bytes.
 */
uint64_t cryptex_total_length(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return sizeof(cryptex_head_t) + (head->length.envelope + head->length.hmac + head->length.body);
}

/**
 * @brief	Get a pointer to the envelope data of a cryptex object.
 * @param	cryptex		the input cryptex object.
 * @return	a pointer to the cryptex object's envelope data.
 */
void * cryptex_envelope_data(cryptex_t *cryptex) {

	return (char *)cryptex + sizeof(cryptex_head_t);
}

/**
 * @brief	Get a pointer to the HMAC data of a cryptex object.
 * @param	cryptex		the input cryptex object.
 * @return	a pointer to the cryptex object's hmac data.
 */
// QUESTION: Rename mac -> hmac?
void * cryptex_mac_data(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return (char *)cryptex + (sizeof(cryptex_head_t) + head->length.envelope);
}

/**
 * @brief	Get a pointer to the encrypted data body of a cryptex object.
 * @param	cryptex		the input cryptex object.
 * @return	a pointer to the cryptex object's encrypted data body.
 */
void * cryptex_body_data(cryptex_t *cryptex) {

	cryptex_head_t *head = (cryptex_head_t *)cryptex;

	return (char *)cryptex + (sizeof(cryptex_head_t) + head->length.envelope + head->length.hmac);
}

/**
 * @brief	Allocate a new cryptex object.

 */
void * cryptex_alloc(uint64_t envelope, uint64_t hmac, uint64_t original, uint64_t body) {

	cryptex_t *cryptex;
	cryptex_head_t *head;

	if (!(cryptex = mm_alloc(sizeof(cryptex_head_t) + envelope + hmac + body))) {
		log_pedantic("Cryptex memory allocation failed.");
		return NULL;
	}

	head = (cryptex_head_t *)cryptex;
	head->length.envelope = envelope;
	head->length.hmac = hmac;
	head->length.original = original;
	head->length.body = body;

	return cryptex;
}

/**
 * @brief	Free a cryptex object.
 * @param	cryptex		the cryptex object to be freed.
 * @return	This function returns no value.
 */
void cryptex_free(cryptex_t *cryptex) {

	free(cryptex);

	return;
}
