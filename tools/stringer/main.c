/**
 * @file /stringer/main.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

FILE *bucket = NULL;
stringer_t *constant = CONSTANT("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

void log_enable(void) {
	if (bucket) {
		fflush(bucket);
		fclose(bucket);
		bucket = NULL;
	}
	return;
}

void log_disable(void) {
	if (!bucket) {
		bucket = fopen("/dev/null", "w+");
	}
	return;
}

bool_t check_string_alloc(char *type, uint32_t check) {

	size_t len;
	stringer_t *s;

	if (!(s = st_alloc(check, st_length_int(constant)))) {
		return false;
	}

	len = snprintf(st_char_get(s), st_length_int(constant) + 1, "%.*s", st_length_int(constant), st_char_get(constant));
	if (check & MAPPED_T || check & MANAGED_T) {
		st_length_set(s, len);
	}

	log_print("%28.28s = %.*s", type, st_length_int(s), st_char_get(s));

	if (memcmp(st_char_get(s), st_char_get(constant), st_length_get(constant))) {
		st_free(s);
		return false;
	}

	st_free(s);

	return true;
}

bool_t check_string_dupe(char *type, uint32_t check) {

	size_t len;
	stringer_t *s, *d;

	if (!(s = st_alloc(check, st_length_int(constant)))) {
		return false;
	}

	len = snprintf(st_char_get(s), st_length_int(constant) + 1, "%.*s", st_length_int(constant), st_char_get(constant));
	if (check & MAPPED_T || check & MANAGED_T) {
		st_length_set(s, len);
	}

	if (!(d = st_dupe(s))) {
		st_free(s);
		return false;
	}

	log_print("%28.28s = %.*s", type, st_length_int(d), st_char_get(d));

	if (memcmp(st_char_get(d), st_char_get(s), st_length_get(s))) {
		st_free(s);
		st_free(d);
		return false;
	}

	st_free(s);
	st_free(d);

	return true;
}

bool_t check_string_realloc(char *type, uint32_t check) {

	size_t len;
	stringer_t *s, *swap;

	if (!(s = st_alloc(check, 1)) || !(swap = st_realloc(s, st_length_get(constant)))) {
		if (s) {
			st_free(s);
		}
		return false;
	}

	// Since mapped allocations are page aligned, we reallocate to a larger than needed size to ensure an actual reallocation occurs.
	else if (check & MAPPED_T && !(swap = st_realloc(s, st_length_get(constant) + (getpagesize() * 128)))) {
		if (s) {
			st_free(s);
		}
		return false;
	}

	// Jointed strings allow reallocation without changing the address of s, of if the string is jointed and s changes, error out.
	else if (check & JOINTED && swap != s) {
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

	len = snprintf(st_char_get(s), st_length_int(constant) + 1, "%.*s", st_length_int(constant), st_char_get(constant));
	if (check & MAPPED_T || check & MANAGED_T) {
		st_length_set(s, len);
	}

	else if (memcmp(st_char_get(s), st_char_get(constant), st_length_get(constant))) {
		return false;
	}

	// Enlarge a buffer by a factor of 128 and make sure the data it contained wasn't changed. For managed or mapped strings, multiply the available space.
	if (check & (MANAGED_T | MAPPED_T) && !(swap = st_realloc(s, st_avail_get(s) * 128))) {
		st_free(s);
		return false;
	}

	// For other string types we multiply the space used by the string since the original allocation size isn't tracked.
	else if (!(check & (MANAGED_T | MAPPED_T)) && !(swap = st_realloc(s, st_length_get(s) * 128))) {
		st_free(s);
		return false;
	}

	// Since mapped allocations are page aligned, we reallocate to a larger than needed size to ensure an actual reallocation occurs.
	else if (check & MAPPED_T && !(swap = st_realloc(s, st_length_get(constant) + (getpagesize() * 128)))) {
		if (s) {
			st_free(s);
		}
		return false;
	}

	// Jointed strings allow reallocation without changing the address of s, of if the string is jointed and s changes, error out.
	else if (check & JOINTED && swap != s) {
		st_free(swap);
		st_free(s);
		return false;
	}

	// Contiguous strings will require the address of s to change, so if it doesn't error out. Except for the mapped type, since the jointed flag doesn't apply to it.
	else if ((check & JOINTED) ^ JOINTED && !(check & MAPPED_T) && swap == s) {
		st_free(s);
		return false;
	}

	// For contiguous types, free the original string and replace it with the value of swap.
	else if (swap != s) {
		st_free(s);
		s = swap;
	}

	else if (memcmp(st_char_get(s), st_char_get(constant), st_length_get(constant))) {
		return false;
	}

	// Now we shrink the buffer back to the bare minimum and check the data one final time.
	if (!(swap = st_realloc(s, st_length_get(constant)))) {
		if (s) {
			st_free(s);
		}
		return false;
	} else if (swap != s) {
		st_free(s);
		s = swap;
	}

	log_print("%28.28s = %.*s", type, st_length_int(s), st_char_get(s));
	st_free(s);
	return true;
}

// Check that we can't specify a length greater than the available buffer.
bool_t check_string_logic(uint32_t check) {

	stringer_t *s;

	if (!(s = st_alloc(check, 128)) || st_length_set(s, st_avail_get(s) * 2) != st_avail_get(s)) {
		if (s)
			st_free(s);
		return false;
	}

	st_free(s);
	return true;
}

bool_t check_string_import(void) {

	stringer_t *s;

	if (!(s = st_import(st_data_get(constant), st_length_int(constant)))) {
		return false;
	}

	log_print("%28.28s = %.*s", "duplicate", st_length_int(s), st_char_get(s));

	if (memcmp(st_char_get(s), st_char_get(constant), st_length_get(constant))) {
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

	strings[0] = st_alloc(PLACER_T | JOINTED | HEAP, 0);
	st_placer_set(strings[0], st_data_get(constant), st_length_get(constant));

	strings[1] = st_merge(NULLER_T | CONTIGUOUS | HEAP, "s", strings[0]);
	strings[2] = st_merge(NULLER_T | JOINTED | HEAP, "s", strings[1]);
	strings[3] = st_merge(BLOCK_T | CONTIGUOUS | HEAP, "s", strings[2]);
	strings[4] = st_merge(BLOCK_T | JOINTED | HEAP, "s", strings[3]);
	strings[5] = st_merge(MANAGED_T | CONTIGUOUS | HEAP, "s", strings[4]);
	strings[6] = st_merge(MANAGED_T | JOINTED | HEAP, "s", strings[5]);
	strings[7] = st_merge(MAPPED_T | JOINTED | HEAP, "s", strings[6]);

	strings[8] = st_merge(NULLER_T | CONTIGUOUS | SECURE, "s", strings[7]);
	strings[9] = st_merge(NULLER_T | JOINTED | SECURE, "s", strings[8]);
	strings[10] = st_merge(BLOCK_T | CONTIGUOUS | SECURE, "s", strings[9]);
	strings[11] = st_merge(BLOCK_T | JOINTED | SECURE, "s", strings[10]);
	strings[12] = st_merge(MANAGED_T | CONTIGUOUS | SECURE, "s", strings[11]);
	strings[13] = st_merge(MANAGED_T | JOINTED | SECURE, "s", strings[12]);
	strings[14] = st_merge(MAPPED_T | JOINTED | SECURE, "s", strings[13]);

	for (int i = 0; i < 15 && strings[i]; i++) {
		for (unsigned int j = total = 0; strings[i] && j < st_length_get(strings[i]); j++) {
			total += *(st_char_get(strings[i]) + j);
		}

		if (total != 5366) {
			result = false;
		}
	}

	if (result) {

		strings[15] = st_merge(MANAGED_T | JOINTED | HEAP, "snsnsnsnsnsnsnsnsnsnsnsnsnsnsnsn", constant, "\n", strings[0], "\n", strings[1], "\n", strings[2], "\n",
				strings[3], "\n", strings[4], "\n", strings[5], "\n", strings[6], "\n", strings[7], "\n", strings[8], "\n", strings[9], "\n", strings[10], "\n", strings[11],
				"\n", strings[12], "\n", strings[13], "\n", strings[14], "\n");

		for (unsigned int i = total = 0; strings[15] && i < st_length_get(strings[15]); i++) {
			total += *(st_char_get(strings[15]) + i);
		}

		if (total != (5376UL * 16)) {
			result = false;
		}

	}

	log_print("%28.28s = %s", "merged", result ? "passed" : "failed");

	for (int i = 0; i < 16; i++) {
		if (strings[i])
			st_free(strings[i]);
	}

	return result;

}

bool_t check_string_print(void) {

	uint64_t total;
	bool_t result = true;
	stringer_t *strings[14];

	mm_set(strings, 0, sizeof(strings));

	strings[0] = st_print(NULLER_T | CONTIGUOUS | HEAP, "%.*s", st_length_int(constant), st_char_get(constant));
	strings[1] = st_print(NULLER_T | JOINTED | HEAP, "%.*s", st_length_int(strings[0]), st_char_get(strings[0]));
	strings[2] = st_print(BLOCK_T | CONTIGUOUS | HEAP, "%.*s", st_length_int(strings[1]), st_char_get(strings[1]));
	strings[3] = st_print(BLOCK_T | JOINTED | HEAP, "%.*s", st_length_int(strings[2]), st_char_get(strings[2]));
	strings[4] = st_print(MANAGED_T | CONTIGUOUS | HEAP, "%.*s", st_length_int(strings[3]), st_char_get(strings[3]));
	strings[5] = st_print(MANAGED_T | JOINTED | HEAP, "%.*s", st_length_int(strings[4]), st_char_get(strings[4]));
	strings[6] = st_print(MAPPED_T | JOINTED | HEAP, "%.*s", st_length_int(strings[5]), st_char_get(strings[5]));

	strings[7] = st_print(NULLER_T | CONTIGUOUS | SECURE, "%.*s", st_length_int(strings[6]), st_char_get(strings[6]));
	strings[8] = st_print(NULLER_T | JOINTED | SECURE, "%.*s", st_length_int(strings[7]), st_char_get(strings[7]));
	strings[9] = st_print(BLOCK_T | CONTIGUOUS | SECURE, "%.*s", st_length_int(strings[8]), st_char_get(strings[8]));
	strings[10] = st_print(BLOCK_T | JOINTED | SECURE, "%.*s", st_length_int(strings[9]), st_char_get(strings[9]));
	strings[11] = st_print(MANAGED_T | CONTIGUOUS | SECURE, "%.*s", st_length_int(strings[10]), st_char_get(strings[10]));
	strings[12] = st_print(MANAGED_T | JOINTED | SECURE, "%.*s", st_length_int(strings[11]), st_char_get(strings[11]));
	strings[13] = st_print(MAPPED_T | JOINTED | SECURE, "%.*s", st_length_int(strings[12]), st_char_get(strings[12]));

	for (int i = 0; i < 14 && strings[i]; i++) {
		for (unsigned int j = total = 0; strings[i] && j < st_length_get(strings[i]); j++) {
			total += *(st_char_get(strings[i]) + j);
		}

		if (total != 5366) {
			result = false;
		}
	}

	log_print("%28.28s = %s", "print", result ? "passed" : "failed");

	for (int i = 0; i < 14; i++) {
		if (strings[i])
			st_free(strings[i]);
	}

	return result;

}

int main(void) {

	if (!mm_sec_start()) {

		log_print("Secure memory pool did not start correctly.");
		return 1;
	}

	else if (sizeof(stringer_t) != 4) {

		log_print("The string options variable should be 4 bytes/32 bits.");
		return 1;
	}

	/*chr_t *data = "Hello world.";
	size_t len = ns_length_get(data);
	stringer_t *place =
		PLACER(data, len);

	log_print("%.*s\n%.*s\n", st_length_int(PLACER(data, len)), st_char_get(PLACER(data, len)),	st_length_int((stringer_t *)place), st_char_get((stringer_t *)place));
	if (st_cmp_cs_eq(PLACER(st_data_get(constant), st_length_get(constant)), constant) || memcmp("Lorem ipsum dolor sit amet, consectetur adipiscing elit.", st_data_get(constant), st_length_get(constant)))
		exit(1);*/

	// Since were intentionally going to trigger errors, disable standard out while doing so.
	log_disable();
	// Check that we can't specify a length greater than the available buffer.
	if (!check_string_logic(MANAGED_T | CONTIGUOUS | HEAP) || !check_string_logic(MANAGED_T | JOINTED | HEAP)	|| !check_string_logic(MAPPED_T | JOINTED | HEAP)) {
		log_print("--------------------------------------- LOGIC -------------------------------------------\n");
		log_print("String logic checks failed.");
		return 1;
	}
	log_enable();

	log_print("--------------------------------------- CONSTANT ----------------------------------------\n%28.28s = %.*s",
			"constant[fixed+contiguous]", st_length_int(constant), st_char_get(constant));

	log_print("--------------------------------------- IMPORT ------------------------------------------");
	if (!check_string_import()) {
		log_print("Import check failed.");
		return 1;
	}

	// Begin allocation checks.
	log_print("--------------------------------------- ALLOCATION --------------------------------------");

	if (!check_string_alloc("nuller[heap+contiguous]", NULLER_T | CONTIGUOUS | HEAP) || !check_string_alloc("block[heap+contiguous]", BLOCK_T | CONTIGUOUS | HEAP)
			|| !check_string_alloc("managed[heap+contiguous]", MANAGED_T | CONTIGUOUS | HEAP)) {
		log_print("Standard allocation checks failed.");
		return 1;
	}

	if (!check_string_alloc("nuller[heap+jointed]", NULLER_T | JOINTED | HEAP) || !check_string_alloc("block[heap+jointed]", BLOCK_T | JOINTED | HEAP)
			|| !check_string_alloc("managed[heap+jointed]", MANAGED_T | JOINTED | HEAP) || !check_string_alloc("mapped[heap+jointed]", MAPPED_T | JOINTED | HEAP)) {
		log_print("Jointed allocation checks failed.");
		return 1;
	}

	if (!check_string_alloc("nuller[secure+contiguous]", NULLER_T | CONTIGUOUS | SECURE) || !check_string_alloc("block[secure+contiguous]", BLOCK_T | CONTIGUOUS | SECURE)
			|| !check_string_alloc("managed[secure+contiguous]", MANAGED_T | CONTIGUOUS | SECURE)) {
		log_print("Secure allocation of contiguous types failed.");
		return 1;
	}

	if (!check_string_alloc("nuller[secure+jointed]", NULLER_T | JOINTED | SECURE) || !check_string_alloc("block[secure+jointed]", BLOCK_T | JOINTED | SECURE)
			|| !check_string_alloc("managed[secure+jointed]", MANAGED_T | JOINTED | SECURE) || !check_string_alloc("mapped[secure+jointed]", MAPPED_T | JOINTED | SECURE)) {
		log_print("Secure allocation of jointed types failed.");
		return 1;
	}

	// Begin reallocation checks.
	log_print("-------------------------------------- REALLOCATION -------------------------------------");

	if (!check_string_realloc("nuller[heap+contiguous]", NULLER_T | CONTIGUOUS | HEAP) || !check_string_realloc("block[heap+contiguous]", BLOCK_T | CONTIGUOUS | HEAP)
			|| !check_string_realloc("managed[heap+contiguous]", MANAGED_T | CONTIGUOUS | HEAP)) {
		log_print("Standard reallocation checks failed.");
		return 1;
	}

	if (!check_string_realloc("nuller[heap+jointed]", NULLER_T | JOINTED | HEAP) || !check_string_realloc("block[heap+jointed]", BLOCK_T | JOINTED | HEAP)
			|| !check_string_realloc("managed[heap+jointed]", MANAGED_T | JOINTED | HEAP) || !check_string_realloc("mapped[heap+jointed]", MAPPED_T | JOINTED | HEAP)) {
		log_print("Jointed reallocation checks failed.");
		return 1;
	}

	if (!check_string_realloc("nuller[secure+contiguous]", NULLER_T | CONTIGUOUS | SECURE) || !check_string_realloc("block[secure+contiguous]", BLOCK_T | CONTIGUOUS
			| SECURE) || !check_string_realloc("managed[secure+contiguous]", MANAGED_T | CONTIGUOUS | SECURE)) {
		log_print("Secure reallocation of contiguous types failed.");
		return 1;
	}

	if (!check_string_realloc("nuller[secure+jointed]", NULLER_T | JOINTED | SECURE) || !check_string_realloc("block[secure+jointed]", BLOCK_T | JOINTED | SECURE)
			|| !check_string_realloc("managed[secure+jointed]", MANAGED_T | JOINTED | SECURE) || !check_string_realloc("mapped[secure+jointed]", MAPPED_T | JOINTED | SECURE)) {
		log_print("Secure reallocation of jointed types failed.");
		return 1;
	}

	// Begin duplication checks.
	log_print("-------------------------------------- DUPLICATION --------------------------------------");

	if (!check_string_dupe("nuller[heap+contiguous]", NULLER_T | CONTIGUOUS | HEAP) || !check_string_dupe("block[heap+contiguous]", BLOCK_T | CONTIGUOUS | HEAP)
			|| !check_string_dupe("managed[heap+contiguous]", MANAGED_T | CONTIGUOUS | HEAP)) {
		log_print("Standard duplication checks failed.");
		return 1;
	}

	if (!check_string_dupe("nuller[heap+jointed]", NULLER_T | JOINTED | HEAP) || !check_string_dupe("block[heap+jointed]", BLOCK_T | JOINTED | HEAP) || !check_string_dupe(
			"managed[heap+jointed]", MANAGED_T | JOINTED | HEAP) || !check_string_dupe("mapped[heap+jointed]", MAPPED_T | JOINTED | HEAP)) {
		log_print("Jointed duplication checks failed.");
		return 1;
	}

	if (!check_string_dupe("nuller[secure+contiguous]", NULLER_T | CONTIGUOUS | SECURE) || !check_string_dupe("block[secure+contiguous]", BLOCK_T | CONTIGUOUS | SECURE)
			|| !check_string_dupe("managed[secure+contiguous]", MANAGED_T | CONTIGUOUS | SECURE)) {
		log_print("Secure duplication of contiguous types failed.");
		return 1;
	}

	if (!check_string_dupe("nuller[secure+jointed]", NULLER_T | JOINTED | SECURE) || !check_string_dupe("block[secure+jointed]", BLOCK_T | JOINTED | SECURE)
			|| !check_string_dupe("managed[secure+jointed]", MANAGED_T | JOINTED | SECURE) || !check_string_dupe("mapped[secure+jointed]", MAPPED_T | JOINTED | SECURE)) {
		log_print("Secure duplication of jointed types failed.");
		return 1;
	}

	log_print("-------------------------------------- MERGE --------------------------------------------");
	if (!check_string_merge()) {
		log_print("The merge function failed.");
		return 1;
	}

	log_print("-------------------------------------- PRINT --------------------------------------------");
	if (!check_string_print()) {
		log_print("The print function failed.");
		return 1;
	}

	mm_sec_stop();
	log_print("---------------------------------------- FINISHED ---------------------------------------");
	return 0;
}
