
/**
 * @file /magma/objects/meta/alias.c
 *
 * @brief Functions used to handle the alias objects.
 */

#include "magma.h"

/**
 * @brief	Allocate and initialize a new user alias object.
 * @param	aliasnum	the numerical identifier of the user alias.
 * @param	address		a managed string containing the email address associated with the alias.
 * @param	display		a managed string containing the display name associated with the alias.
 * @param	selected	a flag specifying whether this alias is the default selection for the user.
 * @param	created		a UNIX timestamp for the creation of this alias.
 * @return	NULL on failure, or a pointer to the newly initialized user alias object on success.
 */
meta_alias_t *  alias_alloc(uint64_t aliasnum, stringer_t *address, stringer_t *display, int_t selected, uint64_t created) {

	meta_alias_t *result;

	if (!(result = mm_alloc(align(16, sizeof(meta_alias_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(address) + 1) + align(8, st_length_get(display) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for an alias structure.", align(16, sizeof(meta_alias_t) + sizeof(placer_t) + sizeof(placer_t)) +
			align(8, st_length_get(address) + 1) + align(8, st_length_get(display) + 1));
		return NULL;
	}

	result->aliasnum = aliasnum;
	result->selected = selected ? true : false;
	result->created = created;

	result->address = (placer_t *)((chr_t *)result + sizeof(meta_alias_t));
	result->display = (placer_t *)((chr_t *)result + sizeof(meta_alias_t) + sizeof(placer_t));

	((placer_t *)result->address)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->address)->length = st_length_get(address);
	((placer_t *)result->address)->data = (chr_t *)result + align(16, sizeof(meta_alias_t) + sizeof(placer_t) + sizeof(placer_t));

	((placer_t *)result->display)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->display)->length = st_length_get(display);
	((placer_t *)result->display)->data = (chr_t *)result + align(16, sizeof(meta_alias_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(address) + 1);

	mm_copy(st_data_get(result->address), st_data_get(address), st_length_get(address));
	mm_copy(st_data_get(result->display), st_data_get(display), st_length_get(display));

	return result;
}
