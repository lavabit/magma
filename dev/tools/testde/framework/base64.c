
#include "framework.h"

const int  BASE64_MAXLINE = 76;
const char BASE64_TAB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char BASE64_DETAB[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    62,        
    0, 0, 0,
    63,        
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
    0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    0, 0, 0, 0, 0, 0,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, };

stringer_t * encode_base64_st(const stringer_t *string) {
	
	int c1;
	int c2;
	int c3;
	int this_line = 0;
   sizer_t size;
	sizer_t position;
	sizer_t output_size = 0;
	stringer_t *output;
	unsigned char *input_holder;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// Size.
	size = used_st(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode a zero length string.");
		#endif
		return NULL;
	}
	
	// Allocate our output buffer.
	output = allocate_st((size * 4/3) + (((size * 4/3) / BASE64_MAXLINE) * 2) + 2);
	if (!output) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Lets get setup.
	input_holder = data_st(string);
	output_holder = data_st(output);
	
	// This will process three bytes at a time.
	for (position = 0; position < size / 3; ++position) {
       
		c1 = (*input_holder++)&0xFF;
      c2 = (*input_holder++)&0xFF;
      c3 = (*input_holder++)&0xFF;
       
      *output_holder++ = BASE64_TAB[c1>>2];
      *output_holder++ = BASE64_TAB[((c1<<4)|(c2>>4))&0x3F];
      *output_holder++ = BASE64_TAB[((c2<<2)|(c3>>6))&0x3F];
      *output_holder++ = BASE64_TAB[c3&0x3F];
        
		this_line += 4;
      output_size += 4;
		
		// If we go over the line length.
		if (this_line > BASE64_MAXLINE) {
      	*output_holder++ = '\r';
         *output_holder++ = '\n';
         this_line = 0;
			output_size += 2;
		}
   }
   
   // Encode the remaining one or two characters in the input buffer
   switch(size % 3) {
		
		case 0: 
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 2;
		break;
        
		case 1:
      c1 = (*input_holder++)&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)];
		*output_holder++ = '=';
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		case 2:
		c1 = (*input_holder++)&0xFF;
		c2 = (*input_holder++)&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)|((c2&0xF0)>>4)];
		*output_holder++ = BASE64_TAB[((c2&0x0F)<<2)];
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		default:
		lavalog("Switch statement did not execute correctly. This should never happen.");
		break;
	}
   
	// Set the used size.
	set_used_st(output, output_size);
	 
   return output;
}

stringer_t * decode_base64_st(const stringer_t *string) {
	
	// Variable section.
	int loop = 0;
	int value = 0;
	sizer_t size;
	sizer_t position;
	sizer_t output_size = 0;
	stringer_t *result;	
	unsigned char *input_holder;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// How much data are we iterating through?
	size = used_st(string);
	if (size < 4) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an encoded string that was less than four characters.");
		#endif
		return NULL;
	}
	
	// Allocate a big enough stringer.
	result = allocate_st((size * 3/4));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate a large enough stringer for the result.");
		#endif
		return NULL;
	}
	
	// Get setup.
	input_holder = data_st(string);
	output_holder = data_st(result);
	
	// Get four characters at a time from the input buffer and decode them.
   for (position = 0; position < size; position++) {
	
		// Only process legit base64 characters.
		if ((*input_holder >= 'A' && *input_holder <= 'Z') || (*input_holder >= 'a' && *input_holder <= 'z') || \
			(*input_holder >= '0' && *input_holder <= '9') || *input_holder == '+' || *input_holder == '/') {
			
			// Do the appropriate operation.
			switch (loop) {
				
				case 0:
				value = BASE64_DETAB[(int)*input_holder++] << 18;
				loop++;
				break;
				
				case 1:
				value += BASE64_DETAB[(int)*input_holder++] << 12;
				*output_holder++ = (value & 0x00ff0000) >> 16;
				output_size++;
				loop++;
				break;
				
	 			case 2:
				value += (unsigned int)BASE64_DETAB[(int)*input_holder++] << 6;
				*output_holder++ = (value & 0x0000ff00) >> 8;
				output_size++;
				loop++;
				break;
	
				case 3:
				value += (unsigned int)BASE64_DETAB[(int)*input_holder++];
				*output_holder++ = value & 0x000000ff;
				output_size++;
				loop = 0;
				break;
			
				default:	
				lavalog("Switch statement did not loop correctly. This should never happen.");
				loop = 0;
				break;
			}
		}
		else if (*input_holder == '=') {
			position = size;
		}
		else {
			input_holder++;
		}
	 
	}
 
	// Set the used size.
	set_used_st(result, output_size);
	 
   return result;
}

