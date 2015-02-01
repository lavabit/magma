
#include "framework.h"

#include <string.h>

#define ITERATIONS 12

// Allocate a NULL string and fill it with random data. Then make sure it all works.
int unit_test_1(void) {

	char *buffer;
	char random[100];
	sizer_t increment;

	// Fill in the random buffer.
	for (increment = 0; increment < 100; increment++) {
		random[increment] = rand() % 256;
	}

	buffer = allocate_ns(100);
	if (buffer == NULL) {
		return 0;
	}

	// Copy the random bytes.
	if (copy_ns_ns_amt(buffer, random, 100) != 1) {
		free_ns(buffer);
		return 0;
	}

	// Use memcmp to make sure it was done right.
	if (memcmp(buffer, random, 100) != 0) {
		free_ns(buffer);
		return 0;
	}

	// Cleanup.
	free_ns(buffer);

	return 1;
}

// Take a NULL string with a sentence in it, and reallocate it to make it larger.
int unit_test_2(void) {

	char *buffer;
	char *test;

	test = duplicate_ns("Mary had a little lamb.");
	if (test == NULL) {
		return 0;
	}

	buffer = reallocate_ns(test, 200);
	if (buffer == NULL) {
		free_ns(test);
		return 0;
	}

	// Make sure our new string contains the same data.
	if (memcmp("Mary had a little lamb.", buffer, size_ns("Mary had a little lamb.") + 1) != 0) {
		free_ns(buffer);
		return 0;
	}

	if (copy_ns_ns(buffer, "Mary had a little lamb.") != 1) {
		free_ns(buffer);
		return 0;
	}

	// Make sure our new string contains the same data.
	if (memcmp("Mary had a little lamb.", buffer, size_ns("Mary had a little lamb.") + 1) != 0) {
		free_ns(buffer);
		return 0;
	}

	// Cleanup.
	free_ns(buffer);

	return 1;
}

// Allocate a stringer, place data into it, and then reallocate it to a larger size.
int unit_test_3(void) {

	int increment;
	char random[1024];
	char *test = "Perry the potter had many pots to plant.";
	stringer_t *original;
	stringer_t *larger;

	original = allocate_st(size_ns(test));
	if (original == NULL) {
		return 0;
	}

	if (copy_st_ns(original, test) != 1) {
		free_st(original);
		return 0;
	}

	// Make sure our new string contains the same data.
	if (memcmp(test, data_st(original), size_ns(test)) != 0) {
		free_st(original);
		return 0;
	}

	larger = reallocate_st(original, 1024);
	if (larger == NULL) {
		free_st(original);
		return 0;
	}

	// Check again.
	if (memcmp(test, data_st(larger), size_ns(test)) != 0) {
		free_st(larger);
		return 0;
	}

	// Fill in the random buffer.
	for (increment = 0; increment < 1024; increment++) {
		random[increment] = rand() % 256;
	}

	if (copy_st_ns_amt(larger, random, 1024) != 1) {
		free_st(larger);
		return 0;
	}

	// Check the random data.
	if (memcmp(random, data_st(larger), 1024) != 0) {
		free_st(larger);
		return 0;
	}

	if (used_st(larger) != 1024) {
		free_st(larger);
		return 0;
	}

	free_st(larger);

	return 1;
}

// Test importing NULL strings and blocks into stringers.
int unit_test_4(void) {

	int increment;
	char random[2048];
	stringer_t *string;
	char *test = "Pickled pipers pine for Peter and his portly pig.";

	string = import_ns(test);
	if (string == NULL) {
		return 0;
	}

	// Check the test data.
	if (memcmp(test, data_st(string), size_ns(test)) != 0) {
		free_st(string);
		return 0;
	}

	free_st(string);

	// Fill in the random buffer.
	for (increment = 0; increment < 2048; increment++) {
		random[increment] = rand() % 256;
	}

	string = import_bl(random, 2048);
	if (string == NULL) {
		return 0;
	}

	if (memcmp(random, data_st(string), 2048) != 0) {
		free_st(string);
		return 0;
	}

	free_st(string);

	return 1;
}

// Export a stringer into a NULL string.
int unit_test_5(void) {

	int increment;
	char *buffer = NULL;
	char random[4096];
	stringer_t *string;

	// Fill in the random buffer.
	for (increment = 0; increment < 4096; increment++) {
		random[increment] = rand() % 256;
	}

	string = import_bl(random, 4096);
	if (string == NULL) {
		return 0;
	}

	if (memcmp(random, data_st(string), 4096) != 0) {
		free_st(string);
		return 0;
	}

	if (export_ns(&buffer, string) != 4096 || buffer == NULL) {
		free_st(string);
		if (buffer) {
			free_ns(buffer);
		}
		return 0;
	}

	if (memcmp(random, buffer, 4096) != 0) {
		free_st(string);
		free_ns(buffer);
		return 0;
	}

	free_st(string);
	free_ns(buffer);

	return 1;
}

// Do some NULL string duplication tests.
int unit_test_6(void) {

	char *string;
	int increment;
	stringer_t *stringer;
	stringer_t *test_st;
	char random[4096];
	char *test = "Picky Peter piped pools of pewter down the porcelein drain.";

	string = duplicate_ns(test);
	if (string == NULL) {
		return 0;
	}

	if (memcmp(test, string, size_ns(test) + 1) != 0) {
		free_ns(string);
		return 0;
	}

	free_ns(string);

	for (increment = 0; increment < 4096; increment++) {
		random[increment] = rand() % 256;
	}

	string = duplicate_ns_amt(random, 4096);
	if (string == NULL) {
		return 0;
	}

	if (memcmp(random, string, 4096) != 0) {
		free_ns(string);
		return 0;
	}

	free_ns(string);

	test_st = import_ns(test);
	if (test_st == NULL) {
		return 0;
	}

	stringer = duplicate_st(test_st);
	if (stringer == NULL) {
		free_st(test_st);
		return 0;
	}

	if (memcmp(test_st, stringer, size_st(test_st) + 9) != 0) {
		free_st(test_st);
		free_st(stringer);
		return 0;
	}

	free_st(test_st);
	free_st(stringer);

	test_st = import_bl(random, 4096);
	if (test_st == NULL) {
		return 0;
	}

	stringer = duplicate_st(test_st);
	if (stringer == NULL) {
		free_st(test_st);
		return 0;
	}

	if (memcmp(test_st, stringer, size_st(test_st) + 9) != 0) {
		free_st(test_st);
		free_st(stringer);
		return 0;
	}

	free_st(test_st);
	free_st(stringer);

	return 1;
}

