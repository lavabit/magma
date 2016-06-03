
/**
 * @file /magma/core/buckets/arrays.c
 *
 * @brief 	A collection of functions used to create, maintain and safely utilize arrays of pointers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Allocate an array of a specified number of elements.
 * @param	size	the number of elements the array is capable of holding.
 * @return	NULL on failure, or a pointer to the newly allocated array on success.
 */
array_t * ar_alloc(size_t size) {

	array_t *result;

	// We can't create zero length arrays.
	if (!size) {
		log_pedantic("A size of zero was passed in.");
		return NULL;
	}

	// We can't have arrays longer than ARRAY_MAX_ELEMENTS.
	if (size >= ARRAY_MAX_ELEMENTS) {
		log_pedantic("An array size greater than %i was passed in.", ARRAY_MAX_ELEMENTS);
		return NULL;
	}

	// Allocate room for the size, the used number, and then a uint32_t plus a pointer for each element.
	result = malloc(sizeof(size_t) + sizeof(size_t) + (size * (sizeof(uint32_t) + sizeof(void *))));

	// Verify the allocation, clear and set the size parameter.
	if (result) {
		mm_wipe(result, sizeof(size_t) + sizeof(size_t) + (size * (sizeof(uint32_t) + sizeof(void *))));
		*(size_t *)result = size;
	}
	else {
		log_error("We were unable to allocate an array of %zu elements totaling %zu bytes.", size, sizeof(size_t) + sizeof(size_t) +
			(size * (sizeof(uint32_t) + sizeof(void *))));
	}

	return result;
}

/**
 * @brief	Get the maximum number of elements allocated in an array.
 * @param	array	a pointer to the array to be examined.
 * @return	NULL on failure, or the number of elements the array can hold.
 */
size_t ar_avail_get(array_t *array) {

	size_t size;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	size = *(size_t *)array;

	return size;
}

/**
 * @brief	Get the number of elements actively stored in an array.
 * @param	array	a pointer to the array to be counted.
 * @return	NULL on failure, or the number of elements currently held in the array.
 */
size_t ar_length_get(array_t *array) {

	size_t size;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	size = *(size_t *)(array + sizeof(size_t));

	return size;
}

/*
 * @brief	Get the data type for a specified element in an array.
 * @note	Valid array types include ARRAY_TYPE_ARRAY, ARRAY_TYPE_STRINGER, ARRAY_TYPE_SIZER, ARRAY_TYPE_NULLER,
 * 			ARRAY_TYPE_PLACER, and ARRAY_TYPE_POINTER.
 * @param	array	a pointer to the array to be examined.
 * @param	element	the index of the element in the array to be queried.
 * @return	the type of the specified array element, or ARRAY_TYPE_EMPTY on failure.
 *
 */
uint32_t ar_field_type(array_t *array, size_t element) {

	size_t len;
	uint32_t result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return ARRAY_TYPE_EMPTY;
	}

	// We store the array length so we can detect requests for the first empty slot without logging it.
	if (element >= (len = ar_length_get(array))) {// && element != 0) {
		log_pedantic("An invalid element number was passed in.");
		return ARRAY_TYPE_EMPTY;
	}
	// QUESTION: Remove code? This condition can never be reached.
	//else if (element == 0 && len == 0) {
	//	return ARRAY_TYPE_EMPTY;
	//}

	result = *(uint32_t *)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))));

	return result;
}

/**
 * @brief	Return the value of an element in an array that holds a managed string.
 * @param	array	a pointer to the array to be examined.
 * @element	element	the index of the element in the array to be queried.
 * @return	NULL on failure, or a pointer to a managed string with the element's data on success.
 */
