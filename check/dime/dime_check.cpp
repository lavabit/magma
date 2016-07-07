#include "gtest/gtest.h"
#include "tap.h"

extern "C" {
#include "symbols.h"
}

GTEST_API_ int main(int argc, char **argv) {

	// Load the OpenSSL symbols used by libdime.
	if (lib_load()) {
		fprintf(stderr, "Error: unable to bind the program to the required dynamic symbols.\n");
		exit(EXIT_FAILURE);
	}

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
