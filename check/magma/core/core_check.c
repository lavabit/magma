
/**
 * @file /check/magma/core/core_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma core module.
 */

#include "magma_check.h"

START_TEST(check_bsearch) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = NULL;

	if (!check_bsearch_months(1, "jan") ||
		!check_bsearch_months(2, "feb") ||
		!check_bsearch_months(3, "mar") ||
		!check_bsearch_months(4, "apr") ||
		!check_bsearch_months(5, "may") ||
		!check_bsearch_months(6, "jun") ||
		!check_bsearch_months(7, "jul") ||
		!check_bsearch_months(8, "aug") ||
		!check_bsearch_months(9, "sep") ||
		!check_bsearch_months(10, "oct") ||
		!check_bsearch_months(11, "nov") ||
		!check_bsearch_months(12, "dec") ||

		check_bsearch_months(1, "january") ||
		check_bsearch_months(2, "february") ||
		check_bsearch_months(3, "march") ||
		check_bsearch_months(4, "april") ||
		check_bsearch_months(5, "mayil") ||
		check_bsearch_months(6, "june") ||
		check_bsearch_months(7, "july") ||
		check_bsearch_months(8, "august") ||
		check_bsearch_months(9, "september") ||
		check_bsearch_months(10, "october") ||
		check_bsearch_months(11, "november") ||
		check_bsearch_months(12, "december")) {
		errmsg = NULLER("bsearch() failed");
		result = false;
	}

	log_test("CORE / STRINGS / BSEARCH / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_compare) {

	log_disable();
	stringer_t *errmsg = NULL;

	log_unit("%-64.64s", "CORE / STRINGS / COMPARE / SINGLE THREADED:");

	if (st_cmp_cs_eq(NULLER(type(M_TYPE_MULTI)), CONSTANT("M_TYPE_MULTI")))
		errmsg = NULLER("type() did not return M_TYPE_MULTI");

	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_ENUM)), CONSTANT("M_TYPE_ENUM")))
		errmsg = NULLER("type() did not return M_TYPE_ENUM");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_BOOLEAN)), CONSTANT("M_TYPE_BOOLEAN")))
		errmsg = NULLER("type() did not return M_TYPE_BOOLEAN");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_BLOCK)), CONSTANT("M_TYPE_BLOCK")))
		errmsg = NULLER("type() did not return M_TYPE_BLOCK");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_NULLER)), CONSTANT("M_TYPE_NULLER")))
		errmsg = NULLER("type() did not return M_TYPE_NULLER");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_PLACER)), CONSTANT("M_TYPE_PLACER")))
		errmsg = NULLER("type() did not return M_TYPE_PLACER");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_STRINGER)), CONSTANT("M_TYPE_STRINGER")))
		errmsg = NULLER("type() did not return M_TYPE_STRINGER");

	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_INT8)), CONSTANT("M_TYPE_INT8")))
		errmsg = NULLER("type() did not return M_TYPE_INT8");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_INT16)), CONSTANT("M_TYPE_INT16")))
		errmsg = NULLER("type() did not return M_TYPE_INT16");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_INT32)), CONSTANT("M_TYPE_INT32")))
		errmsg = NULLER("type() did not return M_TYPE_INT32");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_INT64)), CONSTANT("M_TYPE_INT64")))
		errmsg = NULLER("type() did not return M_TYPE_INT64");

	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_UINT8)), CONSTANT("M_TYPE_UINT8")))
		errmsg = NULLER("type() did not return M_TYPE_UINT8");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_UINT16)), CONSTANT("M_TYPE_UINT16")))
		errmsg = NULLER("type() did not return M_TYPE_UINT16");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_UINT32)), CONSTANT("M_TYPE_UINT32")))
		errmsg = NULLER("type() did not return M_TYPE_UINT32");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_UINT64)), CONSTANT("M_TYPE_UINT64")))
		errmsg = NULLER("type() did not return M_TYPE_UINT64");

	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_FLOAT)), CONSTANT("M_TYPE_FLOAT")))
		errmsg = NULLER("type() did not return M_TYPE_FLOAT");
	else if (st_cmp_cs_eq(NULLER(type(M_TYPE_DOUBLE)), CONSTANT("M_TYPE_DOUBLE")))
		errmsg = NULLER("type() did not return M_TYPE_DOUBLE");

	log_test("CORE / STRINGS / COMPARE / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_inx_linked_s) {

	log_disable();
	char *errmsg = NULL;

	check_indexes_linked_simple(&errmsg);

	log_test("CORE / INDEX / LINKED / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(NULLER(errmsg), errmsg);
}
END_TEST

// TODO
START_TEST (check_inx_linked_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_inx_opt_t *opts = NULL;

	if (status() && (!(opts = mm_alloc(sizeof(check_inx_opt_t))) || !(opts->inx = inx_alloc(M_INX_LINKED, &mm_free)))) {
		outcome = false;
		errmsg = NULLER("Index linked");
	}
	else if (status()) {
		opts->type = M_INX_LINKED;
		outcome = check_inx_mthread(opts);
	}

	if (opts) {
		inx_cleanup(opts->inx);
		mm_free(opts);
	}

	log_test("CORE / INDEX / LINKED / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_inx_tree_s) {

	log_disable();
	bool_t outcome = true;
	char *errmsg = NULL;

	if (!check_indexes_tree_simple(&errmsg)) {
		outcome = false;
	}

	log_test("CORE / INDEX / TREE / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
}
END_TEST

START_TEST (check_inx_tree_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_inx_opt_t *opts = NULL;

	if (status() && (!(opts = mm_alloc(sizeof(check_inx_opt_t))) || !(opts->inx = inx_alloc(M_INX_TREE, &mm_free)))) {
		outcome = false;
		errmsg = NULLER("The check index tree multi-threaded test failed.");
	}
	else if (status()) {
		outcome = check_inx_mthread(opts);
	}

	if (opts) {
		inx_cleanup(opts->inx);
		mm_free(opts);
	}

	log_test("CORE / INDEX / TREE / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_inx_hashed_s) {

	log_disable();
	bool_t outcome = true;
	char *errmsg = NULL;

	if (!check_indexes_hashed_simple(&errmsg)) {
		outcome = false;
	}

	log_test("CORE / INDEX / HASHED / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
}
END_TEST

START_TEST (check_inx_hashed_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_inx_opt_t *opts = NULL;

	if (status() && (!(opts = mm_alloc(sizeof(check_inx_opt_t))) || !(opts->inx = inx_alloc(M_INX_HASHED, &mm_free)))) {
		outcome = false;
		errmsg = NULLER("The check index hashed multi-threaded test failed.");
	}
	else if (!check_inx_mthread(opts)) {
		outcome = false;
		errmsg = NULLER("The check index hashed multi-threaded test failed.");
	}

	if (opts) {
		inx_cleanup(opts->inx);
		mm_free(opts);
	}

	log_test("CORE / INDEX / HASHED / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_inx_linked_cursor_s) {

	log_disable();
	bool_t outcome = true;
	char *errmsg = NULL;

	outcome = check_indexes_linked_cursor(&errmsg);

	log_test("CORE / INDEX / LINKED CURSOR / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
}
END_TEST

START_TEST (check_inx_linked_cursor_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_inx_opt_t *opts = NULL;


	if (status() && (!(opts = mm_alloc(sizeof(check_inx_opt_t))) || !(opts->inx = inx_alloc(M_INX_LINKED, &mm_free)) || !check_inx_mthread(opts))) {
		outcome = false;
		errmsg = NULLER("The check index linked cursor multi-threaded test failed.");
	}
	else if (!check_inx_cursor_mthread(opts)) {
		outcome = false;
		errmsg = NULLER("The check index linked cursor multi-threaded test failed.");
	}

	if (opts) {
		inx_cleanup(opts->inx);
		mm_free(opts);
	}

	log_test("CORE / INDEX / LINKED CURSOR / MULTI THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
}
END_TEST

START_TEST (check_inx_tree_cursor_s) {

	log_disable();
	bool_t outcome = true;
	char *errmsg = NULL;

	if (!check_indexes_tree_cursor(&errmsg)) {
		outcome = false;
	}

	log_test("CORE / INDEX / TREE CURSOR / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
}
END_TEST

START_TEST (check_inx_tree_cursor_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_inx_opt_t *opts = NULL;

	if (status() && (!(opts = mm_alloc(sizeof(check_inx_opt_t))) || !(opts->inx = inx_alloc(M_INX_TREE, &mm_free)) || !check_inx_mthread(opts))) {
		outcome = false;
		errmsg = NULLER("The check index linked cursor multi-threaded test failed.");
	}
	else if (!check_inx_cursor_mthread(opts)) {
		outcome = false;
		errmsg = NULLER("The check index linked cursor multi-threaded test failed.");
	}

	if (opts) {
		inx_cleanup(opts->inx);
		mm_free(opts);
	}

	log_test("CORE / INDEX / TREE CURSOR / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_inx_hashed_cursor_s) {

	log_disable();
	bool_t outcome = true;
	char *errmsg = NULL;

	if (!check_indexes_hashed_cursor(&errmsg)) {
		outcome = false;
	}

	log_test("CORE / INDEX / HASHED CURSOR / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
}
END_TEST

START_TEST (check_inx_hashed_cursor_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_inx_opt_t *opts = NULL;

	if (status() && (!(opts = mm_alloc(sizeof(check_inx_opt_t))) || !(opts->inx = inx_alloc(M_INX_HASHED, &mm_free)) || !check_inx_mthread(opts))) {
		outcome = false;
		errmsg = NULLER("The check index hashed cursor multi-threaded test failed.");
	}
	else if (!check_inx_cursor_mthread(opts)) {
		outcome = false;
		errmsg = NULLER("The check index hashed cursor multi-threaded test failed.");
	}

	if (opts) {
		inx_cleanup(opts->inx);
		mm_free(opts);
	}

	log_test("CORE / INDEX / HASHED CURSOR / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_constants) {

	log_disable();
	uint64_t total;
	stringer_t *errmsg = NULL;
	bool_t outcome = true;

	for (unsigned int i = total = 0; string_check_constant && i < st_length_get(string_check_constant); i++) {
		total += *(st_char_get(string_check_constant) + i);
	}

	if (total != 5366) {
		outcome = false;
		errmsg = NULLER("The single-threaded check string constants test failed.");
	}

	log_test("CORE / STRINGS / CONSTANTS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_allocation) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_string_alloc(NULLER_T | CONTIGUOUS | HEAP) || !check_string_alloc(BLOCK_T | CONTIGUOUS | HEAP) || !check_string_alloc(
		MANAGED_T | CONTIGUOUS | HEAP)) errmsg = NULLER("Standard allocation checks failed.");

	if (!check_string_alloc(NULLER_T | JOINTED | HEAP) || !check_string_alloc(BLOCK_T | JOINTED | HEAP) || !check_string_alloc(
		MANAGED_T | JOINTED | HEAP) || !check_string_alloc(MAPPED_T | JOINTED | HEAP)) errmsg = NULLER("Jointed allocation checks failed.");

	if (!check_string_alloc(NULLER_T | CONTIGUOUS | SECURE) || !check_string_alloc(BLOCK_T | CONTIGUOUS | SECURE) || !check_string_alloc(
		MANAGED_T | CONTIGUOUS | SECURE)) errmsg = NULLER("Secure allocation of contiguous types failed.");

	if (!check_string_alloc(NULLER_T | JOINTED | SECURE) || !check_string_alloc(BLOCK_T | JOINTED | SECURE) || !check_string_alloc(
		MANAGED_T | JOINTED | SECURE) || !check_string_alloc(MAPPED_T | JOINTED | SECURE)) errmsg
		= NULLER("Secure allocation of jointed types failed.");

	log_test("CORE / STRINGS / ALLOCATION / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_reallocation)	{

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_string_realloc(NULLER_T | CONTIGUOUS | HEAP) || !check_string_realloc(BLOCK_T | CONTIGUOUS | HEAP) || !check_string_realloc(
		MANAGED_T | CONTIGUOUS | HEAP)) errmsg = NULLER("Standard reallocation checks failed.");

	if (!check_string_realloc(NULLER_T | JOINTED | HEAP) || !check_string_realloc(BLOCK_T | JOINTED | HEAP) || !check_string_realloc(
		MANAGED_T | JOINTED | HEAP) || !check_string_realloc(MAPPED_T | JOINTED | HEAP)) errmsg = NULLER("Jointed reallocation checks failed.");

	if (!check_string_realloc(NULLER_T | CONTIGUOUS | SECURE) || !check_string_realloc(BLOCK_T | CONTIGUOUS | SECURE)
		|| !check_string_realloc(MANAGED_T | CONTIGUOUS | SECURE)) errmsg = NULLER("Secure reallocation of contiguous types failed.");

	if (!check_string_realloc(NULLER_T | JOINTED | SECURE) || !check_string_realloc(BLOCK_T | JOINTED | SECURE) || !check_string_realloc(
		MANAGED_T | JOINTED | SECURE) || !check_string_realloc(MAPPED_T | JOINTED | SECURE)) errmsg
		= NULLER("Secure reallocation of jointed types failed.");

	log_test("CORE / STRINGS / REALLOCATION / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_duplication) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_string_dupe(NULLER_T | CONTIGUOUS | HEAP) || !check_string_dupe(BLOCK_T | CONTIGUOUS | HEAP) || !check_string_dupe(
		MANAGED_T | CONTIGUOUS | HEAP)) errmsg = NULLER("Standard duplication checks failed.");

	if (!check_string_dupe(NULLER_T | JOINTED | HEAP) || !check_string_dupe(BLOCK_T | JOINTED | HEAP) || !check_string_dupe(
		MANAGED_T | JOINTED | HEAP) || !check_string_dupe(MAPPED_T | JOINTED | HEAP)) errmsg = NULLER("Jointed duplication checks failed.");

	if (!check_string_dupe(NULLER_T | CONTIGUOUS | SECURE) || !check_string_dupe(BLOCK_T | CONTIGUOUS | SECURE) || !check_string_dupe(
		MANAGED_T | CONTIGUOUS | SECURE)) errmsg = NULLER("Secure duplication of contiguous types failed.");

	if (!check_string_dupe(NULLER_T | JOINTED | SECURE) || !check_string_dupe(BLOCK_T | JOINTED | SECURE) || !check_string_dupe(
		MANAGED_T | JOINTED | SECURE) || !check_string_dupe(MAPPED_T | JOINTED | SECURE)) errmsg
		= NULLER("Secure duplication of jointed types failed.");

	log_test("CORE / STRINGS / DUPLICATION / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_merge) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_string_merge()) errmsg = NULLER("The stringer merge function failed.");

	log_test("CORE / STRINGS / MERGE / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_print) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_string_print()) errmsg = NULLER("The stringer print function failed.");

	log_test("CORE / STRINGS / PRINT / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));

}
END_TEST

START_TEST (check_write) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_string_write()) errmsg = NULLER("The stringer write function failed.");

	log_test("CORE / STRINGS / WRITE / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_digits) {

	log_disable();
	int8_t i8;
	uint8_t ui8;
	int16_t i16;
	uint16_t ui16;
	int32_t i32;
	uint32_t ui32;
	int64_t i64;
	uint64_t ui64;

	char buf[1024];
	stringer_t *errmsg = NULL;
	bool_t outcome = true;

	for (uint64_t i = 0; status() && outcome && i < 8192; i++) {

		i8 = rand_get_int8();
		i16 = rand_get_int16();
		i32 = rand_get_int32();
		i64 = rand_get_int64();

		ui8 = rand_get_uint8();
		ui16 = rand_get_uint16();
		ui32 = rand_get_uint32();
		ui64 = rand_get_uint64();

		if (int8_digits(i8) != snprintf(buf, 1024, "%hhi", i8))
			outcome = false;
		else if (int16_digits(i16) != snprintf(buf, 1024, "%hi", i16))
			outcome = false;
		else if (int32_digits(i32) != snprintf(buf, 1024, "%i", i32))
			outcome = false;
		else if (int64_digits(i64) != snprintf(buf, 1024, "%li", i64))
			outcome = false;

		else if (uint8_digits(ui8) != snprintf(buf, 1024, "%hhu", ui8))
			outcome = false;
		else if (uint16_digits(ui16) != snprintf(buf, 1024, "%hu", ui16))
			outcome = false;
		else if (uint32_digits(ui32) != snprintf(buf, 1024, "%u", ui32))
			outcome = false;
		else if (uint64_digits(ui64) != snprintf(buf, 1024, "%lu", ui64))
			outcome = false;
	}

	// Error check
	if (!outcome) errmsg = NULLER("The digit counters didn't match the print function!");

	log_test("CORE / PARSERS / DIGITS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));

}
END_TEST

START_TEST (check_clamp) {

	log_disable();
	chr_t *errmsg = NULL;
	bool_t outcome = true;

	// If any of the test cases return an error message, the unit test is considered a failure.
	if (status() && ((errmsg = check_clamp_min()) ||
		(errmsg = check_clamp_max()) ||
		(errmsg = check_clamp_min_max_equal()) ||
		(errmsg = check_clamp_randomizer()))) {
		outcome = false;
	}
	else {

		// This test case is intentionally using invalid values, so to avoid logging all the meaningless errors, we have to
		// disable logging while the test case is running.
		if (status() && (errmsg = check_clamp_min_max_invalid())) {
			outcome = false;
		}
	}

	log_test("CORE / PARSERS / CLAMP / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(outcome, errmsg);
	ns_cleanup(errmsg);
}
END_TEST

START_TEST (check_capitalization) {

	log_disable();
	char *errmsg = NULL;
	stringer_t *copy, *letters = CONSTANT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	// Lowercase string.
	if (status() && (copy = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, letters))) {
		lower_st(copy);
		for (size_t i = 0; status() && !errmsg && i < st_length_get(copy); i++) {
			if (!islower(*(st_char_get(copy) + i))) {
				errmsg = "The lowercase string function failed.";
			}
		}
		st_free(copy);
	}
	else if (status()){
		errmsg = "The lowercase string function failed.";
	}

	// Uppercase string.
	if (status() && !errmsg && (copy = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, letters))) {
		upper_st(copy);
		for (size_t i = 0; status() && !errmsg && i < st_length_get(copy); i++) {
			if (!isupper(*(st_char_get(copy) + i))) {
				errmsg = "The uppercase string function failed.";
			}
		}
		st_free(copy);
	}
	else if (status() && !errmsg) {
		errmsg = "The uppercase string function failed.";
	}

	// Uppercase/lowercase all of the possible character codes.
	for (uchr_t c = 0; status() && !errmsg && c < UCHAR_MAX; c++) {
		if (tolower(c) != lower_chr(c) || toupper(c) != upper_chr(c)) {
			errmsg = "The uppercase/lowercase character functions failed.";
		}
	}

	log_test("CORE / PARSERS / CASE / SINGLE THREADED:", NULLER(errmsg));
	ck_assert_msg(!errmsg, errmsg);

}
END_TEST

START_TEST (check_classify) {

	log_disable();
	stringer_t *errmsg = NULL;

	for (uchr_t c = 0; status() && !errmsg && c < UCHAR_MAX; c++) {
		if ((isalnum(c) ? true : false) != chr_alphanumeric(c) || (isascii(c) ? true : false) != chr_ascii(c) ||
			(isblank(c) ? true : false) != chr_blank(c) || (islower(c) ? true : false) != chr_lower(c) ||
			(isdigit(c) ? true : false) != chr_numeric(c)	|| (isprint(c) ? true : false) != chr_printable(c) ||
			(isupper(c) ? true : false) != chr_upper(c) || (isspace(c) ? true : false) != chr_whitespace(c) ||
			(ispunct(c) ? true : false) != chr_punctuation(c)) {
			errmsg = NULLER("The ASCII classification functions failed.");
		}
	}

	log_test("CORE / CLASSIFY / ASCII / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_qp) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_encoding_qp()) errmsg = NULLER("The quoted printable encoding functions failed.");

	log_test("CORE / ENCODING / QUOTED PRINTABLE / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));

}
END_TEST

START_TEST (check_hex) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_encoding_hex()) errmsg = NULLER("The hex encoding functions failed.");

	log_test("CORE / ENCODING / HEX / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_url) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_encoding_url()) errmsg = NULLER("The URL encoding functions failed.");

	log_test("CORE / ENCODING / URL / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_base64) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_encoding_base64(false))
		errmsg = NULLER("The base64 encoding functions failed.");
	else if (!check_encoding_base64(true))
		errmsg = NULLER("The base64 secure encoding functions failed.");
	else if (!check_encoding_base64_mod(false))
		errmsg = NULLER("The modified base64 encoding functions failed.");
	else if (!check_encoding_base64_mod(true))
		errmsg = NULLER("The modified base64 encoding functions failed.");

	log_test("CORE / ENCODING / BASE64 / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_zbase32) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_encoding_zbase32()) errmsg = NULLER("The zbase32 encoding functions failed.");

	log_test("CORE / ENCODING / ZBASE32 / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_checksum)
{

	log_disable();
	stringer_t *errmsg = NULL;

	if (status() && !check_checksum_fuzz_sthread()) {
		errmsg = NULLER("Checksum fuzz test failed.");
	}
	else if (status() && !check_checksum_fixed_sthread()) {
		errmsg = NULLER("Checksum output failed to match the expected value.");
	}
	else if (status() && !check_checksum_loop_sthread()) {
		errmsg = NULLER("Checksum output failed to match the expected value.");
	}

	log_test("CORE / MEMORY / CHECKSUMS / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_secmem) {

	log_disable();
	size_t bsize;
	void *blocks[1024];
	stringer_t *errmsg = NULL;

	if (status() && magma.secure.memory.enable && (bsize = (magma.secure.memory.length / 1024))) {

		// Now try and trigger an overflow.
		for (size_t i = 0; i < 1024; i++) {
			blocks[i] = mm_sec_alloc(bsize + 16);
		}

		for (size_t i = 0; i < 1024; i++) {
			if (blocks[i]) {
				mm_sec_free(blocks[i]);
				blocks[i] = NULL;
			}
		}

		// Now try and trigger a single byte overflow.
		for (size_t i = 0; i < 1024; i++) {
			blocks[i] = mm_sec_alloc(bsize + 1);
		}

		for (size_t i = 0; i < 1024; i++) {
			if (blocks[i]) {
				mm_sec_free(blocks[i]);
				blocks[i] = NULL;
			}
		}

		// Now were going to request randomly sized blocks.
		for (size_t i = 0; i < 1024; i++) {
			blocks[i] = mm_sec_alloc(bsize + rand_get_uint32() % 32);
		}

		for (size_t i = 0; i < 1024; i++) {
			if (blocks[i]) {
				mm_sec_free(blocks[i]);
				blocks[i] = NULL;
			}
		}
	}

	log_test("CORE / MEMORY / SECURE ADDRESS RANGE / SINGLE THREADED:",
			!errmsg ? (status() && magma.secure.memory.enable && (magma.secure.memory.length / 1024)
					? errmsg : NULLER("SKIPPED")) : errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_signames_s) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_system_signames()) errmsg = NULLER("The single-threaded check system signames test failed.");

	log_test("CORE / HOST / SIGNAL NAMES / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_errnames_s) {

	log_disable();
	stringer_t *errmsg = NULL;

	if (!check_system_errnonames()) errmsg = NULLER("The single-threaded check system errnonames test failed.");

	log_test("CORE / HOST / ERROR NAMES / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_nbo_s) {

	log_disable();
	bool_t outcome = true;

	bool_t (*checks[])(void) = {
		&check_nbo_parameters,
		&check_nbo_simple
	};

	stringer_t *err = NULL;

	stringer_t *errors[] = {
		NULLER("check_nbo_parameters failed"),
		NULLER("check_nbo_simple failed")
	};

	for(uint_t i = 0; status() && !err && i < sizeof(checks)/sizeof((checks)[0]); ++i) {
		if(!(outcome = checks[i]())) {
			err = errors[i];
		}
	}

	log_test("CORE / NETWORK BYTE ORDER / SINGLE THREADED:", st_data_get(err));
	ck_assert_msg(outcome, st_data_get(err));
}
END_TEST

START_TEST (check_bitwise)
{

	log_disable();
	bool_t outcome = true;

	bool_t (*checks[])(void) = {
		&check_bitwise_parameters,
		&check_bitwise_determinism,
		&check_bitwise_simple
	};

	stringer_t *err = NULL;

	stringer_t *errors[] = {
		NULLER("check_bitwise_parameters failed"),
		NULLER("check_bitwise_determinism failed"),
		NULLER("check_bitwise_simple failed")
	};

	for(uint_t i = 0; status() && !err && i < sizeof(checks)/sizeof((checks)[0]); ++i) {
		if(!(outcome = checks[i]())) {
			err = errors[i];
		}
	}

	log_test("CORE / STRINGS / BITWISE / SINGLE THREADED:", st_data_get(err));
	ck_assert_msg(outcome, st_data_get(err));
}
END_TEST

Suite * suite_check_core(void) {

	TCase *tc;
	Suite *s = suite_create("\tCore");

	testcase(s, tc, "Parsers / Digits", check_digits);
	testcase(s, tc, "Parsers / Clamp", check_clamp);
	testcase(s, tc, "Parsers / Capitalization", check_capitalization);
	testcase(s, tc, "Classify / ASCII", check_classify);
	testcase(s, tc, "Strings / Constants", check_constants);
	testcase(s, tc, "Strings / Allocation", check_allocation);
	testcase(s, tc, "Strings / Reallocation", check_reallocation);
	testcase(s, tc, "Strings / Duplication", check_duplication);
	testcase(s, tc, "Strings / Merge", check_merge);
	testcase(s, tc, "Strings / Print", check_print);
	testcase(s, tc, "Strings / Write", check_write);
	testcase(s, tc, "Strings / Compare", check_compare);
	testcase(s, tc, "Strings / Binary Search", check_bsearch);
	testcase(s, tc, "Strings / Bitwise Operations", check_bitwise);
	testcase(s, tc, "Memory / Checksum", check_checksum);
	testcase(s, tc, "Memory / Secure Address Range", check_secmem);
	testcase(s, tc, "System / Signal Names", check_signames_s);
	testcase(s, tc, "System / Error Names", check_errnames_s);
	testcase(s, tc, "Encoding / Quoted Printable", check_qp);
	testcase(s, tc, "Encoding / Hex", check_hex);
	testcase(s, tc, "Encoding / URL", check_url);
	testcase(s, tc, "Encoding / Base64", check_base64);
	testcase(s, tc, "Encoding / Zbase32", check_zbase32);
	testcase(s, tc, "Encoding / Network Byte Order/S", check_nbo_s);
	testcase(s, tc, "Indexes / Linked/S", check_inx_linked_s);
	testcase(s, tc, "Indexes / Linked/M", check_inx_linked_m);
	testcase(s, tc, "Indexes / Hashed/S", check_inx_hashed_s);
	testcase(s, tc, "Indexes / Hashed/M", check_inx_hashed_m);
	testcase(s, tc, "Indexes / Tree/S", check_inx_tree_s);
	testcase(s, tc, "Indexes / Tree/M", check_inx_tree_m);
	testcase(s, tc, "Indexes / Linked Cursor/S", check_inx_linked_cursor_s);
	testcase(s, tc, "Indexes / Linked Cursor/M", check_inx_linked_cursor_m);
	testcase(s, tc, "Indexes / Hashed Cursor/S", check_inx_hashed_cursor_s);
	testcase(s, tc, "Indexes / Hashed Cursor/M", check_inx_hashed_cursor_m);
	testcase(s, tc, "Indexes / Tree Cursor/S", check_inx_tree_cursor_s);
	testcase(s, tc, "Indexes / Tree Cursor/M", check_inx_tree_cursor_m);

	return s;
}

