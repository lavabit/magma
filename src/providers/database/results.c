
/**
 * @file /magma/providers/database/results.c
 *
 * @brief	A collection of functions for handling MySQL result sets.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

row_t * res_field_generic(row_t *row, uint64_t field, size_t typesize) {

	if (!row) {
		log_info("A NULL pointer was passed in.");
		return 0;
	}

	row += sizeof(chr_t *);

	for (uint64_t i = 0; i < field; i++) {
		row += sizeof(size_t) + sizeof(chr_t *);
	}

	if (!*(size_t *)row) {
		return 0;
	}

	if (typesize && (*(size_t *)row != typesize)) {
		log_info("Attempted to access a database row field that is an improper size {%u vs. %u byte(s) requested}",
			(unsigned int)*(size_t *)row, (unsigned int)typesize);
		debug_hook();
	}

	row += sizeof(size_t);

	return row;
}

/**
 * @brief	Get the value of a specified field in a database result row as generic pointer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a generic pointer, or NULL on failure.
 */
void * res_field_block(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, 0);

	return (result ? *(chr_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as managed string.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a managed string, or NULL on failure.
 */
stringer_t * res_field_string(row_t *row, uint64_t field) {

	size_t length;

	if (!row) {
		log_info("A NULL pointer was passed in.");
		return NULL;
	}

	row += sizeof(chr_t *);

	for (uint64_t i = 0; i < field; i++) {
		row += sizeof(size_t) + sizeof(chr_t *);
	}

	if (!(length = *(size_t *)row)) {
		return NULL;
	}

	row += sizeof(size_t);

	return st_import(*(chr_t **)row, length);
}

/**
 * @brief	Get the value of a specified field in a database result row as a double.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a double, or 0 on failure.
 */
double_t res_field_double(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(double));

	return (result ? **(double **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as a float.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a float, or 0 on failure.
 */
float_t res_field_float(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(float));

	return (result ? **(float **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as an unsigned 64-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as an unsigned 64-bit integer, or 0 on failure.
 */
uint64_t res_field_uint64(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(uint64_t));

	return (result ? **(uint64_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as an unsigned 32-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as an unsigned 32-bit integer, or 0 on failure.
 */
uint32_t res_field_uint32(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(uint32_t));

	return (result ? **(uint32_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as an unsigned 16-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as an unsigned 16-bit integer, or 0 on failure.
 */
uint16_t res_field_uint16(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(uint16_t));

	return (result ? **(uint16_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as an unsigned 8-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as an unsigned 8-bit integer, or 0 on failure.
 */
uint8_t res_field_uint8(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(uint8_t));

	return (result ? **(uint8_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as a signed 64-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a signed 64-bit integer, or 0 on failure.
 */
int64_t res_field_int64(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(int64_t));

	return (result ? **(int64_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as a signed 32-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a signed 32-bit integer, or 0 on failure.
 */
int32_t res_field_int32(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(int32_t));

	return (result ? **(int32_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as a signed 16-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a signed 16-bit integer, or 0 on failure.
 */
int16_t res_field_int16(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(int16_t));

	return (result ? **(int16_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as a signed 8-bit integer.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a signed 8-bit integer, or 0 on failure.
 */
int8_t res_field_int8(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(int8_t));

	return (result ? **(uint8_t **)result : 0);
}

/**
 * @brief	Get the value of a specified field in a database result row as a boolean.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the value of the specified result row as a boolean, or false on failure.
 */
bool_t res_field_bool(row_t *row, uint64_t field) {

	row_t *result = res_field_generic(row, field, sizeof(int8_t));

	return (result ? **(bool_t **)result : 0);
}

/**
 * @brief	Get the length of a specified field in a database result row.
 * @param	row		a pointer to the database result row to be examined.
 * @param	field	the zero-based index of the field in the row to be queried.
 * @return	the length of the specified result row, or 0 on failure.
 */
size_t res_field_length(row_t *row, uint64_t field) {

	if (!row) {
		log_info("A NULL pointer was passed in.");
		return 0;
	}

	row += sizeof(chr_t *);

	for (uint64_t i = 0; i < field; i++) {
		row += sizeof(size_t) + sizeof(chr_t *);
	}

	return *(size_t *)row;
}

/**
 * @brief	Free a MySQL results table.
 * @param	table	the MySQL results table to be freed.
 * @return	This function does not return a value.
 */
void res_table_free(table_t *table) {

	table_t *holder;
	uint64_t fields, rows;

	if (!table) {
		log_info("Passed in a NULL pointer.");
		return;
	}

	// Keep track of how many rows and how many fields per row.
	rows = *(uint64_t *)table;
	fields = *(uint64_t *)(table + sizeof(uint64_t));

	// Advance past the result information.
	holder = table + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t);

	for (uint64_t i = 0; i < rows; i++) {

		// If there is a row buffer.
		if (*(chr_t **)holder != NULL) {
			mm_free(*(chr_t **)holder);
		}

		// Advance to the next row.
		holder += sizeof(chr_t *) + (fields * (sizeof(chr_t *) + sizeof(size_t)));
	}

	mm_free(table);

	return;
}

/**
 * @brief	Free a prepared MySQL statement result set binding.
 * @param	stmt	the input prepared MySQL statement.
 * @param	binding	the binding for the result set.
 * @param	number	the number of fields in the result set.
 * @return	This function does not return a value.
 */
void res_bind_free(MYSQL_STMT *stmt, MYSQL_BIND *binding, uint64_t number) {

	for (uint64_t i = 0; i < number; i++) {
		mm_cleanup((binding + i)->is_null);
		(binding + i)->is_null = NULL;
		mm_cleanup((binding + i)->error);
		(binding + i)->error = NULL;
		mm_cleanup((binding + i)->buffer);
		(binding + i)->buffer = NULL;
		mm_cleanup((binding + i)->length);
		(binding + i)->length = NULL;
	}

	mysql_stmt_bind_result_d(stmt, binding);
	mm_free(binding);

	return;
}

/*
 * @brief	Allocate a MySQL result table.
 * @note	Rows are one-indexed.
 * @param	row	the number of result rows.
 * @param	row	the number of result fields.
 * @return	NULL on failure, or a new result table pointer on success.
 */
table_t * res_table_alloc(uint64_t rows, uint64_t fields) {

	table_t *table;

	// No zero length strings.
	if (!fields) {
		log_info("Attempted to allocate a zero length result.");
		return NULL;
	}

	// Do the allocation. Include room for two sizer_ts plus a terminating NULL.
	if (!(table = mm_alloc(sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t) + rows * (sizeof(chr_t *) + (fields * (sizeof(chr_t *) + sizeof(size_t))))))) {
		log_info("Could not allocate %zu bytes.", sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t)
				+ rows * (sizeof(chr_t *) + (fields * (sizeof(chr_t *) + sizeof(size_t)))));
		return NULL;
	}

	// Store the number of rows.
	*(uint64_t *)table = rows;

	// Store the number of fields.
	*(uint64_t *)(table + sizeof(uint64_t)) = fields;

	return table;
}

/*
 * @brief	Generate a result structure suitable for storing results.
 * @param	stmt	the prepared mysql statement that generated the results.
 * @param	result	a pointer to the binding where the results will be stored.
 * @return	0 on failure, or the number of fields present in the result set.
 */
uint64_t res_bind_create(MYSQL_STMT *stmt, MYSQL_BIND **result) {

	uint64_t fields;
	MYSQL_RES *meta;
	MYSQL_FIELD *field;
	MYSQL_BIND *binding;

	// Get the result meta data. Will tell us how many fields and of what type.
	if (!(meta = mysql_stmt_result_metadata_d(stmt))) {
		log_info("Unable to get the result set meta data. %s", mysql_stmt_error_d(stmt));
		return 0;
	}

	// How many fields.
	if (!(fields = mysql_num_fields_d(meta))) {
		log_info("Unable to count the number of fields needed for the result. %s", mysql_stmt_error_d(stmt));
		mysql_free_result_d(meta);
		return 0;
	}

	if (!(binding = mm_alloc(fields * sizeof(MYSQL_BIND)))) {
		log_info("Unable to allocate %zu bytes for the result set structure.", fields * sizeof(MYSQL_BIND));
		mysql_free_result_d(meta);
		return 0;
	}

	for (uint64_t i = 0; i < fields; i++) {
		field = mysql_fetch_field_d(meta);

		if (!field) {
			log_info("There should have been another field, but none was returned.");
			res_bind_free(stmt, binding, fields);
			mysql_free_result_d(meta);
			return 0;
		}

		// Note that if no rows are returned, the max_length parameter is set to zero, since a buffer is required to bind properly we use the length parameter instead.
		(binding + i)->buffer_type = field->type;

		// Make sure this flag is set properly. If an unsigned field exceeds its underlying type's signed limit the mysql_stmt_fetch() function
		// will fail with a truncation error.
		(binding + i)->is_unsigned = ((field->flags & (UNSIGNED_FLAG | NUM_FLAG)) == (UNSIGNED_FLAG | NUM_FLAG)) ? true : false;

		(binding + i)->is_null = mm_alloc(sizeof(my_bool));
		(binding + i)->error = mm_alloc(sizeof(my_bool));
		(binding + i)->length = mm_alloc(sizeof(unsigned long));

		(binding + i)->buffer = (field->max_length ? mm_alloc(field->max_length + 1) : mm_alloc((field->length > 255 ? 255 : field->length + 1)));
		//(binding + i)->buffer = mm_alloc(field->max_length + 1);
		(binding + i)->buffer_length = (field->max_length ? field->max_length : (field->length > 255 ? 255 : field->length));
		//(binding + i)->buffer_length = field->max_length;
	}

	// Free the meta data.
	mysql_free_result_d(meta);

	// Setup our output and return the number of fields.
	*result = binding;

	return fields;
}

/**
 * @brief	Return the number of fields stored in the database results table.
 * @param	table	the input database results table.
 * @return	0 on failure, or the number of table fields on success.
 */
uint64_t res_field_count(table_t *table) {

	if (!table) {
		log_info("A NULL pointer was passed in.");
		return 0;
	}

	// Extract and return the field count.
	return *(uint64_t *)(table + sizeof(uint64_t));
}

/**
 * @brief	Return the number of rows in the MySQL results table.
 * @note	The row count is provided for a one-indexed table.
 * @param	table	the input MySQL results table.
 * @return	0 on error, or the row count on success.
 */
uint64_t res_row_count(table_t *table) {

	if (!table) {
		log_info("A NULL pointer was passed in.");
		return 0;
	}

	// Extract and return the row count.
	return *(uint64_t *)table;
}

/**
 * @brief	Set the buffer location for a database result row.
 * @param	row		the input database result row.
 * @param	buffer	the buffer to be assigned to the target row.
 * @return	This function returns no value.
 */
void res_row_set(row_t *row, chr_t *buffer) {

	if (!row) {
		log_info("A NULL pointer was passed in.");
		return;
	}

	// Set the row buffer.
	*(chr_t **)row = buffer;

	return;
}

/**
 * @brief	Retrieve a row from a database results table by index.
 * @note	This function operates on a 0-indexed table.
 * @param	table	the input database  results table.
 * @row		row		the row # to be fetched.
 * @return	NULL on failure, or a pointer to a result row on success.
 */
row_t * res_row_get(table_t *table, uint64_t row) {

	uint64_t fields;

	if (!table) {
		log_info("A NULL pointer was passed in.");
		return NULL;
	}

	// Extract the field count.
	fields = *(uint64_t *)(table + sizeof(uint64_t));

	// Return a pointer to the row.
	return (table + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t) + (row * (sizeof(chr_t *) + (fields * (sizeof(chr_t *) + sizeof(size_t))))));
}

/**
 * @brief	Return the next row in the database results table and advance the cursor.
 * @param	table	the input mysql results table.
 * @return	a pointer to the next result row in the table.
 */
row_t * res_row_next(table_t *table) {

	row_t *row = NULL;
	uint64_t cursor, count;

	count = *(uint64_t *)table;
	cursor = *(uint64_t *)(table + sizeof(uint64_t) + sizeof(uint64_t));

	if (cursor < count) {
		row = res_row_get(table, cursor);
		*(uint64_t *)(table + sizeof(uint64_t) + sizeof(uint64_t)) = cursor + 1;
	}
	else {
		*(uint64_t *)(table + sizeof(uint64_t) + sizeof(uint64_t)) = 0;
	}

	return row;
}

// Stores an individual SQL row in the result structure.
bool_t res_row_store(uint64_t num, table_t *table, MYSQL_BIND *binding) {

	row_t *row;
	uint64_t fields;
	size_t blen = 0;
	chr_t *buffer = NULL;

	if (num >= res_row_count(table)) {
		log_info("Asked to store row %lu while only %lu rows were allocated.", num, res_row_count(table));
		return false;
	}

	// Get the field count, then iterate through and calculate how big a buffer we'll need.
	fields = res_field_count(table);

	for (uint64_t i = 0; i < fields; i++) {
		blen += *((binding + i)->length);
	}

	// It's possible to return a row with multiple fields but no data, so we must handle 0 blen specially.
	if (blen && !(buffer = mm_alloc(blen))) {
		log_info("Could not allocate a buffer to store the row.");
		return false;
	}

	// Get the row pointer.
	row = res_row_get(table, num);

	// Set the buffer for the row.
	res_row_set(row, buffer);

	// It's null so there's nothing else to do.
	if (!blen)
		return true;

	// Advance past the row buffer.
	row += sizeof(chr_t *);

	for (uint64_t i = 0; i < fields; i++) {

		// If the value isn't null.
		if ((*((binding + i)->is_null) != 1) && *((binding + i)->length)) {

			// Store the length.
			*(size_t *)row = (size_t)*((binding + i)->length);
			row += sizeof(size_t);

			// Copy into the buffer.
			mm_copy(buffer, (binding + i)->buffer, *((binding + i)->length));
			*(chr_t **)row = buffer;
			buffer += *((binding + i)->length);
			row += sizeof(chr_t *);
		}
		// Its NULL, so we leave the size equal to zero and the pointer equal to NULL.
		else {
			row += sizeof(size_t);
			row += sizeof(chr_t *);
		}

	}

	return true;
}

/*
 * @brief	Store the result set of a prepared MySQL statement in a results table.
 * @param	stmt	the prepared MySQL statement which was executed.
 * @return	NULL on failure, or a pointer to the results table.
 */
table_t * res_stmt_store(MYSQL_STMT *stmt) {

	int_t ret;
	MYSQL_BIND *binding;
	uint64_t rows, fields;
	table_t *table = NULL;

	// Store the result.
	if (mysql_stmt_store_result_d(stmt)) {
		log_info("Could not store the result.");
		return NULL;
	}

	// Generate a binding structure.
	if (!(fields = res_bind_create(stmt, &binding))) {
		log_info("Unable to store the results.");
		mysql_stmt_free_result_d(stmt);
		return NULL;
	}

	// Bind.
	if (mysql_stmt_bind_result_d(stmt, binding)) {
		log_info("Error binding result. %s", mysql_stmt_error_d(stmt));
		mysql_stmt_free_result_d(stmt);
		res_bind_free(stmt, binding, fields);
		return NULL;
	}

	// If no rows are found, return NULL.
	if (!(rows = mysql_stmt_num_rows_d(stmt))) {
		table = res_table_alloc(0, fields);
		mysql_stmt_free_result_d(stmt);
		res_bind_free(stmt, binding, fields);
		return table;
	}

	// Allocate a result structure big enough for the number of fields and rows.
	table = res_table_alloc(rows, fields);

	// Increment through each row and store.
	for (uint64_t i = 0; i < rows; i++) {

		// Fetch the row.
		if ((ret = mysql_stmt_fetch_d(stmt))) {

			if (ret == MYSQL_DATA_TRUNCATED) {
				log_info("Fetching the result failed with because the data would need to be truncated in order to fit within the allocated buffer.");
			}
			else {
				log_info("Error fetching result. %s", mysql_stmt_error_d(stmt));
			}

			res_table_free(table);
			mysql_stmt_free_result_d(stmt);
			res_bind_free(stmt, binding, fields);
			return NULL;
		}

		// Store the row.
		if (!res_row_store(i, table, binding)) {
			log_info("Error storing row.");
			res_table_free(table);
			mysql_stmt_free_result_d(stmt);
			res_bind_free(stmt, binding, fields);
			return NULL;
		}

	}

	mysql_stmt_free_result_d(stmt);
	res_bind_free(stmt, binding, fields);

	return table;
}

