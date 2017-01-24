
/**
 * @file /magma/objects/warehouse/datatier.c
 *
 * @brief	Functions used by the warehouse objects to access the database.
 */

#include "magma.h"

/**
 * @brief	Fetch the list of configured domains from the database.
 * @return	NULL on failure, or an inx object holding all the configured domains on success.
 */
inx_t * warehouse_fetch_domains(void) {

	row_t *row;
	inx_t *output;
	table_t *result;
	domain_t *record;
	multi_t key = { .type = M_TYPE_STRINGER };

	if (!(result = stmt_get_result(stmts.select_domains, NULL))) {
		log_pedantic("An error occurred while trying to fetch the list of configured domains.");
		return NULL;
	}
	else if (!(output = inx_alloc(M_INX_TREE, &mm_free))) {
		log_pedantic("No domains have been configured.");
		res_table_free(result);
		return NULL;
	}

	// Loop through each of the row returned.
	while ((row = res_row_next(result))) {

		// Log any invalid data that might be encountered.
		log_check(res_field_length(row, 0) >= MAGMA_HOSTNAME_MAX);
		log_check(res_field_int8(row, 1) != 0 && res_field_int8(row, 1) != 1);
		log_check(res_field_int8(row, 2) != 0 && res_field_int8(row, 2) != 1);
		log_check(res_field_int8(row, 3) != 0 && res_field_int8(row, 3) != 1);
		log_check(res_field_int8(row, 4) != 0 && res_field_int8(row, 4) != 1);
		log_check(res_field_int8(row, 5) != 0 && res_field_int8(row, 5) != 1);

		// Pass the domain name, wildcard, dkim, and spf_pool values and then insert the resulting domain record into the index.
		if ((record = domain_alloc(PLACER(res_field_block(row, 0), res_field_length(row, 0)), res_field_int8(row, 1),
				res_field_int8(row, 2),	res_field_int8(row, 3), res_field_int8(row, 4),	res_field_int8(row, 5))) &&
			(!(key.val.st = record->domain) || !inx_insert(output, key, record))) {
			log_info("The index refused to accept a domain record. {domain = %.*s}", (int_t)res_field_length(row, 0), (chr_t *)res_field_block(row, 0));
			res_table_free(result);
			mm_free(record);
			inx_free(output);
			return NULL;
		}
	}

	res_table_free(result);

	return output;
}

/**
 * @brief	Fetch the pattern list from the database.
 * @return	NULL on failure or a pointer to an inx holder containing a collection of managed strings with the patterns on success.
 */
inx_t * warehouse_fetch_patterns(void) {

	row_t *row;
	table_t *result;
	inx_t *patterns;
	stringer_t *holder;
	multi_t key = {	.type = M_TYPE_UINT64, .val.u64 = 0	};

	if (!(result = stmt_get_result(stmts.select_patterns, NULL))) {
		return NULL;
	}

	if (!(row = res_row_next(result))) {
		res_table_free(result);
		return NULL;
	}

	if (!(patterns = inx_alloc(M_INX_LINKED, &st_free))) {
		log_pedantic("Unable to allocate memory for the linked list of blocker patterns.");
		res_table_free(result);
		return NULL;
	}

	while (row && (holder = res_field_string(row, 0))) {

		if (inx_insert(patterns, key, holder) != 1) {
			log_pedantic("Unable to add the pattern stringer to the linked list.");
			st_free(holder);
			res_table_free(result);
			inx_free(patterns);
			return NULL;
		}

		key.val.u64++;
		row = res_row_next(result);
	}

	res_table_free(result);

	return patterns;
}