stringer_t * encode_base64_ns(const char *string) {
	
	int c1;
	int c2;
	int c3;
	int this_line = 0;
   sizer_t size;
	sizer_t position;
	sizer_t output_size = 0;
	sizer_t input_position = 0;
	stringer_t *output;
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// Size.
	size = size_ns(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode a zero length string.");
		#endif
		return NULL;
	}
	
	// Allocate our output buffer.
	output = allocate_st((size * 4/3) + (((size * 4/3) / BASE64_MAXLINE) * 2) + 5);
	if (!output) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Lets get setup.
	output_holder = data_st(output);
	
	// This will process three bytes at a time.
	for (position = 0; position < size / 3; ++position) {
       
		c1 = (*(string + (input_position++)))&0xFF;
      c2 = (*(string + (input_position++)))&0xFF;
      c3 = (*(string + (input_position++)))&0xFF;
       
      *output_holder++ = BASE64_TAB[c1>>2];
      *output_holder++ = BASE64_TAB[((c1<<4)|(c2>>4))&0x3F];
      *output_holder++ = BASE64_TAB[((c2<<2)|(c3>>6))&0x3F];
      *output_holder++ = BASE64_TAB[c3&0x3F];
        
		this_line += 4;
      output_size += 4;
		
		// If we go over the line length.
		if (this_line > BASE64_MAXLINE) {
      	*output_holder++ = '\r';
         *output_holder++ = '\n';
         this_line = 0;
			output_size += 2;
		}
   }
   
   // Encode the remaining one or two characters in the input buffer
   switch(size % 3) {
		
		case 0: 
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 2;
		break;
        
		case 1:
      c1 = (*(string + (input_position++)))&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)];
		*output_holder++ = '=';
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		case 2:
		c1 = (*(string + (input_position++)))&0xFF;
		c2 = (*(string + (input_position++)))&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)|((c2&0xF0)>>4)];
		*output_holder++ = BASE64_TAB[((c2&0x0F)<<2)];
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		default:
		lavalog("Switch statement did not execute correctly. This should never happen.");
		break;
	}
   
	// Set the used size.
	set_used_st(output, output_size);
	 
   return output;
	
}

stringer_t * decode_base64_ns(const char *string) {
	
	// Variable section.
	int loop = 0;
	int value = 0;
	sizer_t size;
	sizer_t position;
	sizer_t output_size = 0;
	sizer_t input_position = 0;
	stringer_t *result;	
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// How much data are we iterating through?
	size = size_ns(string);
	if (size < 4) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an encoded string that was less than four characters.");
		#endif
		return NULL;
	}
	
	// Allocate a big enough stringer.
	result = allocate_st((size * 3/4));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate a large enough stringer for the result.");
		#endif
		return NULL;
	}
	
	// Get setup.
	output_holder = data_st(result);
	
	// Get four characters at a time from the input buffer and decode them.
   for (position = 0; position < size; position++) {
	
		// Only process legit base64 characters.
		if ((*(string + input_position) >= 'A' && *(string + input_position) <= 'Z') || (*(string + input_position) >= 'a' && \
			*(string + input_position) <= 'z') || (*(string + input_position) >= '0' && *(string + input_position) <= '9') || \
			*(string + input_position) == '+' || *(string + input_position) == '/') {
			
			// Do the appropriate operation.
			switch (loop) {
				
				case 0:
				value = BASE64_DETAB[(int)*(string + (input_position++))] << 18;
				loop++;
				break;
				
				case 1:
				value += BASE64_DETAB[(int)*(string + (input_position++))] << 12;
				*output_holder++ = (value & 0x00ff0000) >> 16;
				output_size++;
				loop++;
				break;
				
	 			case 2:
				value += (unsigned int)BASE64_DETAB[(int)*(string + (input_position++))] << 6;
				*output_holder++ = (value & 0x0000ff00) >> 8;
				output_size++;
				loop++;
				break;
	
				case 3:
				value += (unsigned int)BASE64_DETAB[(int)*(string + (input_position++))];
				*output_holder++ = value & 0x000000ff;
				output_size++;
				loop = 0;
				break;
			
				default:	
				lavalog("Switch statement did not loop correctly. This should never happen.");
				loop = 0;
				break;
			}
		}
		else if (*(string + input_position) == '=') {
			position = size;
		}
		else {
			input_position++;
		}
	}
 
	// Set the used size.
	set_used_st(result, output_size);
	 
   return result;
}