// Test the block allocation functions.
int unit_test_7(void) {

	unsigned char *buffer;
	unsigned char *dupe;
	int increment;
	unsigned char random[8192];

	buffer = allocate_bl(4096);
	if (buffer == NULL) {
		return 0;
	}

	for (increment = 0; increment < 4096; increment++) {
		random[increment] = buffer[increment] = rand() % 256;
	}

	if (memcmp(random, buffer, 4096) != 0) {
		free_bl(buffer);
		return 0;
	}

	dupe = reallocate_bl(buffer, 8192, 4096);
	if (dupe == NULL) {
		free_bl(buffer);
		return 0;
	}

	for (increment = 4096; increment < 8192; increment++) {
		random[increment] = dupe[increment] = rand() % 256;
	}

	if (memcmp(random, dupe, 8192) != 0) {
		free_bl(buffer);
		free_bl(dupe);
		return 0;
	}

	free_bl(buffer);
	free_bl(dupe);

	return 1;
}

// Test merge and sprint functions.
int unit_test_8(void) {

	stringer_t *one;
	stringer_t *result;
	char *two;
	char random[1025];
	sizer_t length;
	unsigned int increment;
	unsigned char current;

	for (increment = 0; increment < 1024; increment++) {
		current = rand() % 50;
		if (current < 25) {
			random[increment] = 'l';
		}
		else {
			random[increment] = 'L';
		}
	}
	random[1024] = '\0';

	one = import_ns(random);
	two = duplicate_ns(random);

	if (one == NULL || two == NULL) {
		if (one != NULL) {
			free_st(one);
		}
		if (two != NULL) {
			 free_ns(two);
		}
		return 0;
	}

	result = merge_strings("snsnsnsn", one, two, one, two, one, two, one, two);
	if (result == NULL) {
		free_st(one);
		free_ns(two);
		return 0;
	}

	if (size_st(result) != 8 * 1024 || used_st(result) != 8 * 1024) {
		free_st(one);
		free_ns(two);
		free_st(result);
		return 0;
	}

	// Make sure the built string is correct.
	for (increment = 0; increment < 8; increment++) {
		if (memcmp(data_st(result) + (increment * 1024), random, 1024) != 0) {
			free_st(one);
			free_ns(two);
			free_st(result);
			return 0;
		}
	}

	free_st(result);

	result = allocate_st((8 * 1024) + 1);
	if (result == NULL) {
		free_st(one);
		free_ns(two);
		return 0;
	}

	length = sprintf_st(result, "%.*s%s%.*s%s%.*s%s%.*s%s", used_st(one), data_st(one), two, used_st(one), data_st(one), two, used_st(one), data_st(one), two, \
		used_st(one), data_st(one), two);
	if (length != 8 * 1024) {
		free_st(one);
		free_ns(two);
		free_st(result);
		return 0;
	}

	// Make sure the built string is correct.
	for (increment = 0; increment < 8; increment++) {
		if (memcmp(data_st(result) + (increment * 1024), random, 1024) != 0) {
			free_st(one);
			free_ns(two);
			free_st(result);
			return 0;
		}
	}

	free_st(one);
	free_ns(two);
	free_st(result);

	return 1;
}

