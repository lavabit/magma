
/**
 * @file /magma/core/strings/multi.c
 *
 * @brief	Functions for handling the multi-type structure.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

// implement a generic interface for a variant type; or a single struct/union/type that
// can hold all of the commonly used data types

/**
 * @brief	Return an empty multi-type object.
 * @return	an empty multi-type object.
 */
multi_t mt_get_null(void) {
	multi_t multi = {
		.type = EMPTY,
		.val = {NULL}
	};
	return multi;
}

/**
 * @brief	Determine whether a multi-type object is empty.
 * @note	All numeric (including boolean and floating point) types will always return false.
 * @param	multi	the multi-type object to be examined.
 * @return	true if the object is empty (by type or value), or false otherwise.
 */
bool_t mt_is_empty(multi_t multi) {

	bool_t result = false;

	if (multi.type == EMPTY)
		return true;

	switch (multi.type) {

	// Strings
	case (M_TYPE_STRINGER):
		result = st_empty(multi.val.st);
		break;

	case (M_TYPE_NULLER):
		result = ns_empty(multi.val.ns);
		break;

	case (M_TYPE_BLOCK):
		result = (multi.val.bl ? false : true);
		break;

	case (M_TYPE_BOOLEAN):
	case (M_TYPE_UINT64):
	case (M_TYPE_UINT32):
	case (M_TYPE_UINT16):
	case (M_TYPE_UINT8):
	case (M_TYPE_INT64):
	case (M_TYPE_INT32):
	case (M_TYPE_INT16):
	case (M_TYPE_INT8):
	case (M_TYPE_FLOAT):
	case (M_TYPE_DOUBLE):
		result = false;
		break;

	default:
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The mt_is_empty function was called on an unsupported type. {type = %s = %u}", type(multi.type), multi.type);
		break;
	}

	return result;
}

/**
 * @brief	Determine whether a multi-type object is a number.
 * @param	multi	the multi-type object to be examined.
 * return	true if the input object is any integer, boolean, or floating point; false otherwise.
 */
bool_t mt_is_number(multi_t multi) {

	bool_t result = false;

	switch (multi.type) {
	case (M_TYPE_BOOLEAN):
	case (M_TYPE_STRINGER):
	case (M_TYPE_NULLER):
	case (M_TYPE_BLOCK):
	case (EMPTY):
		break;
	case (M_TYPE_UINT64):
	case (M_TYPE_UINT32):
	case (M_TYPE_UINT16):
	case (M_TYPE_UINT8):
	case (M_TYPE_INT64):
	case (M_TYPE_INT32):
	case (M_TYPE_INT16):
	case (M_TYPE_INT8):
	case (M_TYPE_FLOAT):
	case (M_TYPE_DOUBLE):
		result = true;
		break;
	default:
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The mt_is_number function was called on an unsupported type. {type = %s = %u}", type(multi.type), multi.type);
		break;
	}

	return result;
}

/**
 * @brief	Get the value of a numerical multi-type object.
 * @param	multi	the multi-type object to be examined.
 * @return	the numerical value of the input object, or 0 on failure.
 */
uint64_t mt_get_number(multi_t multi) {

	uint64_t result = 0;

	switch (multi.type) {

	case (M_TYPE_UINT64):
		result = multi.val.u64;
		break;
	case (M_TYPE_UINT32):
		result = (uint64_t)multi.val.u32;
		break;
	case (M_TYPE_UINT16):
		result = (uint64_t)multi.val.u16;
		break;
	case (M_TYPE_UINT8):
		result = (uint64_t)multi.val.u8;
		break;
	case (M_TYPE_INT64):
		result = (uint64_t)multi.val.i64;
		break;
	case (M_TYPE_INT32):
		result = (uint64_t)multi.val.i32;
		break;
	case (M_TYPE_INT16):
		result = (uint64_t)multi.val.i16;
		break;
		case (M_TYPE_INT8):
		result = (uint64_t)multi.val.i8;
		break;
	case (M_TYPE_FLOAT):
		result = (uint64_t)multi.val.fl;
		break;
	case (M_TYPE_DOUBLE):
		result = (uint64_t)multi.val.dbl;
		break;

	case (M_TYPE_BOOLEAN):
	case (M_TYPE_STRINGER):
	case (M_TYPE_NULLER):
	case (M_TYPE_BLOCK):
	case (EMPTY):
	default:
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The mt_get_number function was called on an unsupported type. {type = %s = %u}", type(multi.type), multi.type);
		break;
	}

	return result;
}

