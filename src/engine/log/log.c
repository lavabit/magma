
/**
 * @file /magma/engine/log/log.c
 *
 * @brief	Internal logging functions. This function should be accessed using the appropriate macro.
 */

#include "magma.h"

uint64_t log_date;
bool_t log_enabled = true;
FILE *log_descriptor = NULL;
pthread_mutex_t log_mutex =	PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief	Disable logging.
 * @return	This function returns no value.
 */
void log_disable(void) {
	mutex_lock(&log_mutex);
	log_enabled = false;
	mutex_unlock(&log_mutex);
	return;
}

/**
 * @brief	Enable logging.
 * @return	This function returns no value.
 */
void log_enable(void) {
	mutex_lock(&log_mutex);
	log_enabled = true;
	mutex_unlock(&log_mutex);
	return;
}

/**
 * @brief	Log the current stack to stdout.
 * @note	This function was created because backtrace_symbols() can fail if there is heap corruption.
 * @return	-1 if the backtrace failed, or 0 on success.
 */
int_t log_backtrace(FILE *output) {

	int_t pipefds[2];
	char strbuf[1024];
	void *buffer[1024];
	int_t nbt, nread, nfound = 0, result = 0, i;

	nbt = backtrace (buffer, (sizeof (buffer) / sizeof (void *)));

	if (nbt < 0) {
		return -1;
	}

	// Create a pipe to read back the dumped line numbers.
	if (pipe(pipefds) < 0) {
		return -1;
	}

	backtrace_symbols_fd(buffer, nbt, pipefds[1]);

	if (fwrite( "   ", 1, 3, output) != 3) {
		return -1;
	}

	while (nfound < nbt) {
		nread = read(pipefds[0], strbuf, sizeof(strbuf));

		if (nread < 0) {
			result = -1;
			break;
		}
		else if (!nread) {
			break;
		}

		for (i = 0; i < nread; i++) {
			if (fwrite(&strbuf[i], 1, 1, output) != 1) {
				return -1;
			}

			if (strbuf[i] == '\n') {
				nfound++;

				if (nfound != nbt) {
					if (fwrite("   ", 1, 3, output) != 3) {
						return -1;
					}
				}
			}
		}
	}

	fflush(output);
	close(pipefds[0]);
	close(pipefds[1]);

	return result;
}

/**
 *
 * @brief	Logs the message described by format, and provided as a variadic argument list.
 * @param	file	The log macros set this to the caller's filename.
 * @param	function	The log macros set this to the caller's function.
 * @param	line	The log macros set this to the line number where the log function was called.
 * @param	options	Global configuration options can be overridden on a per call basis using the options variable.
 * @param 	format	The printf style format for the log message.
 * @param	va_list	A variadic list of data items to be used by the format string.
 * @return	This function returns no value.
 */
void log_internal(const char *file, const char *function, const int line, M_LOG_OPTIONS options, const char *format, ...) {

	time_t now;
	va_list args;
	struct tm local;
	FILE *fd = NULL;
	bool_t output = false;
	char buffer[128];

	va_start(args, format);

	mutex_lock(&log_mutex);

	// Someone has disabled the log output.
	if (!log_enabled && !magma.output.file) {
		mutex_unlock(&log_mutex);
		return;
	}

	// If we are logging to a file, use the log descriptor as our destination.
	else if (!magma.output.file || !log_descriptor || (M_LOG_CONSOLE == (options & M_LOG_CONSOLE))) {
		fd = stdout;
	}

	// If the log file option is disabled, or the log descriptor is unavailable, we fall back to using standard output.
	else {
		fd = log_descriptor;
	}

	if ((magma.log.time || M_LOG_TIME == (options & M_LOG_TIME)) && !(M_LOG_TIME_DISABLE == (options & M_LOG_TIME_DISABLE))) {
		now = time(NULL);
		localtime_r(&now, &local);
		strftime(buffer, 128, "%T", &local);

		fprintf(fd, "%s%s", (output ? " - " : "["), buffer);
		output = true;
	}

	if ((magma.log.file || M_LOG_FILE == (options & M_LOG_FILE)) && !(M_LOG_FILE_DISABLE == (options & M_LOG_FILE_DISABLE))) {
		fprintf(fd, "%s%s", (output ? " - " : "["), file);
		output = true;
	}

	if ((magma.log.function || M_LOG_FUNCTION == (options & M_LOG_FUNCTION)) && !(M_LOG_FUNCTION_DISABLE == (options & M_LOG_FUNCTION_DISABLE))) {
		fprintf(fd, "%s%s%s", (output ? " - " : "["), function, "()");
		output = true;
	}

	if ((magma.log.line || M_LOG_LINE == (options & M_LOG_LINE)) && !(M_LOG_LINE_DISABLE == (options & M_LOG_LINE_DISABLE))) {
		fprintf(fd, "%s%i", (output ? " - " : "["), line);
		output = true;
	}

	if (output)
		fprintf(fd, "] = ");

	vfprintf(fd, format, args);

	if (!(M_LOG_LINE_FEED_DISABLE == (options & M_LOG_LINE_FEED_DISABLE))) {
		fprintf(fd, "\n");
	}

	if ((magma.log.stack || M_LOG_STACK_TRACE == (options & M_LOG_STACK_TRACE)) && !(M_LOG_STACK_TRACE_DISABLE == (options & M_LOG_STACK_TRACE_DISABLE))) {

		if (log_backtrace(fd) < 0) {
			fprintf(fd, "Error printing stack backtrace to stdout!\n");
		}

		/***
		size = backtrace(array, 1024);
		strings = backtrace_symbols(array, size);

		for (uint64_t i = 0; i < size; i++) {
			fprintf(stdout, "   %s\n", strings[i]);
		}

		if (strings)
			free(strings);

		***/
	}

	fflush(fd);
	mutex_unlock(&log_mutex);

	va_end(args);

	return;
}