stringer_t * encode_base64_st_amt(const stringer_t *string, const sizer_t amount) {
	
	int c1;
	int c2;
	int c3;
	int this_line = 0;
   sizer_t size;
	sizer_t position;
	sizer_t output_size = 0;
	stringer_t *output;
	unsigned char *input_holder;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// Size.
	size = used_st(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode a zero length string.");
		#endif
		return NULL;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (amount > size) {
		lavalog("Asked to encode more data than exists in the stringer. Going to just encode whats there.");
	}
	#endif
	
	if (amount < size) {
		size = amount;
	}
	
	// Allocate our output buffer.
	output = allocate_st((size * 4/3) + (((size * 4/3) / BASE64_MAXLINE) * 2) + 5);
	if (!output) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Lets get setup.
	input_holder = data_st(string);
	output_holder = data_st(output);
	
	// This will process three bytes at a time.
	for (position = 0; position < size / 3; ++position) {
       
		c1 = (*input_holder++)&0xFF;
      c2 = (*input_holder++)&0xFF;
      c3 = (*input_holder++)&0xFF;
       
      *output_holder++ = BASE64_TAB[c1>>2];
      *output_holder++ = BASE64_TAB[((c1<<4)|(c2>>4))&0x3F];
      *output_holder++ = BASE64_TAB[((c2<<2)|(c3>>6))&0x3F];
      *output_holder++ = BASE64_TAB[c3&0x3F];
        
		this_line += 4;
      output_size += 4;
		
		// If we go over the line length.
		if (this_line > BASE64_MAXLINE) {
      	*output_holder++ = '\r';
         *output_holder++ = '\n';
         this_line = 0;
			output_size += 2;
		}
   }
   
   // Encode the remaining one or two characters in the input buffer
   switch(size % 3) {
		
		case 0: 
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 2;
		break;
        
		case 1:
      c1 = (*input_holder++)&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)];
		*output_holder++ = '=';
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		case 2:
		c1 = (*input_holder++)&0xFF;
		c2 = (*input_holder++)&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)|((c2&0xF0)>>4)];
		*output_holder++ = BASE64_TAB[((c2&0x0F)<<2)];
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		default:
		lavalog("Switch statement did not execute correctly. This should never happen.");
		break;
	}
   
	// Set the used size.
	set_used_st(output, output_size);
	 
   return output;
}

stringer_t * decode_base64_st_amt(const stringer_t *string, const sizer_t amount) {
	
	// Variable section.
	int loop = 0;
	int value = 0;
	sizer_t size;
	sizer_t position;
	sizer_t output_size = 0;
	stringer_t *result;	
	unsigned char *input_holder;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// How much data are we iterating through?
	size = used_st(string);
	if (size < 4) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an encoded string that was less than four characters.");
		#endif
		return NULL;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (amount > size) {
		lavalog("Asked to decode more data than exists in the stringer. Going to just decode whats there.");
	}
	#endif
	
	if (amount < size) {
		size = amount;
	}
	
	// Allocate a big enough stringer.
	result = allocate_st((size * 3/4));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate a large enough stringer for the result.");
		#endif
		return NULL;
	}
	
	// Get setup.
	input_holder = data_st(string);
	output_holder = data_st(result);
	
	// Get four characters at a time from the input buffer and decode them.
   for (position = 0; position < size; position++) {
	
		// Only process legit base64 characters.
		if ((*input_holder >= 'A' && *input_holder <= 'Z') || (*input_holder >= 'a' && *input_holder <= 'z') || \
			(*input_holder >= '0' && *input_holder <= '9') || *input_holder == '+' || *input_holder == '/') {
			
			// Do the appropriate operation.
			switch (loop) {
				
				case 0:
				value = BASE64_DETAB[(int)*input_holder++] << 18;
				loop++;
				break;
				
				case 1:
				value += BASE64_DETAB[(int)*input_holder++] << 12;
				*output_holder++ = (value & 0x00ff0000) >> 16;
				output_size++;
				loop++;
				break;
				
	 			case 2:
				value += (unsigned int)BASE64_DETAB[(int)*input_holder++] << 6;
				*output_holder++ = (value & 0x0000ff00) >> 8;
				output_size++;
				loop++;
				break;
	
				case 3:
				value += (unsigned int)BASE64_DETAB[(int)*input_holder++];
				*output_holder++ = value & 0x000000ff;
				output_size++;
				loop = 0;
				break;
			
				default:	
				lavalog("Switch statement did not loop correctly. This should never happen.");
				loop = 0;
				break;
			}
		}
		else if (*input_holder == '=') {
			position = size;
		}
		else {
			input_holder++;
		}
	 
	}
 
	// Set the used size.
	set_used_st(result, output_size);
	 
   return result;
}

