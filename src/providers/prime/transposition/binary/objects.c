
/**
 * @file /magma/providers/prime/transposition/binary/objects.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

chr_t * prime_types[] = {
	"ORGANIZATIONAL SIGNET",
	"ORGANIZATIONAL KEY",
	"ENCRYPTED ORGANIZATIONAL KEY",
	"USER SIGNING REQUEST",
	"USER SIGNET",
	"USER KEY",
	"ENCRYPTED USER KEY",
	"ENCRYPTED MESSAGE"
};

chr_t * prime_object_type(prime_artifact_type_t type) {

	chr_t *result = NULL;

	switch (type) {
		case (PRIME_ORG_SIGNET):
			result = prime_types[0];
			break;
		case (PRIME_ORG_KEY):
			result = prime_types[1];
			break;
		case (PRIME_ORG_KEY_ENCRYPTED):
			result = prime_types[2];
			break;
		case (PRIME_USER_SIGNING_REQUEST):
			result = prime_types[3];
			break;
		case (PRIME_USER_SIGNET):
			result = prime_types[4];
			break;
		case (PRIME_USER_KEY):
			result = prime_types[5];
			break;
		case (PRIME_USER_KEY_ENCRYPTED):
			result = prime_types[6];
			break;
		case (PRIME_MESSAGE_ENCRYPTED):
			result = prime_types[7];
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}
	return result;
}

void prime_object_free(prime_object_t *object) {

	if (object) {
		mm_free(object);
	}

	return;
}

prime_object_t * prime_object_alloc(prime_artifact_type_t type, prime_size_t size, prime_size_t fields) {

	prime_object_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_object_t) + (fields * sizeof(prime_field_t))))) {
		log_pedantic("PRIME object allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_object_t) + (fields * sizeof(prime_field_t)));
	result->type = type;
	result->size = size;
	result->count = fields;

	for (int_t i = 0; i < fields; i++) {
		mm_wipe(&(result->fields[i]), sizeof(prime_field_t));
	}

	return result;
}

size_t prime_object_size_max(prime_artifact_type_t type) {

	size_t max = 0;

	switch (type) {
		case (PRIME_ORG_SIGNET):
		case (PRIME_ORG_KEY):
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
		case (PRIME_USER_KEY_ENCRYPTED):
			max = PRIME_MAX_3_BYTE;
			break;
		case (PRIME_MESSAGE_ENCRYPTED):
			max = PRIME_MAX_4_BYTE;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}
	return max;
}

size_t prime_object_size_min(prime_artifact_type_t type) {

	size_t min = 0;

	switch (type) {
		case (PRIME_ORG_KEY):
		case (PRIME_USER_KEY):
			min = 68;
			break;
		case (PRIME_ORG_SIGNET):
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY_ENCRYPTED):
		case (PRIME_MESSAGE_ENCRYPTED):
			min = 0;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}
	return min;
}

