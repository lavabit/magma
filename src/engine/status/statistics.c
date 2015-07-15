
/**
 * @file /magma/engine/status/statistics.c
 *
 * @brief	A collection of functions used to track and access system statistics.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

struct {
	size_t count;
	pthread_mutex_t locks[128][2];
	uint64_t values[128];
	char *names[128];
} stats = {
		.names = {
			"default",

			// Core Statistics
			"core.threading.workers",

			// SMTP Statistics
			"smtp.connections.total",
			"smtp.connections.secure",

			// DMTP Statistics
			"dmtp.connections.total",
			"dmtp.connections.secure",

			// HTTP Statistics
			"http.connections.total",
			"http.connections.secure",

			// IMAP Statistics
			"imap.connections.total",
			"imap.connections.secure",

			// POP Statistics
			"pop.connections.total",
			"pop.connections.secure",

			// Molten Statistics
			"molten.connections.total",
			"molten.connections.secure",

			// Provider Statistics
			"provider.virus.available",
			"provider.virus.error",
			"provider.virus.scan.total",
			"provider.virus.scan.clean",
			"provider.virus.scan.infected",
			"provider.virus.scan.phishing",
			"provider.virus.signatures.total",
			"provider.virus.signatures.loaded",

			"provider.spf.checked",
			"provider.spf.missing",
			"provider.spf.neutral",
			"provider.spf.error",
			"provider.spf.fail",
			"provider.spf.pass",

			"provider.dkim.signed",
			"provider.dkim.checked",
			"provider.dkim.missing",
			"provider.dkim.neutral",
			"provider.dkim.error",
			"provider.dkim.fail",
			"provider.dkim.pass",


			// Objects
			"objects.users.total",
			"objects.users.expired",
			"objects.sessions.total",
			"objects.sessions.expired",

			// Patterns
			"objects.patterns.checked",
			"objects.patterns.error",
			"objects.patterns.fail",
			"objects.patterns.pass",

			// Web Applications
			"web.register.blocked",

			// TODO: Add stubs for derived statistics like uptime, CPU, memory and secure memory stats.
			// system.pid
			// system.time
			// system.uptime
			// system.load (1, 5, 15, or all?)
			// system.cpu.total
			// system.cpu.users
			// system.cpu.system
			// system.mem.peak
			// system.mem.size
			// system.mem.locked
			// system.mem.resident
			// system.mem.data
			// system.mem.stack
			// system.mem.executable
			// system.mem.libraries
			// system.mem.PTE? HWM?
			// system.mem.swap
			// system.heap.total
			// system.heap.allocated
			// system.heap.items
			// system.secure.total
			// system.secure.allocated
			// system.secure.items
			// system.handles.total
			// system.handles.pipe
			// system.handles.files
			// system.handles.sockets
			// network...
		}
};

// If the position of an entry changes, you must update all of the relevant switch statements.
char *derived[] = {

	// System Statistics
	"system.secure.total",
	"system.secure.allocated",
	"system.secure.items",

	// Error Statistics
	"core.spool.errors",
	"errors.total"
};

/**
 * @brief	Get the total sum of all error statistics maintained by magma (traditional and derived stats).
 * @note	Error statistics are all statistics that have a name that begins with "errors." or ends with ".errors".
 * @return	The total sum of all error statistics counters accumulated by magma.
 */
uint64_t stats_sum_errors(void) {

	bool_t match;
	uint64_t result = 0;

	// LOW: Because this function can easily trigger a recursion death spiral it should get a unit test.
	// LOW: We haven't added a stringer "search" function so we can't match names that include ".errors." -- but we also don't have any names like that right now.

	// Scan the tracked values.
	for (uint64_t i = 0; i < stats_get_count(); i++) {

		// Determine whether the entry starts or ends with the magic string "errors".
		if (!st_cmp_cs_starts(NULLER(stats_get_name(i)), CONSTANT("errors.")) || !st_cmp_cs_ends(NULLER(stats_get_name(i)), CONSTANT(".errors")))	{
			match = true;
		}
		else {
			match = false;
		}

		// Add any matches to our result.
		if (match) result += stats_get_value_by_num(i);
	}

	// Add the derived values to the sum.
	for (uint64_t i = 0; i < derived_count(); i++) {

		// Match any derived error names that are not "errors.total".
		if ((!st_cmp_cs_starts(NULLER(derived_name(i)), CONSTANT("errors.")) || !st_cmp_cs_ends(NULLER(derived_name(i)), CONSTANT(".errors"))) &&
				st_cmp_cs_eq(NULLER(derived_name(i)), CONSTANT("errors.total"))) {
			match = true;
		}
		else {
			match = false;
		}

		// Add any matches to our result. If we ever accidently try to include the derived error total we'll trigger recursion death spiral.
		if (match) result += derived_value(i);
	}

	return result;
}

