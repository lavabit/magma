// This program was created to test the dispatchd


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

static const char * const hostname = "localhost";
static const int port = 7000;

int replace_ns_ns_ns(char **target, ssize_t target_length, const char *pattern, const char *replacement) {

	char *holder;
	char *new_holder;
	unsigned hits = 0;
	ssize_t increment;
	ssize_t increment_internal;
	ssize_t pattern_length;
	ssize_t replacement_length;
	ssize_t new_length;
	char *new_target;

	if (!target || !*target || !pattern || !replacement) {
		//printf("Sanity check failed. Passed a NULL pointer.");
		return target_length;
	}

	// Setup our lengths.
	pattern_length = strlen(pattern);
	replacement_length = strlen(replacement);

	// If one of the passed in strings is empty, we can't replace anything.
	if (pattern_length == 0 || target_length == 0 || replacement_length == 0) {
		//printf("Either the search pattern or the target were empty.");
		return target_length;
	}

	// Check to make sure the target is big enough to hold the pattern.
	if (target_length < pattern_length) {
		//printf("The target isn't long enough to contain the pattern.");
		return target_length;
	}

	// Setup.
	holder = *target;

	// Increment through the entire target and find out how many times the pattern is present.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (strncmp(holder++, pattern, pattern_length) == 0) {
			hits++;
			increment += pattern_length - 1;
			holder += pattern_length - 1;
		}
	}

	// Did we get any hits?
	if (hits == 0) {
		//printf("Searched the target and did not find the pattern.");
		return target_length;
	}

	// Calculate out the new length.
	new_length = target_length - (pattern_length * hits) + (replacement_length * hits);

	// Allocate a new stringer.
	new_target = malloc(new_length + 1);
	if (new_target == NULL) {
		//printf("Could not allocate %u bytes for the new string.", new_length);
		return target_length;
	}

	memset(new_target, 0, new_length + 1);

	// Setup.
	holder = *target;
	new_holder = new_target;

	// Increment through the entire target and copy the bytes.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (strncmp(holder, pattern, pattern_length) == 0) {
			increment += pattern_length - 1;
			holder += pattern_length;
			for (increment_internal = 0; increment_internal < replacement_length; increment_internal++) {
				*new_holder++ = *(replacement + increment_internal);
			}
		} else {
			*new_holder++ = *holder++;
		}
	}

	// Copy the remaining bits.
	for (; increment < target_length; increment++) {
		*new_holder++ = *holder++;
	}

	// Free the old stringer and set the new target.
	free(*target);
	*target = new_target;

	return new_length;
}

void send_message(char *entrypath, FILE *buf_d) {

	int fd;
	int new;
	char *buffer;
	char input[1000];
	struct stat info;

	fgets(input, 1000, buf_d);
	if (*input != '2') {
		printf("%s", input);
		return;
	}
	memset(input, 0, 1000);

	fprintf(buf_d, "HELO jami\r\n");
	fflush(buf_d);
	fgets(input, 1000, buf_d);
	if (*input != '2') {
		printf("%s", input);
		return;
	}
	memset(input, 0, 1000);

	fprintf(buf_d, "MAIL FROM: <kingradar@yahoo.com>\r\n");
	fflush(buf_d);
	fgets(input, 1000, buf_d);
	if (*input != '2') {
		printf("%s", input);
		return;
	}
	memset(input, 0, 1000);

	fprintf(buf_d, "RCPT TO: <magma@lavabit.com>\r\n");
	fflush(buf_d);
	fgets(input, 1000, buf_d);
	if (*input != '2') {
		printf("%s", input);
		return;
	}
	memset(input, 0, 1000);

	fprintf(buf_d, "DATA\r\n");
	fflush(buf_d);
	fgets(input, 1000, buf_d);
	if (*input != '3') {
		printf("%s", input);
		return;
	}
	memset(input, 0, 1000);

	// Read the file.
	if ((fd = open(entrypath, O_RDONLY)) < 0) {
		printf(" - open error\n");
		return;
	}

	if (fstat(fd, &info) != 0) {
		printf(" - fstat error\n");
		close(fd);
		return;
	}

	if ((buffer = malloc(info.st_size + 1)) == NULL) {
		printf(" - malloc error\n");
		close(fd);
		return;
	}

	memset(buffer, 0, info.st_size + 1);

	if (read(fd, buffer, info.st_size) != info.st_size) {
		printf(" - read error\n");
		free(buffer);
		close(fd);
		return;
	}

	close(fd);

	// Dot stuff.
	new = replace_ns_ns_ns(&buffer, info.st_size, "\n.", "\n..");

	// Send the file.
	fprintf(buf_d, "%.*s", new, buffer);
	free(buffer);
	printf(" - %i bytes - ", (int)info.st_size);

	// End the transmission.
	fprintf(buf_d, "\r\n.\r\n");
	fflush(buf_d);

	fgets(input, 1000, buf_d);
	printf("%s", input);
	fflush(stdout);
	memset(input, 0, 1000);

	fprintf(buf_d, "QUIT\r\n");
	fflush(buf_d);
	return;
}

