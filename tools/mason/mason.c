/**
 * @file /mason/mason.c
 *
 * @brief The data file mason.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/07/27 01:43:24 $
 * $Revision: 4103e46b928680d472921cec918c5e7f8e0698d1 $
 *
 */

#include "mason.h"

#define STORAGE_FILE "/home/ladar/Lavabit/magma.universe/sandbox/storage/mason.dat"
#define DATA_PATH "/home/ladar/Lavabit/magma.universe/data/messages/"

bool clean = false;
bool async = false;
bool lzo1 = false;
bool lzo999 = false;
bool bzip = false;
bool gz = false;
bool flush = false;
uint8_t opts = HDBTLARGE;

lzo_byte *wrkmem;
uint64_t total_bytes = 0;
uint64_t total_objects = 0;

/**
 * Runs through a string and converts all the '/' characters to '.' characters.
 *
 * @param key The buffer to search.
 * @param len The length of the buffer.
 */
void period(char *key, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (*(key + i) == '/') {
			*(key + i) = '.';
		}
	}
	return;
}

/**
 * Translates a file path/name into a retrieval key.
 *
 * @param file The file name to translate.
 * @param key The buffer to store the key in.
 * @param len The length of the key buffer.
 * @return The length of the lookup key.
 */
size_t key(char *file, char *key, size_t len) {

	size_t start, out = 0;

	if (strncasecmp(file, DATA_PATH, (start = strlen(DATA_PATH))) == 0) {
		out = snprintf(key, len, "%s", *(file + start) == '/' ? file + start + 1 : file + start);
		period(key, out);
	}

	return out;
}

/**
 * Recursively loads a directory into the provided storage context.
 *
 * @param hdb The storage context.
 * @param path The directory path to load.
 */
void load(TCHDB *hdb, char *path) {

	int fd;
	DIR *working;
	struct stat info;
	struct dirent *entry;
	size_t klen, clen;
	char file[1024], keyb[1024], *buffer;

	if (!(working = opendir(path))) {
		fprintf(stderr, "Unable to open the data path. {path = %s}", path);
		fflush(stderr);
		return;
	}

	while ((entry = readdir(working))) {

		// Reset.
		errno = 0;
		bzero(file, 1024);

		// If we hit a directory, recursively call the load function.
		if (entry->d_type == DT_DIR && *(entry->d_name) != '.') {
			snprintf(file, 1024, "%s%s%s", path, "/", entry->d_name);
			load(hdb, file);
		} else if (entry->d_type == DT_REG && *(entry->d_name) != '.') {

			snprintf(file, 1024, "%s%s%s", path, "/", entry->d_name);

			// Read the file.
			if ((fd = open(file, O_RDONLY)) < 0) {
				fprintf(stderr, "%s - open error\n", file);
				fflush(stderr);
				closedir(working);
				return;
			}

			// How big is the file?
			if (fstat(fd, &info) != 0) {
				fprintf(stderr, "%s - stat error\n", file);
				fflush(stderr);
				closedir(working);
				close(fd);
				return;
			}

			// Allocate a buffer.
			if (!(buffer = malloc(info.st_size + 1))) {
				fprintf(stderr, "%s - malloc error\n", file);
				fflush(stderr);
				closedir(working);
				close(fd);
				return;
			}

			// Clear the buffer.
			memset(buffer, 0, info.st_size + 1);

			// Read the file.
			if (read(fd, buffer, info.st_size) != info.st_size) {
				fprintf(stderr, "%s - read error\n", file);
				fflush(stderr);
				closedir(working);
				free(buffer);
				close(fd);
				return;
			}

			close(fd);

			// Run the buffer through the LZO engine?
			if (lzo1 || lzo999) {
				clen = lzo_compress((void **)&buffer, info.st_size);
			}	else if (gz) {
				clen = gzip_compress((void **)&buffer, info.st_size);
			} else if (bzip) {
				clen = bzip_compress((void **)&buffer, info.st_size);
			}	else {
				clen = info.st_size;
			}

			if (clen && (klen = key(file, keyb, 1024))) {
				if ((async && !tchdbputasync(hdb, keyb, klen, buffer, clen)) || (!async && !tchdbput(hdb, keyb, klen, buffer, clen))) {
					fprintf(stderr, "%s - error {tchdbput%s = %s}\n", file, async ? "async" : "", tchdberrmsg(tchdbecode(hdb)));
					fflush(stderr);
				} else {
					total_objects += 1;
					total_bytes += info.st_size;
				}
			}

			free(buffer);
		}
	}

	closedir(working);
	return;
}


