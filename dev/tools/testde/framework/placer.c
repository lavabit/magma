
#include "framework.h"

inline unsigned char * data_pl(placer_t placer) {

	#ifdef DEBUG_FRAMEWORK
	if (sizeof(sizer_t) != 4 || sizeof(char *) != 4 || sizeof(int) != 4 || sizeof(placer_t) != 8) {
		lavalog("This platform has atomic type lengths that are incompatible with the placer_t functions.");
		return NULL;
	}
	#endif

	return (unsigned char *)placer.data;
}

inline sizer_t size_pl(placer_t placer) {

	sizer_t result;

	#ifdef DEBUG_FRAMEWORK
	if (sizeof(sizer_t) != 4 || sizeof(char *) != 4 || sizeof(int) != 4 || sizeof(placer_t) != 8) {
		lavalog("This platform has atomic type lengths that are incompatible with the placer_t functions.");
		return 0;
	}
	#endif

	result = placer.size;
	return result;
}

inline placer_t set_pl(char *pointer, sizer_t length) {

	placer_t result;

	#ifdef DEBUG_FRAMEWORK
	if (sizeof(sizer_t) != 4 || sizeof(char *) != 4 || sizeof(int) != 4 || sizeof(placer_t) != 8) {
		lavalog("This platform has atomic type lengths that are incompatible with the placer_t functions.");
		return 0;
	}
	#endif

	result.data = (long)pointer;
	result.size = length;
	return result;
}
