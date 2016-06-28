
/**
 * @file /check/core/string_check.c
 *
 * @brief Unit tests for tree based indexes.
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 *
 */

#include "magma_check.h"

stringer_t *string_check_constant = CONSTANT("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

bool_t check_string_alloc(uint32_t check) {

	size_t len;
	stringer_t *s;

	if (!(s = st_alloc_opts(check, st_length_int(string_check_constant)))) {
		return false;
	}

	len = snprintf(st_char_get(s), st_length_int(string_check_constant) + 1, "%.*s", st_length_int(string_check_constant), st_char_get(string_check_constant));
	if ((check & MAPPED_T) || (check & MANAGED_T)) {
		st_length_set(s, len);
	}

	if (memcmp(st_char_get(s), st_char_get(string_check_constant), st_length_get(string_check_constant))) {
		st_free(s);
		return false;
	}

	st_free(s);

	return true;
}

bool_t check_string_dupe(uint32_t check) {

	size_t len;
	stringer_t *s, *d;

	if (!(s = st_alloc_opts(check, st_length_int(string_check_constant)))) {
		return false;
	}

	len = snprintf(st_char_get(s), st_length_int(string_check_constant) + 1, "%.*s", st_length_int(string_check_constant), st_char_get(string_check_constant));
	if ((check & MAPPED_T) || (check & MANAGED_T)) {
		st_length_set(s, len);
	}

	if (!(d = st_dupe(s))) {
		st_free(s);
		return false;
	}

	if (memcmp(st_char_get(d), st_char_get(s), st_length_get(s))) {
		st_free(s);
		st_free(d);
		return false;
	}

	st_free(s);
	st_free(d);

	return true;
}

bool_t check_string_realloc(uint32_t check) {

	size_t len;
	stringer_t *s, *swap;

	if (!(s = st_alloc_opts(check, 1)) || !(swap = st_realloc(s, st_length_get(string_check_constant)))) {
		if (s) {
			st_free(s);
		}
		return false;
	}

	// Since mapped allocations are page aligned, we reallocate to a larger than needed size to ensure an actual reallocation occurs.
	else if ((check & MAPPED_T) && !(swap = st_realloc(s, st_length_get(string_check_constant) + (getpagesize() * 128)))) {
		if (s) {
			st_free(s);
		}
		return false;
	}

	// Jointed strings allow reallocation without changing the address of s, of if the string is jointed and s changes, error out.
	else if ((check & JOINTED) && swap != s) {
		st_free(swap);
		st_free(s);
		return false;
	}

	// Contiguous strings will require the address of s to change, so if it doesn't error out. Except for the mapped type, since the jointed flag doesn't apply to it.
	else if (!(check & JOINTED) && !(check & MAPPED_T) && swap == s) {
		st_free(s);
		return false;
	}

	// For contiguous types, free the original string and replace it with the value of swap.
	else if (swap != s) {
		st_free(s);
		s = swap;
	}

	len = snprintf(st_char_get(s), st_length_int(string_check_constant) + 1, "%.*s", st_length_int(string_check_constant), st_char_get(string_check_constant));
	if ((check & MAPPED_T) || (check & MANAGED_T)) {
		st_length_set(s, len);
	}

	else if (memcmp(st_char_get(s), st_char_get(string_check_constant), st_length_get(string_check_constant))) {
		return false;
	}

	// Enlarge a buffer by a factor of 128 and make sure the data it contained wasn't changed. For managed or mapped strings, multiply the available space.
	if ((check & (MANAGED_T | MAPPED_T)) && !(swap = st_realloc(s, st_avail_get(s) * 128))) {
		st_free(s);
		return false;
	}

	// For other string types we multiply the space used by the string since the original allocation size isn't tracked.
	else if (!(check & (MANAGED_T | MAPPED_T)) && !(swap = st_realloc(s, st_length_get(s) * 128))) {
		st_free(s);
		return false;
	}

	// Since mapped allocations are page aligned, we reallocate to a larger than needed size to ensure an actual reallocation occurs.
	else if ((check & MAPPED_T) && !(swap = st_realloc(s, st_length_get(string_check_constant) + (getpagesize() * 128)))) {
		if (s) {
			st_free(s);
		}
		return false;
	}

	// Jointed strings allow reallocation without changing the address of s, of if the string is jointed and s changes, error out.
	else if ((check & JOINTED) && swap != s) {
		st_free(swap);
		st_free(s);
		return false;
	}

	// Contiguous strings will require the address of s to change, so if it doesn't error out. Except for the mapped type, since the jointed flag doesn't apply to it.
	else if (((check & JOINTED) ^ JOINTED) && !(check & MAPPED_T) && swap == s) {
		st_free(s);
		return false;
	}

	// For contiguous types, free the original string and replace it with the value of swap.
	else if (swap != s) {
		st_free(s);
		s = swap;
	}

	else if (memcmp(st_char_get(s), st_char_get(string_check_constant), st_length_get(string_check_constant))) {
		return false;
	}

	// Now we shrink the buffer back to the bare minimum and check the data one final time.
	if (!(swap = st_realloc(s, st_length_get(string_check_constant)))) {
		if (s) {
			st_free(s);
		}
		return false;
	} else if (swap != s) {
		st_free(s);
		s = swap;
	}

	st_free(s);
	return true;
}

bool_t check_string_import(void) {

	stringer_t *s;

	if (!(s = st_import(st_data_get(string_check_constant), st_length_int(string_check_constant)))) {
		return false;
	}

	if (memcmp(st_char_get(s), st_char_get(string_check_constant), st_length_get(string_check_constant))) {
		st_free(s);
		return false;
	}

	st_free(s);

	return true;
}

bool_t check_string_merge(void) {

	uint64_t total;
	bool_t result = true;
	stringer_t *strings[16];

	mm_set(strings, 0, sizeof(strings));

	strings[0] = st_alloc_opts(PLACER_T | JOINTED | HEAP | FOREIGNDATA, 0);
	st_data_set(strings[0], st_data_get(string_check_constant));
	st_length_set(strings[0], st_length_get(string_check_constant));

	strings[1] = st_merge_opts(NULLER_T | CONTIGUOUS | HEAP, "s", strings[0]);
	strings[2] = st_merge_opts(NULLER_T | JOINTED | HEAP, "s", strings[1]);
	strings[3] = st_merge_opts(BLOCK_T | CONTIGUOUS | HEAP, "s", strings[2]);
	strings[4] = st_merge_opts(BLOCK_T | JOINTED | HEAP, "s", strings[3]);
	strings[5] = st_merge_opts(MANAGED_T | CONTIGUOUS | HEAP, "s", strings[4]);
	strings[6] = st_merge_opts(MANAGED_T | JOINTED | HEAP, "s", strings[5]);
	strings[7] = st_merge_opts(MAPPED_T | JOINTED | HEAP, "s", strings[6]);

	strings[8] = st_merge_opts(NULLER_T | CONTIGUOUS | SECURE, "s", strings[7]);
	strings[9] = st_merge_opts(NULLER_T | JOINTED | SECURE, "s", strings[8]);
	strings[10] = st_merge_opts(BLOCK_T | CONTIGUOUS | SECURE, "s", strings[9]);
	strings[11] = st_merge_opts(BLOCK_T | JOINTED | SECURE, "s", strings[10]);
	strings[12] = st_merge_opts(MANAGED_T | CONTIGUOUS | SECURE, "s", strings[11]);
	strings[13] = st_merge_opts(MANAGED_T | JOINTED | SECURE, "s", strings[12]);
	strings[14] = st_merge_opts(MAPPED_T | JOINTED | SECURE, "s", strings[13]);

	for (int i = 0; i < 15 && strings[i]; i++) {
		for (unsigned int j = total = 0; strings[i] && j < st_length_get(strings[i]); j++) {
			total += *(st_char_get(strings[i]) + j);
		}

		if (total != 5366) {
			result = false;
		}
	}

	if (result) {

		strings[15] = st_merge_opts(MANAGED_T | JOINTED | HEAP, "snsnsnsnsnsnsnsnsnsnsnsnsnsnsnsn", string_check_constant, "\n", strings[0], "\n", strings[1], "\n", strings[2], "\n",
				strings[3], "\n", strings[4], "\n", strings[5], "\n", strings[6], "\n", strings[7], "\n", strings[8], "\n", strings[9], "\n", strings[10], "\n", strings[11],
				"\n", strings[12], "\n", strings[13], "\n", strings[14], "\n");

		for (unsigned int i = total = 0; strings[15] && i < st_length_get(strings[15]); i++) {
			total += *(st_char_get(strings[15]) + i);
		}

		if (total != (5376UL * 16)) {
			result = false;
		}

	}

	for (int i = 0; i < 16; i++) {
		st_cleanup(strings[i]);
	}

	return result;

}

bool_t check_string_print(void) {

	uint64_t total;
	bool_t result = true;
	stringer_t *strings[14];

	mm_set(strings, 0, sizeof(strings));

	strings[0] = st_aprint_opts(NULLER_T | CONTIGUOUS | HEAP, "%.*s", st_length_int(string_check_constant), st_char_get(string_check_constant));
	strings[1] = st_aprint_opts(NULLER_T | JOINTED | HEAP, "%.*s", st_length_int(strings[0]), st_char_get(strings[0]));
	strings[2] = st_aprint_opts(BLOCK_T | CONTIGUOUS | HEAP, "%.*s", st_length_int(strings[1]), st_char_get(strings[1]));
	strings[3] = st_aprint_opts(BLOCK_T | JOINTED | HEAP, "%.*s", st_length_int(strings[2]), st_char_get(strings[2]));
	strings[4] = st_aprint_opts(MANAGED_T | CONTIGUOUS | HEAP, "%.*s", st_length_int(strings[3]), st_char_get(strings[3]));
	strings[5] = st_aprint_opts(MANAGED_T | JOINTED | HEAP, "%.*s", st_length_int(strings[4]), st_char_get(strings[4]));
	strings[6] = st_aprint_opts(MAPPED_T | JOINTED | HEAP, "%.*s", st_length_int(strings[5]), st_char_get(strings[5]));

	strings[7] = st_aprint_opts(NULLER_T | CONTIGUOUS | SECURE, "%.*s", st_length_int(strings[6]), st_char_get(strings[6]));
	strings[8] = st_aprint_opts(NULLER_T | JOINTED | SECURE, "%.*s", st_length_int(strings[7]), st_char_get(strings[7]));
	strings[9] = st_aprint_opts(BLOCK_T | CONTIGUOUS | SECURE, "%.*s", st_length_int(strings[8]), st_char_get(strings[8]));
	strings[10] = st_aprint_opts(BLOCK_T | JOINTED | SECURE, "%.*s", st_length_int(strings[9]), st_char_get(strings[9]));
	strings[11] = st_aprint_opts(MANAGED_T | CONTIGUOUS | SECURE, "%.*s", st_length_int(strings[10]), st_char_get(strings[10]));
	strings[12] = st_aprint_opts(MANAGED_T | JOINTED | SECURE, "%.*s", st_length_int(strings[11]), st_char_get(strings[11]));
	strings[13] = st_aprint_opts(MAPPED_T | JOINTED | SECURE, "%.*s", st_length_int(strings[12]), st_char_get(strings[12]));

	for (int i = 0; i < 14 && strings[i]; i++) {
		for (unsigned int j = total = 0; strings[i] && j < st_length_get(strings[i]); j++) {
			total += *(st_char_get(strings[i]) + j);
		}

		if (total != 5366) {
			result = false;
		}
	}

	for (int i = 0; i < 14; i++) {
		if (strings[i])
			st_free(strings[i]);
	}

	return result;

}
