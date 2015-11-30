
#include "framework.h"

extern int (*mysql_stmt_fetch_d)(MYSQL_STMT *stmt);
extern void (*mysql_free_result_d)(MYSQL_RES *result);
extern int (*mysql_stmt_store_result_d)(MYSQL_STMT *stmt);
extern const char * (*mysql_stmt_error_d)(MYSQL_STMT *stmt);
extern my_bool (*mysql_stmt_free_result_d)(MYSQL_STMT *stmt);
extern unsigned int (*mysql_num_fields_d)(MYSQL_RES *result);
extern my_ulonglong (*mysql_stmt_num_rows_d)(MYSQL_STMT *stmt);
extern MYSQL_FIELD * (*mysql_fetch_field_d)(MYSQL_RES *result);
extern MYSQL_RES * (*mysql_stmt_result_metadata_d)(MYSQL_STMT *stmt);
extern my_bool (*mysql_stmt_bind_result_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind);

// Free the result structure used to store prepared statement results.
inline void free_bind_result(unsigned number, MYSQL_BIND *result) {
	unsigned increment;
	for (increment = 0; increment < number; increment++) {
		if ((result + increment)->is_null != NULL) {
			free_bl((result + increment)->is_null);
		}
		if ((result + increment)->buffer != NULL) {
			free_bl((result + increment)->buffer);
		}
		if ((result + increment)->length != NULL) {
			free_bl((result + increment)->length);
		}
	}
	free_bl(result);
	return;
}

// Generate a result structure suitable for storing results.
unsigned generate_bind_result(MYSQL_STMT *stmt, MYSQL_BIND **result) {
	
	unsigned increment;
	unsigned field_count;
	MYSQL_RES *meta;
	MYSQL_BIND *output;
	MYSQL_FIELD *field;

	// Get the result meta data. Will tell us how many fields and of what type.
 	meta = mysql_stmt_result_metadata_d(stmt);
	if (meta == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get the result set meta data. %s", mysql_stmt_error_d(stmt));
		#endif
		return 0;
	}
	
	// How many fields.
	field_count = mysql_num_fields_d(meta);
	if (field_count == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to count the number of fields needed for the result. %s", mysql_stmt_error_d(stmt));
		#endif
		mysql_free_result_d(meta);
		return 0;
	}
	
	output = allocate_bl(field_count * sizeof(MYSQL_BIND));
	if (output == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate %i bytes for the result set structure.", field_count * sizeof(MYSQL_BIND));
		#endif
		mysql_free_result_d(meta);
		return 0;
	}
	
	for (increment = 0; increment < field_count; increment++) {
		field = mysql_fetch_field_d(meta);
		if (field == NULL) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("There should have been another field, but none was returned.");
			#endif
			free_bind_result(field_count, output);
			mysql_free_result_d(meta);
			return 0;
		}
	
		// Detect NULLs.
		(output + increment)->is_null = allocate_bl(sizeof(my_bool));
		
		// Setup the buffer.
		(output + increment)->buffer_type = field->type;
		(output + increment)->buffer = allocate_bl(field->max_length + 1);
		(output + increment)->buffer_length = field->max_length;
		
		// Setup the output length.
		(output + increment)->length = allocate_bl(sizeof(unsigned long));
	}
	
	// Free the meta data.
	mysql_free_result_d(meta);
	
	// Setup our output and return the number of fields.
	*result = output;
	return field_count;
}

