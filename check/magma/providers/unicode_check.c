
/**
 * @file /check/providers/unicode_check.c
 *
 * @brief The unit tests for functions which provide Unicode functionality.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_unicode_valid(stringer_t *errmsg) {

	stringer_t *utf8 = MANAGEDBUF(128);

	if (!utf8_valid_st(PLACER("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26)) ||
		!utf8_valid_st(PLACER("abcdefghijklmnopqrstuvwxyz", 26))) {
		st_sprint(errmsg, "The Unicode validity test failed.");
		return false;
	}

	if (!(utf8 = hex_decode_st(NULLER("C380C381C382C383C384C385C386C387C388C389C38AC38BC38CC38DC38EC38F" \
		"C390C391C392C393C394C395C396C398C399C39AC39BC39CC39D"), utf8)) || !utf8_valid_st(utf8)) {
		st_sprint(errmsg, "The Unicode validity test failed.");
		return false;
	}

	return true;
}

/**
 * @brief Checks whether an invalid UTF8 string results in anything but false from the validity function, or zero
 * 		from the length function.
 * @return Returns an error message in an allocated buffer.
 */
bool_t check_unicode_invalid(stringer_t *errmsg) {

	stringer_t *utf8 = MANAGEDBUF(128);

	if (!(utf8 = hex_decode_st(NULLER("C380C3"), utf8)) ||
		(utf8_valid_st(utf8) != false) ||
		(utf8_length_st(utf8) != 0)) {
		st_sprint(errmsg, "The Unicode invalidity test failed.");
		return false;
	}

	return true;
}

bool_t check_unicode_length(stringer_t *errmsg) {

	if (utf8_length_st(PLACER("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26)) != 26 ||
		utf8_length_st(PLACER("abcdefghijklmnopqrstuvwxyz", 26)) != 26) {
		st_sprint(errmsg, "The Unicode length check failed to return the right answer for an ASCII string.");
		return false;
	}

	if (utf8_length_st(hex_decode_st(NULLER("C380C381C382C383C384C385C386C387C388C389C38AC38BC38CC38DC38EC38FC390C39" \
			"1C392C393C394C395C396C398C399C39AC39BC39CC39D"), MANAGEDBUF(128))) != 29) {
		st_sprint(errmsg, "The Unicode length check failed to return the right answer for a UTF8 string.");
		return false;
	}

	if (utf8_length_st(hex_decode_st(NULLER("E298BA20E299A1204C61646172204C657669736F6E20E299A520E298BB"), MANAGEDBUF(128))) != 21) {
		st_sprint(errmsg, "The Unicode length check failed to return the right answer for a UTF8 string.");
		return false;
	}

	// Try measuring the length of a string which leads off with the UTF8 byte order mark. The byte order mark should be ignored.
	if (utf8_length_st(hex_decode_st(NULLER("EFBBBF70C3A17373776F7264"), MANAGEDBUF(128))) != 8) {
		st_sprint(errmsg, "The Unicode length check failed to return the right answer for a UTF8 string which begins with the byte order mark.");
		return false;
	}

	return true;
}