/**
 * Recursively loads a directory and compares the data to what's in the storage context.
 *
 * @param hdb The storage context.
 * @param path The directory path to load.
 */
void verify(TCHDB *hdb, char *path) {

	int fd;
	int blen;
//	uint32_t adler;
	DIR *working;
	struct stat info;
	struct dirent *entry;
	size_t klen;
	char file[1024], keyb[1024], *buffer;

	if (!(working = opendir(path))) {
		fprintf(stderr, "Unable to open the data path. {path = %s}", path);
		fflush(stderr);
		return;
	}

	while ((entry = readdir(working))) {

		// Reset.
		blen = 0;
		errno = 0;
		bzero(file, 1024);

		// If we hit a directory, recursively call the load function.
		if (entry->d_type == DT_DIR && *(entry->d_name) != '.') {
			snprintf(file, 1024, "%s%s%s", path, "/", entry->d_name);
			verify(hdb, file);
		} else if (entry->d_type == DT_REG && *(entry->d_name) != '.') {

			snprintf(file, 1024, "%s%s%s", path, "/", entry->d_name);

			// Read the file.
			if ((fd = open(file, O_RDONLY)) < 0) {
				fprintf(stderr, "%s - open error\n", file);
				fflush(stderr);
				closedir(working);
				return;
			}

			// How big is the file?
			if (fstat(fd, &info) != 0) {
				fprintf(stderr, "%s - stat error\n", file);
				fflush(stderr);
				closedir(working);
				close(fd);
				return;
			}

			// Allocate a buffer.
		/*	if (!(buffer = malloc(info.st_size + 1))) {
				fprintf(stderr, "%s - malloc error\n", file);
				fflush(stderr);
				closedir(working);
				close(fd);
				return;
			}

			// Clear the buffer.
			memset(buffer, 0, info.st_size + 1);

			// Read the file.
			if (read(fd, buffer, info.st_size) != info.st_size) {
				fprintf(stderr, "%s - read error\n", file);
				fflush(stderr);
				closedir(working);
				free(buffer);
				close(fd);
				return;
			}

			uint32_t hash_adler32(buffer, info.st_size);

			free(buffer);*/
			close(fd);



			buffer = NULL;
			blen = 0;

			if (!(klen = key(file, keyb, 1024)) || !(buffer = tchdbget(hdb, keyb, klen, &blen))) {
				fprintf(stderr, "%s - retrieve error\n", file);
				fflush(stderr);
			}	else if (lzo1 || lzo999) {
				blen = lzo_decompress((void **)&buffer, blen, info.st_size);
			}	else if (gz) {
				blen = gzip_decompress((void **)&buffer, blen, info.st_size);
			}	else if (bzip) {
				blen = bzip_decompress((void **)&buffer, blen, info.st_size);
			}

			if (buffer && blen != info.st_size) {
				fprintf(stderr, "%s - retrieved data doesn't match the original data\n", file);
				fflush(stderr);
			}

			if (buffer) {
				free(buffer);
			}
		}
	}

	closedir(working);
	return;
}

/**
 * Process the command line arguments.
 *
 * @param argc The number of arguments.
 * @param argv The argument strings.
 */
void args_mason(int argc, char *argv[]) {

	for (int i = 0; i < argc; i++) {
		if (!lzo999 && !lzo1 && !gz && !bzip && !strcasecmp(argv[i], "bzip")) {
			bzip = true;
		} else if (!lzo999 && !lzo1 && !gz && !bzip && !strcasecmp(argv[i], "gzip")) {
			gz = true;
		} else if (!lzo999 && !lzo1 && !gz && !bzip && !strcasecmp(argv[i], "lzo1")) {
			if (!(wrkmem = malloc(LZO1X_1_MEM_COMPRESS))) {
				fprintf(stderr, "Unable to allocate LZO working buffer.\n");
				exit(EXIT_FAILURE);
			}
			lzo1 = true;
			bzero(wrkmem, LZO1X_1_MEM_COMPRESS);
		} else if (!lzo999 && !lzo1 && !gz && !bzip && !strcasecmp(argv[i], "lzo999")) {
			if (!(wrkmem = malloc(LZO1X_999_MEM_COMPRESS))) {
				fprintf(stderr, "Unable to allocate LZO working buffer.\n");
				exit(EXIT_FAILURE);
			}
			lzo999 = true;
			bzero(wrkmem, LZO1X_999_MEM_COMPRESS);
		} else if (!strcasecmp(argv[i], "async")) {
			async = true;
		} else if (!strcasecmp(argv[i], "flush")) {
			flush = true;
		} else if (!strcasecmp(argv[i], "clean")) {
			clean = true;
		}
	}
}

