
/**
 * @file /magma/providers/cryptography/digest.c
 *
 * @brief  Functions used to handle the message digest (aka hash function) primitives.
 */

#include "magma.h"

digest_t * digest_name(stringer_t *name) {

	const EVP_MD *result = NULL;
	if (!st_empty(name) && !(result = EVP_get_digestbyname_d(st_char_get(name)))) {
		log_pedantic("The name provided did not match any of the available digest methods. { name = %.*s / error = %s }",
			st_length_int(name), st_char_get(name), ssl_error_string(MEMORYBUF(256), 256));
	}

	return (digest_t *)result;
}

digest_t * digest_id(int_t id) {

	const EVP_MD *result = NULL;
	if (!(result = EVP_get_digestbyname_d(OBJ_nid2sn_d(id)))) {
		log_pedantic("The id provided did not match any of the available digest methods. { id = %i / name = %s / error = %s }",
			id, OBJ_nid2sn_d(id), ssl_error_string(MEMORYBUF(256), 256));
	}

	return (digest_t *)result;
}

int_t digest_length_output(const digest_t *digest) {

	int_t result = -1;

	if (digest && (result = EVP_MD_size_d((const EVP_MD *)digest)) < 0) {
		log_pedantic("Unable to determine the digest output length. { digest = %s / error = %s }",
			OBJ_nid2sn_d(EVP_MD_type_d((const EVP_MD *)digest)), ssl_error_string(MEMORYBUF(256), 256));
	}

	return result;
}
