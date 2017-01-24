
/**
 * @file /check/magma/core/qp_check.c
 *
 * @brief Quoted printable encoder unit tests.
 */

#include "magma_check.h"

bool_t check_encoding_qp(void) {

	stringer_t *qp, *binary;
	byte_t buffer[QP_CHECK_SIZE];

	for (uint64_t i = 0; status() && i < QP_CHECK_ITERATIONS; i++) {

		// Fill the buffer with random data and convert the buffer to hex.
		if (rand_write(PLACER(buffer, QP_CHECK_SIZE)) != QP_CHECK_SIZE) {
			return false;
		}
		else if (!(qp = qp_encode(PLACER(buffer, QP_CHECK_SIZE)))) {
			return false;
		}

		//log_pedantic("qp = %.*s", st_length_int(qp), st_char_get(qp));

		// Convert the buffer back to binary and compare it with the original array.
		if (!(binary = qp_decode(qp))) {
			st_free(qp);
			return false;
		}
		else if (st_cmp_cs_eq(binary, PLACER(buffer, QP_CHECK_SIZE))) {
			st_free(binary);
			st_free(qp);
			return false;
		}

		st_free(binary);
		st_free(qp);
	}

	return true;
}

