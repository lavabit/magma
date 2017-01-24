#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/procfs.h>

int main(int argc, char** argv) {

	int fd = -1;
	Elf64_Ehdr *header = NULL;

	if (argc < 2) {
		printf("\nUsage: %s <magmad path>\n\n", argv[0]);
		return 1;
	}

	printf("\nAttempting to lock down the magma daemon binaries.\n\n");

	// Open the executable file.
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		printf("File access error. { errno = %i / path = %s }\n", errno, argv[1]);
		return 1;
	}

	// Mmap the file, using MAP_SHARED so we can update the file.
	if ((header = (Elf64_Ehdr *)mmap(NULL, sizeof(header), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		printf("File header mapping failed. { errno = %i / path = %s }\n", errno, argv[1]);
		close(fd);
		return 1;
	}

	printf("------------ CURRENT FILE HEADER ------------\n");
	printf("    %-10.10s %30.*x\n", "e_shoff", sizeof(header->e_shoff), header->e_shoff);
	printf("    %-10.10s %30.*x\n", "e_shnum", sizeof(header->e_shnum), header->e_shnum);
	printf("    %-10.10s %30.*x\n", "e_shstrndx", sizeof(header->e_shstrndx), header->e_shstrndx);

	memset(&(header->e_shoff), 0xFF, sizeof(header->e_shoff));
	memset(&(header->e_shnum), 0xFF, sizeof(header->e_shnum));
	memset(&(header->e_shstrndx), 0xFF, sizeof(header->e_shstrndx));

	printf("------------ PATCHED FILE HEADER ------------\n");
	printf("    %-10.10s %30.*x\n", "e_shoff", sizeof(header->e_shoff), header->e_shoff);
	printf("    %-10.10s %30.*x\n", "e_shnum", sizeof(header->e_shnum), header->e_shnum);
	printf("    %-10.10s %30.*x\n", "e_shstrndx", sizeof(header->e_shstrndx), header->e_shstrndx);

	// Sync the result to disk.
	if (msync(NULL, 0, MS_SYNC) == -1) {
		printf("File syncing failed. { errno = %i / path = %s }\n", errno, argv[1]);
		close(fd);
		return 1;
	}

	munmap(header, 0);
	close(fd);

	printf("------------- LOCKDOWN COMPLETE -------------\n\n");
	printf("The magma daemon binary should be protected from debuggers.\n\n", argv[1]);

	return 0;
}
