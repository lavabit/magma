
/**
 * @file /magma/providers/storage/tank.c
 *
 * @brief The storage system interface. Uses Tokyo Cabinet to store the underlying files.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

uint64_t tanks_num = 4;
uint64_t tanks_next = 0;
pthread_mutex_t tanks_lock = PTHREAD_MUTEX_INITIALIZER;

struct {
	uint8_t tuner;
	TCHDB * system;
	TCHDB **tanks;
} store = {
	.system = NULL,
	.tanks = NULL
};

/**
 * @brief	Get the next storage tank.
 * @note	This function cycles through all the storage tanks in order, to ensure homogeneous data distribution.
 * @return	the next storage tank in line.
 */
uint64_t tank_cycle(void) {

	uint64_t res = 0;

	mutex_get_lock(&tanks_lock);
	res = tanks_next++;
	if (tanks_next >= tanks_num) {
		tanks_next = 0;
	}
	mutex_unlock(&tanks_lock);

	return res;
}

/**
 * @brief	Count the number of objects in all local storage tanks.
 * @return	the total number of all objects contained in all tanks.
 */
uint64_t tank_count(void) {

	uint64_t count = 0;

	// Count the storage tank objects.
	for (uint64_t i = 0; i < tanks_num; i++) {
		count += tchdbrnum_d(*(store.tanks + i));
	}

#ifdef MAGMA_PEDANTIC
	if (tchdbrnum_d(store.system) != count) {
		log_pedantic("The number of system records does not match the number of objects in the storage tanks. {system = %lu / objects = %lu}", tchdbrnum_d(store.system), count);
	}
#endif

	return count;
}

/**
 * @brief	Count the amount of space used by all local storage tanks.
 * @return	the total number of bytes occupied by all objects in all storage tanks.
 */
uint64_t tank_size(void) {

	uint64_t size = 0;

	// Sum the storage tank sizes.
	for (uint64_t i = 0; i < tanks_num; i++) {
		size += tchdbfsiz_d(*(store.tanks + i));
	}

	return size;
}

/**
 * Delete a binary object on the file system.
 *
 * @param hnum The host number.
 * @param tnum The tank number.
 * @param unum The user number.
 * @param onum The object number.
 * @return Returns true if the object was deleted without an error.
 */
bool_t tank_delete(uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t onum) {

	TCHDB *ctx;
	int_t key_len;
	char key_buffer[512];
	int64_t transaction, result;

	// Build the retrieval key.
	if ((key_len = snprintf(key_buffer, 512, "object.%lu.%lu.%lu.%lu", hnum, tnum, unum, onum)) < 14) {
		log_error("An error occurred during setup. {object = object.%lu.%lu.%lu.%lu}", hnum, tnum, unum, onum);
		return false;
	}

	// Create a reference to the specific tank context.
	else if (!(ctx = *(store.tanks + tnum))) {
		log_error("Invalid tank number. {object = object.%lu.%lu.%lu.%lu}", hnum, tnum, unum, onum);
		return false;
	}

	// Start a database transaction.
	else if ((transaction = tran_start()) < 0) {
		log_error("Unable to start a storage transaction. The object was not stored on disk.");
		return false;
	}

	// Delete the database reference.
	else if (!tank_delete_object(transaction, hnum, tnum, unum, onum)) {
		log_error("Unable to delete the object from the database.");
		tran_rollback(transaction);
		return false;
	}

	// Remove the object on disk.
	else if (!tchdbout_d(ctx, key_buffer, key_len)) {
		log_error("Unable to load the object off the disk. {tchdbout = %s / object = %.*s}", tchdberrmsg_d(tchdbecode_d(ctx)), key_len, key_buffer);
		tran_rollback(transaction);
		return false;
	}

	// Commit the database transaction. If the transaction results in an error we simply record the error and continue the delete operation.
	else if ((result = tran_commit(transaction))) {
		log_critical("Unable to commit the database delete operation. {commit = %li / object = %.*s}", result, key_len, key_buffer);
	}

	// And finally, to keep everything synchronized, delete the local system record.
	if (!tchdbout_d(store.system, key_buffer, key_len)) {
		log_error("Unable to delete the system record off the disk. {tchdbout = %s / object = %.*s}", tchdberrmsg_d(tchdbecode_d(ctx)), key_len, key_buffer);
	}

	return true;
}

/**
 * Load and decompress the data for the object described by the input parameters.
 *
 * @param hnum The host number.
 * @param tnum The tank number.
 * @param unum The user number.
 * @param onum The object number.
 * @return Returns the object data inside a stringer_t or NULL to indicate an error occurred.
 */
