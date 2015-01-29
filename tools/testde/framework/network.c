
#include "framework.h"

extern global_config_t config;

// Simple utility function to check whether we have recieved a complete line. Will
// return the number of bytes to the newline or 0 for no.
sizer_t complete_line(char *buffer, int length) {

	int position = 0;
	
	#ifdef DEBUG_FRAMEWORK
	if (buffer == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return 0;
	}
	#endif
	
	if (length == 0) return 0;
	
	while (length > 0 && *buffer != '\n') {
		*buffer++;
		length--;
		position++;
	}
	
	// If we hit a line break before the end of the buffer, return with the line length.
	if (*buffer == '\n' && length) return position + 1;
		
	return 0;
	
}

// This function takes a buffer and reads from the network. It will detect
// SSL sessions, and read from an SSL struct if one is found.
int session_read(session_common_t *session) {
	
	int readin;
	
	#ifdef DEBUG_FRAMEWORK
	if (session == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return -1;
	}
	#endif
	
	// Check to see if there are characters already in the buffer, and move them.
	if (session->current_line < session->buffered_bytes) {
		move_bytes(session->in_buffer, session->in_buffer + session->current_line, session->buffered_bytes - session->current_line);
		session->buffered_bytes -= session->current_line;
	}
	else {
		session->buffered_bytes = 0;
	}
	
	// Perform the read.
	if (session->ssl) {
		readin = ssl_read(session->ssl, session->in_buffer + session->buffered_bytes, config.in_buffer - session->buffered_bytes);
	}
	else {
		readin = read(session->sock_descriptor, session->in_buffer + session->buffered_bytes, config.in_buffer - session->buffered_bytes);
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (readin < 0) {
		lavalog("Received an error while reading from the socket. {errorno = %i}", errno);
	}
	#endif
	
	// Let the customer know the actual amount of data in the buffer.
	if (readin >= 0 && session->buffered_bytes != 0) {
		readin += session->buffered_bytes;
	}
	
	// Reset the counters.
	if (session->current_line || session->buffered_bytes) {
		session->current_line = 0;
		session->buffered_bytes = 0;
	}
	
	return readin;
}

// This function takes a buffer and reads from the network. It will detect
// SSL sessions, and read from an SSL struct if one is found.
int session_readline(session_common_t *session) {
	
	char *place;
	int readin;
	int characters = 0;
	int length = config.in_buffer;

	#ifdef DEBUG_FRAMEWORK
	if (session == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return -1;
	}
	#endif
	
	// Check to see if there are characters already in the buffer, and move them.
	if (session->current_line < session->buffered_bytes) {
		move_bytes(session->in_buffer, session->in_buffer + session->current_line, session->buffered_bytes - session->current_line);
		session->buffered_bytes -= session->current_line;
		characters = complete_line(session->in_buffer, session->buffered_bytes);
	}
	else {
		session->buffered_bytes = 0;
	}
		
	// Reset the current line counter, and advance the place holder to where we are supposed to write.
	session->current_line = 0;
	place = session->in_buffer + session->buffered_bytes;
	
	while (continue_processing(0) == 1 && !characters && length > session->buffered_bytes) {
	
		if (session->ssl) {
			readin = ssl_read(session->ssl, place, length - session->buffered_bytes);
		}
		else {
			readin = read(session->sock_descriptor, place, length - session->buffered_bytes);
		}
			
		if (readin < 0) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Received an error while reading from the socket.");
			#endif
			return -1;
		}
		else {
			session->buffered_bytes += readin;
			place += readin;
		}
		
		// Check whether we have a complete line to return yet.
		characters = complete_line(session->in_buffer, session->buffered_bytes);
	}
	
	// Tell the session how many characters are on the current line.
	if (length <= session->buffered_bytes) {
		return -1;
	}
	else {
		session->current_line = characters;
	}
	
	return characters;
}