void process_directory(char *path) {

	int socket_descriptor;
	struct sockaddr_in pin;
	struct hostent *server_host_name;
	struct linger linger_timeout;
	DIR *wd;
	struct dirent *entry;
	char entrypath[5000];
	FILE *buf_d;
	server_host_name = gethostbyname(hostname);

	if ((wd = opendir(path)) == NULL) {
		printf("\n\nUnable to open the directory. {%s}\n\n", path);
		exit(1);
	}

	while ((entry = readdir(wd)) != NULL) {

		// Reset the error condition.
		errno = 0;

		if (*(entry->d_name) != '.' && entry->d_type == 8) {

			bzero(entrypath, 1000);

			snprintf(entrypath, 5000, "%s%s%s", path, "/", entry->d_name);
			printf("File : %s ", entrypath);

			bzero(&pin, sizeof(pin));
			pin.sin_family = AF_INET;
			pin.sin_addr.s_addr = htonl(INADDR_ANY);
			pin.sin_addr.s_addr = ((struct in_addr *) (server_host_name->h_addr))->s_addr;
			pin.sin_port = htons(port);

			if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				perror("Error opening socket.\n");
				exit(1);
			}

			if ((connect(socket_descriptor, (void *) &pin, sizeof(pin))) == -1) {
				perror("Error connecting to socket.\n");
				exit(1);
			}

			// Setup a low linger timeout so we don't hang around forever.
			linger_timeout.l_onoff = 1;
			linger_timeout.l_linger = 1;
			if (setsockopt(socket_descriptor, SOL_SOCKET, SO_LINGER, &linger_timeout, sizeof(linger_timeout)) != 0) {
				printf("Error while setting main socket timeout.\n");
				return;
			}

			buf_d = fdopen(socket_descriptor, "a+");
			errno = 0;

			send_message(entrypath, buf_d);

			fclose(buf_d);
			fflush(stdout);

			if (errno != 0) {
				perror("random error -");
			}
		}
		else if (*(entry->d_name) != '.' && entry->d_type == 4) {

			bzero(entrypath, 1000);

			snprintf(entrypath, 5000, "%s%s%s", path, "/", entry->d_name);
			printf("\n\nDirectory : %s\n\n", entrypath);
			process_directory(entrypath);

		}
	}

	closedir(wd);

	return;
}

int main(int argc, char *argv[]) {

	int socket_descriptor;
	struct sockaddr_in pin;
	struct hostent *server_host_name;
	struct linger linger_timeout;
	FILE *buf_d;

	server_host_name = gethostbyname(hostname);

	if (argc == 1) {
		process_directory(".");

	} else {

		printf("File : %s ", argv[1]);

		bzero(&pin, sizeof(pin));
		pin.sin_family = AF_INET;
		pin.sin_addr.s_addr = htonl(INADDR_ANY);
		pin.sin_addr.s_addr = ((struct in_addr *) (server_host_name->h_addr))->s_addr;
		pin.sin_port = htons(port);

		if ((socket_descriptor = socket(AF_INET,SOCK_STREAM, 0)) == -1) {
			perror("Error opening socket.\n");
			exit(1);
		}

		if ((connect(socket_descriptor, (void *) &pin, sizeof(pin))) == -1) {
			perror("Error connecting to socket.\n");
			exit(1);
		}

		// Setup a low linger timeout so we don't hang around forever.
		linger_timeout.l_onoff = 1;
		linger_timeout.l_linger = 1;
		if (setsockopt(socket_descriptor, SOL_SOCKET, SO_LINGER,
				&linger_timeout, sizeof(linger_timeout)) != 0) {
			printf("Error while setting main socket timeout.\n");
			return 1;
		}

		buf_d = fdopen(socket_descriptor, "a+");
		errno = 0;

		send_message(argv[1], buf_d);

		fclose(buf_d);
		fflush(stdout);

		if (errno != 0) {
			perror("random error -");
		}

	}

	printf("All finished.\n\n");
	return 0;
}

