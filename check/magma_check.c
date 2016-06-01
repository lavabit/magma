
/**
 * @file /check/magma_check.c
 *
 * @brief	The unit test executable entry point.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t do_virus_check = true, do_tank_check = true, do_dspam_check = true, do_spf_check = true;
chr_t *virus_check_data_path = NULL, *tank_check_data_path = NULL, *dspam_check_data_path = NULL, *barrister_unit_test = NULL;

int_t case_timeout = RUN_TEST_CASE_TIMEOUT;

/**
 * @brief Enable the log so we can print status information. We're only concerned with whether the
		test passed or failed. The actual error message will be recorded by the check library, and
		then printed en masse when all the test cases have concluded.
 * @param test	The NULL terminated string showing the suite, test, and threading information.
 * @param error	The error string, which should be NULL if the test was skipped, or if the test passed.
 */
void log_test(chr_t *test, stringer_t *error) {
	log_enable();
	log_unit("%-64.64s%10.10s\n", test, (!status() ? "SKIPPED" : !error ? "PASSED" : "FAILED"));
	return;
}

Suite * suite_check_magma(void) {
  Suite *s = suite_create("\n\tMagma");
  return s;
}

/**
 * @brief This function will construct a special suite with only a single unit test that is extracted from the running executable based on it's name.
 * @note	This function will allow us to create a special test suite with any single test case available in the global symbols table. This same functionality
 * 		is available using the CK_RUN_CASE environment variable in the current version of libcheck, but wasn't available in the version currently available on our
 * 		target platform, which is why this method of achieving the same end was created.
 * @param testname The name of the unit test we want our barrister to run.
 * @return The initialized suite, assuming we found the requested unit test, otherwise NULL is returned.
 */
Suite * suite_check_barrister(chr_t *testname) {

	TCase *tc;
	void *handle;
	void (*unittest)(int);
	Suite *s = suite_create("\tBarrister");;

	// By calling dlopen() with NULL for the filename, we are attempting to establish a handle for the currently running executable image.
	if (!(handle = dlopen(NULL, RTLD_LOCAL | RTLD_NOW))) {
		log_error("Unable to open a dynamic symbol handle for the currently running program.");
		return NULL;
	}

	// Check the currently running program for the requested function name.
	if (!(*(void **)(&unittest) = dlsym(handle, testname))) {
		log_info("%s", dlerror());
	}
	else {
		testcase(s, tc, testname, unittest);
	}

	dlclose(handle);
	return s;
}

/***
 * @return Will return -1 if the code is unable to determine whether tracing is active, 0 if tracing is disabled and 1 if tracing has been detected.
 */
int_t running_on_debugger(void) {

	int_t status, ret;
	pid_t pid, parent;

	if ((pid = fork()) == -1) {
		return -1;
	}

	// If were the child, we'll try to start tracing the parent process. If our trace request fails, we assume that means its already being traced by a debugger.
	else if (pid == 0) {

		parent = getppid();

		if (ptrace(PTRACE_ATTACH, parent, NULL, NULL) == 0) {
			waitpid(parent, NULL, 0);
			ptrace(PTRACE_CONT, NULL, NULL);
			ptrace(PTRACE_DETACH, getppid(), NULL, NULL);
			exit(0);
		}

		exit(1);

	}
	else if ((ret = waitpid(pid, &status, 0)) == pid && WIFEXITED(status) == true) {

		// Use a ternary to guarantee only two possible return values.
		return WEXITSTATUS(status) ? 1 : 0;
	}


	return -1;
}

