
/**
 * @file /magma/engine/config/servers/servers.h
 *
 * @brief	The types and functions involved in creating and accessing the server structure.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_ENGINE_CONFIG_SERVERS_H
#define MAGMA_ENGINE_CONFIG_SERVERS_H

typedef enum {
	TCP_PORT = 1,
	SSL_PORT
} M_PORT;

typedef enum {
	MOLTEN = 1,
	HTTP,
	POP,
	IMAP,
	SMTP,
	DMTP,
	SUBMISSION
} M_PROTOCOL;

typedef struct {
	size_t offset; /* The location in memory to store the setting value. */
	multi_t norm; /* The default value. */
	chr_t *name; /* The search key/name of the setting. */
	chr_t *description; /* Description of the setting and its default value. */
	bool_t required; /* Is this setting required? */
} server_keys_t;

typedef struct {
	struct {
		SSL_CTX *context;
		char *certificate;
	} ssl;
	struct {
		int sockd;
		bool_t ipv6;
		uint32_t port;
		uint32_t timeout;
		uint32_t listen_queue;
		M_PORT type;
	} network;
	struct {
		uint32_t delay;
		uint32_t cutoff;
	} violations;
	bool_t enabled;
	stringer_t *name, *domain;
	M_PROTOCOL protocol;
} server_t;

// A linked list of servers.
typedef struct {
	SSL_CTX *ssl_ctx;
	char *certificate;
	int ssl_sd, normal_sd;
	stringer_t *name, *spam, *domain, *banner;
	unsigned ssl_port, normal_port, listen_queue, socket_timeout, bad_command_cutoff, bad_command_delay;
	struct server_config_t *next;
} server_config_t;

/// servers.c
server_t *  servers_alloc(uint32_t number);
bool_t      servers_config(stringer_t *name, stringer_t *value);
bool_t      servers_encryption_start(void);
void        servers_encryption_stop(void);
void        servers_free(void);
server_t * 	servers_get_by_socket(int sockd);
uint64_t    servers_get_count_using_port(uint32_t port);
bool_t      servers_network_start(void);
void        servers_network_stop(void);
void        servers_output_settings(void);
bool_t      servers_set_value(server_keys_t *setting, server_t *server, stringer_t *value);
bool_t      servers_validate(void);
void		servers_output_help(void);


#endif