/**
 * @brief	Get a character pointer to the value of a multi-type object.
 * @param	multi	a pointer to the multi-type object to be examined.
 * @param	a pointer to the value of the input object, or NULL on failure.
 * @return	null on error
 */
char * mt_get_char(const multi_t *multi) {

	char *result = NULL;

	if (multi == NULL) {
		return NULL;
	}

	switch (multi->type) {

	// Strings
	case (M_TYPE_STRINGER):
		result = st_char_get(multi->val.st);
		break;

	case (M_TYPE_NULLER):
		result = multi->val.ns;
		break;

	case (M_TYPE_BLOCK):
		result = multi->val.bl;
		break;

	// Boolean
	case (M_TYPE_BOOLEAN):
		result = (char *)&(multi->val.binary);
		break;

		// Unsigned integers
	case (M_TYPE_UINT64):
		result = (char *)&(multi->val.u64);

		break;
	case (M_TYPE_UINT32):
		result = (char *)&(multi->val.u32);
		break;
	case (M_TYPE_UINT16):
		result = (char *)&(multi->val.u16);
		break;
	case (M_TYPE_UINT8):
		result = (char *)&(multi->val.u8);
		break;

		// Signed integers
	case (M_TYPE_INT64):
		result = (char *)&(multi->val.i64);
		break;
	case (M_TYPE_INT32):
		result = (char *)&(multi->val.i32);
		break;
	case (M_TYPE_INT16):
		result = (char *)&(multi->val.i16);
		break;
	case (M_TYPE_INT8):
		result = (char *)&(multi->val.i8);
		break;

	case (M_TYPE_FLOAT):
		result = (char *)&(multi->val.fl);
		break;
	case (M_TYPE_DOUBLE):
		result = (char *)&(multi->val.dbl);
		break;

	default:
		log_options(M_LOG_INFO | M_LOG_STACK_TRACE, "The mt_get_char function was called on an unsupported data type. {type = %s = %u}", type(multi->type), multi->type);
		break;
	}

	return result;
}

/**
 * @brief	Get the length of the data associated with a multi-type object.
 * @param	multi	the multi-type object to be examined.
 * @return	the length in bytes of the data associated with the input object, or 0 on failure.
 *
 */
size_t mt_get_length(multi_t multi) {

	size_t result = 0;

	switch (multi.type) {

	// Strings
	case (M_TYPE_STRINGER):
		result = st_length_get(multi.val.st);
		break;

	case (M_TYPE_NULLER):
		result = ns_length_get(multi.val.ns);
		break;

	case (M_TYPE_BOOLEAN):
		result = sizeof(bool_t);
		break;

		// Unsigned integers
	case (M_TYPE_UINT64):
		result = sizeof(uint64_t);

		break;
	case (M_TYPE_UINT32):
		result = sizeof(uint32_t);
		break;
	case (M_TYPE_UINT16):
		result = sizeof(uint16_t);
		break;
	case (M_TYPE_UINT8):
		result = sizeof(uint8_t);
		break;

		// Signed integers
	case (M_TYPE_INT64):
		result = sizeof(int64_t);
		break;
	case (M_TYPE_INT32):
		result = sizeof(int32_t);
		break;
	case (M_TYPE_INT16):
		result = sizeof(int16_t);
		break;
	case (M_TYPE_INT8):
		result = sizeof(int8_t);
		break;

	case (M_TYPE_FLOAT):
		result = sizeof(float_t);
		break;
	case (M_TYPE_DOUBLE):
		result = sizeof(double_t);
		break;

	default:
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The mt_get_length function was called on an unsupported data type. {type = %s = %u}", type(multi.type), multi.type);
		break;
	}

	return result;
}

/**
 * @brief	Get the data type associated with a multi-type object.
 * @param	multi	the multi-type object to be examined.
 * @return	the data type of the input object, or EMPTY on failure.
 */