// This function takes a buffer and writes it out to the network. It will detect
// SSL sessions, and write to an SSL struct if one is found.
int session_write(session_common_t *session, const stringer_t *string) {
	
	int written;
	int position = 0;
	sizer_t length;
	char *buffer;

	#ifdef DEBUG_FRAMEWORK
	if (session == NULL || string == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return -1;
	}
	#endif
	
	// Get the length.
	length = used_st(string);
	
	#ifdef DEBUG_FRAMEWORK
	if (length == 0) {
		lavalog("Asked to write a zero length stringer.");
		return 0;
	}
	#endif
	
	buffer = data_st(string);
	
	// Loop until bytes have been written to the socket.
	do {
		
		if (session->ssl) written = ssl_write(session->ssl, buffer + position, length);
		else written = write(session->sock_descriptor, buffer + position, length);
		
		if (written > 0) {
			length -= written;
			position += written;
		}
		else {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Not all of the bytes could be written to the socket.");
			#endif
			length = 0;
		}
			
	} while (length);
		
	#ifdef DEBUG_FRAMEWORK
	if (written <= 0) {
		lavalog("Received an error while reading from the socket.");
	}
	#endif
	
	return written;
}

// This function takes a buffer and writes it out to the network. It will detect
// SSL sessions, and write to an SSL struct if one is found.
int session_write_ns(session_common_t *session, const char *string, sizer_t length) {
	
	int written;
	int position = 0;

	#ifdef DEBUG_FRAMEWORK
	if (session == NULL || string == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return -1;
	}
	#endif
	
	#ifdef DEBUG_FRAMEWORK
	if (length == 0) {
		lavalog("Asked to write a zero length stringer.");
		return 0;
	}
	#endif
	
	// Loop until bytes have been written to the socket.
	do {
		
		if (session->ssl) written = ssl_write(session->ssl, string + position, length);
		else written = write(session->sock_descriptor, string + position, length);
		
		if (written > 0) {
			length -= written;
			position += written;
		}
		else {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Not all of the bytes could be written to the socket.");
			#endif
			length = 0;
		}
			
	} while (length);
		
	#ifdef DEBUG_FRAMEWORK
	if (written <= 0) {
		lavalog("Received an error while reading from the socket.");
	}
	#endif
	
	return written;
}

int session_printf(session_common_t *session, const char *format, ...) {
	
	int bytes;
	int written;
	va_list args;
	
	#ifdef DEBUG_FRAMEWORK
	if (session == NULL || format == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return -1;
	}
	#endif
	
	// Initialize our dynamic array.
	va_start(args, format);
	
	bytes = vsnprintf(session->out_buffer, config.out_buffer, format, args);
	
	if (session->ssl) written = ssl_write(session->ssl, session->out_buffer, bytes);
	else written = write(session->sock_descriptor, session->out_buffer, bytes);
	
	#ifdef DEBUG_FRAMEWORK
	if (written != bytes) {
		lavalog("Could not write %i bytes to the socket. write = %i", bytes, written);
	}
	#endif
	
	return written;
}

// Similar to the session function, this will write to a socket descriptor. It takes the socket explicitly. 
// This function is used by the message sending code.
int network_write_ns(int socket_descriptor, const char *buffer, sizer_t length) {
	
	int written;
	int position = 0;
	
	#ifdef DEBUG_FRAMEWORK
	if (socket_descriptor < 0 || buffer == NULL || length == 0) {
		lavalog("Invalid data passed in.");
		return -1;
	}
	#endif

	// Loop until bytes have been written to the socket.
	do {
		
		written = write(socket_descriptor, buffer + position, length);
		
		if (written > 0) {
			length -= written;
			position += written;
		}
		else {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Not all of the bytes could be written to the socket.");
			#endif
			length = 0;
		}
			
	} while (length);
	
	#ifdef DEBUG_FRAMEWORK
	if (written <= 0) {
		lavalog("Received an error while writing to the socket.");
	}
	#endif
	
	return written;
}

