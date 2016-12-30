
/**
 * @file /magma/src/providers/prime/formats/armored/pem.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

stringer_t * prime_pem_wrap(stringer_t *object) {

	long crc;

	if (st_empty(object)) {
		return NULL;
	}

	crc = 0;

	return NULL;
}

stringer_t * prime_pem_unwrap(stringer_t *pem) {

	return NULL;
}