// QUESTION: Wtf is this routine coded like this?
M_TYPE mt_get_type(multi_t multi) {

	M_TYPE result = EMPTY;

	switch (multi.type) {

	// Strings
	case (M_TYPE_STRINGER):
		result = M_TYPE_STRINGER;
		break;

	case (M_TYPE_NULLER):
		result = M_TYPE_NULLER;
		break;

	case (M_TYPE_PLACER):
		result = M_TYPE_PLACER;
		break;

	// Boolean
	case (M_TYPE_BOOLEAN):
		result = M_TYPE_BOOLEAN;
		break;

		// Unsigned integers
	case (M_TYPE_UINT64):
		result = M_TYPE_UINT64;
		break;
	case (M_TYPE_UINT32):
		result = M_TYPE_UINT32;
		break;
	case (M_TYPE_UINT16):
		result = M_TYPE_UINT16;
		break;
	case (M_TYPE_UINT8):
		result = M_TYPE_UINT8;
		break;

		// Signed integers
	case (M_TYPE_INT64):
		result = M_TYPE_INT64;
		break;
	case (M_TYPE_INT32):
		result = M_TYPE_INT32;
		break;
	case (M_TYPE_INT16):
		result = M_TYPE_INT16;
		break;
	case (M_TYPE_INT8):
		result = M_TYPE_INT8;
		break;

	case (M_TYPE_FLOAT):
		result = M_TYPE_FLOAT;
		break;
	case (M_TYPE_DOUBLE):
		result = M_TYPE_DOUBLE;
		break;

	default:
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The function mt_get_type was called using an unsupported type. {type = %s = %u}", type(multi.type), multi.type);
		break;
	}

	return result;
}

/**
 * @brief	Set the data type of a multi-type data object.
 * @param	multi	the multi-type object to be adjusted.
 * @param	target	the value of the new data type for the input object.
 * @return	a copy of the modified multi-type data object.
 */
multi_t mt_set_type(multi_t multi, M_TYPE target) {

	switch (target) {

	// Strings
	case (M_TYPE_STRINGER):
		multi.type = M_TYPE_STRINGER;
		break;

	case (M_TYPE_NULLER):
		multi.type = M_TYPE_NULLER;
		break;

	case (M_TYPE_PLACER):
		multi.type = M_TYPE_PLACER;
		break;

	// Boolean
	case (M_TYPE_BOOLEAN):
		multi.type = M_TYPE_BOOLEAN;
		break;

	// Unsigned integers
	case (M_TYPE_UINT64):
		multi.type = M_TYPE_UINT64;
		break;
	case (M_TYPE_UINT32):
		multi.type = M_TYPE_UINT32;
		break;
	case (M_TYPE_UINT16):
		multi.type = M_TYPE_UINT16;
		break;
	case (M_TYPE_UINT8):
		multi.type = M_TYPE_UINT8;
		break;

	// Signed integers
	case (M_TYPE_INT64):
		multi.type = M_TYPE_INT64;
		break;
	case (M_TYPE_INT32):
		multi.type = M_TYPE_INT32;
		break;
	case (M_TYPE_INT16):
		multi.type = M_TYPE_INT16;
		break;
	case (M_TYPE_INT8):
		multi.type = M_TYPE_INT8;
		break;

	case (M_TYPE_FLOAT):
		multi.type = M_TYPE_FLOAT;
		break;
	case (M_TYPE_DOUBLE):
		multi.type = M_TYPE_DOUBLE;
		break;

	default:
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The function mt_set_type was called using an unsupported type. {type = %s = %u}", type(target), target);
		multi.type = EMPTY;
		break;
	}

	return multi;
}

/**
 * @brief	Free the data underlying a multi-type data object (for managed and null-terminated strings).
 * @param	multi	the multi-type object to be freed.
 * @return	This function returns no value.
 */
void mt_free(multi_t multi) {

	switch (multi.type) {
	case (M_TYPE_STRINGER):
		st_free(multi.val.st);
		break;

	case (M_TYPE_NULLER):
		ns_free(multi.val.ns);
		break;

	default:
		break;
	}
}

/**
 * @brief	Duplicate a multi-type object.
 * @param	multi	the multi-type object to be examined.
 * @return	a copy of the input object (a deep copy for managed strings and null-terminated strings).
 */
