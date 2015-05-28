
/**
 * @file /magma/core/strings/strings.h
 *
 * @brief	Function declarations and types used by the different modules involved with handling stringers and null terminated strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_STRINGS_H
#define MAGMA_CORE_STRINGS_H

// HIGH: We need to add string search functions.
// HIGH: Add number (and object?) support to the string+block type so we can eliminate the multi_t type.

enum {

	// Type
	CONSTANT_T = 1,				// An immutable string
	PLACER_T = 2,				/* Placeholder stores no data on its own and MUST be jointed;
	 	 	 	 	 	 	 	   it points to a chunk of another managed string */
	NULLER_T = 4,				// Null-terminated string
	BLOCK_T = 8,				// A binary blob
	MANAGED_T = 16,				// A vanilla managed string for wrapping string data
	MAPPED_T = 32,				// The managed string is allocated with mmap()

	// Layout
	CONTIGUOUS = 64,			/* The header and string (aka data) buffer are adjacent */
	JOINTED = 128,				/* The header and string (aka data) buffer are separate allocations. */

	// Memory
	STACK = 256,				// More properly, data is not on the heap (stack or static initialization)
	HEAP = 512,
	SECURE = 1024,				// Must be on the heap

	// Flags
	FOREIGNDATA = 4096			// Do not free data upon deallocation - this is somebody else's job!

	// If you add any new flags, make sure you update the info.c arrays!
};

typedef struct __attribute__ ((packed)) {
	uint32_t opts;
	char data[];
} constant_t;

typedef struct __attribute__ ((packed)) {
	uint32_t opts;
	char *data;
} nuller_t;

typedef struct __attribute__ ((packed)) {
	uint32_t opts;
	size_t length;
	void *data;
} block_t;

typedef struct __attribute__ ((packed)) {
	uint32_t opts;
	size_t length;
	void *data;
} placer_t;

typedef struct __attribute__ ((packed)) {
	uint32_t opts;
	size_t length;
	size_t avail;
	void *data;
} managed_t;

typedef struct __attribute__ ((packed)) {
	uint32_t opts;
	size_t length;
	size_t avail;
	int handle;
	void *data;
} mapped_t;

typedef void stringer_t;

/// nuller.c
chr_t *  ns_alloc(size_t len);
chr_t *  ns_append(chr_t *s, chr_t *append);
chr_t *  ns_dupe(chr_t *s);
bool_t   ns_empty(chr_t *s);
bool_t   ns_empty_out(chr_t *s, chr_t **ptr, size_t *len);
void     ns_free(chr_t *s);
void     ns_cleanup(chr_t *s);
chr_t *  ns_import(void *block, size_t len);
size_t   ns_length_get(const chr_t *s);
int      ns_length_int(chr_t *s);
void     ns_wipe(chr_t *s, size_t len);

// Options to String
const chr_t * st_info_type(uint32_t opts);
const chr_t * st_info_layout(uint32_t opts);
const chr_t * st_info_allocator(uint32_t opts);
chr_t * st_info_opts(uint32_t opts, chr_t *s, size_t len);

// Option Checks
bool_t st_valid_free(uint32_t opts);
bool_t st_valid_opts(uint32_t opts);
bool_t st_valid_avail(uint32_t opts);
bool_t st_valid_append(uint32_t opts);
bool_t st_valid_placer(uint32_t opts);
bool_t st_valid_jointed(uint32_t opts);
bool_t st_valid_tracked(uint32_t opts);
bool_t st_valid_destination(uint32_t opts);

// Length
int_t st_length_int(stringer_t *s);
size_t st_avail_get(stringer_t *s);
size_t st_length_get(stringer_t *s);
size_t st_avail_set(stringer_t *s, size_t avail);
size_t st_length_set(stringer_t *s, size_t len);

/// data.c
chr_t *   st_char_get(stringer_t *s);
uchr_t *  st_uchar_get(stringer_t *s);
void *    st_data_get(stringer_t *s);
void      st_data_set(stringer_t *s, void *data);
bool_t    st_empty(stringer_t *s);
bool_t    st_empty_out(stringer_t *s, uchr_t **ptr, size_t *len) __attribute__((nonnull (2, 3)));
void      st_wipe(stringer_t *s);

// Creation/Destruction
void st_free(stringer_t *s);
void st_cleanup(stringer_t *s);
//stringer_t * st_alloc(size_t len);
stringer_t * st_dupe(stringer_t *s);
//stringer_t * st_merge(chr_t *format, ...);
//stringer_t * st_aprint(chr_t *format, va_list list);
stringer_t * st_import(const void *s, size_t len);
stringer_t * st_copy_in(stringer_t *s, void *buf, size_t len);
stringer_t * st_realloc(stringer_t *s, size_t len);
stringer_t * st_output(stringer_t *output, size_t len);
stringer_t * st_nullify(chr_t *input, size_t len);

