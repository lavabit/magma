
/**
 * @file /magma/engine/config/relay/keys.h
 *
 * @brief	A collection of keys that define access rules, default values and other parameters needed to configure relay instances.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OPIONS_RELAY_KEYS_H
#define MAGMA_OPIONS_RELAY_KEYS_H

relay_keys_t relay_keys[] = {
	{
		.offset = offsetof (relay_t, name),
		.norm.type = M_TYPE_NULLER,
		.norm.val.st = NULL,
		.name = ".name",
		.description = "The host name or address of the mail relay server.",
		.required = true
	},
	{
		.offset = offsetof (relay_t, port),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 25,
		.name = ".port",
		.description = "The port used by the mail relay server.",
		.required = false
	},
	{
		.offset = offsetof (relay_t, secure),
		.norm.type = M_TYPE_BOOLEAN,
		.norm.val.binary = false,
		.name = ".secure",
		.description = "Determines whether connections should be made using TLS.",
		.required = false
	}
};

#endif