// Allocate our special SQL result datatype. Rows is one based.
sql_result_t * allocate_sql(unsigned long long rows, unsigned fields) {
	
	sql_result_t *result;

	// No zero length strings.
	if (rows == 0 || fields == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to allocate a zero length sql result.");
		#endif
		return NULL;
	}
	
	// Do the allocation. Include room for two sizer_ts plus a terminating NULL.
	result = malloc(sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned) 
		+ rows * (sizeof(char *) + (fields * (sizeof(char *) + sizeof(sizer_t)))));
	
	// If memory was allocated clear.
	if (result != NULL) {
		clear_bl(result, sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned) 
			+ rows * (sizeof(char *) + (fields * (sizeof(char *) + sizeof(sizer_t)))));
		// Store the number of rows.
		*(unsigned long long *)result = rows;
		// Store the number of fields.
		*(unsigned *)(result + sizeof(unsigned long long) + sizeof(unsigned long long)) = fields;
	}
	
	// If no memory was allocated, discover that here.
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Could not allocate %llu bytes.", sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned) 
			+ rows * (sizeof(char *) + (fields * (sizeof(char *) + sizeof(sizer_t)))));
	}
	#endif
	
	return result;
}

// Returns the number of fields stored in the result.
inline unsigned field_count_sql(sql_result_t *sql) {
	
	if (sql == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	// Extract and return the field count.
	return *(unsigned *)(sql + sizeof(unsigned long long) + sizeof(unsigned long long));
}

// Free all of the data associated with the SQL result.
inline void free_sql(sql_result_t *sql) {
	
	sql_result_t *holder;
	unsigned fields;
	unsigned long long rows;
	unsigned long long increment;
	
	if (sql == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return;
	}
	
	// Keep track of how many rows and how many fields per row.
	rows = *(unsigned long long *)sql;
	fields = *(unsigned *)(sql + sizeof(unsigned long long) + sizeof(unsigned long long));
	
	// Advance past the result information.
	holder = sql + sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned);
	
	for (increment = 0; increment < rows; increment++) {
		
		// If there is a row buffer.
		if (*(char **)holder != NULL) {
			free(*(char **)holder);
		}
		
		// Advance to the next row.
		holder += sizeof(char *) + (fields * (sizeof(char *) + sizeof(sizer_t)));
	}
	
	free(sql);
	return;
}

// Get a specific row. Zero based.
inline sql_row_t * get_row_sql(sql_result_t *sql, unsigned long long row) {
	
	unsigned fc;
	
	if (sql == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	// Extract the field count.
	fc = *(unsigned *)(sql + sizeof(unsigned long long) + sizeof(unsigned long long));
	
	// Return a pointer to the row.
	return (sql + sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned)
		+ (row * (sizeof(char *) + (fc * (sizeof(char *) + sizeof(sizer_t))))));
}