// QUESTION: Also a lot of unnecessary code
multi_t mt_dupe(multi_t multi) {

	multi_t result = mt_get_null();

	switch (multi.type) {

	// Strings
	case (M_TYPE_STRINGER):
		result.type = M_TYPE_STRINGER;
		result.val.st = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, multi.val.st);
		break;

	case (M_TYPE_NULLER):
		result.type = M_TYPE_NULLER;
		result.val.ns = ns_dupe(multi.val.ns);
		break;

		// Boolean
	case (M_TYPE_BOOLEAN):
		result.type = M_TYPE_BOOLEAN;
		result.val.binary = multi.val.binary;
		break;

		// Unsigned integers
	case (M_TYPE_UINT64):
		result.type = M_TYPE_UINT64;
		result.val.u64 = multi.val.u64;
		break;
	case (M_TYPE_UINT32):
		result.type = M_TYPE_UINT32;
		result.val.u32 = multi.val.u32;
		break;
	case (M_TYPE_UINT16):
		result.type = M_TYPE_UINT16;
		result.val.u16 = multi.val.u16;
		break;
	case (M_TYPE_UINT8):
		result.type = M_TYPE_UINT8;
		result.val.u8 = multi.val.u8;
		break;

		// Signed integers
	case (M_TYPE_INT64):
		result.type = M_TYPE_INT64;
		result.val.i64 = multi.val.i64;
		break;
	case (M_TYPE_INT32):
		result.type = M_TYPE_INT32;
		result.val.i32 = multi.val.i32;
		break;
	case (M_TYPE_INT16):
		result.type = M_TYPE_INT16;
		result.val.i16 = multi.val.i16;
		break;
	case (M_TYPE_INT8):
		result.type = M_TYPE_INT8;
		result.val.i8 = multi.val.i8;
		break;

	case (M_TYPE_FLOAT):
		result.type = M_TYPE_FLOAT;
		result.val.fl = multi.val.fl;
		break;
	case (M_TYPE_DOUBLE):
		result.type = M_TYPE_DOUBLE;
		result.val.dbl = multi.val.dbl;
		break;

	default:
		log_pedantic("The function mt_dupe was called using an unsupported type. {type = %s = %u}", type(multi.type), multi.type);
		break;
	}

	return result;
}

/**
 * @brief	Check to see if the values of two multi-type objects are identical.
 * @note	The types of the two input objects must match, or be a comparison between a managed string and null-terminated string.
 * @param	one		the first multi-type object to be compared.
 * @param	two		the second multi-type object to be compared.
 * @return	true if the two objects have identical values; false if they don't, or on failure.
 */
bool_t ident_mt_mt(multi_t one, multi_t two) {

	bool_t result = false;

	if (mt_get_type(one) != mt_get_type(two)) {

		// Stringer + Nuller
		if (mt_get_type(one) == M_TYPE_STRINGER && mt_get_type(two) == M_TYPE_NULLER) {
			result = (st_cmp_cs_eq(one.val.st, PLACER(two.val.ns, ns_length_get(two.val.ns))) == 0);
		} else if (mt_get_type(one) == M_TYPE_NULLER && mt_get_type(two) == M_TYPE_STRINGER) {
			result = (st_cmp_cs_eq(PLACER(one.val.ns, ns_length_get(one.val.ns)), two.val.st) == 0);
		}

		// For now, this function only supports comparing non-matched types if both types are strings.
		else {
			log_pedantic("Invalid multi-type comparison.");
		}

	}
	// Handle matching types.
	else {

		switch (one.type) {

			// Strings
			case (M_TYPE_STRINGER):
				result = (st_cmp_cs_eq(one.val.st, two.val.st) == 0);
				break;

			case (M_TYPE_NULLER):
				result = (st_cmp_cs_eq(PLACER(one.val.ns, ns_length_get(one.val.ns)), PLACER(two.val.ns, ns_length_get(two.val.ns))) == 0);
				break;

			// Boolean
			case (M_TYPE_BOOLEAN):
				if (one.val.binary == two.val.binary) {
					result = true;
				}
				break;

			// Unsigned integers
			case (M_TYPE_UINT64):
				if (one.val.u64 == two.val.u64) {
					result = true;
				}
				break;
			case (M_TYPE_UINT32):
				if (one.val.u32 == two.val.u32) {
					result = true;
				}
				break;
			case (M_TYPE_UINT16):
				if (one.val.u16 == two.val.u16) {
					result = true;
				}
				break;
			case (M_TYPE_UINT8):
				if (one.val.u8 == two.val.u8) {
					result = true;
				}
				break;

			// Signed integers
			case (M_TYPE_INT64):
				if (one.val.i64 == two.val.i64) {
					result = true;
				}
				break;
			case (M_TYPE_INT32):
				if (one.val.i32 == two.val.i32) {
					result = true;
				}
				break;
			case (M_TYPE_INT16):
				if (one.val.i16 == two.val.i16) {
					result = true;
				}
				break;
			case (M_TYPE_INT8):
				if (one.val.i8 == two.val.i8) {
					result = true;
				}
				break;

			// Decimal numbers
			case (M_TYPE_FLOAT):
				if (one.val.fl == two.val.fl) {
					result = true;
				}
				break;
			case (M_TYPE_DOUBLE):
				if (one.val.dbl == two.val.dbl) {
					result = true;
				}
				break;

			default:
				log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The ident_mt_mt function was called on an unsupported data type.");
				result = false;
				break;
		}

	}
	return result;
}

