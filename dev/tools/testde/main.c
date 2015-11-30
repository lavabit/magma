
#include "framework.h"

void * lavalib;
global_config_t config;

int main(int argc, char *argv[]) {

	int fd;
	char *buf;
	stringer_t *compressed;
	stringer_t *uncompressed;
	struct stat info;
	char filepath[1000], lib[1000], storage[1000];
	unsigned long long messagenum;

	if (argc != 2 || getenv("MAGMA_LIBRARY") == NULL  || getenv("MAGMA_STORAGE") == NULL) {
		printf("Usage : testde messagenum\n\nMAGMA_LIBRARY = location magmad.so%s\nMAGMA_STORAGE = location of the storage tree%s\n\n",
			getenv("MAGMA_LIBRARY") ? "" : "               [missing]", getenv("MAGMA_STORAGE") ? "" : "     [missing]");
		return 1;
	}

	printf("\nMagma decompression tester.\n");

	if (getenv("MAGMA_LIBRARY") == NULL || snprintf(lib, 1000, "%s", getenv("MAGMA_LIBRARY")) == 0) {
		printf ("Please set the MAGMA_LIBRARY environment variable.\n\n");
		return 1;
	}

	// Setup the pointer to the dynamic library.
	lavalib = dlopen(lib, RTLD_LOCAL | RTLD_LAZY);
	if (lavalib == NULL) {
		printf("Could not open the magma library.\n");
		printf("%s", dlerror());
		return 0;
	}

	// Load the LZO library symbols.
	if (load_symbols_lzo() != 1) {
		printf("Could not dynamically load the LZO functions.\n");
		dlclose(lavalib);
		return 0;
	}

	if (initialize_lzo() != 1) {
		printf("LZO could not init.\n");
		return 0;
	}


	messagenum = strtoull(argv[1], (char **)NULL, 10);

	if (messagenum == 0) {
		printf("Could not get a valid message number.\n\n");
		return 1;
	}

	if (getenv("MAGMA_STORAGE") == NULL || snprintf(storage, 1000, "%s", getenv("MAGMA_STORAGE")) == 0) {
		printf ("Please set the MAGMA_STORAGE environment variable.\n\n");
		return 1;
	}
	sprintf(filepath, "%s/%lli/%lli/%lli/%lli/%lli", storage, \
		messagenum / 32768 / 32768 / 32768 / 32768, messagenum / 32768 / 32768 / 32768 , \
		messagenum / 32768 / 32768,  messagenum / 32768, messagenum);

	fd = open(filepath, O_RDONLY);

	if (fd < 0) {
		printf("Could not create a file descriptor. Does the file  %s exist?\n\n", filepath);
		return 1;
	}

	if (fstat(fd, &info) != 0) {
		printf("Could figure out how big the file was.\n\n");
		return 1;
	}

	buf = malloc(info.st_size);
	if (buf == NULL) {
		printf("Could not create a buffer of size %d to hold the file.\n\n", (int)info.st_size);
		return 1;
	}

	if (read(fd, buf, (int)info.st_size) <= 0) {
		printf("Could not read the file.\n\n");
		return 1;
	}

	uncompressed = decompress_lzo(buf);


	if (uncompressed != NULL) {

		replace_st_ns_ns(&uncompressed, "\r\n", "\n");

		printf("\n\n----------------------------- START OF MESSAGE -----------------------------\n%.*s", (int)used_st(uncompressed), data_st(uncompressed));

		if (*(data_st(uncompressed) + used_st(uncompressed) - 1) != '\n') {
			printf("\n----------------------------- END OF MESSAGE -----------------------------\n\n" \
				"Was able to decompress %d bytes into %i bytes.\n\n", (int)info.st_size, (int)used_st(uncompressed));
		}
		else {
			printf("----------------------------- END OF MESSAGE -----------------------------\n\n" \
				"Was able to decompress %d bytes into %i bytes.\n\n", (int)info.st_size, (int)used_st(uncompressed));
		}
	}
	else {
		printf("Unable to decompress the message.\n\n");
		return 1;
	}

	return 0;
}
