
/**
 * @file /magma/providers/storage/storage.h
 *
 * @brief The Tokyo Cabinet interface, which is primarily used for memory and disk based storage.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PROVIDE_TOKYO_PRIVATE_H
#define MAGMA_PROVIDE_TOKYO_PRIVATE_H

// TODO: Add support for loading/storing data on multiple nodes in a cluster.
// TODO: Finish integrating cryptex (aka ECIES) support into the storage tank.
// TODO: Integrate storage provider with the configuration and statistics code.
// LOW: Validate that the code is making proper use of the updated string code.
// LOW: Update/validate header file name spaces... preferably with the assistance of a script.


#define TANK_ENTRY_VERSION 100
#define TANK_RECORD_VERSION 100

enum {
	TANK_COMPRESS_LZO = 1,
	TANK_COMPRESS_ZLIB = 2,
	TANK_COMPRESS_BZIP = 4
} TANK_FLAGS_E;

typedef struct {

	uint8_t ver; /*!< Number indicating the entry version, which also tells us the layout of the data. */
	uint8_t rec; /*!< The length of the record data. */
	uint64_t flags; /*!< A collection of bit mask flags to indicate whether the object was compressed, encrypted, or replicated.  */

	struct {
		uint64_t tnum; /*!< Which local storage tank was used to store the object. */
		uint64_t unum; /*!< The user number of the object owner. */
		uint64_t onum; /*!< The object number. */
		uint64_t snum; /*!< The serial number. Starts at zero, and increments for each update. */
		uint64_t created; /*!< Time stamp taken when the object is stored. */
	} meta;

	struct {
		uint64_t length; /*!< The full length of the original data. */
		uint64_t compressed; /*!< The compressed length of the data, if applicable. */
		uint64_t encrypted; /*!< The length of the encrypted data block, if applicable. */
	} data;

} __attribute__ ((packed)) record_t;

typedef struct {

	uint8_t ver; /*!< Number indicating the entry version, which also tells us the layout of the data. */

	struct {
			uint64_t tnum; /*!< Which local storage tank was used to store the object. */
			uint64_t unum; /*!< The user number of the object owner. */
			uint64_t onum; /*!< The object number. */
			uint64_t snum; /*!< The serial number. Starts at zero, and increments for each update. */
			uint64_t stamp; /*!< Time stamp taken when the object is stored. */
		} meta;

	struct {
			uint64_t created; /*!< When the object was first created. */
			uint64_t updated; /*!< When the object was last updated. */
			uint64_t deleted; /*!< When the object was flagged for deletion. */
			uint64_t expiration; /*!< When archived/deleted objects can be permanently purged. */
		} stamps;

} __attribute__ ((packed)) entry_t;


bool_t lib_load_tokyo(void);
const chr_t * lib_version_tokyo(void);

//! Binary trees.
uint64_t tree_count(void *inx);
inx_t * tree_alloc(uint64_t options, void *data_free);

//! Startup and shutdown.
void tank_stop(void);
bool_t tank_start(void);

//! Info functions.
uint64_t tank_size(void);
uint64_t tank_count(void);
uint64_t tank_cycle(void);

//! Maintenance
void tank_maintain(void);

//! Object handling.
bool_t tank_delete(uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t onum);
stringer_t * tank_load(uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t onum);
uint64_t tank_store(uint64_t hnum, uint64_t tnum, uint64_t unum, stringer_t *data, uint64_t flags);

// Storage Tank
bool_t tank_delete_object(int64_t transaction, uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t onum);
uint64_t tank_insert_object(int64_t transaction, uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t size, uint64_t flags);

#endif

