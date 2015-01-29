
/**
 * @file /magma/core/parsers/formats/nvp.c
 *
 * @brief	Interface to the name/value pair parser.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * on success the number of valid pairs is returned
 * on error -1 is returned
 */
int nvp_parse(nvp_t *nvp, stringer_t *data) {
	#ifdef MAGMA_PEDANTIC
	if (nvp == NULL) log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Attempted to access a NULL nvp pointer. Printing stack:");
	else if (data == NULL || st_length_get(data) == 0)	log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Attempted to access a NULL or zero length string. Printing stack:");
	#endif

	multi_t key;
	int r, count = 0;
	tok_state_t state;
	placer_t line, name, value;

	if (nvp == NULL || data == NULL || st_length_get(data) == 0)
		return -1;

	tok_pop_init_st(&state, data, nvp->tokens.line);
	r = tok_pop(&state, &line);
	key = mt_set_type(key, M_TYPE_STRINGER);

	while (r != -1) {

		value = name = pl_null();

		if (!pl_empty(line)) {

			if (tok_get_bl(pl_char_get(line), pl_length_get(line), nvp->tokens.value, 0, &name) != -1)
				name = pl_trim(name);

			if (tok_get_bl(pl_char_get(line), pl_length_get(line), nvp->tokens.value, 1, &value) != -1)
				value = pl_trim(value);

			if (!pl_empty(name) && !pl_starts_with_char(name, nvp->tokens.comment)) {

				if ((key.val.st = st_import(pl_data_get(name), pl_length_get(name))) == NULL)
					return -1;
				else if (inx_insert(nvp->pairs, key, st_import(pl_data_get(value), pl_length_get(value))) != true) {
					st_free(key.val.st);
					return -1;
				}

				count++;
				st_free(key.val.st);
			}
		}

		if (r == 0)
			r = tok_pop(&state, &line);
		else
			r = -1;
	}

	return count;
}

/**
 * @brief	Allocate a new name/value pair and initialize it with the default settings.
 * @note	Defaults use "\n" for a line separator, "#" for a comment starting character, and "=" as the assignment character.
 * @return	NULL on failure, or a pointer to the newly allocated name/value pair object.
 */
nvp_t * nvp_alloc() {

	nvp_t *result = NULL;

	if ((result = mm_alloc(sizeof(nvp_t))) == NULL) {
		log_info("Could not allocate %zu bytes.", sizeof(nvp_t));
		return NULL;
	}
	else if ((result->pairs = inx_alloc(M_INX_HASHED, &st_free)) == NULL) {
		log_info("Could not allocate the key/value index.");
		mm_free(result);
		return NULL;
	}

	// The default tokens for nvp data.
	result->tokens.line = '\n';
	result->tokens.value = '=';
	result->tokens.comment = '#';

	return result;
}

/**
 * @brief	Free a name/value pair object from memory.
 * @param	nvp	the name/value pair object to be freed.
 * @return	This function returns no value.
 */
void nvp_free(nvp_t *nvp) {

	if (nvp != NULL) {
		inx_free(nvp->pairs);
		mm_free(nvp);
	}
}