/**
 * @brief	Get the number of derived statistics that are tracked.
 * @return	the number of derived statistics being maintained by magma.
 */
uint64_t derived_count(void) {

	return sizeof(derived) / sizeof(char *);
}

/**
 * @brief	Get the name of a derived statistic by index.
 * @param	position	the zero-based index of the derived statistic to be queried.
 * @return	NULL on failure, or a pointer to a null-terminated string containing the name of the derived statistic on success.
 */
char * derived_name(uint64_t position) {

	if (position >= derived_count()) {
		return NULL;
	}

	return derived[position];
}

/**
 * @brief	Get the value of a derived statistic by index.
 * @see		mm_sec_stats()
 * @param	position	the zero-based index of the derived statistic to be queried.
 * @return	0 on failure, or the value of the specified derived statistic on success.
 */
uint64_t derived_value(uint64_t position) {

	uint64_t result = 0;
	size_t total, bytes, items;

	switch (position) {

	// Secure subsystem statistics
	case (0):
		if (mm_sec_stats(&total, &bytes, &items)) result = total;
		break;
	case (1):
		if (mm_sec_stats(&total, &bytes, &items)) result = bytes;
		break;
	case (2):
		if (mm_sec_stats(&total, &bytes, &items)) result = items;
		break;

	// Spool errors
	case (3):
		result = spool_error_stats();
		break;

	// Total all of the error counts.
	case (4):
		result = stats_sum_errors();
		break;

	default:
		log_pedantic("We don't know how to calculate the derived value requested! {position = %lu}", position);
		break;
	}

	return result;
}

// LOW: There should be some index validation for functions that reference statistics by index.

/**
 * @brief	Get the index of a statistic by name.
 * @param	name	the name of the statistic to be queried.
 * @return	0 on failure, or the zero-based index of the requested statistic on success.
 */
uint64_t stats_get_name_pos(char *name) {

	bool_t match = false;

	for (uint64_t i = 0; i < stats.count; i++) {
		mutex_get_lock(&stats.locks[i][0]);
		match = !st_cmp_cs_eq(NULLER(name), NULLER(stats.names[i]));
		mutex_unlock(&stats.locks[i][0]);
		if (match) return i;
	}

	log_info("Could not find the statistic requested. {name = %s}", name);

	return 0;
}

/**
 * @brief	Get the name of a statistic by its index.
 * @param	position	the zero-based index of the statistic to be queried.
 * @return	0 on failure, or the name of the requested statistic on success.
 */
char * stats_get_name(uint64_t position) {

	char *name;

	mutex_get_lock(&stats.locks[position][0]);
	name = stats.names[position];
	mutex_unlock(&stats.locks[position][0]);

	return name;
}

/**
 * @brief	Provided a statistic by name, set its value.
 * @param	name	a null-terminated string containing the name of the statistic to be set.
 * @param	value	the new value of the specified statistic.
 * @return	This function returns no value.
 */
void stats_set_by_name(char *name, uint64_t value) {

	uint64_t position;

	if (!(position = stats_get_name_pos(name))) {
		return;
	}

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position] = value;
	mutex_unlock(&stats.locks[position][1]);
	return;
}

/**
 * @brief	Provided a statistic by index, set its value.
 * @param	position	the zero-based index of the statistic to be set.
 * @param	value		the new value of the specified statistic.
 * @return	This function returns no value.
 */