/* modeled closely after display_usage() */
void check_display_help (chr_t *invalid_option) {

	log_info("%s%s%s" \
			"\tmagmad.check [options] [config_file]\n\n" \
			"\t%-25.25s\t\t%s\n\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\n",
			(invalid_option ? "The command line option \"" : ""), (invalid_option ? invalid_option : ""),
			(invalid_option ? "\" is invalid. Please consult the text below and try again.\n\n" : "\n"),
			"-c, --check NAME", "run a single unit test",
			"    --dspam-path PATH", "set the DSPAM checker path, or disable the check if none is specified.",
			"    --tank-path PATH",  "set the tank checker path, or disable the check if none is specified.",
			"    --virus-path PATH", "set the virus checker path, or disable the check if none is specified.",
			"    --disable-spf", "disable the SPF checker.",
			"-h, --help", "display the magma unit tester command line options and exit",
			"-v, --version", "display the magma unit tester version information and exit");

	return;
}

/**
 * @brief	Check to see if the specified command line option specified an optional parameter, and adjust the index accordingly.
 * @note	This function will automatically generate an error through display_usage() if the parameters are incorrect.
 * @param	xargv	a pointer to the main function's argv array.
 * @param	iptr	a pointer to the xargv option index to be checked and updated.
 * @param	xargc	the number of items in the xargv array.
 * @return	a pointer to the value of the current index's optional parameter if specified, or NULL if one wasn't.
 */
chr_t * check_next_opt(char *xargv[], int *iptr, int xargc) {

	chr_t *result;

	// If this is an optional parameter then there still must be a config file specified after it.
	if (*iptr == (xargc-1)) {
		return NULL;
	}
	// If the next argument begins with '-' then our option has a null parameter.
	else if (!mm_cmp_cs_eq(xargv[*iptr + 1], "-", 1)) {
		(*iptr)++;
		return NULL;
	}

	(*iptr) += 2;

	if (!(result = ns_dupe(xargv[*iptr - 1]))) {
		log_error("Memory allocation failure encountered while parsing the command line options.");
	}

	return result;
}

/**
 * @brief	Process any command line arguments supplied to magma unit tester.
 * @note	A few command line options are supported: -c (--check), -h (--help), -v (--version), and along with options for overriding
 * 			the hard coded file system paths, and the disabling the spf check. If the final option doesn't start with "-" then it's assumed
 * 			to be the config file path.
 * @return	Returns -1 if the program should exit with a failure code, returns 0 if the program should simply exit and returns 1 if the program should continue.
 */
int_t check_args_parse(int argc, char *argv[]) {

	int_t i = 1;

	while (i < argc) {

		if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-c", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--check", 7))) {

			if (barrister_unit_test) {
				log_error("The \"--check\" parameter may only be used once. Exiting.\n");
				return -1;
			}
			else if (!(barrister_unit_test = check_next_opt(argv, &i, argc))) {
				log_error("The individual unit test name is missing. Exiting.\n");
				return -1;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("--virus-path", 12))) {

			if (!(virus_check_data_path = check_next_opt(argv, &i, argc))) {
				do_virus_check = false;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("--tank-path", 11))) {

			if (!(tank_check_data_path = check_next_opt(argv, &i, argc))) {
				do_tank_check = false;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("--dspam-path", 12))) {

			if (!(dspam_check_data_path = check_next_opt(argv, &i, argc))) {
				do_dspam_check = false;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("--disable-spf", 13))) {
			do_spf_check = false;
			i++;
		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-v", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--version", 9))) {
			log_info("\n\t%s\n\n\t%-20.20s %14.14s\n\t%-20.20s %14.14s\n\t%-20.20s %14.14s\n\n", "magmad.check",
				"version", build_version(), "commit", build_commit(), "build", build_stamp());
			return 0;
		}
		// Display the help if it's explicitly requested, or if we encounter an option we don't recognize.
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-h", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--help", 6))) {
			check_display_help(NULL);
			return 0;
		}
		// Otherwise it's the config file
		else if (i == (argc - 1) && mm_cmp_cs_eq(argv[i], "-", 1)) {
			snprintf(magma.config.file, MAGMA_FILEPATH_MAX, "%s", argv[i]);
			i++;
		}
		else {
			check_display_help(argv[i]);
			return -1;
		}

	}

	return 1;
}