int main(int argc, char *argv[]) {

	TCHDB *hdb;
	time_t start_load, end_load, end_verify;

	args_mason(argc, argv);

	if (clean) {
		unlink(STORAGE_FILE);
	}

	// Create the hash context.
	if (!(hdb = tchdbnew())) {
		fprintf(stderr, "Unable to create a storage context.\n");
		if (lzo1 || lzo999) {
			free(wrkmem);
		}
		exit(EXIT_FAILURE);
	}

	// Set our tuning params
	if (!tchdbtune(hdb, -1, -1, -1, opts)) {
		fprintf(stderr, "An error occurred while tuning the storage context. {tchdbtune = %s}\n", tchdberrmsg(tchdbecode(hdb)));
		if (lzo1 || lzo999) {
			free(wrkmem);
		}
		exit(EXIT_FAILURE);
	}

	// Open an existing file or create a new one.
	if (!tchdbopen(hdb, STORAGE_FILE, HDBOWRITER | HDBOCREAT | HDBOLCKNB)) {
		fprintf(stderr, "An error occurred while opening magma.dat. {tchdbopen = %s}\n", tchdberrmsg(tchdbecode(hdb)));
		if (lzo1 || lzo999) {
			free(wrkmem);
		}
		exit(EXIT_FAILURE);
	}

	// Store some data.
	start_load = time(NULL);
	load(hdb, DATA_PATH);
	end_load = time(NULL);


	// Verify the same data.
	verify(hdb, DATA_PATH);
	end_verify = time(NULL);

	// Flush the file to disk before closing it.
	if (flush && !tchdbsync(hdb)) {
		fprintf(stderr, "An error occurred while flushing the storage context. {tchdbsync = %s}\n", tchdberrmsg(tchdbecode(hdb)));
	}

	// Status data.
	fprintf(stdout, "--------------------------------------------------------------------\n");
	fprintf(stdout, "%9.9s %10.10s %10.10s\n", " ", "Stored", "Original");
	fprintf(stdout, "%9.9s %10lu %10lu\n", "Objects:", tchdbrnum(hdb), total_objects);
	fprintf(stdout, "%9.9s %10lu %10lu  (%3.1f %%)\n", "Size:", tchdbfsiz(hdb), total_bytes, ((float)tchdbfsiz(hdb) / (float)total_bytes) * 100);
	fprintf(stdout, "--------------------------------------------------------------------\n");
	fprintf(stdout, "%9.9s %10.10s %10.10s\n", " ", "Load", "Verify");
	fprintf(stdout, "%9.9s %9lus %9lus\n", "Time:", end_load - start_load, end_verify - end_load);
	fprintf(stdout, "--------------------------------------------------------------------\n");
	fprintf(stdout, "%9.9s %5.5sclean %sflush %sasync\n", "Options:", clean ? "+" : "-", flush ? "+" : "-", async ? "+" : "-");
	fprintf(stdout, "%9.9s %5.5sgzip %sbzip %slzo1 %slzo999\n", "Engine:", gz ? "+" : "-",	bzip ? "+" : "-", lzo1 ? "+" : "-", lzo999 ? "+" : "-");
	fprintf(stdout, "--------------------------------------------------------------------\n");

	// Close the file handle and flush the data buffers.
	if (!tchdbclose(hdb)) {
		fprintf(stderr, "An error occurred while trying to close the magma.dat file. {tchdbclose = %s}\n", tchdberrmsg(tchdbecode(hdb)));
	}

	// Always free the hash context.
	tchdbdel(hdb);

	if (lzo1 || lzo999) {
		free(wrkmem);
	}

	exit(EXIT_SUCCESS);

}
