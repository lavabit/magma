
#include "framework.h"

int extract_ull(stringer_t *string, unsigned long long *number) {
	
	int increment = 0;
	char *data;
	sizer_t length;
	unsigned long long add = 1;
	unsigned long long before;	
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_ull_ns(char *data, sizer_t length, unsigned long long *number) {
	
	int increment = 0;
	unsigned long long add = 1;
	unsigned long long before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_ul(stringer_t *string, unsigned long *number) {
	
	int increment = 0;
	char *data;
	sizer_t length;
	unsigned long add = 1;
	unsigned long before;	
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_ul_ns(char *data, sizer_t length, unsigned long *number) {
	
	int increment = 0;
	unsigned long add = 1;
	unsigned long before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_ui(stringer_t *string, unsigned int *number) {
	
	int increment = 0;
	char *data;
	sizer_t length;
	unsigned int add = 1;
	unsigned int before;	
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_ui_ns(char *data, sizer_t length, unsigned int *number) {
	
	int increment = 0;
	unsigned int add = 1;
	unsigned int before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_us(stringer_t *string, unsigned short int *number) {
	
	int increment = 0;
	char *data;
	sizer_t length;
	unsigned short int add = 1;
	unsigned short int before;	
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_us_ns(char *data, sizer_t length, unsigned short int *number) {
	
	int increment = 0;
	unsigned short int add = 1;
	unsigned short int before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check for numeric data.
		if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		
		// Calculate the new number.
		before = *number;
		*number += (*data-- - '0') * add;
		
		// Check for overflows.
		if (*number < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
		
		// Update the base ten position.
		before = add;
		add *= 10;
		
		if (add < before) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Numberic overflow.");
			#endif
			return -3;
		}
	}
	
	return 1;
}

int extract_ll(stringer_t *string, long long *number) {
	
	int increment = 0;
	long long add = 1;
	long long before;
	char *data;
	sizer_t length;
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_ll_ns(char *data, sizer_t length, long long *number) {
	
	int increment = 0;
	long long add = 1;
	long long before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_l(stringer_t *string, long *number) {
	
	int increment = 0;
	long add = 1;
	long before;
	char *data;
	sizer_t length;
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_l_ns(char *data, sizer_t length, long *number) {
	
	int increment = 0;
	long add = 1;
	long before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_i(stringer_t *string, int *number) {
	
	int increment = 0;
	int add = 1;
	int before;
	char *data;
	sizer_t length;
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_i_ns(char *data, sizer_t length, int *number) {
	
	int increment = 0;
	int add = 1;
	int before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_s(stringer_t *string, short int *number) {
	
	int increment = 0;
	short add = 1;
	short before;
	char *data;
	sizer_t length;
	
	if (!string || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An empty string was passed in.");
		#endif
		return -1;
	}
	
	data = data_st(string);
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

int extract_s_ns(char *data, sizer_t length, short int *number) {
	
	int increment = 0;
	short int add = 1;
	short int before;	
	
	if (!data || !length || !number) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed.");
		#endif
		return -1;
	}
	
	*number = 0;
	data += length - 1;
	
	for (increment = 0; increment < length; increment++) {
		// Check to see if the number leads with a negative sign. If so, multiply by -1.
		if (increment == (length - 1) && (*data == '+' || *data == '-')) {
			if (*data == '-') {
				*number *= -1;
			}
		}
		// Check for numeric data.
		else if (*data < '0' && *data > '9') {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Non numeric data found.");
			#endif
			return -2;
		}
		else {
			// Calculate the new number.
			before = *number;
			*number += (*data-- - '0') * add;
			
			// Check for overflows.
			if (*number < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
			
			// Update the base ten position.
			before = add;
			add *= 10;
			
			if (add < before) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Numberic overflow.");
				#endif
				return -3;
			}
		}
	}
	
	return 1;
}

// For extracting DB values.
inline stringer_t * extract_string(char *string) {
	
	if (string == NULL) {
		return NULL;
	}
	return import_ns(string);
}

inline long long extract_number(char *string) {
	
	long long result;
	
	if (string == NULL) {
		return 0;
	}
	
	if (extract_ll_ns(string, size_ns(string), &result) != 1) {
		return 0;
	}
	
	return result;
}

inline unsigned long long extract_unsigned_number(char *string) {
	
	unsigned long long result;
	
	if (string == NULL) {
		return 0;
	}
	
	if (extract_ull_ns(string, size_ns(string), &result) != 1) {
		return 0;
	}
	
	return result;
}