stringer_t * encode_base64_ns_amt(const char *string, const sizer_t amount) {
	
	int c1;
	int c2;
	int c3;
	int this_line = 0;
	sizer_t position;
	sizer_t output_size = 0;
	sizer_t input_position = 0;
	stringer_t *output;
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}

	// Allocate our output buffer.
	output = allocate_st((amount * 4/3) + (((amount * 4/3) / BASE64_MAXLINE) * 2) + 5);
	if (!output) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Lets get setup.
	output_holder = data_st(output);
	
	// This will process three bytes at a time.
	for (position = 0; position < amount / 3; ++position) {
       
		c1 = (*(string + (input_position++)))&0xFF;
      c2 = (*(string + (input_position++)))&0xFF;
      c3 = (*(string + (input_position++)))&0xFF;
       
      *output_holder++ = BASE64_TAB[c1>>2];
      *output_holder++ = BASE64_TAB[((c1<<4)|(c2>>4))&0x3F];
      *output_holder++ = BASE64_TAB[((c2<<2)|(c3>>6))&0x3F];
      *output_holder++ = BASE64_TAB[c3&0x3F];
        
		this_line += 4;
      output_size += 4;
		
		// If we go over the line length.
		if (this_line > BASE64_MAXLINE) {
      	*output_holder++ = '\r';
         *output_holder++ = '\n';
         this_line = 0;
			output_size += 2;
		}
   }
   
   // Encode the remaining one or two characters in the input buffer
   switch(amount % 3) {
		
		case 0: 
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 2;
		break;
        
		case 1:
      c1 = (*(string + (input_position++)))&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)];
		*output_holder++ = '=';
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		case 2:
		c1 = (*(string + (input_position++)))&0xFF;
		c2 = (*(string + (input_position++)))&0xFF;
		*output_holder++ = BASE64_TAB[(c1&0xFC)>>2];
		*output_holder++ = BASE64_TAB[((c1&0x03)<<4)|((c2&0xF0)>>4)];
		*output_holder++ = BASE64_TAB[((c2&0x0F)<<2)];
		*output_holder++ = '=';
		*output_holder++ = '\r';
		*output_holder++ = '\n';
		output_size += 6;
		break;
        
		default:
		lavalog("Switch statement did not execute correctly. This should never happen.");
		break;
	}
   
	// Set the used size.
	set_used_st(output, output_size);
	 
   return output;
	
}

stringer_t * decode_base64_ns_amt(const char *string, const sizer_t amount) {
	
	// Variable section.
	int loop = 0;
	int value = 0;
	sizer_t position;
	sizer_t output_size = 0;
	sizer_t input_position = 0;
	stringer_t *result;	
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}
	
	// Allocate a big enough stringer.
	result = allocate_st((amount * 3/4));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate a large enough stringer for the result.");
		#endif
		return NULL;
	}
	
	// Get setup.
	output_holder = data_st(result);
	
	// Get four characters at a time from the input buffer and decode them.
   for (position = 0; position < amount; position++) {
	
		// Only process legit base64 characters.
		if ((*(string + input_position) >= 'A' && *(string + input_position) <= 'Z') || (*(string + input_position) >= 'a' && \
			*(string + input_position) <= 'z') || (*(string + input_position) >= '0' && *(string + input_position) <= '9') || \
			*(string + input_position) == '+' || *(string + input_position) == '/') {
			
			// Do the appropriate operation.
			switch (loop) {
				
				case 0:
				value = BASE64_DETAB[(int)*(string + (input_position++))] << 18;
				loop++;
				break;
				
				case 1:
				value += BASE64_DETAB[(int)*(string + (input_position++))] << 12;
				*output_holder++ = (value & 0x00ff0000) >> 16;
				output_size++;
				loop++;
				break;
				
	 			case 2:
				value += (unsigned int)BASE64_DETAB[(int)*(string + (input_position++))] << 6;
				*output_holder++ = (value & 0x0000ff00) >> 8;
				output_size++;
				loop++;
				break;
	
				case 3:
				value += (unsigned int)BASE64_DETAB[(int)*(string + (input_position++))];
				*output_holder++ = value & 0x000000ff;
				output_size++;
				loop = 0;
				break;
			
				default:	
				lavalog("Switch statement did not loop correctly. This should never happen.");
				loop = 0;
				break;
			}
		}
		else if (*(string + input_position) == '=') {
			position = amount;
		}
		else {
			input_position++;
		}
	}
 
	// Set the used size.
	set_used_st(result, output_size);
	 
   return result;
}