// Allocation with Options
stringer_t * st_alloc_opts(uint32_t opts, size_t len);
stringer_t * st_dupe_opts(uint32_t opts, stringer_t *s);
stringer_t * st_merge_opts(uint32_t opts, chr_t *format, ...);
stringer_t * st_append_opts(size_t align, stringer_t *s, stringer_t *append);

/// shortcuts.c
chr_t *    pl_char_get(placer_t place);
void *     pl_data_get(placer_t place);
bool_t     pl_empty(placer_t place);
placer_t   pl_init(void *data, size_t len);
placer_t   pl_clone(placer_t place);
size_t     pl_length_get(placer_t place);
int_t      pl_length_int(placer_t place);
placer_t   pl_null(void);
placer_t   pl_set(placer_t place, placer_t set);
bool_t     pl_starts_with_char(placer_t place, chr_t c);
bool_t     pl_inc(placer_t *place, bool_t more);

/// opts.c
bool_t   st_opt_get(stringer_t *s, uint32_t opt);
int_t    st_opt_set(stringer_t *s, uint32_t opt, bool_t enabled);

/// print.c
stringer_t * st_aprint(chr_t *format, ...) __attribute__((format (printf, 1, 2)));
stringer_t * st_aprint_opts(uint32_t opts, chr_t *format, ...) __attribute__((format (printf, 2, 3)));
stringer_t * st_quick(stringer_t *s, chr_t *format, ...) __attribute__((format (printf, 2, 3)));
size_t st_sprint(stringer_t *s, chr_t *format, ...) __attribute__((format (printf, 2, 3)));
stringer_t * st_vaprint_opts(uint32_t opts, chr_t *format, va_list args);
size_t st_vsprint(stringer_t *s, chr_t *format, va_list args);

/// replace.c
int_t         st_replace(stringer_t **target, stringer_t *pattern, stringer_t *replacement);
stringer_t *  st_swap(stringer_t *target, uchr_t pattern, uchr_t replacement);

// Shortcut Macros
#define st_append(s, append) st_append_opts(1024, s, append)
#define st_alloc(len) st_alloc_opts(MANAGED_T | CONTIGUOUS | HEAP, len)
#define st_merge(...) st_merge_opts(MANAGED_T | CONTIGUOUS | HEAP, __VA_ARGS__)
#define st_vaprint(format, args) st_vaprint_opts(MANAGED_T | CONTIGUOUS | HEAP, format, args)

// Usage: constant_t *constant = CONSTANT("Hello world.");
#define CONSTANT(string) (stringer_t *)((constant_t *)"\x41\x01\x00\x00" string)

// Usage: nuller_t *nuller = NULLER(data);
#define NULLER(d) (stringer_t *)&((nuller_t){ .opts = (NULLER_T | JOINTED | STACK | FOREIGNDATA), .data = d })

// Usage: block_t *block = BLOCK(data, length);
#define BLOCK(d, l) (stringer_t *)&((block_t){ .opts = (BLOCK_T | JOINTED | STACK | FOREIGNDATA), .data = d, .length = l })

// Usage: placer_t *placer = PLACER(data, length);
#define PLACER(d, l) (stringer_t *)&((placer_t){ .opts = (PLACER_T | JOINTED | STACK | FOREIGNDATA), .data = d, .length = l })

// Usage: managed_t *managed = MANAGED(data, length, avail);
#define MANAGED(d, l, a) (stringer_t *)&((managed_t){ .opts = (MANAGED_T | JOINTED | STACK | FOREIGNDATA), .data = d, .length = l, .avail = a })

// Usage: block_t *buffer = BLOCKBUF(length);
#define BLOCKBUF(l) (stringer_t *)&((block_t){ .opts = (BLOCK_T | CONTIGUOUS | STACK), .data = &((chr_t []){ [ 0 ... l ] = 0 }), .length = l })

// Usage: managed_t *buffer = MANAGEDBUF(length);
#define MANAGEDBUF(l) (stringer_t *)&((managed_t){ .opts = (MANAGED_T | CONTIGUOUS | STACK), .data = &((chr_t []){ [ 0 ... l ] = 0 }), .length = 0, .avail = l })

/************ TYPES ************/
typedef struct {
	M_TYPE type;
	union {
		bool_t binary;
		void *bl;
		char *ns;
		stringer_t *st;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		float fl;
		double dbl;
	} val;
} multi_t;
/************ TYPES ************/

/// multi.c
int32_t    cmp_mt_mt(multi_t one, multi_t two);
bool_t     ident_mt_mt(multi_t one, multi_t two);
multi_t    mt_dupe(multi_t multi);
void       mt_free(multi_t multi);
char *     mt_get_char(multi_t *multi);
size_t     mt_get_length(multi_t multi);
multi_t    mt_get_null(void);
uint64_t   mt_get_number(multi_t multi);
M_TYPE     mt_get_type(multi_t multi);
bool_t     mt_is_empty(multi_t multi);
bool_t     mt_is_number(multi_t multi);
multi_t    mt_set_type(multi_t multi, M_TYPE target);


#endif
