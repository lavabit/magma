
/**
 * @file /magma/engine/context/process.c
 *
 * @brief	Functions used to start and stop the daemon, including the execution of the module init and module clean up functions.
 */

#include "magma.h"

extern bool_t exit_and_dump;
uint64_t day = 0;
pthread_t *maint = NULL;

/**
 * @brief	The entry point for the process maintenance thread, which runs in a continuous loop unless canceled.
 * @note	Execute once daily: rotate the log files, update the warehouse, and perform tank maintenance.
 * 			Execute every few (0-10) minutes: refresh the virus engine and prune the object cache.
 * @return	This function returns no value.
 */
void process_maint(void) {

	uint_t delay;

	thread_start();
	thread_cancel_disable();

	do {

		// Execute these functions one per day.
		if ((time_datestamp() != day)) {
			day = time_datestamp();
			log_rotate();
			warehouse_update();
			// Not yet...
			// tank_maintain();
		}

		// Execute these functions every few minutes.
		virus_engine_refresh();
		obj_cache_prune();

		// If were close to midnight, sleep until midnight, otherwise sleep a random number of seconds up to ten minutes.
		if (status()) {

			delay = (time_till_midnight() <= 600 ? time_till_midnight() : rand_get_uint32() % 600) + 1;

			// Enable cancellation so exit attempts don't have to wait around for the sleep call to return.
			thread_cancel_enable();
			sleep(delay);
			thread_cancel_disable();
		}

	} while (status());

	thread_cancel_enable();
	thread_stop();
	pthread_exit(NULL);
}

/**
 * @brief	Execute the magma shutdown routines.
 * @note	This function will execute a shutdown routine for each of the startup initialization routines that was performed.
 * 			It will also signal and terminate the maintenance thread.
 * @return	This function returns no value.
 */
void process_stop(void) {

	int_t ret;
	void (*stoppers[])() = {
		NULL, /* Sanity check. */
		spool_stop, /* The spool needs to be purged absolutely last other wise files could be left behind. */
		config_free, /* Config defaults are loaded. */
		NULL, /* The config file is loaded. */
		lib_unload, /* Shared library is now open and needs to be closed. */

		sql_stop, /* Database connections are active. */
		NULL, /* Database magma_keys. */
		NULL, /* Nothing to do with command line options */
		NULL, /* Settings validated. */
		NULL, /* The spool shutdown is done right before we exit. */

		mm_sec_stop, /* Shutdown the secure memory pool. */
		servers_network_stop,
		NULL, /* Fork daemon process. */
		NULL, /* Change root directory. */
		NULL, /* Core dumps. */
		NULL, /* System resource limits. */
		NULL, /* Impersonation. */
		NULL, /* Umask. */
		NULL, /* Signal handlers. */

		stats_shutdown, /* Shutdown the statistics interface. */
		ssl_stop, /* Shutdown the OpenSSL interface. */
		rand_stop, /* Shutdown the random number generator. */
		deprecated_ecies_stop, /* Release the elliptical curve group. */
		prime_stop, /* Release the privacy respecting internet mail environment objects. */

		xml_stop,
		virus_stop, /* Shutdown the anti-virus engine. */
		spf_stop,
		dkim_stop,
		dspam_stop,
		cache_stop,
		tank_stop, /* Shutdown the storage system. This should flush any pending write operations and cleanly close the tank data files. */

		obj_cache_stop,
		mail_cache_stop,
		warehouse_stop,
		http_content_stop,
		NULL, /* Protocol handlers. */
		servers_encryption_stop,
		queue_shutdown, /* Shutdown the thread pool. */
		NULL /* Logging */
	};

#ifdef MAGMA_PEDANTIC

	// Detects situations where the number of startup functions doesn't precisely match the number of shutdown functions.
	if (magma.init != sizeof(stoppers) / sizeof(void *)) {
		log_pedantic("magma.init != shutdown {%u != %lu}", magma.init, sizeof(stoppers) / sizeof(void *));
	}

#endif
	// QUESTION: Shouldn't this be sizeof(stoppers)/sizeof(void *) also?
	if (magma.init > sizeof(stoppers)) {
		log_critical("The run level exceeds the number of shutdown tasks.");
		return;
	}


	// Explicitly canceling the maintenance thread will it from calling the thread cleanup function and that would result in a complaint
	// from my_thread_global_end(). Instead we send along a signal to trigger a wakeup, if the thread were sleeping.
	if (maint) {

		if ((ret = thread_signal(*maint, SIGALRM)) && status()) {
			log_info("Unable to signal the maintenance thread. {ret = %i}", ret);
		}

		thread_join(*maint);
		mm_free(maint);
		maint = NULL;
	}

	while (magma.init--) {
		if (stoppers[magma.init]) {
			stoppers[magma.init]();
		}
	}

	log_info("Magma shutdown complete.");
	return;
}

