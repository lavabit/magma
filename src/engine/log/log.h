
/**
 * @file /magma/engine/log/log.h
 *
 * @brief	The function declarations and macros needed to access the error log subsystem.
 *
 * @Author	Ladar
 */

#ifndef MAGMA_CORE_LOG_H
#define MAGMA_CORE_LOG_H

// Options used to control the behavior of the log subsystem.
typedef enum {

	// The message severity level.
	M_LOG_PEDANTIC = 1,
	M_LOG_INFO = 2,
	M_LOG_WARN = 4,
	M_LOG_ERROR = 8,
	M_LOG_CRITICAL = 16,

	// Flags used to control what information is recorded.
	M_LOG_TIME = 32,
	M_LOG_FILE = 64,
	M_LOG_LINE = 128,
	M_LOG_FUNCTION = 256,
	M_LOG_STACK_TRACE = 512,

	// This flag forces a log message to output on the console, if available, even though the log file has been enabled.
	M_LOG_CONSOLE = 1024,

	// Contra flags can be used to overide a flag as needed.
	M_LOG_PEDANTIC_DISABLE = 2048,
	M_LOG_INFO_DISABLE = 4096,
	M_LOG_WARN_DISABLE = 8192,
	M_LOG_ERROR_DISABLE = 16384,
	M_LOG_CRITICAL_DISABLE = 32768,
	M_LOG_LINE_FEED_DISABLE = 65536,
	M_LOG_TIME_DISABLE = 131072,
	M_LOG_FILE_DISABLE = 262144,
	M_LOG_LINE_DISABLE = 524288,
	M_LOG_FUNCTION_DISABLE = 1048576,
	M_LOG_STACK_TRACE_DISABLE = 2097152

} M_LOG_OPTIONS;

// All of the different log levels.
#define MAGMA_LOG_LEVELS (M_LOG_PEDANTIC | M_LOG_INFO | M_LOG_WARN | M_LOG_ERROR | M_LOG_CRITICAL)

// log.c
void     log_internal(const char *file, const char *function, const int line, M_LOG_OPTIONS options, const char *format, ...) __attribute__((format (printf, 5, 6)));
void     log_disable(void);
void     log_enable(void);
void     log_rotate(void);
bool_t   log_start(void);
int_t    log_backtrace(FILE *output);

#undef log_pedantic
#undef log_check
#undef log_info
#undef log_error
#undef log_critical
#undef log_options

#ifdef MAGMA_PEDANTIC

// Macro used record debug information during development.
#define log_pedantic(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_PEDANTIC, __VA_ARGS__)

// Log an error message if the specified conditional evaluates to true.
#define log_check(expr) do { if (expr) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_PEDANTIC, __STRING (expr)); } while (0)

#else

#define log_pedantic(...) do {} while (0)
#define log_check(expr) do {} while (0)

#endif

// Used to record information related to daemon performance, and other epehemeral messages which are not the result,
// of a problem. Generally this involves logging extra information based on the daemon configuration.
#define log_info(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_INFO, __VA_ARGS__)

// Used to log warnings which affect the system daemon, but are not critical. In particularly, warnings are used
// hen valid, but potentially unwise configuration values are used, and or, when the server is running with a
// potentially insecure configuration.
#define log_warn(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_WARN, __VA_ARGS__)

// Used to log errors that may indicate a problem requiring user intervention to solve.
#define log_error(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_ERROR, __VA_ARGS__)

// Used to record errors that could cause system instability.
#define log_critical(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_CRITICAL, __VA_ARGS__)

// Used to override the globally configured log options for a specific entry.
#define log_options(options, ...) log_internal (__FILE__, __FUNCTION__, __LINE__, options, __VA_ARGS__)

#endif

