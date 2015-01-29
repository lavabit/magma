
#include "framework.h"

inline short length_ull(unsigned long long number) {

	short length = 1;
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_ul(unsigned long long number) {
	
	short length = 1;
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_ui(unsigned int number) {
	
	short length = 1;
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_us(unsigned short number) {
	
	short length = 1;
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_ll(long long number) {
	
	short length = 1;
	
	if (number < 0) {
		length++;
	}
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_l(long number) {
	
	short length = 1;
	
	if (number < 0) {
		length++;
	}
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_i(int number) {
	
	short length = 1;
	
	if (number < 0) {
		length++;
	}
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}

inline short length_s(short number) {

	short length = 1;
	
	if (number < 0) {
		length++;
	}
	
	while((number /= 10) != 0) {
		length++;
	}
	
	return length;
}
