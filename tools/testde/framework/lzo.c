
#include "framework.h"

extern void *lavalib;
char * (*lzo_version_string_d)(void) = NULL;
int (*__lzo_init_v2_d)(unsigned,int,int,int,int,int,int,int,int,int) = NULL;
lzo_uint32 (*lzo_adler32_d)(lzo_uint32 _adler, const lzo_bytep _buf, lzo_uint _len) = NULL;
int (*lzo1x_1_compress_d)(const lzo_byte *src, lzo_uint  src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem) = NULL;
int (*lzo1x_decompress_safe_d)(const lzo_byte *src, lzo_uint  src_len, lzo_byte *dst, lzo_uintp dst_len, lzo_voidp wrkmem) = NULL;

uint32_t hash_adler32(void *buffer, size_t length) {

	size_t input;
	uint32_t a = 1, b = 0;

	while (length > 0) {

		// Every 5550 octets we need to modulo.
		input = length > 5550 ? 5550 : length;
		length -= input;

		do {
			a += *(char *)buffer++;
			b += a;
		} while (--input);

		a = (a & 0xffff) + (a >> 16) * (65536 - 65521);
      b = (b & 0xffff) + (b >> 16) * (65536 - 65521);
	}

	// If a is greater than the mod number, modulo.
	if (a >= 65521) {
		a -= 65521;
	}

	b = (b & 0xffff) + (b >> 16) * (65536 - 65521);
	if (b >= 65521) {
		b -= 65521;
	}

	return (b << 16) | a;
}

int load_symbols_lzo(void) {

	if (lavalib == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The lava library pointer was NULL.");
		#endif
		return 0;
	}

	lzo_version_string_d = dlsym(lavalib, "lzo_version_string");
	if (lzo_version_string_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function lzo_version_string.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	__lzo_init_v2_d = dlsym(lavalib, "__lzo_init_v2");
	if (__lzo_init_v2_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function __lzo_init_v2.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	lzo_adler32_d = dlsym(lavalib, "lzo_adler32");
	if (lzo_adler32_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function lzo_adler32.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	lzo1x_1_compress_d = dlsym(lavalib, "lzo1x_1_compress");
	if (lzo1x_1_compress_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function lzo1x_1_compress.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	lzo1x_decompress_safe_d = dlsym(lavalib, "lzo1x_decompress_safe");
	if (lzo1x_decompress_safe_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function lzo1x_decompress_safe.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	#ifdef DEBUG_FRAMEWORK
	lavalog("The LZO library symbols have been loaded.");
	#endif

	return 1;
}

int initialize_lzo(void) {

	if (__lzo_init_v2_d(LZO_VERSION,(int)sizeof(short),(int)sizeof(int),\
    (int)sizeof(long),(int)sizeof(lzo_uint32),(int)sizeof(lzo_uint),\
    (int)lzo_sizeof_dict_t,(int)sizeof(char *),(int)sizeof(lzo_voidp),\
    (int)sizeof(lzo_callback_t)) != LZO_E_OK) {
		 #ifdef DEBUG_FRAMEWORK
		lavalog("Could not initialize the LZO library.");
		#endif
		return 0;
	}

	#ifdef DEBUG_FRAMEWORK
	lavalog("LZO library initialized.");
	#endif

	return 1;
}

char * version_lzo(void) {
	return lzo_version_string_d();
}

reducer_t * allocate_rt(sizer_t size) {

	reducer_t *result;

	// No zero length strings.
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to allocate a zero length reducer.");
		#endif
		return NULL;
	}

	// Do the allocation. Include room for two sizer_ts plus a terminating NULL.
	result = malloc(sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint) + sizeof(lzo_uint32) + sizeof(lzo_uint32) + size + 1);

	// If memory was allocated clear.
	if (result != NULL) {
		clear_bl(result, sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint) + sizeof(lzo_uint32) + sizeof(lzo_uint32) + size + 1);
		*(sizer_t *)result = size;
	}

	// If no memory was allocated, discover that here.
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Could not allocate %u bytes.", sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint) + sizeof(lzo_uint32) +
			sizeof(lzo_uint32) + size + 1);
	}
	#endif

	return result;
}

inline void free_rt(reducer_t *buffer) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
	}
	else {
		free(buffer);
	}

	return;
}

inline sizer_t size_rt(reducer_t *buffer) {

	sizer_t size;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	size = *(sizer_t *)buffer;
	return size;
}

inline sizer_t used_rt(reducer_t *buffer) {

	sizer_t size;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	size = *(sizer_t *)(buffer + sizeof(sizer_t));
	return size;
}

inline lzo_uint in_size_rt(reducer_t *buffer) {

	lzo_uint size;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	size = *(lzo_uint *)(buffer + sizeof(sizer_t) + sizeof(sizer_t));
	return size;
}

inline lzo_uint32 in_check_rt(reducer_t *buffer) {

	lzo_uint32 check;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	check = *(lzo_uint32 *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint));
	return check;
}

inline lzo_uint out_size_rt(reducer_t *buffer) {

	lzo_uint size;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	size = *(lzo_uint *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint32));
	return size;
}

inline lzo_uint32 out_check_rt(reducer_t *buffer) {

	lzo_uint32 check;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	check = *(lzo_uint32 *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint32) + sizeof(lzo_uint));
	return check;
}

