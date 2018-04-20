
/**
 * @file /magma/core/type.c
 *
 * @brief	A function for translating the M_TYPE enum into an equivalent string, typically so that a type code can be recorded as a string in error messages.
 */

#include "core.h"

/**
 *
 * Takes a type code and returns the fully enumerated string associated with that type.
 *
 * @param type The type code to evaluate.
 * @return Null terminated string containing type name. String is stored in static buffer and returned as a pointer.
 */
char * type(M_TYPE type) {

	char *answer = "";

	switch (type) {

	// Strings
	case (M_TYPE_STRINGER):
		answer = "M_TYPE_STRINGER";
		break;

	case (M_TYPE_NULLER):
		answer = "M_TYPE_NULLER";
		break;

	case (M_TYPE_PLACER):
		answer = "M_TYPE_PLACER";
		break;

	case (M_TYPE_BLOCK):
		answer = "M_TYPE_BLOCK";
		break;

	// Enum
	case (M_TYPE_ENUM):
		answer = "M_TYPE_ENUM";
		break;

	// Multi
	case (M_TYPE_MULTI):
		answer = "M_TYPE_MULTI";
		break;

	// Boolean
	case (M_TYPE_BOOLEAN):
		answer = "M_TYPE_BOOLEAN";
		break;

	// Unsigned integers
	case (M_TYPE_UINT64):
		answer = "M_TYPE_UINT64";
		break;
	case (M_TYPE_UINT32):
		answer = "M_TYPE_UINT32";
		break;
	case (M_TYPE_UINT16):
		answer = "M_TYPE_UINT16";
		break;
	case (M_TYPE_UINT8):
		answer = "M_TYPE_UINT8";
		break;

	// Signed integers
	case (M_TYPE_INT64):
		answer = "M_TYPE_INT64";
		break;
	case (M_TYPE_INT32):
		answer = "M_TYPE_INT32";
		break;
	case (M_TYPE_INT16):
		answer = "M_TYPE_INT16";
		break;
	case (M_TYPE_INT8):
		answer = "M_TYPE_INT8";
		break;

	case (M_TYPE_FLOAT):
		answer = "M_TYPE_FLOAT";
		break;
	case (M_TYPE_DOUBLE):
		answer = "M_TYPE_DOUBLE";
		break;

	default:
		answer = "EMPTY";
		break;
	}
	return answer;
}