stringer_t * tank_load(uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t onum) {

	TCHDB *ctx;
	void *block;
	record_t record;
	char key_buffer[512];
	int key_len, block_len;
	stringer_t *result = NULL;

	// Build the retrieval key.
	if ((key_len = snprintf(key_buffer, 512, "object.%lu.%lu.%lu.%lu", hnum, tnum, unum, onum)) < 14) {
		log_error("An error occurred during setup. {object = object.%lu.%lu.%lu.%lu}", hnum, tnum, unum, onum);
		return NULL;
	}

	// Create a reference to the specific tank context.
	else if (!(ctx = *(store.tanks + tnum))) {
		log_error("Invalid tank number. {object = object.%lu.%lu.%lu.%lu}", hnum, tnum, unum, onum);
		return NULL;
	}

	else if (!(block = tchdbget_d(ctx, key_buffer, key_len, &block_len))) {
		log_error("Unable to load the object off the disk. {tchdbget = %s / object = %.*s}", tchdberrmsg_d(tchdbecode_d(ctx)), key_len, key_buffer);
		return NULL;
	}

	// Were assuming that the front of the returned buffer contains a record structure.
	else if (block_len < sizeof(record_t)) {
		log_error("The object isn't long enough to contain a valid record heading. {length = %i / object = %.*s}", block_len, key_len, key_buffer);
		tcfree_d(block);
		return NULL;
	}

	// Copy into our structure for easier access.
	mm_copy(&record, block, sizeof(record_t));

	// Check if the record version is supported, and the record length is what we expect.
	if (record.ver != TANK_RECORD_VERSION) {
		log_error("Unrecognized object record version number. {version = %hhu / object = %.*s}", record.ver, key_len, key_buffer);
		tcfree_d(block);
		return NULL;
	} else if (record.rec != sizeof(record_t)) {
		log_error("Invalid record length. {length = %hhu / object = %.*s}", record.rec, key_len, key_buffer);
		tcfree_d(block);
		return NULL;
	}
	// Make sure the amount of data read from disk matches what the object heading indicated should be there.
	else if (((record.flags & (TANK_COMPRESS_LZO | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP)) && record.data.compressed != block_len - sizeof(record_t)) || ((record.flags & (TANK_COMPRESS_LZO | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP)) == 0 && record.data.length
			!= block_len - sizeof(record_t))) {
		log_error("The amount of data read from disk does not match what was indicated by the object header. {expected = %lu / read = %lu / object = %.*s}",
				record.flags & (TANK_COMPRESS_LZO | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP) ? record.data.compressed : record.data.length, block_len - sizeof(record_t), key_len, key_buffer);
		tcfree_d(block);
		return NULL;
	}

#ifdef MAGMA_PEDANTIC

	// Compare the variables used to create the object retrieval with the record heading and flag any discrepancies.
	else if (record.meta.tnum != tnum || record.meta.unum != unum || record.meta.onum != onum) {
		log_check(record.meta.tnum != tnum);
		log_check(record.meta.unum != unum);
		log_check(record.meta.onum != onum);
		log_pedantic("Object header did not match what was expected given the retrieval variables used. {object = %.*s}", key_len, key_buffer);
		tcfree_d(block);
		return NULL;
	}

#endif

	// Were cheating here. Instead of allocating a new buffer and copying the data, we use the memory that held the record heading to store the additional length
	// information needed by a compress_t block.
	if (record.flags & (TANK_COMPRESS_LZO | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP)) {

		if (record.flags & TANK_COMPRESS_LZO) {
			if (!(result = decompress_lzo(block + sizeof(record_t)))) {
				log_error("LZO decompression failed. {object = %.*s}", key_len, key_buffer);
			}
		} else if (record.flags & TANK_COMPRESS_ZLIB) {
			if (!(result = decompress_zlib(block + sizeof(record_t)))) {
				log_error("ZLIB decompression failed. {object = %.*s}", key_len, key_buffer);
			}
		} else if (record.flags & TANK_COMPRESS_BZIP) {
			if (!(result = decompress_bzip(block + sizeof(record_t)))) {
				log_error("BZIP decompression failed. {object = %.*s}", key_len, key_buffer);
			}
		}
	}
	// The data wasn't compressed.
	else if (!(result = st_import(block + sizeof(record_t), block_len - sizeof(record_t)))) {
		log_error("Unable to import the object into a stringer. {object = %.*s}", key_len, key_buffer);
	}

	tcfree_d(block);
	return result;
}