/**
 * @brief	Compare the values of two multi-type objects of the same data type.
 * @note	The types of the two input objects must match, or be a comparison between a managed string and null-terminated string.
 * @param	one		the first multi-type object to be compared.
 * @param	two		the second multi-type object to be compared.
 * @return
 *
 *
 *
 */
// QUESTION: Why do we have this function AND the one above?
// QUESTION: Not sure on the return at all. Why would failure and equality both return 0 for certain data types?
int32_t cmp_mt_mt(multi_t one, multi_t two) {

	int32_t result = 0;

	if (mt_get_type(one) != mt_get_type(two)) {

		if (mt_get_type(one) == M_TYPE_STRINGER && mt_get_type(two) == M_TYPE_NULLER) {
			result = st_cmp_cs_eq(one.val.st, NULLER(two.val.ns));
		} else if (mt_get_type(one) == M_TYPE_NULLER && mt_get_type(two) == M_TYPE_STRINGER) {
			result = st_cmp_cs_eq(NULLER(one.val.ns), two.val.st);
		}

		// For now, this function only supports comparing non-matched types if both types are strings.
		else {
			log_pedantic("Invalid multi-type comparison.");
		}

	}
	// Handle matching types.
	else {

		switch (one.type) {

			// Strings
			case (M_TYPE_STRINGER):
				result = st_cmp_cs_eq(one.val.st, two.val.st);
				break;

			case (M_TYPE_NULLER):
				result = st_cmp_cs_eq(NULLER(one.val.ns), NULLER(two.val.ns));
				break;

			// The tuple below should return -1 if one > two, 1 if one < two and 0 if they are equal. Note that is only works because the second
			// conditional results in 0 if evaluated as false and 1 if evaluated as true. Were relying on the C implementation to function as expected
			// so we test for this behavior as part of our sanity checks.

			// Boolean
			case (M_TYPE_BOOLEAN):
				result = (one.val.binary == false && two.val.binary == true) ? -1 : one.val.binary == true && two.val.binary == false;
				break;

			// Unsigned integers
			case (M_TYPE_UINT64):
				result = (one.val.u64 < two.val.u64) ? -1 : one.val.u64 > two.val.u64;
				break;

			case (M_TYPE_UINT32):
				result = (one.val.u32 < two.val.u32) ? -1 : one.val.u32 > two.val.u32;
				break;

			case (M_TYPE_UINT16):
				result = (one.val.u16 < two.val.u16) ? -1 : one.val.u16 > two.val.u16;
				break;

			case (M_TYPE_UINT8):
				result = (one.val.u8 < two.val.u8) ? -1 : one.val.u8 > two.val.u8;
				break;

			// Signed integers
			case (M_TYPE_INT64):
				result = (one.val.i64  < two.val.i64) ? -1 : one.val.i64 > two.val.i64;
				break;

			case (M_TYPE_INT32):
				result = (one.val.i32 < two.val.i32) ? -1 : one.val.i32 > two.val.i32;
				break;

			case (M_TYPE_INT16):
				result = (one.val.i16 < two.val.i16) ? -1 : one.val.i16 > two.val.i16;
				break;

			case (M_TYPE_INT8):
				result = (one.val.i8 < two.val.i8) ? -1 : one.val.i8 > two.val.i8;
				break;

			// Decimal numbers
			case (M_TYPE_FLOAT):
				result = (one.val.fl < two.val.fl) ? -1 : one.val.fl > two.val.fl;
				break;

			case (M_TYPE_DOUBLE):
				result = (one.val.dbl < two.val.dbl) ? -1 : one.val.dbl > two.val.dbl;
				break;

			default:
				log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The cmp_mt_mt function was called on an unsupported data type.");
				result = false;
				break;
		}
	}

	return result;
}
