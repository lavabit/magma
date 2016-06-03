
/**
 * @file /magma/core/log/log.h
 *
 * @brief	The function declarations and macros needed to access the error log subsystem.
 *
 * @Author	Ladar
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_LOG_H
#define MAGMA_CORE_LOG_H

// Options used to control the behavior of the log subsystem.
typedef enum {
	M_LOG_PEDANTIC = 1,
	M_LOG_INFO,
	M_LOG_ERROR,
	M_LOG_CRITICAL,
	M_LOG_TIME,
	M_LOG_FILE,
	M_LOG_LINE,
	M_LOG_FUNCTION,
	M_LOG_STACK_TRACE,
	M_LOG_PEDANTIC_DISABLE,
	M_LOG_INFO_DISABLE,
	M_LOG_ERROR_DISABLE,
	M_LOG_CRITICAL_DISABLE,
	M_LOG_LINE_FEED_DISABLE,
	M_LOG_TIME_DISABLE,
	M_LOG_FILE_DISABLE,
	M_LOG_LINE_DISABLE,
	M_LOG_FUNCTION_DISABLE,
	M_LOG_STACK_TRACE_DISABLE
} M_LOG_OPTIONS;

// All of the different log levels.
#define MAGMA_LOG_LEVELS (M_LOG_PEDANTIC | M_LOG_INFO | M_LOG_ERROR | M_LOG_CRITICAL)

// log.c
int_t    print_backtrace();
void     log_internal(const char *file, const char *function, const int line, M_LOG_OPTIONS options, const char *format, ...) __attribute__((format (printf, 5, 6)));
void     log_disable(void);
void     log_enable(void);
void     log_rotate(void);
bool_t   log_start(void);
void     debug_hook(void);

#ifdef MAGMA_PEDANTIC

// Macro used record debug information during development.
#define log_pedantic(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_PEDANTIC, __VA_ARGS__)

// Log an error message if the specified conditional evaluates to true.
#define log_check(expr) do { if (expr) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_PEDANTIC, __STRING (expr)); } while (0)

#else

#define log_pedantic(...) do {} while (0)
#define log_check(expr) do {} while (0)

#endif

// Used to record information related to daemon performance.
#define log_info(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_INFO, __VA_ARGS__)

// Used to log errors that may indicate a problem requiring user intervention to solve.
#define log_error(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_ERROR, __VA_ARGS__)

// Used to record errors that could cause system instability.
#define log_critical(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_CRITICAL, __VA_ARGS__)

// Used to override the globally configured log options for a specific entry.
#define log_options(options, ...) log_internal (__FILE__, __FUNCTION__, __LINE__, options, __VA_ARGS__)

#endif