int main(int argc, char *argv[]) {

	SRunner *sr;
	int_t failed = 0, result;
	time_t prog_start, test_start, test_end;

	if (process_kill(PLACER("magmad", 6), SIGTERM, 10) < 0 || process_kill(PLACER("magmad.check", 12), SIGTERM, 10) < 0) {
		log_error("Another instance of the Magma Daemon is already running and refuses to die.");
		exit(EXIT_FAILURE);
	}

	// Setup
	prog_start = time(NULL);

	// Updates the location of the config file if it was specified on the command line.
	if ((result = check_args_parse(argc, argv)) != 1) {
		exit(result ? EXIT_FAILURE : EXIT_SUCCESS);
	}

	if (do_virus_check && !virus_check_data_path) {
		virus_check_data_path = ns_dupe(VIRUS_CHECK_DATA_PATH);
	}

	if (do_tank_check && !tank_check_data_path) {
		tank_check_data_path = ns_dupe(TANK_CHECK_DATA_PATH);
	}

	if (do_dspam_check && !dspam_check_data_path) {
		dspam_check_data_path = ns_dupe(DSPAM_CHECK_DATA_PATH);
	}

	if (!process_start()) {
		log_error("Initialization error. Exiting.\n");
		status_set(-1);
		process_stop();
		ns_cleanup(virus_check_data_path);
		ns_cleanup(tank_check_data_path);
		ns_cleanup(dspam_check_data_path);
		exit(EXIT_FAILURE);
	}

	// Only during development...
	cache_flush();

	// Unit Test Config
	sr = srunner_create(suite_check_magma());

	// If the command line told us to run a specific test only add that individual test using the special barrister suite.
	if (barrister_unit_test) {
		srunner_add_suite(sr, suite_check_barrister(barrister_unit_test));

	}
	// Otherwise add all of the unit tests to the suite runner.
	else {
		srunner_add_suite(sr, suite_check_core());
		srunner_add_suite(sr, suite_check_provide());
		srunner_add_suite(sr, suite_check_network());
		srunner_add_suite(sr, suite_check_objects());
		srunner_add_suite(sr, suite_check_users());
	}

	// If were being run under Valgrind, we need to disable forking and increase the default timeout.
	// Under Valgrind, forked checks appear to improperly timeout.
	if (RUNNING_ON_VALGRIND == 0 && (failed = running_on_debugger()) == 0) {
		log_unit("Not being traced or profiled...\n");
		srunner_set_fork_status (sr, CK_FORK);
		case_timeout = RUN_TEST_CASE_TIMEOUT;
	}
	else {
		// Trace detection attempted was thwarted.
		if (failed == -1) log_unit("Trace detection was thwarted.\n");
		else log_unit("Tracing or debugging is active...\n");
		srunner_set_fork_status (sr, CK_NOFORK);
		case_timeout = PROFILE_TEST_CASE_TIMEOUT;
	}

	// Execute
	log_unit("--------------------------------------------------------------------------\n");

	test_start = time(NULL);
	srunner_run_all(sr, CK_SILENT);
	test_end = time(NULL);

	// Output timing.
	log_unit("--------------------------------------------------------------------------\n");
	log_unit("%-63.63s %9lus\n", "TEST DURATION:", test_end - test_start);
	log_unit("%-63.63s %9lus\n", "TOTAL DURATION:", test_end - prog_start);

	// Summary
	log_unit("--------------------------------------------------------------------------\n");
	failed = srunner_ntests_failed(sr);
	srunner_print(sr, CK_NORMAL);

	// The Check Output Ending
	log_unit("--------------------------------------------------------------------------\n");

	// Cleanup and free the resources allocated by the check code.
	status_set(-1);
	srunner_free(sr);

	// Cleanup and free the resources allocated by the magma code.
	process_stop();
	system_init_umask();

	ns_cleanup(virus_check_data_path);
	ns_cleanup(tank_check_data_path);
	ns_cleanup(dspam_check_data_path);

	exit((failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);

}
