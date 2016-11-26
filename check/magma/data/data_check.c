
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
#include "message.2.h"
#include "message.3.h"
#include "message.4.h"
#include "message.5.h"
#include "message.6.h"
#include "message.7.h"
#include "message.8.h"
#include "message.9.h"
#include "message.10.h"
#include "message.11.h"
#include "message.12.h"
#include "message.13.h"
#include "message.14.h"
#include "message.15.h"
#include "message.16.h"

stringer_t *messages[] = {
	NULLER(MESSAGE_1),
	NULLER(MESSAGE_2),
	NULLER(MESSAGE_3),
	NULLER(MESSAGE_4),
	NULLER(MESSAGE_5),
	NULLER(MESSAGE_6),
	NULLER(MESSAGE_7),
	NULLER(MESSAGE_8),
	NULLER(MESSAGE_9),
	NULLER(MESSAGE_10),
	NULLER(MESSAGE_11),
	NULLER(MESSAGE_12),
	NULLER(MESSAGE_13),
	NULLER(MESSAGE_14),
	NULLER(MESSAGE_15),
	NULLER(MESSAGE_16)
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
