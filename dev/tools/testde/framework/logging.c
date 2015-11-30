
#include "framework.h"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void lavalog_i(const char *file, const char *function, const int line, const char *format, ...) {
	
	va_list args;
	
	// Initialize our dynamic array.
	va_start(args, format);
	
	pthread_mutex_lock(&log_mutex);
	
	printf("(%s - %s - %i) = ", file, function, line);
	vprintf(format, args);
	printf("\n");
	fflush(stdout);
	
	pthread_mutex_unlock(&log_mutex);
	
	va_end(args);
	
	return;
}
