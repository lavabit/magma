
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>

#include "gtest/gtest.h"
#include "tap.h"

extern "C" {
#include "symbols.h"
bool lib_load_openssl(void);
bool lib_load_utf8proc(void);
extern void *lib_magma;
}

/**
 * @brief	Create a handle for the currently loaded program, and dynamically resolve all the symbols for external dependencies.
 * @return	0 for success, and a negative number when an error occurs.
 */
int dime_lib_load(void) {

	char *lib_error = NULL;
	lib_magma = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
	if (!lib_magma || (lib_error = dlerror())) {
		if (lib_error) {
			fprintf(stderr, "The dlerror() function returned: %s\n", lib_error);
		}
		return -1;
	}

	else if (!lib_load_openssl() || !lib_load_utf8proc()) {
		return -1;
	}

	return 0;
}

GTEST_API_ int main(int argc, char **argv) {

	char *path = NULL;

	// Load the OpenSSL/utf8proc symbols used by libdime.
	if (dime_lib_load()) {
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
