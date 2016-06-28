
/**
 * @file /check/providers/virus_check.c
 *
 * @brief Check the anti-virus provider.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

chr_t * check_virus_sthread(chr_t *location) {

	int fd;
	DIR *working;
	chr_t *err = NULL;
	struct stat info;
	struct dirent *entry;
	char file[1024], *buffer;

	if (!(working = opendir(location))) {
		return "Unable to open the data path";
	}

	while (status() && (entry = readdir(working))) {

		// Reset.
		errno = 0;
		bzero(file, 1024);
		bzero(&info, sizeof(struct stat));

		// Build an absolute path.
		snprintf(file, 1024, "%s%s%s", location, "/", entry->d_name);

		// If we hit a directory, recursively call the load function.
		if (entry->d_type == DT_DIR && *(entry->d_name) != '.') {
			if ((err = check_virus_sthread(file))) {
				return err;
			}
		}
		// Otherwise if its a regular file try storing it.
		else if (entry->d_type == DT_REG && *(entry->d_name) != '.') {

			// Read the file.
			if ((fd = open(file, O_RDONLY)) < 0) {
				log_info("%s - open error", file);
				closedir(working);
				return "open error";
			}

			// How big is the file?
			if (fstat(fd, &info) != 0) {
				log_info("%s - stat error", file);
				closedir(working);
				close(fd);
				return "stat error";
			}

			// Allocate a buffer.
			if (!(buffer = mm_alloc(info.st_size + 1))) {
				log_info("%s - malloc error", file);
				closedir(working);
				close(fd);
				return "malloc error";
			}

			// Clear the buffer.
			memset(buffer, 0, info.st_size + 1);

			// Read the file.
			if (read(fd, buffer, info.st_size) != info.st_size) {
				log_info("%s - read error", file);
				closedir(working);
				mm_free(buffer);
				close(fd);
				return "read error";
			}

			close(fd);

			if (virus_check(PLACER(buffer, info.st_size)) == -1) {
				log_info("%s - virus check error", file);
				closedir(working);
				mm_free(buffer);
				close(fd);
				return "virus check error";
			}

			mm_free(buffer);
			close(fd);
		}
	}

	closedir(working);
	return NULL;
}
