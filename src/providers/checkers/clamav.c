
/**
 * @file /magma/providers/checkers/clamav.c
 *
 * @brief	Interface to the ClamAV library.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * The virus engine spool directory.
 */
stringer_t *virus_spool = NULL;

/**
 * The status of the signatures directory.
 */
struct cl_stat virus_stat;

/**
 * The number of signatures loaded by the virus engine.
 */
unsigned int virus_sigs = 0;

/**
 * The virus engine context pointer.
 */
struct cl_engine *virus_engine = NULL;

/**
 * The virus engine read/write lock.
 */
pthread_rwlock_t virus_lock =	PTHREAD_RWLOCK_INITIALIZER;

/**
 * @brief	Get the number of virus signatures loaded by the ClamAV engine context.
 * @return	the number of virus signatures loaded by the ClamAV engine context.
 */
uint64_t virus_sigs_loaded(void) {

	uint64_t loaded = 0;

	pthread_rwlock_rdlock(&virus_lock);
	loaded = virus_sigs;
	pthread_rwlock_unlock(&virus_lock);

	return loaded;
}

/**
 * @brief	Get the number of official signatures available inside the ClamAV signature directory.
 * @see		magma.iface.virus.signatures
 * @return	the number of official signatures available inside the ClamAV signature directory, or 0 on failure.
 */
uint64_t virus_sigs_total(void) {

	int state;
	unsigned int total = 0;

	if ((state = cl_countsigs_d(magma.iface.virus.signatures, CL_COUNTSIGS_OFFICIAL, &total)) != CL_SUCCESS) {
		log_error("ClamAV was unable to count the number of available signatures. {cl_countsigs = %i = %s}", state, cl_strerror_d(state));
		return 0;
	}

	return total;
}

/**
 * @brief	Destroy a ClamAV engine context.
 * @param	target	the address of a pointer to a ClamAV engine context to be freed and reset.
 * @return	This function returns no value.
 */
void virus_engine_destroy(struct cl_engine **target) {
	log_check(!target || !*target);
	cl_engine_free_d(*target);
	*target = NULL;
	return;
}

/**
 * Generates a new ClamAV engine context.
 *
 * @param	signatures An optional pointer which will be used to record the number of signatures loaded.
 * @return Returns a pointer to the newly created context or NULL if an error occurs.
 */
struct cl_engine * virus_engine_create(uint64_t *signatures) {

	int state;
	unsigned int loaded = 0;
	struct cl_engine *target = NULL;

	// Reset the signatures pointer if one was passed in.
	if (signatures) {
		*signatures = 0;
	}

	// Allocate ClamAV engine context.
	if ((target = cl_engine_new_d()) == NULL) {
		log_error("ClamAV returned an error while allocating the engine context. {cl_engine = NULL}");
		return NULL;
	}

