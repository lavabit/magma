
#ifndef __LAVAD_STRUCTS_H__
#define __LAVAD_STRUCTS_H__


typedef struct {

	uint8_t engine;

	struct {
		uint64_t original;
		uint64_t compressed;
	} length;

	struct {
		uint64_t original;
		uint64_t compressed;
	} hash;

} __attribute__ ((packed)) compress_head_t;

typedef stringer_t compress_t;


// Hashed password results.
typedef struct {
	stringer_t *passkey;
	stringer_t *final;
} hashed_password_t;

// A linked list of blacklists used by the SMTP server.
typedef struct {
	char *domain;
	struct blacklist_config_t *next;
} blacklist_config_t;

// A linked list of lavacache server instances.
typedef struct {
	unsigned int port;
	unsigned int weight;
	stringer_t *name;
	stringer_t *hostname;
	stringer_t *ip;
	struct lavacache_config_t *next;
} lavacache_config_t;

// The database configuration parameters.
typedef struct {
	int connections;
	unsigned int interval;
	unsigned int iterations;
	char *hostname;
	char *ip;
	char *username;
	char *password;
	char *db;
	stringer_t *name;
} database_config_t;

// Contains a linked list of the outgoing relay servers.
typedef struct {
	short importance;
	char *port;
	char *hostname;
	stringer_t *name;
	struct outgoing_config_t *next;
} outgoing_config_t;

// A linked list of servers.
typedef struct {
	int ssl_sd;
	int normal_sd;
	unsigned ssl_port;
	unsigned normal_port;
	unsigned listen_queue;
	unsigned socket_timeout;
	unsigned bad_command_cutoff;
	unsigned bad_command_delay;
	char *certificate;
	stringer_t *name;
	stringer_t *spam;
	stringer_t *domain;
	stringer_t *banner;
	stringer_t *advertising;
	SSL_CTX *ssl_ctx;
	struct server_config_t *next;
} server_config_t;

// Global configuration settings.
typedef struct {
	sizer_t in_buffer;
	sizer_t out_buffer;
	short lb_ip_one;
	short lb_ip_two;
	short lb_ip_three;
	short lb_ip_four;
	short max_relay_hops;
	short max_recipients;
	short max_helo_length;
	short max_line_length;
	short max_address_length;
	short default_relays;
	short premium_relays;
	short log_method;
	short lock_iterations;
	short lock_delay;
	short processes;
	char *virus_db;
	char *log_file;
	char *emulate_user;
	unsigned short threads;
	stringer_t *salt;
	stringer_t *hostname;
	stringer_t *mail_storage_root;
	stringer_t *mail_storage_server;
	database_config_t db;
	server_config_t *smtp;
	server_config_t *pop;
	server_config_t *imap;
	lavacache_config_t *lavacache;
	blacklist_config_t *blacklist;
	outgoing_config_t *outgoing;
} global_config_t;

// An incoming connection.
typedef struct {
	void *function;
	int sock_descriptor;
	unsigned long address;
	server_config_t *server;
} incoming_t;

// A linked list of incoming connections.
typedef struct {
	incoming_t *incoming;
	struct incoming_list_t *next;
} incoming_list_t;

// A message structure.
typedef struct {
	placer_t to;
	placer_t from;
	placer_t date;
	placer_t subject;
	sizer_t header_length;
	stringer_t *id;
	stringer_t *text;
} smtp_message_t;

typedef struct {
	placer_t to;
	placer_t from;
	placer_t date;
	placer_t subject;
	stringer_t *text;
} basic_message_t;

// A linked list of recipients.
typedef struct {
	stringer_t *address;
	struct smtp_recipients_t *next;
} smtp_recipients_t;

// The structure for storing recipient preferences on outbound data.
typedef struct {
	int ssl;
	short importance;
	stringer_t *domain;
	unsigned sent_today;
	unsigned daily_send_limit;
	unsigned send_size_limit;
	unsigned long long usernum;
	smtp_recipients_t *recipients;
} smtp_outbound_prefs_t;

