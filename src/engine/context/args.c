
/**
 *
 * @file /magma/engine/context/args.c
 *
 * @brief	Functions for parsing command line arguments.
 */

#include "magma.h"

extern stringer_t *cmdline_config_data;
extern bool_t exit_and_dump;


/**
 * @brief	Display the magma usage info if a user requests help or invokes it with incorrect options.
 * @return	This function returns no value.
 */
void display_usage(void) {

	log_info("\n"
			"\tmagmad [options] [config_file]\n\n"
			"\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\n\t%-25.25s\t\t%s\n\t%-25.25s\t\t%s\n\n",
			"-d, --dump", "dump active magma config and exit before running daemon",
			"-c, --config STRING", "specify an individual config parameter name and it's value using the command line",
			"-h, --help", "display the magma command line help with default values and exit",
			"-v, --version", "display the magma version information and exit");

	return;
}

/**
 * @brief	Process any command line arguments supplied to magma.
 * @note	A few command line options are supported: -h (--help), -d (--dump), -v (--version), and -c (--config).
 * 			Otherwise, the magma config file path will be loaded from the command line.
 * 			If not an option starting with "-", the final command line argument is assumed to be the path of the magma configuration file.
 * @return	true if the command line arguments were parsed successfully, or false if there was an error.
 */
bool_t args_parse(int argc, char *argv[]) {

	bool_t result = true;

	for (int_t i = 1; i < argc; i++) {

		if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-h", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--help", 6))) {
			config_load_defaults();
			display_usage();
			config_output_help();
			result = false;
			break;
		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-d", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--dump", 6))) {
			exit_and_dump = true;
		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-v", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--version", 9))) {
			log_info("\n\t%s\n\n\t%-20.20s %14.14s\n\t%-20.20s %14.14s\n\t%-20.20s %14.14s\n\n", "magmad",
				"version", build_version(), "commit", build_commit(), "build", build_stamp());
			result = false;
			break;
		}
		else if (!st_cmp_cs_eq(NULLER(argv[i]), PLACER("-c", 2)) || !st_cmp_cs_eq(NULLER(argv[i]), PLACER("--config", 8))) {

			if (i == (argc - 1)) {
				display_usage();
				result = false;
				break;
			}

			if (!cmdline_config_data) {

				//if (!(cmdline_config_data = st_import(argv[i+1], ns_length_get(argv[i+1])))) {
				if (!(cmdline_config_data = st_dupe_opts(MANAGED_T | JOINTED | HEAP, NULLER(argv[i+1])))) {
					log_critical("Failed to allocate storage for command line option.");
					result = false;
					break;
				}

			}
			else if (!(cmdline_config_data = st_append(cmdline_config_data, PLACER("\n",2))) ||
					!(cmdline_config_data = st_append(cmdline_config_data, NULLER(argv[i+1])))) {
				log_critical("Failed to reallocate storage for command line option.");
				result = false;
				break;
			}

			i++;
		}
		// See if it's an illegal parameter beginning with "-"
		else if (!mm_cmp_cs_eq(argv[i], "-", 1)) {
			display_usage();
			result = false;
			break;
		}
		// Otherwise it's the config file
		else if (i == (argc -1)) {
			snprintf(magma.config.file, MAGMA_FILEPATH_MAX, "%s", argv[i]);
		}
		else {
			display_usage();
			result = false;
			break;
		}

	}

	return result;
}
