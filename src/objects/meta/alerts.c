
/**
 * @file /magma/objects/meta/alerts.c
 *
 * @brief	Functions for handling user alerts and warnings.
 */

#include "magma.h"

/**
 * @brief	Allocate and initialize a new user alert message object.
 * @param	alertnum	the numerical id of the user alert.
 * @param	type		a pointer to a managed string containing the type of the alert (e.g. "warning" or "alert").
 * @param	message		a pointer to a managed string containing the alert message.
 * @param	created		the UTC timecode for when the alert message was created.
 * @return	NULL on error or a pointer to the newly initialized user alert message object on success.
 */
meta_alert_t * alert_alloc(uint64_t alertnum, stringer_t *type, stringer_t *message, uint64_t created) {

	meta_alert_t *result;

	if (!(result = mm_alloc(align(16, sizeof(meta_alert_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(type) + 1) + align(8, st_length_get(message) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for an alert structure.", align(16, sizeof(meta_alert_t) + sizeof(placer_t) + sizeof(placer_t)) +
			align(8, st_length_get(type) + 1) + align(8, st_length_get(message) + 1));
		return NULL;
	}

	result->alertnum = alertnum;
	result->created = created;

	result->type = (placer_t *)((chr_t *)result + sizeof(meta_alert_t));
	result->message = (placer_t *)((chr_t *)result + sizeof(meta_alert_t) + sizeof(placer_t));

	((placer_t *)result->type)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->type)->length = st_length_get(type);
	((placer_t *)result->type)->data = (chr_t *)result + align(16, sizeof(meta_alert_t) + sizeof(placer_t) + sizeof(placer_t));

	((placer_t *)result->message)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->message)->length = st_length_get(message);
	((placer_t *)result->message)->data = (chr_t *)result + align(16, sizeof(meta_alert_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(type) + 1);

	mm_copy(st_data_get(result->type), st_data_get(type), st_length_get(type));
	mm_copy(st_data_get(result->message), st_data_get(message), st_length_get(message));

	return result;
}
