
#include "framework.h"

int continue_indicator = 1;
pthread_mutex_t continue_mutex = PTHREAD_MUTEX_INITIALIZER;

int continue_processing(int value) {
	
	int result;

	if (value != 0) {
		pthread_mutex_lock(&continue_mutex);
		continue_indicator = value;
		pthread_mutex_unlock(&continue_mutex);
		result = value;
	}
	else {
		pthread_mutex_lock(&continue_mutex);
		result = continue_indicator;
		pthread_mutex_unlock(&continue_mutex);
	}

	return result;
}
