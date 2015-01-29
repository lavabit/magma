
#include "magma.open.check.h"

void symbols_check(void);

int main(int argc, char **argv) {

	void *handle;
	char libpath[1024];

	// Check and see if any of our symbols trigger an error.
	symbols_check();

	if (argc >= 2)
			snprintf(libpath, 1024, "%s", argv[1]);
	else
		snprintf(libpath, 1024, "%s", "magmad.so");

	if (!(handle = dlopen(libpath, RTLD_NOW | RTLD_LOCAL))) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	printf("%s loaded...\n\n", libpath);

	if (dlclose(handle)) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