/**
 * @brief	Execute the magma initializations routines, making sure they all run properly.
 * @note	If any of the init routines returns with failure, this function fails and logs an error message.
 * 			Certain checks are made that the init routines are run in a particular order, especially in
 *			waiting for daemonization and privilege dropping before starting logging.
 * 			The system maintenance thread is also launched from here.
 * @return	true if all starter functions returned true, or false if any of them failed..
 */
bool_t process_start(void) {

	bool_t (*starters[])() = {
		(void *)&sanity_check,
		NULL,
		(void *)&config_load_defaults,
		(void *)&config_load_file_settings,
		(void *)&lib_load,

		(void *)&sql_start,
		(void *)&config_load_database_settings,
		(void *)&config_load_cmdline_settings,
		(void *)&config_validate_settings,

		(void *)&spool_start,

		// We want to allocate the secure memory pool before we drop privileges.
		(void *)&mm_sec_start,
		(void *)&servers_network_start,
		(void *)&system_fork_daemon,
		(void *)&system_change_root_directory,
		(void *)&system_init_core_dumps,
		(void *)&system_init_resource_limits,
		(void *)&system_init_impersonation,
		(void *)&system_init_umask,
		(void *)&signal_start,

		(void *)&stats_init,
		(void *)&ssl_start,
		(void *)&rand_start,
		(void *)&deprecated_ecies_start,
		(void *)&prime_start,

		(void *)&xml_start,
		(void *)&virus_start,
		(void *)&spf_start,
		(void *)&dkim_start,
		(void *)&dspam_start,
		(void *)&cache_start,
		(void *)&tank_start,

		(void *)&obj_cache_start,
		(void *)&mail_cache_start,
		(void *)&warehouse_start,
		(void *)&http_content_start,
		(void *)&protocol_init,
		(void *)&servers_encryption_start,
		(void *)&queue_init,
		(void *)&log_start
	};

	char *errors[] = {
		"Platform sanity check failed. Exiting.",
		"",
		"Unable to load default settings. Exiting.",
		"Unable to load configuration file. Exiting.",
		"Unable to load the Magma shared library. Exiting.",

		"Unable to open the Magma database. Exiting.",
		"Unable to load configuration settings stored inside the Magma database. Exiting.",
		"Unable to load configuration settings from command line. Exiting.",
		"The provided configuration is invalid. Exiting.",
		"Unable to remove stale data found inside the spool directory tree.",

		"Initialization of the secure memory system failed. Exiting.",
		"Unable to initialize all of the server instances. Exiting.",
		"Unable to fork a background daemon process. Exiting.",
		"Unable to jail the process inside the provided root directory. Exiting.",
		"The system does not allow core dumps. Exiting.",
		"An error occurred while trying to increase the system resource limits. Exiting.",
		"Unable to impersonate the user provided by the configuration. Exiting.",
		"Unable to set the default creation mask for newly created files and directories. Exiting.",
		"Unable to register signal handlers. Exiting.",

		"Unable to initialize the statistics interface. Exiting.",
		"Unable to initialize the encryption interface. Exiting.",
		"Unable to initialize the random number generator. Exiting.",
		"Unable to initialize the elliptical curve group. Exiting.",
		"Unable to initialize the privacy respecting internet mail environment. Exiting.",

		"Unable to initialize the XML parsing engine. Exiting.",
		"Unable to initialize the anti-virus engine. Exiting.",
		"Unable to initialize the SPF engine. Exiting.",
		"Unable to initialize the DKIM engine. Exiting.",
		"Unable to initialize the DSPAM engine. Exiting.",
		"Unable to initialize the distributed cache system. Exiting.",
		"Unable to initialize the storage system. Exiting.",

		"Unable to initialize the local object cache. Exiting.",
		"Unable to initialize the thread local mail cache. Exiting.",
		"Unable to initialize the data warehouse engine. Exiting.",
		"Unable to initialize the web content cache. Exiting.",
		"Unable to initialize the protocol handlers. Exiting.",
		"Unable to initialize the server encryption context. Exiting.",
		"Unable to initialize the thread pool. Exiting.",
		"Initialization of the log configuration failed. Exiting."
	};

	if (sizeof(starters) != sizeof(errors)) {
		log_critical("The number of startup functions doesn't match the number of error messages. Exiting.");
		return false;
	}

#ifdef MAGMA_PEDANTIC
	// The ClamAV docs dictate that the random number generator be seeded after a process fork to prevent temp file collisions. The following
	// logic ensures the start order remains correct.
	for (uint32_t order = 0, i = 0; i < (sizeof(starters) / sizeof(void *)); i++) {
		if (starters[i] == &system_fork_daemon) {
			order++;
		}
		else if (order == 1 && starters[i] == &rand_start) {
			order++;
		}
		else if (order != 2 && starters[i] == &virus_start) {
			log_critical("The anti-virus engine requires the random number generator be seeded following a process fork and before being initialized.");
			return false;
		}
	}

	// Since file logging redirects the stdout handle, and the daemonize forking process closes the stdout handles, we need to make sure file logging
	// isn't started until after the process has forked. We also wait until after impersonation to prevent the log files from being created with the
	// wrong owner.
	for (uint32_t order = 0, i = 0; i < (sizeof(starters) / sizeof(void *)); i++) {
		if (starters[i] == &servers_network_start && order != 0) {
			log_critical("The server instances must Magma logging system must not be started until the execution environment has been configured.");
			return false;
		}
		else if (starters[i] == &servers_network_start || starters[i] == &system_fork_daemon || starters[i] == &system_init_impersonation ||
			starters[i] == &system_change_root_directory) {
			order++;
		}
		else if (starters[i] == &log_start && order != 4) {
			log_critical("The Magma logging system must not be started until the execution environment has been initialized.");
			return false;
		}
		else if (starters[i] == &servers_encryption_start && order != 4) {
			log_critical("The server certificates must be loaded after the execution environment has been initialized");
			return false;
		}
	}
#endif

	// Record the process identity.
	status_process();

	// Record the name of the host.
	if (gethostname(magma.host.name, MAGMA_HOSTNAME_MAX + 1) != 0) {
		log_critical("We could not determine the name of the local host.");
		return false;
	}
	// Store the page length so we can properly align our memory requests.
	else if ((magma.page_length = getpagesize()) <= 0) {
		log_critical("The memory page alignment size is invalid.");
		return false;
	}

	for (uint32_t i = 0; i < (sizeof(starters) / sizeof(void *)); i++) {
		if (starters[i] && starters[i]() == false) {

			// A special case.
			if ((starters[i] == (void *)&config_validate_settings) && exit_and_dump) {
				return false;
			}

			log_critical("%s", errors[i]);
			return false;
		}

		magma.init++;
	}

	// Spawn the maintenance thread.
	if ((maint = mm_alloc(sizeof(pthread_t))) && thread_launch(maint, &process_maint, NULL)) {
		log_critical("Could not start the maintenance thread.");
		if (maint) {
			mm_free(maint);
			maint = NULL;
		}
		return false;
	}

	log_info("Magma initialization complete.");
	return true;
}

