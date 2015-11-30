/**
 * @file /stringer/stringer.h
 *
 * @brief The functions used to manipulate stringers.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#ifndef MAGMA_STRINGER_H
#define MAGMA_STRINGER_H

// LOW: Replace malloc, memset, memcpy, pthread_mutex_lock/pthread_mutex_unlock functions with custom variants.
// TODO: Create global temp file function that is location configurable.
// TODO: Since the page size doesn't change, we could ideally store that value in a global variable instead of retrieving it each time.
// TODO: Use global statistics interface for mm_sec.
// TODO: Add hooks for setting the mm_sec slab size via the config file.
// Token
// Remove
// Search
// Replace
// Trim
// Parsers/encoders = numbers, base64, base32, quoted printable, ip addresses

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>
#include <regex.h>
#include <search.h>
#include <semaphore.h>
#include <sys/mman.h>

typedef bool bool_t;
typedef int32_t int_t;
typedef char chr_t;
typedef unsigned char uchr_t;

extern FILE *bucket;
#define log_print(...) do { fflush(stdout); fflush(stderr); fprintf (stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while(0)
#define log_pedantic(...) do {  fflush(stdout); fflush(stderr); bucket ? fprintf(bucket, __VA_ARGS__) : fprintf(stderr, __VA_ARGS__); bucket ? fprintf(bucket, "\n") : fprintf(stderr, "\n"); } while(0)

enum {

	// Type
	CONSTANT_T = 1,
	PLACER_T = 2,
	NULLER_T = 4,
	BLOCK_T = 8,
	MANAGED_T = 16,
	MAPPED_T = 32,

	// Layout
	CONTIGUOUS = 64,
	JOINTED = 128,

	// Memory
	STACK = 256,
	HEAP = 512,
	SECURE = 1024
};

typedef struct {
	uint32_t opts;
	char *data;
} nuller_t;

typedef struct {
	uint32_t opts;
	size_t length;
	void *data;
} block_t;

typedef struct {
	uint32_t opts;
	size_t length;
	void *data;
} placer_t;

typedef struct {
	uint32_t opts;
	size_t length;
	size_t avail;
	void *data;
} managed_t;

typedef struct {
	uint32_t opts;
	size_t length;
	size_t avail;
	int handle;
	void *data;
} mapped_t;

typedef struct {
	uint32_t opts;
} stringer_t;

// For debug only
void mm_sec_dump_stats(void);
uint32_t bits_count(uint64_t value);
bool_t st_opts_valid(uint32_t opts);

// Secure Memory Blocks
void mm_sec_stop(void);
bool_t mm_sec_start(void);
void mm_sec_free(void *a);
bool_t mm_sec_secured(void *a);
void * mm_sec_alloc(size_t len);
void * mm_sec_realloc(void *orig, size_t new_length);

// Memory
void mm_free(void *block);
void * mm_alloc(size_t len);
void mm_wipe(void *block, size_t len);
bool_t mm_empty(void *block, size_t len);
void * mm_copy(void *dst, void *src, size_t len);
void * mm_move(void *dst, void *src, size_t len);
void mm_set(void *block, uchr_t set, size_t len);

// Null Strings
bool_t ns_empty(chr_t *s);
void ns_free(char *string);
int ns_length_int(chr_t *s);
chr_t * ns_alloc(size_t len);
chr_t * ns_dupe(char *s);
size_t ns_length_get(chr_t *s);
void ns_wipe(char *s, size_t len);
chr_t * ns_import(void *block, size_t len);
bool_t ns_empty_out(chr_t *s, chr_t **ptr, size_t *len) __attribute__((nonnull (2, 3)));

// Character Capitalization
uchr_t c_lowercase(uchr_t c);
uchr_t c_uppercase(uchr_t c);
void st_lowercase(stringer_t *s);
void st_uppercase(stringer_t *s);
void mm_lowercase(void *mem, size_t len);
void mm_uppercase(void *mem, size_t len);

// Options to String
const chr_t * st_info_type(uint32_t opts);
const chr_t * st_info_layout(uint32_t opts);
const chr_t * st_info_allocator(uint32_t opts);
chr_t * st_info_opts(uint32_t opts, chr_t *s, size_t len);

// Option Checks
bool_t st_opts_free(uint32_t opts);
bool_t st_opts_avail(uint32_t opts);
bool_t st_opts_placer(uint32_t opts);
bool_t st_opts_jointed(uint32_t opts);
bool_t st_opts_tracked(uint32_t opts);
bool_t st_opts_destination(uint32_t opts);

// Length
int st_length_int(stringer_t *s);
size_t st_avail_get(stringer_t *s);
size_t st_length_get(stringer_t *s);
size_t st_avail_set(stringer_t *s, size_t avail);
size_t st_length_set(stringer_t *s, size_t len);

// Data
bool_t st_empty(stringer_t *s);
void * st_data_get(stringer_t *s);
chr_t * st_char_get(stringer_t *s);
void st_data_set(stringer_t *s, void *data);
bool_t st_empty_out(stringer_t *s, uchr_t **ptr, size_t *len) __attribute__((nonnull (2, 3)));

// Creation/Destruction
void st_free(stringer_t *s);
stringer_t * st_dupe(stringer_t *s);
stringer_t * st_import(void *s, size_t len);
stringer_t * st_alloc(uint32_t opts, size_t len);
stringer_t * st_realloc(stringer_t *s, size_t len);
stringer_t * st_print(uint32_t opts, chr_t *format, ...);
stringer_t * st_merge(uint32_t opts, chr_t *format, ...);

// Placer Shortcuts
stringer_t * st_placer_set(stringer_t *s, void *data, size_t len);
stringer_t * st_placer_init(stringer_t *s, void *data, size_t len);

// Comparison
int_t st_cmp_cs_eq(stringer_t *a, stringer_t *b);
int_t st_cmp_ci_eq(stringer_t *a, stringer_t *b);
int_t st_cmp_cs_ends(stringer_t *s, stringer_t *ends);
int_t st_cmp_ci_ends(stringer_t *s, stringer_t *ends);
int_t st_cmp_cs_starts(stringer_t *s, stringer_t *starts);
int_t st_cmp_ci_starts(stringer_t *s, stringer_t *starts);

// Usage: char *constant = CONSTANT("Hello world.");
#define CONSTANT(string) ((stringer_t *) "\x41\x01\x00\x00" string)

// Usage: placer_t place = PLACER(data, length);
//#define PLACER(d, l) { .opts = (PLACER_T | JOINTED | STACK), .data = d, .length = l }
#define PLACER(d, l) (stringer_t *)&((placer_t){ .opts = (PLACER_T | JOINTED | STACK), .data = d, .length = l })
#endif