inline void set_used_rt(reducer_t *buffer, sizer_t used) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}

	#ifdef DEBUG_FRAMEWORK
	if (used > size_rt(buffer)) {
		lavalog("Trying to set the used variable to %u when the allocated size is %u.", used, size_rt(buffer));
	}
	#endif

	*(sizer_t *)(buffer + sizeof(sizer_t)) = used;

	return;
}

inline void set_in_size_rt(reducer_t *buffer, lzo_uint size) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}

	*(lzo_uint *)(buffer + sizeof(sizer_t) + sizeof(sizer_t)) = size;
	return;
}

inline void set_in_check_rt(reducer_t *buffer, lzo_uint32 check) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}

	*(lzo_uint32 *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint)) = check;
	return;
}

inline void set_out_size_rt(reducer_t *buffer, lzo_uint size) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}

	*(lzo_uint *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint32)) = size;
	return;
}

inline void set_out_check_rt(reducer_t *buffer, lzo_uint32 check) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}

	*(lzo_uint32 *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint32) + sizeof(lzo_uint)) = check;
	return;
}

inline unsigned char * data_rt(reducer_t *buffer) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}

	return (char *)(buffer + sizeof(sizer_t) + sizeof(sizer_t) + sizeof(lzo_uint) + sizeof(lzo_uint) + sizeof(lzo_uint32) + sizeof(lzo_uint32));
}

inline unsigned char * data_buf_rt(reducer_t *buffer) {

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}

	return (char *)(buffer + sizeof(sizer_t) + sizeof(sizer_t));
}


inline sizer_t size_buf_rt(reducer_t *buffer) {

	sizer_t size;

	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	size = *(sizer_t *)(buffer + sizeof(sizer_t));
	size += sizeof(lzo_uint) + sizeof(lzo_uint) + sizeof(lzo_uint32) + sizeof(lzo_uint32);
	return size;
}

reducer_t * compress_lzo(stringer_t *input) {

	int state;
	lzo_uint in_len;
	lzo_uint out_len;
	lzo_bytep wrkmem;
	stringer_t *result = NULL;

	in_len = used_st(input);
	if (in_len == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to compress a string of zero length.");
		#endif
		return NULL;
	}

	wrkmem = allocate_bl(LZO1X_1_MEM_COMPRESS);
	if (wrkmem == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes for a compression buffer.", LZO1X_1_MEM_COMPRESS);
		#endif
		return NULL;
	}


	result = allocate_rt((in_len + in_len / 16 + 64 + 3));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %li bytes for holding the compressed data.", in_len + in_len / 16 + 64 + 3);
		#endif
		free_bl(wrkmem);
		return NULL;
	}

	state = lzo1x_1_compress_d(data_st(input), in_len, data_rt(result), &out_len, wrkmem);
	free_rt(wrkmem);

	if (state != LZO_E_OK) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to compress the buffer.");
		#endif
		free_rt(result);
		return NULL;
	}

	set_used_rt(result, out_len);
	set_in_size_rt(result, in_len);
	set_out_size_rt(result, out_len);
	set_in_check_rt(result, lzo_adler32_d(0, data_st(input), in_len));
	set_out_check_rt(result, lzo_adler32_d(0, data_rt(result), out_len));
	/*
	lavalog("We compressed %lu bytes into %lu bytes. We allocated %lu bytes for the output. We used a buffer "
		"of %i bytes. And we got back checksums of %u on the input %u and on the output.", in_len, out_len,  in_len + in_len / 16 + 64 + 3,
		LZO1X_1_MEM_COMPRESS, lzo_adler32_d(0, data_st(input), in_len), lzo_adler32_d(0, data_rt(result), out_len));
	*/
	return result;
}

stringer_t * decompress_lzo(compress_t *compressed) {
	int ret;
	void *bptr;
	uint64_t hash, rlen, blen;
	stringer_t *result = NULL;
	compress_head_t *head;

	if (!(head = (compress_head_t *)compressed)) {
		lavalog("Invalid compression header. {compress_head = NULL}");
		return NULL;
	}
	else if (head->engine != 1) {
		lavalog("The buffer passed in was not compressed using the LZO engine. {engine = %hhu}", head->engine);
		return NULL;
	}
	else if (!(bptr = (compressed + sizeof(compress_head_t))) || !(blen = head->length.compressed) || !(rlen = head->length.original) ||
		head->hash.compressed != (hash = hash_adler32(bptr, blen))) {
		lavalog("The compressed has been corrupted. {expected = %lu / input = %lu}", head->hash.compressed, hash);
		return NULL;
	}
	else if (!(result = allocate_st(rlen *= 4))) {
		lavalog("Could not allocate a block of %lu bytes for the uncompressed data.", head->length.original);
		return NULL;
	}
	else if ((ret = lzo1x_decompress_safe_d(bptr, blen, data_st(result), &rlen, NULL)) != LZO_E_OK) {
		lavalog("Unable to decompress the buffer. {lzo1x_decompress_safe = %i}", ret);
		free_st(result);
		return NULL;
	}
	else if (head->length.original != rlen || head->hash.original != (hash = hash_adler32(data_st(result), rlen))) {
		lavalog("The uncompressed data is corrupted. {input = %lu != %lu / hash = %lu != %lu}", head->length.original, rlen, head->hash.original, hash);
		free_st(result);
		return NULL;
	}

	set_used_st(result, rlen);
	return result;
}