void log_rotate(void) {

	uint64_t date;
	FILE *orig_out, *orig_err;
	chr_t *log_file = MEMORYBUF(1024);

	if (magma.output.file && magma.output.path && (date = time_datestamp()) != log_date) {

		orig_out = stdout;
		orig_err = stderr;
		log_date = date;

		if (snprintf(log_file, 1024, "%s%smagmad.%lu.log", magma.output.path, (*(ns_length_get(magma.output.path) + magma.output.path) == '/') ? "" : "/",
			date) >= 1024) {
			log_critical("Log file path exceeded available buffer. { file = %s%smagmad.%lu.log }", magma.output.path,
				(*(ns_length_get(magma.output.path) + magma.output.path) == '/') ? "" : "/", date);
			return;
		}

		pthread_mutex_lock(&log_mutex);

		// If daemonized, then we rely on standard output descriptor for writing out the log.
		if (magma.system.daemonize) {
			if (!(stdout = freopen64(log_file, "a", stdout))) {
				stdout = orig_out;
				log_critical("Unable to rotate the error log. { file = %s }", log_file);
				pthread_mutex_unlock(&log_mutex);
				return;
			}


			if (!(stderr = freopen64(log_file, "a", stderr))) {
				fclose(stdout);
				stdout = orig_out;
				stderr = orig_err;
				log_critical("Unable to rotate the error log. { file = %s }", log_file);
				pthread_mutex_unlock(&log_mutex);
				return;
			}
		}

		// Otherwise we ignore the standard output/error handles and reopen the log descriptor.
		else if (!(log_descriptor = freopen64(log_file, "a+", log_descriptor))) {
			log_critical("Unable to open the error log, falling back to console output. { file = %s }", log_file);
			pthread_mutex_unlock(&log_mutex);
			return;
		}
		pthread_mutex_unlock(&log_mutex);
	}

	return;
}

bool_t log_start(void) {

	FILE *file_out, *file_err;
	chr_t *log_file = MEMORYBUF(1024);

	// File logging.
	if (magma.output.file && magma.output.path) {

		log_date = time_datestamp();

		if (snprintf(log_file, 1024, "%s%smagmad.%lu.log", magma.output.path, (*(ns_length_get(magma.output.path) + magma.output.path) == '/') ? "" : "/",
			log_date) >= 1024) {
			log_critical("Log file path exceeded available buffer. { file = %s%smagmad.%lu.log }", magma.output.path,
				(*(ns_length_get(magma.output.path) + magma.output.path) == '/') ? "" : "/", log_date);
			return false;
		}

		if (folder_exists(NULLER(magma.output.path), false)) {
			log_critical("The path configured to hold the output log files does not exist. { path = %s }", magma.output.path);
			return false;
		}

		// If we are daemonizing, then we need repoint standard output/error at our designated log files.
		if (magma.system.daemonize) {
			if (!(file_out = freopen64(log_file, "a+", stdout))) {
				log_critical("Unable to open the error log, sticking with standard out. { file = %s }", log_file);
				return false;
			}

			if (!(file_err = freopen64(log_file, "a+", stderr))) {
				log_critical("Unable to open the error log, sticking with standard error. { file = %s }", log_file);
				fclose(file_out);
				return false;
			}

			stdout = file_out;
			stderr = file_err;
		}

		// Otherwise we track write to the log file using the log descriptor.
		else if (!(log_descriptor = fopen64(log_file, "a+"))) {
			log_critical("Unable to open the error log, falling back to console output. { file = %s }", log_file);
			return false;
		}

	}

	// We always close standard input.
	fclose(stdin);
	return true;
}