	// Max scan size. 2048 MB.
	// Sets the maximum amount of data to be scanned for each input file.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_MAX_SCANSIZE, 2048LL * 1048576LL)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Max file size. 512 MB.
	// Files larger than this limit won't be scanned.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_MAX_FILESIZE, 512 * 1048576)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Maximum recursion level for archives.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_MAX_RECURSION, 32)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Maximum number of files to scan within an archive.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_MAX_FILES, 65536)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// This option sets the lowest number of social security numbers found in a file to generate a detect.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_MIN_SSN_COUNT, 1000000)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// This option sets the lowest number of credit card numbers found in a file to generate a detect.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_MIN_CC_COUNT, 1000000)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Configure the bytecode engine evaluate all bytecode instructions with suspicion.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_BYTECODE_SECURITY, CL_BYTECODE_TRUST_NOTHING)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Configure the directory where ClamAV should store spool/temp data during scans.
	if (virus_spool != NULL && (state = cl_engine_set_str_d(target, CL_ENGINE_TMPDIR, st_char_get(virus_spool))) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_str = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Configure the library to automatically delete temporary files when its finished. Without historical data
	// to reference the ClamAV engine will be unable to detect viruses that have been split across multiple messages.
	if ((state = cl_engine_set_num_d(target, CL_ENGINE_KEEPTMP, 0)) != CL_SUCCESS) {
		log_error("ClamAV configuration error. {cl_engine_set_num = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Load the current signature database.
	if ((state = cl_load_d(magma.iface.virus.signatures, target, &loaded, CL_DB_STDOPT)) != CL_SUCCESS) {
		log_error("ClamAV returned an error while loading the database. {cl_load = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Compile the internal lookup structures.
	if ((state = cl_engine_compile_d(target)) != CL_SUCCESS) {
		log_error("ClamAV database compilation error. {cl_engine_compile = %i = %s}", state, cl_strerror_d(state));
		cl_engine_free_d(target);
		return NULL;
	}

	// Reset the signatures pointer if one was passed in.
	if (signatures) {
		*signatures = loaded;
	}

	return target;
}

/**
 * Initializes the global ClamAV engine context and configures it appropriately.
 *
 * @return Returns true if the ClamAV engine was loaded correctly.
 */
bool_t virus_start(void) {

	int state;
	uint64_t loaded;

	// If we are not supposed to be scanning messages. So don't initialize the engine.
	if (!magma.iface.virus.available) {
		return true;
	}

	// The ClamAV library must be initialized before any library function is used.
	if ((state = cl_init_d(CL_INIT_DEFAULT)) != CL_SUCCESS) {
		log_critical("ClamAV returned an error during initialization. {cl_init = %i = %s}", state, cl_strerror_d(state));
		stats_increment_by_name("provider.virus.error");
		return false;
	}

	// Configure the scanner spool directory. If spool is empty, use the ClamAV default values.
	if (magma.spool && (!(virus_spool = spool_path(MAGMA_SPOOL_SCAN)) || spool_check(virus_spool))) {
		log_critical("The virus spool path is invalid. {path = %.*s}", st_length_int(virus_spool), st_char_get(virus_spool));
		stats_increment_by_name("provider.virus.error");
		return false;
	}

	// Setup the refresh data.
	mm_wipe(&virus_stat, sizeof(struct cl_stat));
	cl_statinidir_d(magma.iface.virus.signatures, &virus_stat);

	if ((virus_engine = virus_engine_create(&loaded)) == NULL) {
		log_critical("Failed to construct a new ClamAV engine context.");
		stats_increment_by_name("provider.virus.error");
		cl_statfree_d(&virus_stat);
		return false;
	}

	// Record the number of signatures loaded.
	virus_sigs = loaded;

	// Update the ClamAV engine trackers.
	stats_set_by_name("provider.virus.available", 1);
	stats_set_by_name("provider.virus.signatures.loaded", loaded);
	stats_set_by_name("provider.virus.signatures.total", virus_sigs_total());

	// TODO: Create function to load each signature database found inside the ClamAV directory, and then output the ver/sig count using cl_cvdparse().
	log_pedantic("------------------------------- SIGNATURES -------------------------------\n%-10.10s %63.lu\n%-10.10s %63.lu\n", "LOADED:", loaded, "AVAILABLE:", virus_sigs_total());

	return true;
}

/**
 * @brief	Shut down the ClamAV library and free all its data and temporary files.
 * @return	This function returns no value.
 */
void virus_stop(void) {

	// If we are not supposed to be scanning messages. So don't free the engine.
	if (!magma.iface.virus.available) {
		return;
	}

	// Frees the engine context.
	if (virus_engine) {
		virus_engine_destroy(&virus_engine);
		virus_engine = NULL;
		virus_sigs = 0;
	}

	// Free the memory associated with the virus scanning engine.
	if (virus_spool) {
		st_free(virus_spool);
		virus_spool = NULL;
	}

	// Frees the database directory status.
	cl_statfree_d(&virus_stat);

	// My very own ClamAV shutdown function.
	cl_shutdown_d();

	// The lt_dlexit was added to the cl_shutdown function by a custom patch.
	// A call to lt_dlexit() is required to release all of the memory used by ClamAV.
	// lt_dlexit_d();

	// Update the ClamAV engine trackers. The values below are used to indicate a shutdown state.
	stats_set_by_name("provider.virus.available", 0);
	stats_set_by_name("provider.virus.signatures.loaded", 0);

	return;
}

/**
 * Checks the virus database directory for new signatures. If new signatures are detected an updated ClamAV engine context is created.
 *
 * @return Returns 1 if the engine context is updated, 0 if no updates are necessary and -1 in the event of an error.
 */
int virus_engine_refresh(void) {

	int state;
	time_t utime;
	struct tm now;
	uint64_t loaded, total;
	struct cl_engine *original, *new = NULL;

	// If we are not supposed to be scanning messages. So don't bother refreshing engine.
	if (!magma.iface.virus.available) {
		return 0;
	}

	if (cl_statchkdir_d(&virus_stat) == 1) {

		if ((new = virus_engine_create(&loaded)) == NULL) {
			log_error("Failed to construct a new ClamAV engine context.");
			stats_increment_by_name("provider.virus.error");
			return -1;
		}

		// Lock and then swap the pointer.
		pthread_rwlock_wrlock(&virus_lock);
		original = virus_engine;
		virus_engine = new;
		virus_sigs = loaded;
		pthread_rwlock_unlock(&virus_lock);

		// Free the old engine context.
		virus_engine_destroy(&original);

		// Refresh the statistics, so we can properly log the update.
		cl_statfree_d(&virus_stat);
		mm_wipe(&virus_stat, sizeof(struct cl_stat));
		cl_statinidir_d(magma.iface.virus.signatures, &virus_stat);

		// Update the engine counters with counts from the new signature database.
		stats_set_by_name("provider.virus.signatures.loaded", loaded);
		stats_set_by_name("provider.virus.signatures.total", (total = virus_sigs_total()));

		// If we have a problem calculating the local time, output the message without the time.
		if ((utime = time(NULL)) == ((time_t)-1) || (localtime_r(&utime, &now)) == NULL) {
			log_info("%lu out of %lu signatures were loaded.", loaded, total);
		} else {

			// Get the hour on a 12 hour clock.
			if (now.tm_hour == 0) {
				state = 12;
			} else if (now.tm_hour > 12) {
				state = now.tm_hour - 12;
			} else {
				state = now.tm_hour;
			}

			log_info("%lu out of %lu signatures were loaded. (%.2i:%.2i %s %s)", loaded, total, state, now.tm_min, (now.tm_hour < 12 ? "AM" : "PM"),
					tzname[(now.tm_isdst > 0 ? 1 : 0)]);
		}
	}

	return 1;
}

/**
 * @brief	Virus scan a block of data.
 * @param	data	a managed string containing the block of data to be scanned.
 * @return	1 if the message passed the scan, or < 0 on failure.
 *        -1: general failure, or the virus scanner was not enabled.
 *        -2: the data matches a worm, trojan, or virus.
 *        -3: the data matches a phishing attempt.
 */
int virus_check(stringer_t *data) {

	int fd, state;
	char *virname;
	ssize_t written;
	unsigned long int scanned;

	// If we are not supposed to be scanning messages.
	if (!magma.iface.virus.available) {
		return -1;
	}

	// Lets make sure an actual message was passed..
	if (!data || !st_length_get(data)) {
		log_error("An invalid message pointer was passed in.");
		return -1;
	}

	// Create a temporary file to store the message being scanned.
	if ((fd = spool_mktemp(MAGMA_SPOOL_SCAN, "virus")) < 0) {
		log_pedantic("Unable to open a temporary file to hold the message being scanned.");
		stats_increment_by_name("provider.virus.error");
		return -1;
	}

	// Stick the message in the file for ClamAV.
	if ((written = write(fd, st_data_get(data), st_length_get(data))) != st_length_get(data)) {
		log_error("Not all of the bytes were written to disk. Was %zi, but should have been %zu.", written, st_length_get(data));
		stats_increment_by_name("provider.virus.error");
		close(fd);
		return -1;
	}

	// Scan the message.
	pthread_rwlock_rdlock(&virus_lock);
	state = cl_scandesc_d(fd, (const char **)&virname, &scanned, virus_engine, CL_SCAN_STDOPT);

	// If we found something, then spit it back.
	// http://wiki.clamav.net/Main/MalwareNaming has naming conventions.
	if (state == CL_VIRUS) {

		log_pedantic("%s detected by ClamAV.", virname);

		// These are signature based phishing matches.
		if (!st_cmp_ci_starts(PLACER(virname, ns_length_get(virname)), CONSTANT("Email.Phishing")) || !st_cmp_ci_starts(PLACER(virname, ns_length_get(virname)), CONSTANT("HTML.Phishing"))) {
			pthread_rwlock_unlock(&virus_lock);
			stats_increment_by_name("provider.virus.scan.total");
			stats_increment_by_name("provider.virus.scan.phishing");
			close(fd);
			return -3;
		}
		// We ignore email that ClamAV thinks is a phishing based on scanner's internal heuristic checks.
		else if (!st_cmp_ci_starts(PLACER(virname, ns_length_get(virname)), CONSTANT("Phishing")) ||
			!st_cmp_ci_starts(PLACER(virname, ns_length_get(virname)), CONSTANT("Joke"))) {
			pthread_rwlock_unlock(&virus_lock);
			stats_increment_by_name("provider.virus.scan.total");
			stats_increment_by_name("provider.virus.scan.clean");
			close(fd);
			return 1;
		}
		// Its probably a worm, trojan, virus or something similar.
		else {
			pthread_rwlock_unlock(&virus_lock);
			stats_increment_by_name("provider.virus.scan.total");
			stats_increment_by_name("provider.virus.scan.infected");
			close(fd);
			return -2;
		}
	}

	pthread_rwlock_unlock(&virus_lock);
	close(fd);

	// Track the number of clean messages. We can do the tracking after the mutex is released.
	if (state == CL_CLEAN) {
		stats_increment_by_name("provider.virus.scan.total");
		stats_increment_by_name("provider.virus.scan.clean");
	} else {
		log_error("An error occurred while scanning a message. {cl_scandesc = %i = %s}", state, cl_strerror_d(state));
		stats_increment_by_name("provider.virus.error");
	}

	return 1;
}

/**
 * Returns the version of ClamAV that was loaded at runtime.
 *
 * @return The ClamAV version as a constant string.
 */
const char * lib_version_clamav(void) {
	return cl_retver_d();
}

/**
 * @brief	Loads the external functions needed by the ClamAV interface.
 * @return 	false on failure or true on success.
 */
bool_t lib_load_clamav(void) {

	symbol_t clamav[] = {
		M_BIND(cl_countsigs), M_BIND(cl_engine_compile), M_BIND(cl_engine_free), M_BIND(cl_engine_new),	M_BIND(cl_engine_set_num),
		M_BIND(cl_engine_set_str), M_BIND(cl_init),	M_BIND(cl_load), M_BIND(cl_retver),	M_BIND(cl_scandesc), M_BIND(cl_shutdown),
		M_BIND(cl_statchkdir), M_BIND(cl_statfree), M_BIND(cl_statinidir), M_BIND(cl_strerror),	M_BIND(lt_dlexit),
	};

	if (!lib_symbols(sizeof(clamav) / sizeof(symbol_t), clamav)) {
		return false;
	}

	return true;
}

