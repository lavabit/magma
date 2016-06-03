
/**
 * @file /magma/providers/compress/compress.c
 *
 * @brief	Interface to the compression functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the compression engine's (LZO1X-1) block size.
 * @note	In the future this might get stored in the compression header and/or change depending on the engine being used.
 * @return	the size, in bytes, of the compression engine's block size.
 */
size_t compress_block_length(void) {

	return LZO1X_MEM_DECOMPRESS;
}

/**
 * @brief	Return the total length of a compressed header and body.
 * @return	the total length, in bytes, of the compressed header and body.
 */
uint64_t compress_total_length(compress_t *buffer) {

	return compress_body_length(buffer) + sizeof(compress_head_t);
}

/**
 * @brief	Get the hash value of a compressed buffer's original data.
 * @param	buffer	a pointer to the head of the compressed data.
 * @return	the hash value of the original (uncompressed) data.
 */
uint64_t compress_orig_hash(compress_t *buffer) {

	compress_head_t *head = (compress_head_t *)buffer;

	return head->hash.original;
}

/**
 * @brief	Get the original length of a compressed buffer's data.
 * @param	buffer	a pointer to the head of the compressed data.
 * @return	the original length of the (uncompressed) data.
 */
uint64_t compress_orig_length(compress_t *buffer) {

	compress_head_t *head = (compress_head_t *)buffer;

	return head->length.original;
}

/**
 * @brief	Get the hash value of a compressed buffer's (compressed) body.
 * @param	buffer	a pointer to the header of the compressed data.
 * @return	the hash value of the (compressed) of the compressed data.
 */
uint64_t compress_body_hash(compress_t *buffer) {

	compress_head_t *head = (compress_head_t *)buffer;

	return head->hash.compressed;
}

/**
 * @brief	Return the length of a compressed buffer's body.
 * @return	the total length in bytes of the compressed data body, excluding the compressed header.
 */
uint64_t compress_body_length(compress_t *buffer) {

	compress_head_t *head = (compress_head_t *)buffer;

	return head->length.compressed;
}

/**
 * @brief	Return the compressed body of data associated with a compressed header.
 * @param	buffer	the input compressed header.
 * @return	a pointer to the body (start) of the compressed data.
 */
void * compress_body_data(compress_t *buffer) {

	return buffer + sizeof(compress_head_t);
}

/**
 * @brief	Get the offset to the compressed body from the compressed header.
 * @param	the size, in bytes, of the offset from the start of the compressed data body from the start of the compressed header.
 */
size_t compress_body_offset(void) {

	return sizeof(compress_head_t);
}

/**
 * @brief	Parse and validate a managed string with compressed data and return a compressed header.
 * @param	s	the managed string containing the compressed data.
 * @return	NULL on general failure or if the Adler 32 hash doesn't match, or a pointer to the data's compressed header on success.
 */
compress_t * compress_import(stringer_t *s) {

	void *bptr;
	uint64_t blen;
	compress_t *buffer;
	compress_head_t *head;
#ifdef MAGMA_PEDANTIC
	uint64_t hash = 0;
#endif

	if (st_empty(s) || st_length_get(s) < sizeof(compress_t)) {
		log_pedantic("The provided string is not large enough to hold compressed data. {s = %zu / min = %zu}", st_empty(s) ? 0 : st_length_get(s), sizeof(compress_t));
		return NULL;
	}
	else if (!(buffer = ((compress_t *)st_data_get(s))) || !(head = ((compress_head_t *)buffer)) || !(bptr = compress_body_data(buffer))) {
		log_pedantic("The string appears to be corrupted because NULL data pointers were generated.");
		return NULL;
	}
	else if (!(blen = head->length.compressed) || st_length_get(s) != (blen + sizeof(compress_head_t))) {
		log_pedantic("The provided string length doesn't contain the amount of data indicated by the compression header. {length = %zu / expected = %zu + %lu}",
				st_length_get(s), sizeof(compress_head_t), blen);
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	// This step should be unnecessary in production since all of the decompression functions should be validating the compressed data buffer hashes as well.
	else if (head->hash.compressed != (hash = hash_adler32(bptr, blen))) {
		log_pedantic("The compressed data buffer appears to be corrupted. {hash = %lu / expected = %lu}", hash, head->hash.compressed);
		return NULL;
	}
#endif

	return buffer;
}

/**
 * @brief	Allocate a new compressed object.
 * @param	length	the size, in bytes, of the data after compression.
 * @return	a pointer to the head of the newly allocated compressed object.
 */
compress_t * compress_alloc(size_t length) {

	return mm_alloc(sizeof(compress_head_t) + length);
}

/**
 * @brief	Free a compressed object.
 * @param	buffer	a pointer to the head of the compressed object to be freed.
 * @return	This function returns no value.
 */
void compress_free(compress_t *buffer) {

	mm_free(buffer);
	return;
}
