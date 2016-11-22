
/**
 * @file /magma/check/magma/data/data.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"
#include "message.1.h"

stringer_t *messages[] = {
	NULLER(MESSAGE_1)
};

uint32_t check_message_max(void) {
	return sizeof(messages) / sizeof(stringer_t *);
}

stringer_t *check_message_get(uint32_t index) {

	stringer_t *result = NULL;

	if (index < check_message_max()) {
		result = base64_decode(messages[index], NULL);
	}

	return result;
};