stringer_t * ar_field_st(array_t *array, size_t element) {

	stringer_t *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if (ar_field_type(array, element) != ARRAY_TYPE_STRINGER) {
		log_pedantic("A stringer_t pointer was requested, but the type code does not match.");
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return NULL;
	}

	result = *(stringer_t **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Return the value of an element in an array that holds a null-terminated string.
 * @param	array	a pointer to the array to be examined.
 * @element	element	the index of the element in the array to be queried.
 * @return	NULL on failure, or a pointer to a null-terminated string with the element's data on success.
 */
chr_t * ar_field_ns(array_t *array, size_t element) {

	chr_t *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if (ar_field_type(array, element) != ARRAY_TYPE_NULLER) {
		log_pedantic("A stringer_t pointer was requested, but the type code does not match.");
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return NULL;
	}

	result = *(chr_t **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Return the value of an element in an array that holds another array.
 * @param	array	a pointer to the array to be examined.
 * @element	element	the index of the element in the array to be queried.
 * @return	NULL on failure, or a pointer to an array object with the element's data on success.
 */
array_t * ar_field_ar(array_t *array, size_t element) {

	array_t *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if (ar_field_type(array, element) != ARRAY_TYPE_ARRAY) {
		log_pedantic("A stringer_t pointer was requested, but the type code does not match.");
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return NULL;
	}

	result = *(array_t **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Return the value of an element in an array that holds a placer.
 * @param	array	a pointer to the array to be examined.
 * @element	element	the index of the element in the array to be queried.
 * @return	NULL on failure, or a pointer to a placer with the element's data on success.
 */
placer_t * ar_field_pl(array_t *array, size_t element) {

	placer_t *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	if (ar_field_type(array, element) != ARRAY_TYPE_PLACER) {
		log_pedantic("A placer_t pointer was requested, but the type code does not match.");
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return 0;
	}

	result = *(placer_t **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Return the value of an element in an array that holds a pointer.
 * @param	array	a pointer to the array to be examined.
 * @element	element	the index of the element in the array to be queried.
 * @return	NULL on failure, or a generic pointer to the element's data on success.
 */
void * ar_field_ptr(array_t *array, size_t element) {

	void *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	if (ar_field_type(array, element) != ARRAY_TYPE_POINTER) {
		log_pedantic("A pointer was requested, but the type code does not match.");
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return 0;
	}

	result = *(void **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Manually set the number of used elements in an array.
 * @param	array	a pointer to the array to be adjusted.
 * @param	used	the new number of elements (less than the array's maximum size) to be set as the array's used length.
 * @return	This function returns no value.
 */
void ar_length_set(array_t *array, size_t used) {

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return;
	}

	if (used > ar_avail_get(array)) {
		log_pedantic("Trying to set the used variable to %zu when the allocated size is %zu.", used, ar_avail_get(array));
	}

	*(size_t *)(array + sizeof(size_t)) = used;

	return;
}

/*
 * @brief	Add an item to an array by placing it in the first available slot, or by appending it.
 * @note	If a previously allocated array does not have enough available room for the new item, more space will be allocated automatically.
 * @note	Arrays passed as NULL will be replaced with a freshly allocated array of size 1 to hold the new element.
 * @param	array	a pointer to the address of an array object, which will be replaced with a newly allocated array if the specified array is NULL.
 * @param	type	the data type of the new element in the array.
 * @param	item	the data of the new element to be added to the array.
 * @return	0 on failure or 1 on success.
 */
int_t ar_append(array_t **array, uint32_t type, void *item) {

	size_t size;
	size_t increment = 0;
	array_t *holder = NULL;

	if (!array) {
		log_pedantic("An NULL pointer was passed in.");
		return 0;
	}

	if (type == ARRAY_TYPE_EMPTY) {
		log_pedantic("An invalid item was passed in.");
		return 0;
	}

	// We need to allocate a new array.
	if (!*array) {

		if (!(holder = ar_alloc(1))) {
			log_pedantic("We were unable to allocate a buffer for the array.");
			return 0;
		}

		size = 1;
	}
	// There is an array already, so check whether its full. If so allocate a bigger array and copy.
	else {
		size = ar_avail_get(*array);

		if (size == ar_length_get(*array)) {

			if (!(holder = ar_alloc(size + 1))) {
				log_pedantic("We were unable to allocate a new buffer of %zu bytes for the array.", size + 1);
				return 0;
			}

			mm_move(holder + sizeof(size_t), *array + sizeof(size_t), sizeof(size_t) + (size * (sizeof(uint32_t) + sizeof(void *))));
			size++;
		}

	}

	// If this is a new array.
	if (holder != NULL) {
		*(uint32_t *)(holder + sizeof(size_t) + sizeof(size_t) + ((size - 1) * (sizeof(uint32_t) + sizeof(void *)))) = type;
		*(void **)(holder + sizeof(size_t) + sizeof(size_t) + ((size - 1) * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t)) = item;
		ar_length_set(holder, size);
		if (*array == NULL) {
			*array = holder;
		}
		else {
			mm_free(*array);
			*array = holder;
		}
	}
	// Find the first empty element.
	else {
		while (increment < size && *(uint32_t *)(*array + sizeof(size_t) + sizeof(size_t) + (increment * (sizeof(uint32_t) + sizeof(void *)))) != ARRAY_TYPE_EMPTY) {
			increment++;
		}
		*(uint32_t *)(*array + sizeof(size_t) + sizeof(size_t) + (increment * (sizeof(uint32_t) + sizeof(void *)))) = type;
		*(void **)(*array + sizeof(size_t) + sizeof(size_t) + (increment * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t)) = item;
		ar_length_set(*array, ar_length_get(*array) + 1);
	}

	return 1;
}

/**
 * @brief	Free an array object and all of its underlying elements.
 * @note	Array elements that are managed strings, and aren't empty or of ARRAY_TYPE_POINTER will be freed.
 * @return	This function returns no value.
 */
void ar_free(array_t *array) {

	uint32_t type;
	size_t size;
	void *pointer;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return;
	}

	size = ar_avail_get(array);

	// Go through and free each pointer.
	while (size != 0) {
		type = *(uint32_t *)(array + sizeof(size_t) + sizeof(size_t) + ((size - 1) * (sizeof(uint32_t) + sizeof(void *))));
		pointer = *(void **)(array + sizeof(size_t) + sizeof(size_t) + ((size - 1) * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));
		if (type == ARRAY_TYPE_ARRAY && pointer != NULL) {
			ar_free(pointer);
		}
		else if ((type == ARRAY_TYPE_STRINGER || type == ARRAY_TYPE_PLACER) && pointer) {
			st_free(pointer);
		}
		// We make the assumption that if its a pointer type, its been freed by the consumer.
		else if (type != ARRAY_TYPE_EMPTY && type != ARRAY_TYPE_POINTER && pointer != NULL) {
			mm_free(pointer);
		}
		size--;
	}

	// Finally, free the buffer for the array itself.
	mm_free(array);

	return;
}

/**
 * @brief	Create a duplicate copy of the array.
 * @note	The new array might not correspond precisely to the layout of the original.
 * @note	Deep copies will be made of all elements that are also arrays.. Managed strinsg will be copied, but neither empty strings nor pointers will be.
 * @param	array	a pointer to the array to be copied.
 * @return	NULL on failure, or a pointer to the new copy of the array on success.
 */
array_t * ar_dupe(array_t *array) {

	size_t size;
	uint32_t type;
	void *pointer;
	array_t *result = NULL;

	if (!array || !(size = ar_avail_get(array)) || !(result = ar_alloc(size))) {
		return NULL;
	}

	// Go through and free each pointer.
	// QUESTION: "Free" each pointer? Also, why aren't the rest of the types being replicated?
	while (size != 0) {
		type = *(uint32_t *)(array + sizeof(size_t) + sizeof(size_t) + ((size - 1) * (sizeof(uint32_t) + sizeof(void *))));
		pointer = *(void **)(array + sizeof(size_t) + sizeof(size_t) + ((size - 1) * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));
		if (type == ARRAY_TYPE_ARRAY && pointer != NULL) {
			ar_append(&result, ARRAY_TYPE_ARRAY, ar_dupe(pointer));
		}
		else if (type == ARRAY_TYPE_STRINGER && pointer != NULL) {
			ar_append(&result, ARRAY_TYPE_STRINGER, st_dupe(pointer));
		}
		// We make the assumption that if its a pointer type, its been freed by the consumer.
		else if (type != ARRAY_TYPE_EMPTY && type != ARRAY_TYPE_POINTER && pointer != NULL) {
			ar_append(&result, type, pointer);
		}

		size--;
	}


	return result;
}
