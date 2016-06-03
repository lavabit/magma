
/**
 * @file /magma/providers/parsers/json.c
 *
 * @brief JSON serialization.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Return the version string for libjansson.
 * @return	a pointer to a character string containing the libjansson version information.
 */
chr_t * lib_version_jansson(void) {
	return (chr_t *)jansson_version_d();
}

/**
 * @brief	Initialize the Jansson library and dynamically bind to the required symbols.
 * @return	true on success or false on failure.
 */
bool_t lib_load_jansson(void) {

	symbol_t jansson[] = {
		M_BIND(jansson_version), M_BIND(json_array), M_BIND(json_array_append),	M_BIND(json_array_append_new), M_BIND(json_array_clear),
		M_BIND(json_array_extend), M_BIND(json_array_get), M_BIND(json_array_insert), M_BIND(json_array_insert_new), M_BIND(json_array_remove),
		M_BIND(json_array_set),	M_BIND(json_array_set_new),	M_BIND(json_array_size), M_BIND(json_copy),	M_BIND(json_decref),
		M_BIND(json_deep_copy),	M_BIND(json_delete), M_BIND(json_dumpf), M_BIND(json_dump_file), M_BIND(json_dumps), M_BIND(json_equal),
		M_BIND(json_false),	M_BIND(json_incref), M_BIND(json_integer), M_BIND(json_integer_set), M_BIND(json_integer_value),
		M_BIND(json_loadf),	M_BIND(json_load_file),	M_BIND(json_loads),	M_BIND(json_null), M_BIND(json_number_value), M_BIND(json_object),
		M_BIND(json_object_clear), M_BIND(json_object_del),	M_BIND(json_object_get), M_BIND(json_object_iter), M_BIND(json_object_iter_at),
		M_BIND(json_object_iter_key), M_BIND(json_object_iter_next), M_BIND(json_object_iter_set), M_BIND(json_object_iter_set_new),
		M_BIND(json_object_iter_value),	M_BIND(json_object_set), M_BIND(json_object_set_new), M_BIND(json_object_set_new_nocheck),
		M_BIND(json_object_set_nocheck), M_BIND(json_object_size), M_BIND(json_object_update), M_BIND(json_pack), M_BIND(json_pack_ex),
		M_BIND(json_real), M_BIND(json_real_set), M_BIND(json_real_value), M_BIND(json_set_alloc_funcs), M_BIND(json_string),
		M_BIND(json_string_nocheck), M_BIND(json_string_set), M_BIND(json_string_set_nocheck), M_BIND(json_string_value), M_BIND(json_true),
		M_BIND(json_type_string), M_BIND(json_unpack), M_BIND(json_unpack_ex), M_BIND(json_vpack_ex), M_BIND(json_vunpack_ex)
	};

	if (lib_symbols(sizeof(jansson) / sizeof(symbol_t), jansson) != 1) {
		return false;
	}

	return true;
}
