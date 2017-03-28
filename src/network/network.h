
/**
 * @file /magma/network/network.h
 *
 * @brief	The types and functions for abstracting access to network functionality.
 */

#ifndef MAGMA_NETWORK_H
#define MAGMA_NETWORK_H

#include "meta.h"
#include "sessions.h"
#include "pop.h"
#include "smtp.h"
#include "dmtp.h"
#include "imap.h"
#include "http.h"

typedef int16_t octet_t;
typedef int32_t segment_t;

enum {
	REVERSE_ERROR = -1,
	REVERSE_EMPTY = 0,
	REVERSE_PENDING = 1,
	REVERSE_COMPLETE = 2
};

typedef struct {
	char *string;
	size_t length;
	void *function;
} command_t;

// Setup the structure of variables used to relay and bounce messages.
typedef struct {
	ip_t *ip; /* The remote host address information. */
	void *tls; /* The TLS connection object. */
	int sockd; /* The socket connection. */
	int status; /* Track whether the last network generated an error. */
	placer_t line; /* The current line being processed. */
	stringer_t *buffer; /* The connection buffer. */
} client_t;

typedef struct {
	union {
		pop_session_t pop;
		imap_session_t imap;
		smtp_session_t smtp;
		dmtp_session_t dmtp;
		http_session_t http;
	};

	struct {
		uint32_t spins;
		uint32_t violations;
	} protocol;

	struct {
		void *tls; /* The TLS connection object. */
		int sockd; /* The socket connection. */
		int status; /* Track whether the last network operation generated an error. */
		placer_t line; /* The current line being processed. */
		stringer_t *buffer; /* The connection buffer. */

		struct {
			ip_t *ip;
			int_t status;
			stringer_t *domain;
		} reverse;

	} network;
	uint64_t refs; /* The number of memory references or threads pointing at this structure. */
	pthread_mutex_t lock; /* The mutex used for locking during non-thread save operations. */
	server_t *server; /* The server instance that accepted the connection. */
	command_t *command; /* The command structure. */
} connection_t;

/// addresses.c
ip_t *        con_addr(connection_t *con, ip_t *output);
octet_t       con_addr_octet(connection_t *con, int_t position);
stringer_t *  con_addr_presentation(connection_t *con, stringer_t *output);
stringer_t *  con_addr_reversed(connection_t *con, stringer_t *output);
segment_t     con_addr_segment(connection_t *con, int_t position);
stringer_t *  con_addr_standard(connection_t *con, stringer_t *output);
stringer_t *  con_addr_subnet(connection_t *con, stringer_t *output);
uint32_t      con_addr_word(connection_t *con, int_t position);
ip_t * 		  ip_copy(ip_t *dst, ip_t *src);
octet_t       ip_octet(ip_t *address, int_t position);
stringer_t *  ip_presentation(ip_t *address, stringer_t *output);
stringer_t *  ip_reversed(ip_t *address, stringer_t *output);
bool_t        ip_address_equal(ip_t *ip1, ip_t *ip2);
segment_t     ip_segment(ip_t *address, int_t position);
stringer_t *  ip_standard(ip_t *address, stringer_t *output);
stringer_t *  ip_subnet(ip_t *address, stringer_t *output);
uint32_t      ip_word(ip_t *address, int_t position);
bool_t        ip_str_addr(chr_t *ipstr, ip_t *out);
bool_t        ip_str_subnet(chr_t *substr, subnet_t *out);
bool_t        ip_matches_subnet(subnet_t *subnet, ip_t *addr);


/// connections.c
uint64_t        con_decrement_refs(connection_t *con);
void            con_destroy(connection_t *con);
uint64_t        con_increment_refs(connection_t *con);
connection_t *  con_init(int cond, server_t *server);
bool_t          con_init_network_buffer(connection_t *con);
int_t           con_secure(connection_t *con);
int_t           con_status(connection_t *con);

/// clients.c
void        client_close(client_t *client);
client_t *  client_connect(chr_t *host, uint32_t port);
int_t       client_secure(client_t *client);
int_t       client_status(client_t *client);

/// options.c

bool_t   net_set_buffer_length(int sd, int buffer_recv, int buffer_send);
bool_t   net_set_keepalive(int sd, bool_t keepalive, int_t idle, int_t interval, int_t tolerance);
bool_t   net_set_linger(int sd, bool_t linger, int_t timeout);
bool_t   net_set_nodelay(int sd, bool_t nodelay);
bool_t   net_set_non_blocking(int sd, bool_t blocking);
bool_t   net_set_reuseable_address(int sd, bool_t reuse);
bool_t   net_set_timeout(int sd, uint32_t timeout_recv, uint32_t timeout_send);

/// read.c
int64_t   client_read(client_t *client);
int64_t   client_read_line(client_t *client);
int64_t   con_read(connection_t *con);
int64_t   con_read_line(connection_t *con, bool_t block);

/// reverse.c
stringer_t *  con_reverse_check(connection_t *con, uint32_t timeout);
void          con_reverse_domain(connection_t *con, stringer_t *domain, int_t status);
void          con_reverse_enqueue(connection_t *con);
void          con_reverse_lookup(connection_t *con);
void          con_reverse_status(connection_t *con, int_t status);

/// listeners.c
bool_t   net_init(server_t *server);
void     net_listen(void);
void     net_shutdown(server_t *server);

/// write.c
int64_t   client_print(client_t *client, chr_t *format, ...);
int64_t   client_write(client_t *client, stringer_t *s);
int64_t   con_print(connection_t *con, chr_t *format, ...);
int64_t   con_write_bl(connection_t *con, char *block, size_t length);
int64_t   con_write_ns(connection_t *con, char *string);
int64_t   con_write_pl(connection_t *con, placer_t string);
int64_t   con_write_st(connection_t *con, stringer_t *string);

stringer_t * protocol_type(connection_t *con);

#endif