void stats_set_by_num(uint64_t position, uint64_t value) {

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position] = value;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Provided a statistic by name, get its value.
 * @param	name	a null-terminated string containing the name of the statistic to be queried.
 * @return	the specified statistic's value, as an unsigned 64 bit integer.
 */
uint64_t stats_get_value_by_name(char *name) {

	uint64_t position, value;

	if (!(position = stats_get_name_pos(name))) {
			return 0;
	}

	mutex_get_lock(&stats.locks[position][1]);
	value = stats.values[position];
	mutex_unlock(&stats.locks[position][1]);

	return value;
}

/**
 * @brief	Provided a statistic by index, get its value.
 * @param	position	the zero-based index of the statistic to be queried.
 * @return	the specified statistic's value, as an unsigned 64 bit integer.
 */
uint64_t stats_get_value_by_num(uint64_t position) {

	uint64_t value;

	mutex_get_lock(&stats.locks[position][1]);
	value = stats.values[position];
	mutex_unlock(&stats.locks[position][1]);

	return value;
}

/**
 * @brief	Provided a statistic by name, increment its value by a specified amount.
 * @param	name	a null-terminated string containing the name of the statistic to be set.
 * @param	value	the value by which to increment the specified statistic.
 * @return	This function returns no value.
 */
void stats_adjust_by_name(char *name, int32_t value) {

	uint64_t position;

	if (!(position = stats_get_name_pos(name))) {
		return;
	}

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position] += value;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Provided a statistic by index, increment its value by a specified amount.
 * @param	position	the zero-based index of the statistic to be set.
 * @param	value		the value by which to increment the specified statistic.
 * @return	This function returns no value.
 */
void stats_adjust_by_num(uint64_t position, int32_t value) {

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position] += value;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Provided a statistic by name, increment its value by 1.
 * @param	name	a null-terminated string containing the name of the statistic to be incremented.
 * @return	This function returns no value.
 */
void stats_increment_by_name(char *name) {

	uint64_t position;

	if (!(position = stats_get_name_pos(name))) {
		return;
	}

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position]++;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Provided a statistic by index, increment its value by 1.
 * @param	position	the zero-based index of the statistic to be incremented.
 * @return	This function returns no value.
 */
void stats_increment_by_num(uint64_t position) {

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position]++;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Provided a statistic by name, decrement its value by 1.
 * @param	name	a null-terminated string containing the name of the statistic to be decremented.
 * @return	This function returns no value.
 */
void stats_decrement_by_name(char *name) {

	uint64_t position;

	if (!(position = stats_get_name_pos(name))) {
		return;
	}

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position]--;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Provided a statistic by index, decrement its value by 1.
 * @param	position	the zero-based index of the statistic to be decremented.
 * @return	This function returns no value.
 */
void stats_decrement_by_num(uint64_t position) {

	mutex_get_lock(&stats.locks[position][1]);
	stats.values[position]--;
	mutex_unlock(&stats.locks[position][1]);

	return;
}

/**
 * @brief	Get the number of statistics being tracked.
 * @return	the total number of statistics counters maintained by magma.
 */
uint64_t stats_get_count(void) {

	return stats.count;
}

/**
 * @brief	Initialize and reset all statistics counters.
 * @return	false on failure or true on success.
 */
bool_t stats_init(void) {

	mm_wipe(stats.locks, sizeof(stats.locks));
	mm_wipe(stats.values, sizeof(stats.values));

	for (uint64_t i = 0; i < sizeof(stats.names) / sizeof(char *); i++) {
		if (stats.names[i]) stats.count++;
	}

	for (uint64_t i = 0; i < stats.count; i++) {
		if (mutex_init(&stats.locks[i][0], NULL) || mutex_init(&stats.locks[i][1], NULL)) {
			log_critical("Could not initialize the statistic locks.");
			return false;
		}
	}

	return true;
}

/**
 * @brief	Destroy all statistics locks.
 * @return	This function returns no value.
 */
void stats_shutdown(void) {

	for (uint64_t i = 0; i < stats.count; i++) {
		mutex_destroy(&stats.locks[i][0]);
		mutex_destroy(&stats.locks[i][1]);
	}

	return;
}
