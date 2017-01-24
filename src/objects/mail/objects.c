
/**
 * @file /magma/objects/mail/objects.c
 *
 * @brief Functions used to interface with and manage message data.
 */

#include "magma.h"

/**
 * @brief	Destroy an smtp message and free all of its underlying data.
 * @param	message		a pointer to the smtp message to be destroyed.
 * @return	This function returns no value.
 */
void mail_destroy_message(smtp_message_t *message) {

	if (message) {
		st_cleanup(message->text);
		st_cleanup(message->id);
		mm_free(message);
	}

	return;
}

/**
 * @note	This function is not currently referenced by any other code.
 */
void mail_setup_basic(basic_message_t *message, stringer_t *text) {

	chr_t *stream;
	int_t next = 1;
	size_t length;
	size_t increment;

	message->text = text;
	length = mail_header_end(text);
	stream = st_char_get(message->text);

	// Increment through. When key headers are found, store the locations in placer's.
	for (increment = 0; increment < length; increment++) {

		// Locate the start of headers.
		if (next == 1 && *stream != '\n') {

			if (length - increment >= 3 && mm_cmp_ci_eq(stream, "To:", 3) == 0) {
				message->to = mail_store_header(stream + 3, length - increment - 3);
			}
			else if (length - increment >= 5 && mm_cmp_ci_eq(stream, "From:", 5) == 0) {
				message->from = mail_store_header(stream + 5, length - increment - 5);
			}
			else if (length - increment >= 5 && mm_cmp_ci_eq(stream, "Date:", 5) == 0) {
				message->date = mail_store_header(stream + 5, length - increment - 5);
			}
			else if (length - increment >= 8 && mm_cmp_ci_eq(stream, "Subject:", 8) == 0) {
				message->subject = mail_store_header(stream + 8, length - increment - 8);
			}

			next = 0;
		}

		// So we check the start of the next line for a key header.
		if (next == 0 && (*stream == '\n' || *stream == '\r')) {
			next = 1;
		}

		stream++;
	}

	return;
}

/**
 * @brief	Free a mail message and all its underlying data.
 * @param	message		a pointer to the mail message object to be freed.
 * @return	This function returns no value.
 */
void mail_destroy(mail_message_t *message) {

	if (message) {
		st_cleanup(message->text);

		if (message->mime) {
			mail_mime_free(message->mime);
		}

		mm_free(message);
	}

	return;
}

/**
 * @brief	Parse a raw mail data string and return a new mail message object containing the processed message.
 * @param	text	a pointer to the managed string containing the raw (uncompressed) mail data to be parsed.
 * @return	NULL on failure or a pointer to a newly allocated mail message object with the message on success.
 */
mail_message_t * mail_message(stringer_t *text) {

	chr_t *stream;
	int_t next = 1;
	size_t length;
	size_t increment;
	mail_message_t *result;

	if (!text) {
		log_pedantic("Invalid message passed in.");
		return NULL;
	}

	if (!(result = mm_alloc(sizeof(mail_message_t)))) {
		log_pedantic("Unable to allocate %zu bytes for the SMTP message structure.", sizeof(smtp_message_t));
		return NULL;
	}

	// Setup the message structure.
	result->text = text;
	result->header_length = mail_header_end(result->text);

	length = result->header_length;
	stream = st_char_get(result->text);

	if (length > st_length_get(result->text)) {
		log_pedantic("The header length is longer than the message.");
		mm_free(result);
		return NULL;
	}

	// Increment through. When key headers are found, store the locations in placer's.
	for (increment = 0; increment < length; increment++) {

		// Locate the start of headers.
		if (next == 1 && *stream != '\n') {

			if (length - increment >= 3 && mm_cmp_ci_eq(stream, "To:", 3) == 0) {
				result->to = mail_store_header(stream + 3, length - increment - 3);
			}
			else if (length - increment >= 5 && mm_cmp_ci_eq(stream, "From:", 5) == 0) {
				result->from = mail_store_header(stream + 5, length - increment - 5);
			}
			else if (length - increment >= 5 && mm_cmp_ci_eq(stream, "Date:", 5) == 0) {
				result->date = mail_store_header(stream + 5, length - increment - 5);
			}
			else if (length - increment >= 8 && mm_cmp_ci_eq(stream, "Subject:", 8) == 0) {
				result->subject = mail_store_header(stream + 8, length - increment - 8);
			}

			next = 0;
		}

		// So we check the start of the next line for a key header.
		if (next == 0 && (*stream == '\n' || *stream == '\r')) {
			next = 1;
		}

		stream++;
	}

	return result;
}

/**
 * @brief	Create an smtp message object out of a buffer of raw data supplied via the smtp DATA command.
 * @note	This function will also generate a random message ID.
 * @param	text	a pointer to a managed string containing the smtp data to be parsed.
 * @return	NULL on failure or a pointer to the newly initialized smtp message object wrapping the data on success.
 */
smtp_message_t * mail_create_message(stringer_t *text) {

	smtp_message_t *result;

	if (!text) {
		log_pedantic("Invalid message passed in.");
		return NULL;
	}

	if (!(result = mm_alloc(sizeof(smtp_message_t)))) {
		log_pedantic("Unable to allocate %zu bytes for the SMTP message structure.", sizeof(smtp_message_t));
		return NULL;
	}

	// Setup the message structure.
	result->text = text;
	result->header_length = mail_header_end(result->text);
	result->id = rand_choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 12, NULL);

	if (!result->id) {
		log_pedantic("An error occurred while generating the message ID.");
		mm_free(result);
		return NULL;
	}

	if (!mail_headers(result)) {
		log_pedantic("An error occurred while scanning the message for its headers.");
		st_free(result->id);
		mm_free(result);
		return NULL;
	}

	return result;
}
