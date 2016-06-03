
/**
 * @file /magma/core/buckets/stacked.c
 *
 * @brief	An interface for handling FIFO stacks.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free a stacked list and all of its underlying data nodes.
 * @param	stack	a pointer to the stacked list to be freed.
 * @return	This function returns no value.
 */
void stacker_free(stacker_t *stack) {

	stacker_node_t *holder, *node;

	if (stack == NULL) {
		return;
	}
	else {
		node = stack->list;
	}

	// Iterate through and free.
	while (node != NULL) {
		if (stack->free_function != NULL) {
			stack->free_function(node->data);
		}
		holder = node;
		node = (stacker_node_t *)node->next;
		mm_free(holder);
	}

	mutex_destroy(&(stack->mutex));
	mm_free(stack);
	return;
}

/**
 * @brief	Allocate a new instance of a stacked list.
 * @param	free_function	if not NULL, a pointer to a function that will be used to free the data underlying each node in the stacked list.
 * @return	NULL on failure, or a pointer to the newly created stack list on success.
 */
stacker_t * stacker_alloc(void *free_function) {

	stacker_t *result;

	if ((result = mm_alloc(sizeof(stacker_t))) == NULL) {
		log_pedantic("Unable to allocate %zu bytes for a stacked list.", sizeof(stacker_t));
		return NULL;
	}

	// Initialize the mutex.
	if (mutex_init(&(result->mutex), NULL) != 0) {
		log_pedantic("Unable to initialize the mutex.");
		mm_free(result);
		return NULL;
	}

	// Store the free memory function pointer.
	else if (free_function != NULL) {
		result->free_function = free_function;
	}

	return result;
}

/**
 * @brief	Get the number of nodes in a stacked list.
 * @param	stack	a pointer to the stacked list to be queried.
 * @return	the number of nodes currently held by the specified stacked list.
 */
uint64_t stacker_nodes(stacker_t *stack) {

	uint64_t result;

	if (stack == NULL) {
		return 0;
	}

	mutex_get_lock(&(stack->mutex));
	result = stack->items;
	mutex_unlock(&(stack->mutex));
	return result;
}

/**
 * @brief	Push a new entry onto the end of a stacked list.
 * @param	stack	a pointer to the stacked list to be updated.
 * @param	data	a pointer to the data to associated with the new stacked list node.
 * @return	0 on failure or 1 on success.
 */
// QUESTION: Shouldn't this return a boolean value?
int_t stacker_push(stacker_t *stack, void *data) {

	stacker_node_t *holder, *node;

	if (stack == NULL || data == NULL) {
		log_pedantic("Passed a NULL pointer.");
		return 0;
	}
	else if ((node = mm_alloc(sizeof(stacker_node_t))) == NULL) {
		log_pedantic("Unable to allocate %zu bytes for a linked node.", sizeof(stacker_node_t));
		return 0;
	}

	// Store the data pointer.
	node->data = data;

	mutex_get_lock(&(stack->mutex));

	if (stack->list == NULL) {
		stack->last = stack->list = node;
	}
	else if (stack->last != NULL) {
		stack->last->next = (struct stacker_node_t *)node;
		stack->last = node;
	}
	else {
		log_error("Last was NULL, but the list pointer wasn't. Finding the end the old fashioned way.");
		holder = stack->list;
		while (holder->next != NULL) {
			holder = (stacker_node_t *)holder->next;
		}
		stack->last = node;
		holder->next = (struct stacker_node_t *)node;
	}
	stack->items++;
	mutex_unlock(&(stack->mutex));

	return 1;
}

/**
 * @brief	Pop the last entry off a stacked list.
 * @param	stack	a pointer to the stacked list to be queried.
 * @return	NULL on failure or the value of the last node in the stacked list.
 */
void * stacker_pop(stacker_t *stack) {

	void *result = NULL;
	stacker_node_t *node;

	if (stack == NULL) {
		return NULL;
	}

	mutex_get_lock(&(stack->mutex));
	if (stack->list != NULL) {
		node = stack->list;
		stack->list = (stacker_node_t *)node->next;
		if (stack->list == NULL) {
			stack->last = NULL;
		}
		result = node->data;
		mm_free(node);
		stack->items--;
	}
	mutex_unlock(&(stack->mutex));

	return result;
}