// Fetches the next row.
inline sql_row_t * fetch_row_sql(sql_result_t *sql) {
	
	unsigned fc;
	unsigned long long cr;
	sql_row_t *result;

	if (sql == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	// Extract the field count.
	fc = *(unsigned *)(sql + sizeof(unsigned long long) + sizeof(unsigned long long));
	
	// Extract the current row.
	cr = *(unsigned long long *)(sql + sizeof(unsigned long long));
	
	// We've hit the end, reset the counter and return NULL.
	if (cr >= *(unsigned long long *)sql) {
		*(unsigned long long *)(sql + sizeof(unsigned long long)) = 0;
		return NULL;
	}
	
	// Store the result, increment the counter and return the row.
	result = (sql + sizeof(unsigned long long) + sizeof(unsigned long long) + sizeof(unsigned)
		+ (cr * (sizeof(char *) + (fc * (sizeof(char *) + sizeof(sizer_t))))));
	(*(unsigned long long *)(sql + sizeof(unsigned long long)))++;
	return result;
}

// Set the fetch row pointer.
inline void set_row_sql(sql_result_t *sql, unsigned long long row) {
	
	if (sql == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (row >= row_count_sql(sql)) {
		lavalog("Asked to set the row pointer to %llu while only %llu rows were allocated.", row, row_count_sql(sql));
	}
	#endif
	
	*(unsigned long long *)(sql + sizeof(unsigned long long)) = row;
	return;
}
	
// Sets the buffer location for a specific row.
inline void set_buf_sql(sql_row_t *sql, char *buffer) {
	
	if (sql == NULL || buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}
	
	// Set the row buffer.
	*(char **)sql = buffer;
	return;
}

// Get the length of a specific field.
inline sizer_t get_len_sql(sql_row_t *row, unsigned field) {

	sizer_t result;
	unsigned increment;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	result = *(sizer_t *)row;
	return result;
}

// Return a char pointer to the data.
inline char * get_char_sql(sql_row_t *row, unsigned field) {

	char *result;
	unsigned increment;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return NULL;
	}
	
	row += sizeof(sizer_t);
	result = *(char **)row;
	return result;
}

// Copies the data into a stringer and returns the stringer.
inline stringer_t * get_st_sql(sql_row_t *row, unsigned field) {

	sizer_t length;
	unsigned increment;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	length = *(sizer_t *)row;
	if (length == 0) {
		return NULL;
	}
	row += sizeof(sizer_t);
	
	#ifdef DEBUG_FRAMEWORK
	stringer_t *result;
	result = import_bl(*(char **)row, length);
	if (result == NULL) {
		lavalog("Unable to import the data into a stringer.");
	}
	return result;
	#else
	return import_bl(*(char **)row, length);
	#endif
}

// Returns a double.
inline double get_d_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	double result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(double)) {
		lavalog("Attempting to access an double that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(double **)row;
	return result;
}

// Returns a double.
inline float get_f_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	float result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(float)) {
		lavalog("Attempting to access an float that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(float **)row;
	return result;
}

// Returns an unsigned long long.
inline unsigned long long get_ull_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	unsigned long long result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(unsigned long long)) {
		lavalog("Attempting to access an unsigned long long that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(unsigned long long **)row;
	return result;
}

// Returns a signed long long.
inline long long get_ll_sql(sql_row_t *row, unsigned field) {

	long long result;
	unsigned increment;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(long long)) {
		lavalog("Attempting to access an long long that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(long long **)row;
	return result;
}

// Returns an unsigned long.
inline unsigned long get_ul_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	unsigned long result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(unsigned long)) {
		lavalog("Attempting to access an unsigned long that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(unsigned long **)row;
	return result;
}

// Returns a signed long.
inline long get_l_sql(sql_row_t *row, unsigned field) {

	long result;
	unsigned increment;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(long)) {
		lavalog("Attempting to access an long that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(long **)row;
	return result;
}

// Returns an unsigned int.
inline unsigned get_ui_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	unsigned result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(unsigned)) {
		lavalog("Attempting to access an unsigned int that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(unsigned **)row;
	return result;
}

// Returns a signed int.
inline int get_i_sql(sql_row_t *row, unsigned field) {

	int result;
	unsigned increment;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(int)) {
		lavalog("Attempting to access an int that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(int **)row;
	return result;
}

// Returns an unsigned short.
inline unsigned short get_us_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	unsigned short result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(unsigned short)) {
		lavalog("Attempting to access an unsigned short that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(unsigned short **)row;
	return result;
}

// Returns a signed short.
inline short get_s_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	short result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(short)) {
		lavalog("Attempting to access an short that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(short **)row;
	return result;
}

// Returns an unsigned char.
inline unsigned char get_uc_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	unsigned char result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(unsigned char)) {
		lavalog("Attempting to access an unsigned char that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(unsigned char **)row;
	return result;
}

// Returns a signed char.
inline char get_c_sql(sql_row_t *row, unsigned field) {

	unsigned increment;
	char result;
	
	if (row == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}

	row += sizeof(char *);
	for (increment = 0; increment < field; increment++) {
		row += sizeof(sizer_t);
		row += sizeof(char *);
	}
	
	if (*(sizer_t *)row == 0) {
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (*(sizer_t *)row != sizeof(char)) {
		lavalog("Attempting to access an char that is an improper size.");
	}
	#endif
	
	row += sizeof(sizer_t);
	result = **(char **)row;
	return result;
}

// Return the number of rows in the SQL result. One based number.
inline unsigned long long row_count_sql(sql_result_t *sql) {
	
	if (sql == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	// Extract and return the row count.
	return *(unsigned long long*)sql;
}

// Stores an individual SQL row in the result structure.
int store_sql_row(unsigned long long row, sql_result_t *result, MYSQL_BIND *output) {
	
	char *buffer;
	sql_row_t *cur_row;
	unsigned increment;
	unsigned field_count;
	sizer_t buf_length = 0;
	
	#ifdef DEBUG_FRAMEWORK
	if (row >= row_count_sql(result)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to store row %llu while only %llu rows were allocated.", row, row_count_sql(result));
		#endif
		return 0;
	}
	#endif
	
	// Get the field count, then iterate through and calculate how big a buffer we'll need.
	field_count = field_count_sql(result);
	for (increment = 0; increment < field_count; increment++) {
		buf_length += *((output + increment)->length);
	}
	
	// Allocate a buffer big enough for the row.
	buffer = allocate_bl(buf_length);
	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer to store the row.");
		#endif
		return 0;
	}
	
	// Get the row pointer.
	cur_row = get_row_sql(result, row);
	
	// Set the buffer for the row.
	set_buf_sql(cur_row, buffer);
	
	// Advance past the row buffer.
	cur_row += sizeof(char *);
	
	for (increment = 0; increment < field_count; increment++) {
		
		// If the value isn't null.
		if (*((output + increment)->is_null) != 1) {
			// Store the length.
			*(sizer_t *)cur_row = (sizer_t)*((output + increment)->length);
			cur_row += sizeof(sizer_t);
			
			// Copy into the buffer.
			memcpy(buffer, (output + increment)->buffer, *((output + increment)->length));
			*(char **)cur_row = buffer;
			buffer += *((output + increment)->length);
			cur_row += sizeof(char *);
		}
		// Its NULL, so we leave the size equal to zero and the pointer equal to NULL.
		else {
			cur_row += sizeof(sizer_t);
			cur_row += sizeof(char *);
		}
	}
	
	return 1;
}

// Stores the result set of a prepared statement.
sql_result_t * store_db_result(MYSQL_STMT *stmt) {
	
	MYSQL_BIND *output;
	unsigned long long row_count;
	unsigned field_count, increment;
	sql_result_t *result = NULL;
	
	// Store the result.
	if (mysql_stmt_store_result_d(stmt) != 0) {
		printf("Could not store the result.\n");
		return NULL;
	}
	
	// Generate a bind structure.
	field_count = generate_bind_result(stmt, &output);
	if (field_count == 0) {
		mysql_stmt_free_result_d(stmt);
		printf("Unable to store the results.\n");
		return NULL;
	}
	
	// Bind.
	if (mysql_stmt_bind_result_d(stmt, output) != 0) {
		printf("Error binding result. %s\n", mysql_stmt_error_d(stmt));
		mysql_stmt_free_result_d(stmt);
		free_bind_result(field_count, output);
		return NULL;
	}
	
	// How many rows?
	row_count = mysql_stmt_num_rows_d(stmt);
	
	// No rows were found.
	if (row_count == 0) {
		mysql_stmt_free_result_d(stmt);
		free_bind_result(field_count, output);
		return NULL;
	}
	
	// Allocate a result structure big enough for the number of fields and rows.
	result = allocate_sql(row_count, field_count);
	
	// Increment through each row and store.
	for (increment = 0; increment < row_count; increment++) {
			
		// Fetch the row.
		if (mysql_stmt_fetch_d(stmt) != 0) {
			printf("Error fetching result. %s\n", mysql_stmt_error_d(stmt));
			free_sql(result);
			mysql_stmt_free_result_d(stmt);
			free_bind_result(field_count, output);
			return NULL;
		}
		
		// Store the row.
		if (store_sql_row(increment, result, output) != 1) {
			printf("Error storing row.\n");
			free_sql(result);
			mysql_stmt_free_result_d(stmt);
			free_bind_result(field_count, output);
			return NULL;
		}	
	}
	
	// Cleanup.
	mysql_stmt_free_result_d(stmt);
	free_bind_result(field_count, output);
	
	return result;
}