/**
 * @brief	Store a binary object on the file system.
 * @note	Flags include (TANK_COMPRESS_LZO | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP),
 * @param	hnum	the host number.
 * @param	tnum	the tank number.
 * @param	unum	the userid number.
 * @param	data	a managed string (placer) with the data to be stored.
 * @param	flags	a bitmask of flags specifying encryption, compression, and replication.
 * @return 	0 on failure, If the object is stored the object number is returned, or 0 if an error occurs.
 */
uint64_t tank_store(uint64_t hnum, uint64_t tnum, uint64_t unum, stringer_t *data, uint64_t flags) {

	TCHDB *ctx;
	int key_len;
	int64_t transaction;
	char key_buffer[512];
	void *complete = NULL;
	compress_t *compressed = NULL;

	mm_wipe(key_buffer, 512);

	// Build the record template.
	record_t record = {
		.ver = TANK_RECORD_VERSION,
		.rec = sizeof(record_t),
		.flags = flags,
		.meta.tnum = 0,
		.meta.unum = unum,
		.meta.onum = 0,
		.meta.snum = 0,
		.meta.created = time(NULL),
		.data.compressed = 0,
		.data.encrypted = 0,
		.data.length = st_length_get(data)
	};

	// Build the entry template.
	entry_t entry = {
		.ver = TANK_ENTRY_VERSION,
		.meta.tnum = 0,
		.meta.unum = unum,
		.meta.onum = 0,
		.meta.snum = 0,
		.stamps.created = time(NULL),
		.stamps.updated = 0,
		.stamps.deleted = 0,
		.stamps.expiration = 0
	};

	// Check whether the compression flag has been set.
	if (flags & (TANK_COMPRESS_LZO | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP)) {

		// We use a carefully crafted logic tree to ensures only one compression engine can be applied. Additional logic ensures that only one compression engine
		// flag can be written to disk.
		if (flags & TANK_COMPRESS_LZO) {
			compressed = compress_lzo(data);
			record.flags = flags = (flags | TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP) ^ (TANK_COMPRESS_ZLIB | TANK_COMPRESS_BZIP);
		} else if (flags & TANK_COMPRESS_ZLIB) {
			compressed = compress_zlib(data);
			record.flags = flags = (flags | TANK_COMPRESS_LZO | TANK_COMPRESS_BZIP) ^ (TANK_COMPRESS_LZO | TANK_COMPRESS_BZIP);
		} else if (flags & TANK_COMPRESS_BZIP) {
			compressed = compress_bzip(data);
			record.flags = flags = (flags | TANK_COMPRESS_ZLIB | TANK_COMPRESS_LZO) ^ (TANK_COMPRESS_ZLIB | TANK_COMPRESS_LZO);
		}

		// Make sure we got back a valid pointer, and that the buffer and body lengths are not zero.
		if (!compressed || !(record.data.compressed = compress_total_length(compressed)) || !compress_body_length(compressed)) {
			log_error("An error occurred while trying to compress object. The object was not stored on disk.");
			if (compressed) {
				compress_free(compressed);
			}
			return 0;
		}
	}

	// Validate the tank number.
	if ((entry.meta.tnum = record.meta.tnum = tnum) >= tanks_num || !(ctx = *(store.tanks + record.meta.tnum))) {
		log_error("An error occurred while cycling the storage tanks. The object was not stored on disk. {tank = %lu}", tnum);
		if (compressed) {
			compress_free(compressed);
		}
		return 0;
	}

	// Start a database transaction.
	else if ((transaction = tran_start()) < 0) {
		log_error("Unable to start a storage transaction. The object was not stored on disk.");
		if (compressed) {
			compress_free(compressed);
		}
		return 0;
	}

	// Insert the database reference.
	else if (!(entry.meta.onum = (record.meta.onum = tank_insert_object(transaction, hnum, record.meta.tnum, record.meta.unum, st_length_get(data), flags)))) {
		log_error("Unable to obtain an object number. The object was not stored on disk.");
		tran_rollback(transaction);
		if (compressed) {
			compress_free(compressed);
		}
		return 0;
	}

	// Create the object key using the scheme TYPE.HOST.TANK.USER.OBJECT.
	else if ((key_len = snprintf(key_buffer, 512, "object.%lu.%lu.%lu.%lu", hnum, record.meta.tnum, record.meta.unum, record.meta.onum)) < 14) {
		log_error("An error occurred during setup. The object was not be stored on disk. {object = object.%lu.%lu.%lu.%lu}", hnum, record.meta.tnum,
				record.meta.onum, record.meta.unum);
		tran_rollback(transaction);
		if (compressed) {
			compress_free(compressed);
		}
		return 0;
	}

	// Create a buffer to store the object record and data in the same block of memory. The inline conditional should detect if a compressed version was created, and
	// use the size of the compressed data block instead of the original size.
	else if (!(complete = mm_alloc((compressed ? compress_total_length(compressed) : st_length_get(data)) + sizeof(record_t) + 1))) {
		log_error("An error occurred while trying to allocate a buffer for merging the record and object data. The object was not stored on disk. "
				"{length = %zu / object = %.*s}", ((compressed ? compress_total_length(compressed) : st_length_get(data)) + sizeof(record_t)), key_len, key_buffer);
		tran_rollback(transaction);
		if (compressed) {
			compress_free(compressed);
		}
		return 0;
	}

	// Copy the record into the buffer, then the data. The inline conditional should detect if a compressed version should be copied instead of the original data buffer.
	mm_copy(complete, &record, sizeof(record_t));
	mm_copy(complete + sizeof(record_t), compressed ? compressed : st_data_get(data), compressed ? compress_total_length(compressed) : st_length_get(data));

	// Store the object.
	//if (!tchdbputasync_d(ctx, &key_buffer, key_len, complete, (compressed ? compress_total_length(compressed) : pl_get_length(data)) + sizeof(record_t))) {
	if (!tchdbputasync_d(ctx, &key_buffer, key_len, complete, (compressed ? compress_total_length(compressed) : st_length_get(data)) + sizeof(record_t))) {
		log_error("Unable to put the object in storage tank %lu. The object was not stored on disk. {tchdbputasync = %s / object = %.*s}", record.meta.tnum,
		tchdberrmsg_d(tchdbecode_d(ctx)), key_len, key_buffer);
		tran_rollback(transaction);
		if (compressed) {
				compress_free(compressed);
			}
		mm_free(complete);
		return 0;
	}

	// Release the data.
	if (compressed) {
		compress_free(compressed);
	}
	mm_free(complete);

	// Store the entry in the local system database. If this fails, rollback the database transaction and delete the object data from the storage tank.
	if (!tchdbputasync_d(store.system, &key_buffer, key_len, &entry, sizeof(entry_t))) {
		log_error("Unable to create the object entry in the system database. Removing the object from storage tank %lu. {tchdbputasync = %s / object = %.*s}", record.meta.tnum,
				tchdberrmsg_d(tchdbecode_d(store.system)), key_len, key_buffer);
		tran_rollback(transaction);
		tchdbout_d(ctx, key_buffer, key_len);
		return 0;
	}

	// All done, so commit the object transaction to the central database.
	tran_commit(transaction);

	// TODO: now all we need to do is write unit tests; then start tracking tank info in the db; and finally stop auto-creating tank files, instead detect
	// missing files, and/or ask to create new files, we also need to add the crypto layer from lavad

	return record.meta.onum;
}

