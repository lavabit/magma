
/**
 * @file /check/magma/data/data_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
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
#include "message.17.h"
#include "message.18.h"

#define DKIM_TEST_NONE 0x00000000
#define DKIM_TEST_VERIFY 0x00000001
#define DKIM_TEST_SIGNING 0x00000010

typedef struct {
	uint32_t flags;
	stringer_t *data;
} check_message_data_t;

check_message_data_t messages[] = {
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_1) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_2) },
	{ DKIM_TEST_NONE, NULLER(MESSAGE_3) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_4) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_5) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_6) },
	{ DKIM_TEST_NONE, NULLER(MESSAGE_7) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_8) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_9) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_10) },
	{ DKIM_TEST_NONE, NULLER(MESSAGE_11) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_12) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_13) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_14) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_15) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_16) },
	{ DKIM_TEST_VERIFY, NULLER(MESSAGE_17) },
	{ DKIM_TEST_SIGNING, NULLER(MESSAGE_18) }

};

uint32_t check_message_max(void) {
	return sizeof(messages) / sizeof(check_message_data_t);
}

bool_t check_message_dkim_sign(uint32_t index) {

	bool_t result = false;

	if (index < check_message_max()) {
		result = (messages[index].flags & DKIM_TEST_SIGNING) == DKIM_TEST_SIGNING;
	}

	return result;
}

bool_t check_message_dkim_verify(uint32_t index) {

	bool_t result = false;

	if (index < check_message_max()) {
		result = (messages[index].flags & DKIM_TEST_VERIFY) == DKIM_TEST_VERIFY;
	}

	return result;
}

stringer_t *check_message_get(uint32_t index) {

	stringer_t *result = NULL;

	if (index < check_message_max()) {
		result = base64_decode(messages[index].data, NULL);
	}

	return result;
};