// Setup the structure of variables used to relay and bounce messages.
typedef struct {
	int buffered_bytes;
	int line_length;
	int sock_descriptor;
	char *in_buffer;
	char *out_buffer;
	outgoing_config_t *server;
} smtp_client_connection_t;

// The structure for storing recipient preferences on inbound data.
typedef struct {
	short mark;
	short secure;
	short rollout;
	short blacklist;
	short whitelist;
	short spam;
	short virus;
	short greylist;
	short spf;
	short rbl;
	short phish;
	short overquota;
	short advertising;
	short bounces;
	short outcome;
	short spfaction;
	short rblaction;
	short spamaction;
	short virusaction;
	short phishaction;
	short filters;
	long int spamkey;
	unsigned int greytime;
	unsigned int local_size;
	unsigned int daily_recv_limit;
	unsigned int daily_recv_limit_ip;
	unsigned long long usernum;
	unsigned long long signum;
	unsigned long long quota;
	unsigned long long stor_size;
	unsigned long long inbox;
	unsigned long long autoreply;
	unsigned long long messagenum;
	unsigned long long foldernum;
	sizer_t recv_size_limit;
	stringer_t *rcptto;
	stringer_t *domain;
	stringer_t *forwarded;
	stringer_t *whitepassphrase;
	stringer_t *spamsig;
	struct smtp_inbound_prefs_t *next;
} smtp_inbound_prefs_t;

typedef struct {
	char *in_buffer;
	char *out_buffer;
	SSL *ssl;
	int buffered_bytes;
	int current_line;
	int sock_descriptor;
	int ending;
	short bypass;
	unsigned int badcommands;
	unsigned long address;
	server_config_t *server;
	short int oct_one;
	short int oct_two;
	short int oct_three;
	short int oct_four;

	// SMTP specific variables below.
	short esmtp;
	short vscanned;
	short authorized;
	short rbl_checked;
	short spf_checked;
	short num_recipients;
	sizer_t max_size;
	stringer_t *helo;
	stringer_t *mailfrom;
	stringer_t *reverse;
	pthread_t *reverse_thread;
	pthread_attr_t reverse_attrs;
	smtp_inbound_prefs_t *in_prefs;
	smtp_outbound_prefs_t *out_prefs;
	smtp_message_t *message;
} __attribute__((__packed__)) smtp_session_t ;

typedef struct {
	char *in_buffer;
	char *out_buffer;
	SSL *ssl;
	int buffered_bytes;
	int current_line;
	int sock_descriptor;
	int ending;
	short bypass;
	unsigned int badcommands;
	unsigned long address;
	server_config_t *server;
	short int oct_one;
	short int oct_two;
	short int oct_three;
	short int oct_four;

	// POP specific variables below.

} __attribute__((__packed__)) pop_session_t;

typedef struct {
	char *in_buffer;
	char *out_buffer;
	SSL *ssl;
	int buffered_bytes;
	int current_line;
	int sock_descriptor;
	int ending;
	short bypass;
	unsigned int badcommands;
	unsigned long address;
	server_config_t *server;
	short int oct_one;
	short int oct_two;
	short int oct_three;
	short int oct_four;

	// IMAP specific variables below.

} __attribute__((__packed__)) imap_session_t;

// Common session variables.
typedef struct {
	char *in_buffer;
	char *out_buffer;
	SSL *ssl;
	int buffered_bytes;
	int current_line;
	int sock_descriptor;
	int ending;
	short bypass;
	unsigned int badcommands;
	unsigned long address;
	server_config_t *server;
	short int oct_one;
	short int oct_two;
	short int oct_three;
	short int oct_four;

} __attribute__((__packed__)) session_common_t;

union session_t {
	pop_session_t pop;
	imap_session_t imap;
	smtp_session_t smtp;
};


#endif