/**
 * @brief	Perform periodic maintenance on the storage tanks (defragment them in the background).
 * @return	This function returns no value.
 */
void tank_maintain(void) {
	for (uint64_t i = 0; i < tanks_num; i++) {
		// By specifying a negative step value, the defrag operation should be run in the background.
		tchdbdefrag_d(*(store.tanks + i), -1);
	}
	return;
}

/**
 * Open a storage context for the given location. Use the store.tuner parameters to configure the context.
 *
 * @param location The path to the file where the data is (or will be) stored.
 * @return Returns a pointer to the storage context on success and NULL on failure.
 */
TCHDB * tank_open(char *location) {

	TCHDB *ctx = NULL;

	// Create the storage context.
	if (!(ctx = tchdbnew_d())) {
		log_critical("Unable allocate a new storage context. {tchdbnew = NULL}");
		return NULL;
	}
	// The documentation is ambiguous, but in looking at the code, it appears tchdbsetmutex() needs to be called or the storage context isn't thread safe.
	else if (!tchdbsetmutex_d(ctx)) {
		log_critical("An error occurred while tuning the storage context. {tchdbsetmutex = %s}", tchdberrmsg_d(tchdbecode_d(ctx)));
		tchdbdel_d(ctx);
		return NULL;
	}
	// The context needs to be tuned using HDBTLARGE flag, or the data file won't grow past 2 GB. The fpow value of 16 should equal 2^16 = 65536, allowing up to 64 megabytes
	// of free space to remain allocated after shrink operations. The default value is 1 megabyte, or 2^10 = 1024.
	else if (!tchdbtune_d(ctx, -1, -1, 16, HDBTLARGE)) {
		log_critical("An error occurred while tuning the storage context. {tchdbtune = %s}", tchdberrmsg_d(tchdbecode_d(ctx)));
		tchdbdel_d(ctx);
		return NULL;
	}
	// Open the file if it already exists otherwise create a new one. Request an exclusive file lock from the operating system. Do not block if the file is locked by
	// another process, instead return an error.
	else if (!tchdbopen_d(ctx, location, HDBOREADER | HDBOWRITER | HDBOCREAT | HDBOLCKNB)) {
		log_critical("An error occurred while trying to open %s. {tchdbopen = %s}", location, tchdberrmsg_d(tchdbecode_d(ctx)));
		tchdbdel_d(ctx);
		return NULL;
	}

	return ctx;
}