// Test the identical class of functions.
int unit_test_9(void) {

	int current;
	int increment;
	char *test_1 = "Harry Potter posted pines of perfect people porking powers.";
	char *test_2 = "Harry Potter posted pines of perfect people porking Powers.";
	char *test_3 = "Harry Potter posted pines of perfect people porking powers";
	char random[1025];
	char random_2[1025];
	stringer_t *test_st_1;
	stringer_t *test_st_2;
	stringer_t *test_st_3;

	if (identical_ns_ns(test_1, test_1) != 1) {
		return 0;
	}

	if (identical_ns_ns(test_1, test_2) != 0) {
		return 0;
	}

	if (identical_ns_ns(test_1, test_3) != 0) {
		return 0;
	}

	if (identical_ns_ns_case(test_1, test_2) != 1) {
		return 0;
	}

	if (identical_ns_ns_case(test_1, test_3) != 0) {
		return 0;
	}

	for (increment = 0; increment < 1024; increment++) {
		current = rand() % 50;
		if (current < 25) {
			random[increment] = 'l';
		}
		else {
			random[increment] = 'L';
		}
	}

	random[1024] = '\0';

	for (increment = 0; increment < 1024; increment++) {
		current = rand() % 50;
		if (current < 25) {
			random_2[increment] = 'l';
		}
		else {
			random_2[increment] = 'L';
		}
	}

	random_2[1024] = '\0';

	if (identical_ns_ns(random, random) != 1) {
		return 0;
	}

	if (identical_ns_ns(random, random_2) != 0) {
		return 0;
	}

	if (identical_ns_ns_case(random, random_2) != 1) {
		return 0;
	}

	test_st_1 = import_ns(test_1);
	if (test_st_1 == NULL) {
		return 0;
	}

	test_st_2 = import_ns(test_2);
	if (test_st_2 == NULL) {
		free_st(test_st_1);
		return 0;
	}

	test_st_3 = import_ns(test_3);
	if (test_st_3 == NULL) {
		free_st(test_st_1);
		free_st(test_st_2);
		return 0;
	}


	if (identical_st_st(test_st_1, test_st_1) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_st(test_st_1, test_st_2) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_st(test_st_1, test_st_3) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_st_case(test_st_1, test_st_2) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_st_case(test_st_1, test_st_3) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	// Now compare st's to nulls.
	if (identical_st_ns(test_st_1, test_1) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_ns(test_st_1, test_2) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_ns(test_st_1, test_3) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_ns_case(test_st_1, test_2) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_ns_case(test_st_1, test_3) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	if (identical_st_ns(test_st_1, test_1) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		free_st(test_st_3);
		return 0;
	}

	free_st(test_st_1);
	free_st(test_st_2);
	free_st(test_st_3);

	test_st_1 = import_ns(random);
	if (test_st_1 == NULL) {
		return 0;
	}

	test_st_2 = import_ns(random_2);
	if (test_st_2 == NULL) {
		free_st(test_st_1);
		return 0;
	}


	if (identical_ns_ns(random, random) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		return 0;
	}

	if (identical_ns_ns(random, random_2) != 0) {
		free_st(test_st_1);
		free_st(test_st_2);
		return 0;
	}

	if (identical_ns_ns_case(random, random_2) != 1) {
		free_st(test_st_1);
		free_st(test_st_2);
		return 0;
	}

	free_st(test_st_1);
	free_st(test_st_2);

	return 1;
}

// Test the case changing functions.
int unit_test_10(void) {

	char *test_ns;
	stringer_t *test_st;
	const char *all_upper = "STRING FOR TESTING CAP CHANGING FUNCTIONS.";
	const char *all_lower = "string for testing cap changing functions.";

	test_ns = duplicate_ns(all_upper);
	if (test_ns == NULL) {
		return 0;
	}

	test_st = import_ns(all_upper);
	if (test_st == NULL) {
		free_ns(test_ns);
		return 0;
	}

	lowercase_st(test_st);
	lowercase_ns(test_ns, size_ns(test_ns));

	if (identical_st_ns(test_st, test_ns) != 1 || identical_st_ns(test_st, all_lower) != 1 || memcmp(test_ns, all_lower, size_ns(all_lower)) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		return 0;
	}

	uppercase_st(test_st);
	uppercase_ns(test_ns, size_ns(test_ns));

	if (identical_st_ns(test_st, test_ns) != 1 || identical_st_ns(test_st, all_upper) != 1 || memcmp(test_ns, all_upper, size_ns(all_upper)) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		return 0;
	}

	free_ns(test_ns);
	free_st(test_st);

	if (lowercase_c('A') != 'a' || uppercase_c('a') != 'A' || lowercase_c('a') != 'a' || uppercase_c('A') != 'A') {
		return 0;
	}

	return 1;
}

// Test the starts class of functions.
int unit_test_11(void) {

	stringer_t *test_1;
	stringer_t *test_2;
	stringer_t *test_3;
	stringer_t *test_4;
	stringer_t *sentence_st;
	const char *sentence = "Plucking poor pickels produces platform pandimonium.";

	sentence_st = import_ns(sentence);
	if (sentence_st == NULL) {
		return 0;
	}

	test_1 = import_ns("Plucking");
	if (test_1 == NULL) {
		free_st(sentence_st);
		return 0;
	}

	test_2 = import_ns("Porking");
	if (test_2 == NULL) {
		free_st(test_1);
		free_st(sentence_st);
		return 0;
	}

	test_3 = import_ns("Plucking poor pickels produces platform pandimonium..");
	if (test_3 == NULL) {
		free_st(test_1);
		free_st(test_2);
		free_st(sentence_st);
		return 0;
	}

	test_4 = import_ns("PLUCKING");
	if (test_3 == NULL) {
		free_st(test_1);
		free_st(test_2);
		free_st(test_3);
		free_st(sentence_st);
		return 0;
	}

	if (starts_st_ns(sentence_st, "Plucking") != 1 || starts_st_ns(sentence_st, "Porking") != 0 || \
		starts_st_ns(sentence_st, "Plucking poor pickels produces platform pandimonium..") != 0 || \
		starts_st_ns_case(sentence_st, "PLUCKING") != 1 || starts_st_ns_case(sentence_st, "PORKING") != 0 || \
		starts_st_ns_amt(sentence_st, "Plucking poor pickels produces platform pandimonium..", 52) != 1 || \
		starts_st_ns_amt(sentence_st, "Porkingg poor pickels produces platform pandimonium..", 52) != 0 || \
		starts_st_ns_case_amt(sentence_st, "PLUCKING PIGS", 7) != 1 || starts_st_ns_case_amt(sentence_st, "PORKING PIGS", 5) != 0) {
		free_st(test_1);
		free_st(test_2);
		free_st(test_3);
		free_st(test_4);
		free_st(sentence_st);
		return 0;
	}

	if (starts_st_st(sentence_st, test_1) != 1 || starts_st_st(sentence_st, test_2) != 0 || \
		starts_st_st(sentence_st, test_3) != 0 || \
		starts_st_st_case(sentence_st, test_4) != 1 || starts_st_st_case(sentence_st, test_2) != 0 || \
		starts_st_st_amt(sentence_st, test_3, 52) != 1 || \
		starts_st_st_amt(sentence_st, test_2, 5) != 0 || \
		starts_st_st_case_amt(sentence_st, test_2, 1) != 1 || starts_st_ns_case_amt(sentence_st, test_2, 3) != 0) {
		free_st(test_1);
		free_st(test_2);
		free_st(test_3);
		free_st(test_4);
		free_st(sentence_st);
		return 0;
	}

	if (starts_ns_st(sentence, test_1) != 1 || starts_ns_st(sentence, test_2) != 0 || \
		starts_ns_st(sentence, test_3) != 0 || \
		starts_ns_st_case(sentence, test_4) != 1 || starts_ns_st_case(sentence, test_2) != 0 || \
		starts_ns_st_amt(sentence, test_3, 52) != 1 || \
		starts_ns_st_amt(sentence, test_2, 5) != 0 || \
		starts_ns_st_case_amt(sentence, test_2, 1) != 1 || starts_ns_ns_case_amt(sentence, test_2, 3) != 0) {
		free_st(test_1);
		free_st(test_2);
		free_st(test_3);
		free_st(test_4);
		free_st(sentence_st);
		return 0;
	}

	if (starts_ns_ns(sentence, "Plucking") != 1 || starts_ns_ns(sentence, "Porking") != 0 || \
		starts_ns_ns(sentence, "Plucking poor pickels produces platform pandimonium..") != 0 || \
		starts_ns_ns_case(sentence, "PLUCKING") != 1 || starts_ns_ns_case(sentence, "PORKING") != 0 || \
		starts_ns_ns_amt(sentence, "Plucking poor pickels produces platform pandimonium..", 52) != 1 || \
		starts_ns_ns_amt(sentence, "Porkingg poor pickels produces platform pandimonium..", 52) != 0 || \
		starts_ns_ns_case_amt(sentence, "PLUCKING PIGS", 7) != 1 || starts_ns_ns_case_amt(sentence, "PORKING PIGS", 5) != 0) {
		free_st(test_1);
		free_st(test_2);
		free_st(test_3);
		free_st(test_4);
		free_st(sentence_st);
		return 0;
	}

	free_st(test_1);
	free_st(test_2);
	free_st(test_3);
	free_st(test_4);
	free_st(sentence_st);

	return 1;
}

// Test copy functions.
int unit_test_12(void) {

	char *test_ns;
	stringer_t *test_st;
	stringer_t *test_st_2;

	test_ns = allocate_ns(100);
	if (test_ns == NULL) {
		return 0;
	}

	test_st = allocate_st(100);
	if (test_st == NULL) {
		free_ns(test_ns);
		return 0;
	}

	if (copy_ns_ns(test_ns, "A string.") != 1 || memcmp(test_ns, "A string.", size_ns("A string.") + 1) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		return 0;
	}

	if (copy_st_ns(test_st, "A string.") != 1 || identical_st_ns(test_st, "A string.") != 1) {
		free_ns(test_ns);
		free_st(test_st);
		return 0;
	}

	if (copy_ns_st(test_ns, test_st) != 1 || memcmp(test_ns, "A string.", size_ns("A string.") + 1) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		return 0;
	}

	test_st_2 = allocate_st(100);
	if (test_st_2 == NULL) {
		free_ns(test_ns);
		free_st(test_st);
		return 0;
	}

	if (copy_st_st(test_st_2, test_st) != 1 || memcmp(test_st, test_st_2, 100) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		free_st(test_st_2);
		return 0;
	}

	clear_bl(test_ns, 100);

	if (copy_ns_ns_amt(test_ns, "A string.", 3) != 1 || memcmp(test_ns, "A s", 4) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		free_st(test_st_2);
		return 0;
	}

	if (copy_st_ns_amt(test_st, "A string.", 3) != 1 || identical_st_ns(test_st, "A s") != 1) {
		free_ns(test_ns);
		free_st(test_st);
		free_st(test_st_2);
		return 0;
	}

	clear_bl(test_ns, 100);

	if (copy_ns_st_amt(test_ns, test_st_2, 3) != 1 || memcmp(test_ns, "A s", 4) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		free_st(test_st_2);
		return 0;
	}

	clear_bl(data_st(test_st_2), 100);
	if (copy_st_ns(test_st, "A string.") != 1) {
		free_ns(test_ns);
		free_st(test_st);
		free_st(test_st_2);
		return 0;
	}

	if (copy_st_st_amt(test_st_2, test_st, 3) != 1 || memcmp(data_st(test_st_2), "A s", 4) != 0) {
		free_ns(test_ns);
		free_st(test_st);
		free_st(test_st_2);
		return 0;
	}

	free_ns(test_ns);
	free_st(test_st);
	free_st(test_st_2);

	return 1;
}

// Test base64 functions.
int unit_test_13(void) {

	char random[4096];
	char *decoded_ns;
	sizer_t increment;
	stringer_t *random_st;
	stringer_t *encoded_st;
	stringer_t *decoded_st;

	// Fill in the random buffer.
	for (increment = 0; increment < 4096; increment++) {
		random[increment] = rand() % 256;
	}

	random_st = import_bl(random, 4096);
	if (random_st == NULL) {
		return 0;
	}

	encoded_st = encode_base64_st(random_st);
	if (encoded_st == NULL) {
		free_st(random_st);
		return 0;
	}

	decoded_st = decode_base64_st(encoded_st);
	if (decoded_st == NULL) {
		free_st(random_st);
		free_st(encoded_st);
		return 0;
	}

	if (starts_ns_ns_amt(data_st(decoded_st), random, 4096) != 1) {
		free_st(random_st);
		free_st(encoded_st);
		free_st(decoded_st);
		return 0;
	}

	free_st(encoded_st);
	free_st(decoded_st);

	encoded_st = encode_base64_ns("A random string of text.");
	if (encoded_st == NULL) {
		free_st(random_st);
		return 0;
	}

	if (export_ns(&decoded_ns, encoded_st) == 0) {
		free_st(random_st);
		free_st(encoded_st);
		return 0;
	}

	decoded_st = decode_base64_ns(decoded_ns);
	if (decoded_st == NULL) {
		free_st(random_st);
		free_st(encoded_st);
		free_ns(decoded_ns);
		return 0;
	}

	if (memcmp(data_st(decoded_st), "A random string of text.", size_ns("A random string of text.")) != 0) {
		free_st(random_st);
		free_st(encoded_st);
		free_st(decoded_st);
		free_ns(decoded_ns);
		return 0;
	}

	free_st(encoded_st);
	free_st(decoded_st);
	free_ns(decoded_ns);

	encoded_st = encode_base64_st_amt(random_st, 400);
	if (encoded_st == NULL) {
		free_st(random_st);
		return 0;
	}
	decoded_st = decode_base64_st_amt(encoded_st, used_st(encoded_st));
	if (decoded_st == NULL) {
		free_st(random_st);
		free_st(encoded_st);
		return 0;
	}
	if (used_st(decoded_st) != 400) {
		free_st(random_st);
		free_st(encoded_st);
		free_st(decoded_st);
		return 0;
	}

	if (starts_ns_ns_amt(data_st(decoded_st), random, 400) != 1) {
		free_st(random_st);
		free_st(encoded_st);
		free_st(decoded_st);
		return 0;
	}

	free_st(encoded_st);
	free_st(decoded_st);

	encoded_st = encode_base64_ns_amt("A random string of text.", 1);
	if (encoded_st == NULL) {
		free_st(random_st);
		return 0;
	}

	if (export_ns(&decoded_ns, encoded_st) == 0) {
		free_st(random_st);
		free_st(encoded_st);
		return 0;
	}

	decoded_st = decode_base64_ns_amt(decoded_ns, size_ns(decoded_ns));
	if (decoded_st == NULL) {
		free_st(random_st);
		free_st(encoded_st);
		free_ns(decoded_ns);
		return 0;
	}

	if (memcmp(data_st(decoded_st), "A random string of text.", 1) != 0) {
		free_st(random_st);
		free_st(encoded_st);
		free_st(decoded_st);
		free_ns(decoded_ns);
		return 0;
	}

	free_st(random_st);
	free_st(encoded_st);
	free_st(decoded_st);
	free_ns(decoded_ns);

	return 1;
}

// Test the QP functions, and the HEX functions.
int unit_test_14(void) {

	char output[2];
	stringer_t *qp_st_1;
	stringer_t *qp_st_2;
	stringer_t *reg_st_1;
	stringer_t *reg_st_2;
	const char *test = "==https://www.nerdshack.com/apps/teacher?sig=blah&key=blah";

	encode_hex_c('A', output);
	if (output[0] != '4' || output[1] != '1') {
		return 0;
	}

	if (decode_hex_c(output) != 'A') {
		return 0;
	}

	qp_st_1 = encode_qp_ns(test);
	if (qp_st_1 == NULL) {
		return 0;
	}

	qp_st_2 = encode_qp_ns_amt(test, size_ns(test));
	if (qp_st_2 == NULL) {
		free_st(qp_st_1);
		return 0;
	}

	if (identical_st_st(qp_st_1, qp_st_2) != 1) {
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(qp_st_2);

	qp_st_2 = encode_qp_ns_amt(test, 10);
	if (qp_st_2 == NULL) {
		free_st(qp_st_1);
		return 0;
	}

	reg_st_1 = decode_qp_st(qp_st_1);
	if (reg_st_1 == NULL) {
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	if (identical_st_ns(reg_st_1, test) != 1) {
		free_st(reg_st_1);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	reg_st_2 = decode_qp_st(qp_st_2);
	if (reg_st_2 == NULL) {
		free_st(reg_st_1);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	if (starts_st_ns_amt(reg_st_2, test, 10) != 1) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(qp_st_2);

	qp_st_2 = encode_qp_st(reg_st_1);
	if (qp_st_2 == NULL) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		return 0;
	}

	if (identical_st_st(qp_st_1, qp_st_2) != 1) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(qp_st_2);

	qp_st_2 = encode_qp_st_amt(reg_st_1, 10);
	if (qp_st_2 == NULL) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		return 0;
	}

	if (starts_st_st(qp_st_1, qp_st_2) != 1) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(reg_st_2);

	reg_st_2 = decode_qp_st_amt(qp_st_2, used_st(qp_st_2));
	if (reg_st_2 == NULL) {
		free_st(reg_st_1);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	if (starts_st_ns_amt(reg_st_2, test, 10) != 1) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(reg_st_2);

	reg_st_2 = decode_qp_ns_amt(data_st(qp_st_2), used_st(qp_st_2));
	if (reg_st_2 == NULL) {
		free_st(reg_st_1);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	if (starts_st_ns_amt(reg_st_2, test, 10) != 1) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(reg_st_2);

	reg_st_2 = decode_qp_ns(data_st(qp_st_2));
	if (reg_st_2 == NULL) {
		free_st(reg_st_1);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	if (starts_st_ns_amt(reg_st_2, test, 10) != 1) {
		free_st(reg_st_1);
		free_st(reg_st_2);
		free_st(qp_st_1);
		free_st(qp_st_2);
		return 0;
	}

	free_st(reg_st_1);
	free_st(reg_st_2);
	free_st(qp_st_1);
	free_st(qp_st_2);

	return 1;
}

// Test the number extraction functions.
int unit_test_15(void) {

	int i;
	long l;
	long long ll;
	short int s;
	unsigned short int us;
	unsigned int ui;
	unsigned long ul;
	unsigned long long ull;
	stringer_t *test;

	if (extract_s_ns("100", 3, &s) != 1 && s != 100) {
		return 0;
	}

	if (extract_i_ns("1000", 4, &i) != 1 && i != 1000) {
		return 0;
	}

	if (extract_l_ns("10000", 5, &l) != 1 && l != 10000) {
		return 0;
	}

	if (extract_ll_ns("100000", 6, &ll) != 1 && ll != 100000) {
		return 0;
	}

	if (extract_us_ns("100", 3, &us) != 1 && us != 100) {
		return 0;
	}

	if (extract_ui_ns("1000", 4, &ui) != 1 && ui != 1000) {
		return 0;
	}

	if (extract_ul_ns("10000", 5, &ul) != 1 && ul != 10000) {
		return 0;
	}

	if (extract_ull_ns("100000", 6, &ull) != 1 && ull != 100000) {
		return 0;
	}

	test = import_ns("-100");
	if (test == NULL) {
		return 0;
	}

	if (extract_s(test, &s) != 1 && s != -100) {
		free_st(test);
		return 0;
	}

	if (extract_i(test, &i) != 1 && i != -100) {
		free_st(test);
		return 0;
	}

	if (extract_l(test, &l) != 1 && l != -100) {
		free_st(test);
		return 0;
	}

	if (extract_ll(test, &ll) != 1 && ll != -100) {
		free_st(test);
		return 0;
	}

	free_st(test);

	test = import_ns("102");
	if (test == NULL) {
		return 0;
	}

	if (extract_us(test, &us) != 1 && us != 102) {
		free_st(test);
		return 0;
	}

	if (extract_ui(test, &ui) != 1 && ui != 102) {
		free_st(test);
		return 0;
	}

	if (extract_ul(test, &ul) != 1 && ul != 102) {
		free_st(test);
		return 0;
	}

	if (extract_ull(test, &ull) != 1 && ull != 102) {
		free_st(test);
		return 0;
	}

	free_st(test);

	return 1;

}

// Test the IP parsing functions.
int unit_test_16(void) {

	stringer_t *test;

	test = import_ns("127.0.0.1");
	if (test == NULL) {
		return 0;
	}

	if (get_octet_v4(test, 1) != 127) {
		free_st(test);
		return 0;
	}

	if (get_octet_v4(test, 2) != 0) {
		free_st(test);
		return 0;
	}

	if (get_octet_v4(test, 3) != 0) {
		free_st(test);
		return 0;
	}

	if (get_octet_v4(test, 4) != 1) {
		free_st(test);
		return 0;
	}

	free_st(test);

	test = import_ns("255.254.253.252");
	if (test == NULL) {
		return 0;
	}

	if (get_octet_v4(test, 1) != 255) {
		free_st(test);
		return 0;
	}

	if (get_octet_v4(test, 2) != 254) {
		free_st(test);
		return 0;
	}

	if (get_octet_v4(test, 3) != 253) {
		free_st(test);
		return 0;
	}

	if (get_octet_v4(test, 4) != 252) {
		free_st(test);
		return 0;
	}

	free_st(test);

	return 1;

}

// Run database tests.
int unit_test_17(void) {

	int connection;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (exec_query_ns("INSERT INTO Receiving VALUES (1, 127,0,0, NOW())", size_ns("INSERT INTO Receiving VALUES (1, 127,0,0, NOW())")) != 0) {
		return 0;
	}

	connection = start_tran();
	if (connection < 0) {
		return 0;
	}

	if (exec_query_tran_ns("INSERT INTO Receiving VALUES (1, 127,0,0, NOW())", size_ns("INSERT INTO Receiving VALUES (1, 127,0,0, NOW())"), connection) != 0) {
		rollback_tran(connection);
		return 0;
	}

	if (commit_tran(connection) != 1) {
		return 0;
	}

	result = exec_query_res_ns("SELECT NOW()", size_ns("SELECT NOW()"));
	if (result == NULL) {
		return 0;
	}

	row = get_row(result);
	if (row == NULL) {
		free_res(result);
		return 0;
	}

	free_res(result);

	if (exec_query_rows_ns("SELECT NOW()", size_ns("SELECT NOW()")) != 1) {
		return 0;
	}

	if (exec_query_ns("DELETE FROM Receiving", size_ns("DELETE FROM Receiving")) != 0) {
		return 0;
	}

	return 1;

}

// Test the replacement and removal functions.
int unit_test_18(void) {

	char *string_ns;
	stringer_t *string_st;
	stringer_t *holder;
	stringer_t *third;

	string_ns = duplicate_ns("TEST1 TEST2");
	if (string_ns == NULL) {
		return 0;
	}

	// Replace and test for a match.
	if (replace_ns_ns_ns(&string_ns, "TEST1", "TEST2") != 1) {
		free_ns(string_ns);
		return 0;
	}
	else if (identical_ns_ns(string_ns, "TEST2 TEST2") != 1) {
		free_ns(string_ns);
		return 0;
	}

	string_st = import_ns("TEST1");
	if (string_st == NULL) {
		free_ns(string_ns);
		return 0;
	}

	// Replace and test for a match.
	if (replace_ns_ns_st(&string_ns, "TEST2", string_st) != 2) {
		free_ns(string_ns);
		free_st(string_st);
		return 0;
	}
	else if (identical_ns_ns(string_ns, "TEST1 TEST1") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		return 0;
	}

	// Replace and test for a match.
	if (replace_ns_st_ns(&string_ns, string_st, "TEST2") != 2) {
		free_ns(string_ns);
		free_st(string_st);
		return 0;
	}
	else if (identical_ns_ns(string_ns, "TEST2 TEST2") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		return 0;
	}

	holder = import_ns("TEST1 TEST2");
	if (holder == NULL) {
		free_ns(string_ns);
		free_st(string_st);
		return 0;
	}

	// Replace and test for a match.
	if (replace_st_ns_ns(&holder, "TEST1", "TEST2") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_st_ns(holder, "TEST2 TEST2") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	// Replace and test for a match.
	if (replace_st_ns_st(&holder, "TEST2", string_st) != 2) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_st_ns(holder, "TEST1 TEST1") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	// Replace and test for a match.
	if (replace_st_st_ns(&holder, string_st, "TEST2") != 2) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_st_ns(holder, "TEST2 TEST2") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	free_st(holder);

	holder = import_ns("TEST2");
	if (holder == NULL) {
		free_ns(string_ns);
		free_st(string_st);
		return 0;
	}

	// Replace and test for a match.
	if (replace_ns_st_st(&string_ns, holder, string_st) != 2) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_ns_ns(string_ns, "TEST1 TEST1") != 1) {
		free_ns(string_ns);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	free_ns(string_ns);

	third = import_ns("TEST1 TEST1 TEST2");
	if (third == NULL) {
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	// Replace and test for a match.
	if (replace_st_st_st(&third, string_st, holder) != 2) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_st_ns(third, "TEST2 TEST2 TEST2") != 1) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	if (remove_st_st(&third, holder) != 3) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_st_ns(third, "  ") != 1) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	if (remove_st_ns(&holder, "TEST") != 1) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_st_ns(holder, "2") != 1) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	string_ns = duplicate_ns("TEST1 TEST1 TEST2 TEST1");
	if (string_ns == NULL) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	if (remove_ns_ns(&string_ns, "TEST1") != 3) {
		free_ns(string_ns);
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_ns_ns(string_ns, "  TEST2 ") != 1) {
		free_ns(string_ns);
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	free_ns(string_ns);

	string_ns = duplicate_ns("TEST1 TEST1 TEST2 TEST1");
	if (string_ns == NULL) {
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	if (remove_ns_st(&string_ns, string_st) != 3) {
		free_ns(string_ns);
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}
	else if (identical_ns_ns(string_ns, "  TEST2 ") != 1) {
		free_ns(string_ns);
		free_st(third);
		free_st(string_st);
		free_st(holder);
		return 0;
	}

	free_ns(string_ns);
	free_st(third);
	free_st(string_st);
	free_st(holder);

	return 1;
}

// Test the length functions.
int unit_test_19(void) {

	if (length_s(-1) != 2 || length_s(111) != 3) {
		return 0;
	}

	if (length_i(-128000) != 7 || length_i(129000) != 6) {
		return 0;
	}

	if (length_l(12345678) != 8 || length_l(-12345678) != 9) {
		return 0;
	}

	if (length_ll(1234567890) != 10 || length_ll(-1234567890) != 11) {
		return 0;
	}

	if (length_us(1) != 1) {
		return 0;
	}

	if (length_ui(1236) != 4) {
		return 0;
	}

	if (length_ul(123456789) != 9) {
		return 0;
	}

	if (length_ull(12345678901234567LL) != 17) {
		return 0;
	}

	return 1;
}

// Test the tokenization functions.
int unit_test_20(void) {

	stringer_t *test_st;
	stringer_t *result;

	result = get_token_ns("This is a NULL delimited \0string\0 of characters.", 48, '\0', 2);
	if (result == NULL) {
		return 0;
	}
	else if (identical_st_ns(result, "string") != 1) {
		free_st(result);
		return 0;
	}

	free_st(result);

	result = get_token_ns("This is a NULL delimited \0string\0 of characters.", 48, '\0', 3);
	if (result == NULL) {
		return 0;
	}
	else if (identical_st_ns(result, " of characters.") != 1) {
		free_st(result);
		return 0;
	}

	free_st(result);

	result = get_token_ns("This is a NULL delimited \0string\0 of characters.", 48, '\0', 1);
	if (result == NULL) {
		return 0;
	}
	else if (identical_st_ns(result, "This is a NULL delimited ") != 1) {
		free_st(result);
		return 0;
	}

	free_st(result);

	test_st = import_ns("This is a semicolon delimited ;string; of characters.");
	if (test_st == NULL) {
		return 0;
	}

	result = get_token(test_st, ';', 2);
	if (result == NULL) {
		free_st(test_st);
		return 0;
	}
	else if (identical_st_ns(result, "string") != 1) {
		free_st(test_st);
		free_st(result);
		return 0;
	}

	free_st(result);

	result = get_token(test_st, ';', 3);
	if (result == NULL) {
		free_st(test_st);
		return 0;
	}
	else if (identical_st_ns(result, " of characters.") != 1) {
		free_st(test_st);
		free_st(result);
		return 0;
	}

		free_st(result);

	result = get_token(test_st, ';', 1);
	if (result == NULL) {
		free_st(test_st);
		return 0;
	}
	else if (identical_st_ns(result, "This is a semicolon delimited ") != 1) {
		free_st(test_st);
		free_st(result);
		return 0;
	}

	free_st(test_st);
	free_st(result);

	return 1;
}

// Test the search functions.
int unit_test_21(void) {

	stringer_t *needle;
	stringer_t *haystack;

	if (search_ns_ns("This is a test of the emergency broadcast system.", "test") != 10) {
		return 0;
	}

	if (search_ns_ns("This is a test of the emergency broadcast system.", "not there") != 0) {
		return 0;
	}

	if (search_ns_ns_amt("This is a test of the emergency broadcast system.", 20, "test") != 10) {
		return 0;
	}

	if (search_ns_ns_case("This is a test of the emergency broadcast system.", "TEST") != 10) {
		return 0;
	}

	if (search_ns_ns_case("This is a test of the emergency broadcast system.", "don't find") != 0) {
		return 0;
	}

	if (search_ns_ns_case_amt("This is a test of the emergency broadcast system.", 20, "TEST") != 10) {
		return 0;
	}

	haystack = import_ns("This is a test of the emergency broadcast system.");
	if (haystack == NULL) {
		return 0;
	}

	if (search_st_ns(haystack, "test") != 10) {
		free_st(haystack);
		return 0;
	}

	if (search_st_ns_amt(haystack, 20, "test") != 10) {
		free_st(haystack);
		return 0;
	}

	if (search_st_ns_case(haystack, "TEST") != 10) {
		free_st(haystack);
		return 0;
	}

	if (search_st_ns_case_amt(haystack, 20, "TEST") != 10) {
		free_st(haystack);
		return 0;
	}

	needle = import_ns(".");
	if (needle == NULL) {
		free_st(haystack);
		return 0;
	}

	if (search_st_st(haystack, needle) != 48) {
		free_st(needle);
		free_st(haystack);
		return 0;
	}

	if (search_st_st_amt(haystack, 49, needle) != 48) {
		free_st(needle);
		free_st(haystack);
		return 0;
	}

	free_st(needle);

	needle = import_ns("EMERGENCY");
	if (needle == NULL) {
		free_st(haystack);
		return 0;
	}

	if (search_st_st_case(haystack, needle) != 22) {
		free_st(needle);
		free_st(haystack);
		return 0;
	}

	if (search_st_st_case_amt(haystack, 49, needle) != 22) {
		free_st(needle);
		free_st(haystack);
		return 0;
	}

	free_st(haystack);

	if (search_ns_st_case("This is a test of the emergency broadcast system.", needle) != 22) {
		free_st(needle);
		return 0;
	}

	if (search_ns_st_case_amt("This is a test of the emergency broadcast system.", 49, needle) != 22) {
		free_st(needle);
		return 0;
	}

	free_st(needle);

	needle = import_ns(".");
	if (needle == NULL) {
		return 0;
	}

	if (search_ns_st("This is a test of the emergency broadcast system.", needle) != 48) {
		free_st(needle);
		return 0;
	}

	if (search_ns_st_amt("This is a test of the emergency broadcast system.", 49, needle) != 48) {
		free_st(needle);
		return 0;
	}

	free_st(needle);

	return 1;
}

// Test the placer functions.
int unit_test_22(void) {

	placer_t test;
	char *string = "Polly had a perfect picture of Sky Captain.";


	if (starts_ns_ns_amt("Sky Captain", data_pl(test), size_pl(test)) != 1) {
		return 0;
	}

	return 1;
}

// Test the mail and append functions.
int unit_test_23(void) {
	/*
	placer_t pass;
	stringer_t *message_st;
	smtp_message_t *message;
	smtp_session_t smtp;
	char *address = "From: \"Bubba Gump\" <adddress@domain.com>\r\n";

	message_st = import_ns("\r\nBlah Blah Blah.\n.\n");
	if (message_st == NULL) {
		return 0;
	}

	if (mail_message_cleanup(&message_st) != 1) {
		if (message_st != NULL) {
			free_st(message_st);
		}
		return 0;
	}

	message = mail_create_message(message_st);
	if (message == NULL) {
		if (message_st != NULL) {
			free_st(message_st);
		}
		return 0;
	}

	clear_bl(&smtp, sizeof(smtp_session_t));
	smtp.mailfrom = import_ns("bubba@gump.com");
	if (smtp.mailfrom == NULL) {
		if (message != NULL) {
			mail_destroy_message(message);
		}
		return 0;
	}

	if (mail_add_required_headers(&smtp, message) != 1) {
		if (message != NULL) {
			mail_destroy_message(message);
		}
		return 0;
	}

	// Cleanup.

	if (message != NULL) {
		mail_destroy_message(message);
	}
	free_st(smtp.mailfrom);

	pass = set_pl(address, size_ns(address));
	message_st = mail_extract_address(pass);
	if (message_st == NULL) {
		return 0;
	}
	else if (identical_st_ns(message_st, "adddress@domain.com") != 1) {
		free_st(message_st);
		return 0;
	}

	free_st(message_st);*/
	return 1;
}

// Test the compression routines.
int unit_test_24(void) {

	stringer_t *string;
	stringer_t *string2;
	reducer_t *reduced;

	string = import_ns("This is a test string. Were going to compress it and then uncompress it.");
	if (string == NULL) {
		return 0;
	}

	reduced = compress_lzo(string);
	if (reduced == NULL) {
		free_st(string);
		return 0;
	}

	free_st(string);

	string = decompress_lzo(reduced);
	free_rt(reduced);
	if (string == NULL) {
		return 0;
	}

	if (identical_st_ns(string, "This is a test string. Were going to compress it and then uncompress it.") != 1) {
		free_st(string);
		return 0;
	}

	free_st(string);

	string = random_st_choices(4096, "ABCDEFGHIJKLKMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890   \t\t\n\n\n", 71);
	if (string == NULL) {
		return 0;
	}

	reduced = compress_lzo(string);
	if (reduced == NULL) {
		free_st(string);
		return 0;
	}

	string2 = decompress_lzo(reduced);
	if (string2 == NULL) {
		free_st(string);
		free_rt(reduced);
		return 0;
	}

	if (identical_st_st(string, string2) != 1) {
		free_st(string);
		free_st(string2);
		free_rt(reduced);
		return 0;
	}

	free_st(string);

	string = import_bl(data_buf_rt(reduced), size_buf_rt(reduced));
	free_rt(reduced);

	reduced = string;

	string = decompress_lzo(reduced);
	if (string == NULL) {
		free_st(string2);
		free_rt(reduced);
		return 0;
	}

	if (identical_st_st(string, string2) != 1) {
		free_st(string);
		free_st(string2);
		free_rt(reduced);
		return 0;
	}

	free_st(string);
	free_st(string2);
	free_rt(reduced);
	return 1;
}

int run_unit_tests(void) {

	int increment;

	lavalog("Testing framework version %s.", FRAMEWORK_VERSION);

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_1() != 1) {
			lavalog("Unit test one, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_2() != 1) {
			lavalog("Unit test two, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_3() != 1) {
			lavalog("Unit test three, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_4() != 1) {
			lavalog("Unit test four, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_5() != 1) {
			lavalog("Unit test five, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_6() != 1) {
			lavalog("Unit test six, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_7() != 1) {
			lavalog("Unit test seven, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_8() != 1) {
			lavalog("Unit test eight, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_9() != 1) {
			lavalog("Unit test nine, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_10() != 1) {
			lavalog("Unit test ten, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_11() != 1) {
			lavalog("Unit test eleven, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_12() != 1) {
			lavalog("Unit test twelve, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_13() != 1) {
			lavalog("Unit test thirteen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_14() != 1) {
			lavalog("Unit test fourteen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_15() != 1) {
			lavalog("Unit test fifteen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_16() != 1) {
			lavalog("Unit test sixteen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_17() != 1) {
			lavalog("Unit test seventeen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_18() != 1) {
			lavalog("Unit test eighteen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_19() != 1) {
			lavalog("Unit test nineteen, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_20() != 1) {
			lavalog("Unit test twenty, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_21() != 1) {
			lavalog("Unit test twenty-one, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_22() != 1) {
			lavalog("Unit test twenty-two, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_23() != 1) {
			lavalog("Unit test twenty-three, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	for (increment = 0; increment < ITERATIONS; increment++) {
		if (unit_test_24() != 1) {
			lavalog("Unit test twenty-four, iteration %i FAILED.", increment + 1);
			return 0;
		}
	}

	lavalog("Twenty-three unit tests PASSED. Ran %i iterations.", ITERATIONS);
	return 1;
}
