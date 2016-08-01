
#include <unistd.h>
#include <libgen.h>

extern "C" {
#include "symbols.h"
}

#include "gtest/gtest.h"
#include "tap.h"

GTEST_API_ int main(int argc, char **argv) {

	char *path = NULL;

	// Load the OpenSSL symbols used by libdime.
	if (lib_load()) {
		fprintf(stderr, "Error: unable to bind the program to the required dynamic symbols.\n");
		exit(EXIT_FAILURE);
	}

	// Make sure the working directory is where the dime.check executable gets stored. Otherwise
	// the DIME_CHECK_OUTPUT_PATH won't point to a valid folder and the unit tests will fail.
	path = dirname(argv[0]);
	chdir(path);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