/**
 * Cleanly closes a storage file context and releases the memory.
 *
 * @param ctx The storage context that needs to be closed.
 */
void tank_close(TCHDB *ctx) {
	// Flush any remaining operations to the physical device.
	if (!tchdbsync_d(ctx)) {
		log_error("An error occurred while flushing the storage context. {tchdbsync = %s}", tchdberrmsg_d(tchdbecode_d(ctx)));
	}
	// Optimize the file, which includes the task of freeing any unused data blocks.
	if (!tchdboptimize_d(ctx, -1, -1, 2, HDBTLARGE)) {
		log_error("An error occurred while trying to optimize the %s file. {tchdboptimize = %s}", tchdbpath_d(ctx), tchdberrmsg_d(tchdbecode_d(ctx)));
	}
	// Defrag the file, which should also release any unused data blocks. Specifying a step of -1 triggers a complete defrag without locking the file.
	if (!tchdbdefrag_d(ctx, -1)) {
		log_error("An error occurred while trying to defrag the %s file. {tchdbdefrag = %s}", tchdbpath_d(ctx), tchdberrmsg_d(tchdbecode_d(ctx)));
	}
	// The file handle must be closed cleanly or the underlying storage tank may be corrupted.
	if (!tchdbclose_d(ctx)) {
		log_error("An error occurred while trying to close the %s file. {tchdbclose = %s}", tchdbpath_d(ctx), tchdberrmsg_d(tchdbecode_d(ctx)));
	}
	// Always free the hash context.
	tchdbdel_d(ctx);

	return;
}

/**
 * @brief	Initialize the local tank storage system.
 *
 * @return Returns true if all of the storage tanks were setup successfully and the system is ready to store data.
 */
bool_t tank_start(void) {

	size_t expected;
	char location[MAGMA_FILEPATH_MAX + 1];

	// Calculate out how long the path should be.
	expected = ns_length_get(magma.storage.tank) + 11 + (*(magma.storage.tank + ns_length_get(magma.storage.tank)) == '/' ? 0 : 1);
	// Open the storage control file.
	if (snprintf(location, MAGMA_FILEPATH_MAX + 1, "%s%ssystem.data", magma.storage.tank, *(magma.storage.tank + ns_length_get(magma.storage.tank)) == '/' ? "" : "/") != expected || !(store.system = tank_open(location))) {
		log_critical("Storage system startup failed.");
		return false;
	}
	// Allocate the array of storage handles.
	if (!(store.tanks = mm_alloc(sizeof(TCHDB *) * tanks_num))) {
		log_critical("Unable to allocate the array of storage handles. {size = %zu}", sizeof(TCHDB *) * tanks_num);
		return false;
	}
	// Open the storage tanks.
	for (uint64_t i = 0; i < tanks_num; i++) {

		// Calculate out the length of the storage tank location string.
		expected = ns_length_get(magma.storage.tank) + 11 + (*(magma.storage.tank + ns_length_get(magma.storage.tank)) == '/' ? 0 : 1);
		// Open the storage control file.
		if (snprintf(location, MAGMA_FILEPATH_MAX + 1, "%s%stank.%lu.data", magma.storage.tank, *(magma.storage.tank + ns_length_get(magma.storage.tank)) == '/' ? "" : "/", i + 1) < expected || !(*(store.tanks + i) = tank_open(location))) {
			log_critical("Storage system startup failed.");
			return false;
		}
	}

	return true;
}

/**
 * Close all of the storage system file handles and release the resources they were using.
 */
void tank_stop(void) {

	for (uint64_t i = 0; i < tanks_num; i++) {
		tank_close(*(store.tanks + i));
	}

	tank_close(store.system);

	mm_free(store.tanks);
	store.tanks = NULL;
	store.system = NULL;

	return;
}