int network_write(int socket_descriptor, const stringer_t *string) {
	
	int written;
	char *buffer;
	sizer_t length;
	int position = 0;
	
	#ifdef DEBUG_FRAMEWORK
	if (socket_descriptor < 0 || string == NULL) {
		lavalog("Invalid data passed in.");
		return -1;
	}
	#endif
	
	buffer = data_st(string);
	length = used_st(string);

	// Loop until bytes have been written to the socket.
	do {
		
		written = write(socket_descriptor, buffer + position, length);
		
		if (written > 0) {
			length -= written;
			position += written;
		}
		else {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Not all of the bytes could be written to the socket.");
			#endif
			length = 0;
		}
			
	} while (length);
	
	#ifdef DEBUG_FRAMEWORK
	if (written <= 0) {
		lavalog("Received an error while writing to the socket.");
	}
	#endif
	
	return written;
}

// Similar to the session function, this will read from a socket descriptor. It takes the socket explicitly.
int network_read(int socket_descriptor, char *buffer, sizer_t length) {
	
	int readin;
	
	#ifdef DEBUG_FRAMEWORK
	if (socket_descriptor < 0 || buffer == NULL || length == 0) {
		lavalog("Invalid data passed in.");
		return -1;
	}
	#endif
	
	// Perform the read.
	readin = read(socket_descriptor, buffer, length);
	
	#ifdef DEBUG_FRAMEWORK
	if (readin < 0) {
		lavalog("Received an error while reading from the socket. {errno = %i}", errno);
	}
	#endif
	
	return readin;
}

// Similar to the session function, this will keep looping until it has read a line form a socket descriptor. It takes 
// the socket explicitly. This function is used by the message sending code.
int network_readline(int socket_descriptor, char *buffer, sizer_t buffer_length, sizer_t *line_length, sizer_t *buffered_bytes) {

	char *place;
	int readin;
	int characters = 0;
	
	#ifdef DEBUG_FRAMEWORK
	if (socket_descriptor < 0 || buffer == NULL || buffer_length == 0 || line_length == NULL || buffered_bytes == NULL) {
		lavalog("Invalid data passed in.");
		return -1;
	}
	#endif
	
	// Check to see if there are characters already in the buffer, and move them.
	if (*line_length < *buffered_bytes) {
		move_bytes(buffer, buffer + *line_length, *buffered_bytes - *line_length);
		*buffered_bytes -= *line_length;
		characters = complete_line(buffer, *buffered_bytes);
	}
	else {
		*buffered_bytes = 0;
	}
		
	// Reset the current line counter, and advance the place holder to where we are supposed to write.
	*line_length = 0;
	place = buffer + *buffered_bytes;
	
	while (continue_processing(0) && !characters && buffer_length > *buffered_bytes) {
	
		readin = read(socket_descriptor, place, buffer_length - *buffered_bytes);
			
		if (readin < 0) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Received an error while reading from the socket. {errno = %i}", errno);
			#endif
			return -1;
		}
		else {
			*buffered_bytes += readin;
			place += readin;
		}
		
		// Check whether we have a complete line to return yet.
		characters = complete_line(buffer, *buffered_bytes);
	}
	
	// Tell the session how many characters are on the current line.
	if (buffer_length <= *buffered_bytes) {
		return -1;
	}
	else {
		*line_length = characters;
	}

	return characters;
}

// Similar to the session function, this will take a variable number of arguments, and print them to a socket descriptor.
int network_printf(int socket_descriptor, char *buffer, sizer_t buffer_length, const char *format, ...) {
	
	int bytes;
	int written;
	va_list args;
	
	#ifdef DEBUG_FRAMEWORK
	if (socket_descriptor < 0 || buffer == NULL || buffer_length == 0 || format == NULL) {
		lavalog("Passed an invalid pointer. This should never happen.");
		return -1;
	}
	#endif
	
	// Initialize our dynamic array.
	va_start(args, format);
	
	bytes = vsnprintf(buffer, buffer_length, format, args);
	
	written = write(socket_descriptor, buffer, bytes);
	
	#ifdef DEBUG_NETWORK
	if (written != bytes) {
		slog("(network:network_printf) Could not write %i bytes to the socket. write = %i", bytes, written);
	}
	#endif
	
	return written;
}
