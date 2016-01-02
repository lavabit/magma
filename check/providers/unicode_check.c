
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

chr_t * check_unicode_valid(void) {

	stringer_t *utf8 = MANAGEDBUF(128);

	if (!utf8_valid_st(PLACER("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26)) ||
		!utf8_valid_st(PLACER("abcdefghijklmnopqrstuvwxyz", 26))) {
		return ns_dupe("The Unicode validity test failed.");
	}

	if (!(utf8 = hex_decode_st(NULLER("C380C381C382C383C384C385C386C387C388C389C38AC38BC38CC38DC38EC38F" \
		"C390C391C392C393C394C395C396C398C399C39AC39BC39CC39D"), utf8)) || !utf8_valid_st(utf8)) {
		return ns_dupe("The Unicode validity test failed.");
	}

	return NULL;
}

/**
 * @brief Checks whether an invalid UTF8 string results in anything but false from the validity function, or zero
 * 		from the length function.
 * @return Returns an error message in an allocated buffer.
 */
chr_t * check_unicode_invalid(void) {

	stringer_t *utf8 = MANAGEDBUF(128);

	if (!(utf8 = hex_decode_st(NULLER("C380C3"), utf8)) ||
		(utf8_valid_st(utf8) != false) ||
		(utf8_length_st(utf8) != 0)) {
		return ns_dupe("The Unicode invalidity test failed.");
	}

	return NULL;
}

chr_t * check_unicode_length(void) {

	stringer_t *utf8 = MANAGEDBUF(128);

	if (utf8_length_st(PLACER("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26)) != 26 ||
		utf8_length_st(PLACER("abcdefghijklmnopqrstuvwxyz", 26)) != 26) {
		return ns_dupe("The Unicode length check failed to return the right answer for an ASCII string.");
	}

	if (!(utf8 = hex_decode_st(NULLER("C380C381C382C383C384C385C386C387C388C389C38AC38BC38CC38DC38EC38FC390C39" \
			"1C392C393C394C395C396C398C399C39AC39BC39CC39D"), utf8)) || utf8_length_st(utf8) != 29) {
		return ns_dupe("The Unicode length check failed to return the right answer for a UTF8 string.");
	}

	if (!(utf8 = hex_decode_st(NULLER("E298BA20E299A1204C61646172204C657669736F6E20E299A520E298BB"), utf8)) ||
		utf8_length_st(utf8) != 21) {
		return ns_dupe("The Unicode length check failed to return the right answer for a UTF8 string.");
	}
	return NULL;
}

