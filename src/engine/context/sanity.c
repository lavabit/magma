
/**
 * @file /magma/engine/context/sanity.c
 *
 * @brief	A collection of checks performed at launch to make sure the system will operate as expected.
 */
#include "magma.h"

/**
 * @brief	Perform a series of system-wide sanity checks at process startup.
 * @note	This function ensures that the standard data types are their expected data sizes, and performs other
 * 			checks, such as data primitive maximum values.
 * @return	true if the system environment passed all checks and should function correctly, or false if any checks failed.
 */
bool_t sanity_check(void) {

	stringer_t *st;
	size_t sz[2] = { 0, 0 };
	int64_t i64[2] = { 0, 0 };
	ssize_t ssz[2] = { 0, 0 };
	uint64_t ui64[2] = { 0, 0 }, endian = 1;

	// Let's make sure this is a little endian system, or various shift, and/or byte swap logic scattered throughout
	// won't work as intended.
	if (*((unsigned char *)&endian) != 1) {
		return false;
	}

	// Make sure the standard types are what we expect.
	if (sizeof(char) != 1 || sizeof(short) != 2 || sizeof(int) != 4 || sizeof(long) != 8) {
		return false;
	}

	// Check on the standard integer sizes in use.
	if (sizeof(int8_t) != 1 || sizeof(int16_t) != 2 || sizeof(int32_t) != 4 || sizeof(int64_t) != 8) {
		return false;
	}

	if (sizeof(uint8_t) != 1 || sizeof(uint16_t) != 2 || sizeof(uint32_t) != 4 || sizeof(uint64_t) != 8) {
		return false;
	}

	// Check the system specific typedefs we use.
	if (sizeof(bool_t) != 1 || sizeof(time_t) != 8 || sizeof(size_t) != 8) {
		return false;
	}

	// Check on the different types of pointers.
	if (sizeof(char *) != 8 || sizeof(void *) != 8) {
		return false;
	}

	// Ensure the type aliases are properly sized.
	if (sizeof(byte_t) != 1 || sizeof(uchr_t) != 1 || sizeof(chr_t) != 1) {
		return false;
	}

	if (sizeof(float_t) != 4 || sizeof(double_t) != 8) {
		return false;
	}

	// That strings are properly formed and sized.
	if (sizeof(stringer_t) != 1 || sizeof(stringer_t *) != 8) {
		return false;
	}

	// That IP addresses are properly formed and sized.
	if (sizeof(octet_t) != 2 || sizeof(segment_t) != 4 || sizeof(struct in_addr) != 4 || sizeof(struct in6_addr) != 16) {
			return false;
		}


	// A placer_t (aka a placeholder) should be 20 bytes, which should be culmlative length of a size_t, a pointer, and an
	// unsigned 32 bit integer.
	if (sizeof(placer_t) != 20 || sizeof(placer_t) != (sizeof(size_t) + sizeof(void *) + sizeof(uint32_t))) {
		return false;
	}

	// Store the minimum and maximum values using constants stored in header files at compilation.
	sz[0] = SIZE_MAX + 1;
	sz[1] = SIZE_MAX;
	ssz[0] = (SIZE_MAX / 2) + 1;
	ssz[1] = (SIZE_MAX / 2);
	i64[0] = INT64_MIN;
	i64[1] = INT64_MAX;
	ui64[0] = UINT64_MAX + 1;
	ui64[1] = UINT64_MAX;

	// The functions used to convert character strings into size_t and ssize_t are aliases for uint64_t and int64_t.
	// Aliases are also used by the serialization functions. The following test uses the minimum and maximum values
	// calculated above to ensure the aliases we are using remain accurate.
	if (sizeof(size_t) != sizeof(uint64_t) || sz[0] != ui64[0] || sz[1] != ui64[1] ||	sizeof(ssize_t) != sizeof(int64_t) || ssz[0] != i64[0] || ssz[1] != i64[1] ||
			sz[1] != SIZE_MAX || i64[0] != INT64_MIN || i64[1] != INT64_MAX || ui64[1] != UINT64_MAX) {
		return false;
	}

	// We are using a semaphore to block access to our connection pools,so the internal max must be lower than the
	// system limit.
	if (MAGMA_CORE_POOL_OBJECTS_LIMIT > SEM_VALUE_MAX) {
		return false;
	}

	// The pool counters are unsigned 32 bit integers, so we also make sure the max is below that limit.
	if (MAGMA_CORE_POOL_OBJECTS_LIMIT > UINT32_MAX || MAGMA_CORE_POOL_TIMEOUT_LIMIT > UINT32_MAX) {
		return false;
	}

	// This tuple must evaluate correctly or the comparison functions will not function correctly.
	if (((100 < 200) ? -1 : 100 > 200) != -1 || ((100 < 100) ? -1 : 100 > 100) != 0 || ((200 < 100) ? -1 : 200 > 100) != 1) {
		return false;
	}

	// The tank interface uses the memory that initially holds a the object heading to store the two size_t variables
	// need for a compress_t block. If the length of two size_t variables is greater than the length of of a record_t,
	// then the math being used to write data out would be susceptible to a buffer underflow. To ensure we never forget
	// this we check to ensure this system doesn't break the type assumptions.
	if ((sizeof(size_t) + sizeof(size_t)) > sizeof(record_t)) {
	 return false;
	}

	// We also need to make sure the stringer_t format doesn't change in a way that would break the implementation.
	if (!(st = st_alloc(1))) {
		return false;
	}
	else if ((size_t)((st_data_get(st) + st_avail_get(st)) - (void *)(st)) !=
		(sizeof(uint32_t) + sizeof(size_t) + sizeof(size_t) + sizeof(void *) + 1)) {
		st_free(st);
		return false;
	}
	else {
		st_free(st);
	}

	// We assume the select_domains query is at the beginning of our structure holding all of the prepared statements,
	// but this code will confirm that assumption. If the order is ever changed this code will need to be updated
	// or the prepared statement pointers won't correlate with the correct SQL query. This order based calculation is used
	// by the functions stmt_start(), stmt_rebuild(), and stmt_stop().
	if (st_cmp_cs_eq(NULLER(queries[0]), CONSTANT(SELECT_DOMAINS))) {
		return false;
	}

	return true;
}

