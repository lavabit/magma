
/**
 * @file /magma/engine/config/servers/keys.h
 *
 * @brief	A collection of keys that define access rules, default values and other parameters needed when configuring server instances.
 */

#ifndef MAGMA_OPIONS_SERVERS_KEYS_H
#define MAGMA_OPIONS_SERVERS_KEYS_H

server_keys_t server_keys[] = {
	{
		.offset = offsetof (server_t, name),
		.norm.type = M_TYPE_STRINGER,
		.norm.val.st = NULL,
		.name = ".name",
		.description = "The name of the server instance.",
		.required = false
	},
	{
		.offset = offsetof (server_t, protocol),
		.norm.type = M_TYPE_ENUM,
		.norm.val.u64 = EMPTY,
		.name = ".protocol",
		.description = "The protocol provided by the server instance.",
		.required = true
	},
	{
		.offset = offsetof (server_t, domain),
		.norm.type = M_TYPE_STRINGER,
		.norm.val.st = NULL,
		.name = ".domain",
		.description = "The domain used by the server instance.",
		.required = false
	},
	{
		.offset = offsetof (server_t, enabled),
		.norm.type = M_TYPE_BOOLEAN,
		.norm.val.binary = true,
		.name = ".enabled",
		.description = "A boolean which can be used to disable a server configuration without removing it from the configuration.",
		.required = false

	},
	{
		.offset = offsetof (server_t, network.ipv6),
		.norm.type = M_TYPE_BOOLEAN,
		.norm.val.binary = false,
		.name = ".network.ipv6",
		.description = "Whether to enable IPv6 for the server instance.",
		.required = false
	},
	{
		.offset = offsetof (server_t, network.port),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 0,
		.name = ".network.port",
		.description = "The port used by the instance.",
		.required = true
	},
	{
		.offset = offsetof (server_t, tls.certificate),
		.norm.type = M_TYPE_NULLER,
		.norm.val.ns = NULL,
		.name = ".tls.certificate",
		.description = "The TLS certificate and private key used for transport security by this server instance.",
		.required = false
	},
	{
		.offset = offsetof (server_t, network.timeout),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 600,
		.name = ".network.timeout",
		.description = "The network timeout used by the instance.",
		.required = false
	},
	{
		.offset = offsetof (server_t, network.listen_queue),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 128,
		.name = ".network.listen_queue",
		.description = "The size of the listen queue used by the instance.",
		.required = false
	},
	{
		.offset = offsetof (server_t, network.type),
		.norm.type = M_TYPE_ENUM,
		.norm.val.u64 = TCP_PORT,
		.name = ".network.type",
		.description = "The type of port. Either TCP or TLS can be specified.",
		.required = false
	},
	{
		.offset = offsetof (server_t, violations.delay),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 1000,
		.name = ".violations.delay",
		.description = "The delay triggered by protocol or authentication errors.",
		.required = false
	},
	{
		.offset = offsetof (server_t, violations.cutoff),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 100,
		.name = ".violations.delay",
		.description = "The number of protocol or authentication errors allowed before terminating a connection.",
		.required = false
	}
};

#endif

