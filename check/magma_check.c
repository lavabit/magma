
/**
 * @file /magma.check/magma_check.c
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
chr_t *virus_check_data_path = NULL, *tank_check_data_path = NULL, *dspam_check_data_path = NULL;

int_t case_timeout = RUN_TEST_CASE_TIMEOUT;

Suite * suite_check_magma(void) {
  Suite *s = suite_create("\n\tMagma");
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
void check_display_usage(char *progname) {

	log_info("\n"
			"\t%s [options] [config_file]\n\n"
			"\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\n",
			progname,
			"-v [virus_check_path]", "set the virus checker path, or disable the check if none is specified.",
			"-t [tank_check_path]",  "set the tank checker path, or disable the check if none is specified.",
			"-d [dspam_check_path]", "set the DSPAM checker path, or disable the check if none is specified.",
			"-s", "disable the SPF checker.");
	exit(EXIT_FAILURE);
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
		check_display_usage(xargv[0]);
	}
	// If the next argument begins with '-' then our option has a null parameter.
	else if (!mm_cmp_cs_eq(xargv[*iptr+1], "-", 1)) {
		(*iptr)++;
		return NULL;
	}

	// If the following parameter is the last one, it must be the config file and this is a null option.
	if (*iptr+1 == (xargc-1)) {
		(*iptr)++;
		return NULL;
	}

	(*iptr) += 2;

	if (!(result = ns_dupe(xargv[*iptr-1]))) {
		log_unit("Memory allocation encountered while preparing checks. Exiting.\n");
	}

	return result;
}

/* modeled closely after args_parse() */
void check_args_parse(int argc, char *argv[]) {

	int_t i = 1;

	while (i < argc) {

		if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-v", 2))) {

			if (!(virus_check_data_path = check_next_opt(argv, &i, argc))) {
				do_virus_check = false;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-t", 2))) {

			if (!(tank_check_data_path = check_next_opt(argv, &i, argc))) {
				do_tank_check = false;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-d", 2))) {

			if (!(dspam_check_data_path = check_next_opt(argv, &i, argc))) {
				do_dspam_check = false;
			}

		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-s", 2))) {
			do_spf_check = false;
			i++;
		}
		// See if it's an illegal parameter beginning with "-"
		else if (!mm_cmp_cs_eq(argv[i], "-", 1)) {
			check_display_usage(argv[0]);
		}
		// Otherwise it's the config file
		else if (i == (argc-1)) {
			snprintf(magma.config.file, MAGMA_FILEPATH_MAX, "%s", argv[i]);
			i++;
		}
		else {
			check_display_usage(argv[0]);
		}

	}

	return;
}

int main(int argc, char *argv[]) {

	SRunner *sr;
	int_t failed = 0;
	time_t prog_start, test_start, test_end;

	if (process_kill(PLACER("magmad", 6), SIGTERM, 10) < 0 || process_kill(PLACER("magmad.check", 12), SIGTERM, 10) < 0) {
		log_unit("Another instance of the Magma Daemon is already running and refuses to die.");
		exit(EXIT_FAILURE);
	}

	// Setup
	prog_start = time(NULL);

	// Updates the location of the config file if it was specified on the command line.
	check_args_parse(argc, argv);

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
		log_unit("Initialization error. Exiting.\n");
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

	// Add the suites.
	srunner_add_suite(sr, suite_check_core());
	srunner_add_suite(sr, suite_check_provide());
	srunner_add_suite(sr, suite_check_network());
	srunner_add_suite(sr, suite_check_objects());
	srunner_add_suite(sr, suite_check_users());

	// If were being run under Valgrind, we need to disable forking and increase the default timeout.
	// Under Valgrind, forked checks appear to improperly timeout.
	if (RUNNING_ON_VALGRIND == 0 && (failed = running_on_debugger()) == 0) {
		log_unit("Not being traced or profiled...\n");
		srunner_set_fork_status (sr, CK_FORK);
		case_timeout = RUN_TEST_CASE_TIMEOUT;
	}
	else {
		// Trace detection attempted was thwarted.
		if (failed == -1)	log_unit("Trace detection was thwarted.\n");
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
