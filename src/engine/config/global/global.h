
/**
 * @file /magma/engine/config/global/global.h
 *
 * @brief	The global configuration structure used for overall system settings, and functions to initialize it at startup and free it at shutdown.
 */

#ifndef MAGMA_ENGINE_CONFIG_GLOBAL_H
#define MAGMA_ENGINE_CONFIG_GLOBAL_H

typedef struct {
	void *store; /* The location in memory to store the setting value. */
	multi_t norm; /* The default value. */
	chr_t *name; /* The search key/name of the setting. */
	chr_t *description; /* Description of the setting and its default value. */
	bool_t file; /* Can this value be set using the config file? */
	bool_t database; /* Can this value be set using the config file? */
	bool_t overwrite; /* Can this value be overwritten? */
	bool_t set; /* Has this setting already been provided? */
	bool_t required; /* Is this setting required? */
} magma_keys_t;

typedef struct {

	struct {
		bool_t output_config; /* Dump the configuration to the log file. */
		bool_t output_resource_limits; /* Should attempts to increase system limits trigger an error. */

		// LOW: Filenames are limited to 255 characters, but file paths can be up to 4096. As such we should probably be storing this using a block of memory off the heap.
		chr_t file[MAGMA_FILEPATH_MAX + 1]; /* Path to the magmad.config file. */
	} config;

	struct {
		stringer_t *contact; /* The general purpose contact email address. */
		stringer_t *abuse; /* The contact email address for abuse complaints. */
	} admin;

	struct {
		uint64_t number; /* The unique numeric identifier for this host. */
		chr_t name[MAGMA_HOSTNAME_MAX + 1]; /* The hostname. Size can be MAGMA_HOSTNAME_MAX (64) or MAGMA_HOSTNAME_MAX (255). Make sure the gethostname() calls match. */
	} host;

	struct {
		chr_t *file; /* Path to the magma.open.so library. */
		bool_t unload; /* Unload the magma.open.so symbols at exit. */
	} library;

	struct {
		bool_t daemonize; /* Spawn a daemon process and release the console session. */
		char * root_directory; /* Change the root path to the provided value. */
		char * impersonate_user; /* Change the effective user account of the process to the user provided. */
		bool_t increase_resource_limits; /* Attempt to increase system limits. */
		uint32_t thread_stack_size; /* How much memory should be allocated for thread stacks? */
		uint32_t worker_threads; /* How many worker threads should we spawn? */
		uint32_t network_buffer; /* The size of the network buffer? */

		bool_t enable_core_dumps; /* Should fatal errors leave behind a core dump. */
		uint64_t core_dump_size_limit; /* If core dumps are enabled, what size should they be limited too. */

		stringer_t *domain; /* The default domain name used in new user email addresses and for unqualified login names. */
	} system;

	struct {
		struct {
			bool_t enable; /* Should the secure memory sub-system be enabled. */
			uint64_t length; /* The size of the secure memory pool. The pool must fit within any memory locking limits. */
		} memory;


		uint32_t minimum_password_length; /* The minimum number of characters a valid password must contain. */
		stringer_t *salt; /* The string added to hash operations to improve security. */
		stringer_t *links; /* The string used to encrypt links that reflect back to the daemon. */
		stringer_t *sessions; /* The string used to encrypt session tokens. */
	} secure;

	struct {
		bool_t file; /* Send log messages to a file. */
		chr_t *path; /* If log files are enabled, this will control whether the logs are stored. */
	} output;

	struct {

		bool_t imap; /* Output IMAP request details. */
		bool_t http; /* Output HTTP request details. */
		bool_t content; /* Output the web resource files loaded. */

		bool_t file; /* Output the source file that recorded the log entry. */
		bool_t line; /* Output the source line that recorded the log entry. */
		bool_t time; /* Output time that the log entry was recorded. */
		bool_t stack; /* Output the stack that triggered the log entry. */
		bool_t function; /* Output the function that made the log entry. */
	} log;

	struct {
		chr_t *tank; /* The path of the storage tank. */
		stringer_t *active; /* The default storage server used by the legacy mail storage logic. */
		stringer_t *root; /* The root portion of the storage server directory paths. */
	} storage;

	struct {
		uint32_t relay_limit; /* The number of relay hops allowed before assuming a message is stuck in a loop and discarding it. */
		uint32_t recipient_limit; /* The maximum number of recipients for an inbound/outbound message. */
		uint32_t address_length_limit; /* What is the maximum size of a legal email address? */
		uint32_t helo_length_limit; /* How big of a HELO/EHLO parameter will the system allow? */
		uint32_t wrap_line_length; /* When formatting email messages attempt to wrap lines longer than this length. */
		uint64_t message_length_limit; /* How big of a message will the system allow via SMTP? */

		// Store information about the realtime blacklists used to block messages via SMTP.
		struct {
			uint32_t count;
			stringer_t *domain[MAGMA_BLACKLIST_INSTANCES];
		} blacklists;

		stringer_t *bypass_addr; /* Bypass address/subnet string for smtp checks. This value used only by config. */
		inx_t *bypass_subnets; /* Holder for all the address/subnets to be waived through for bypass */
	} smtp;

	struct {
		bool_t close; /* Automatically close HTTP connections after each request? */
		bool_t allow_cross_domain; /* Provide the necessary headers in response to OPTION requests to allow cross domain JSON-RPC requests. */
		chr_t *fonts; /* The web fonts directory. */
		chr_t *pages; /* The static web pages directory. */
		chr_t *templates; /* The web application templates. */
		uint32_t session_timeout; /* Number of seconds before a session cookie expires. */
	} http;

	struct {
		relay_t *host[MAGMA_RELAY_INSTANCES];
		struct {
			uint32_t premium;
			uint32_t standard;
		} count;
		uint32_t timeout;
	} relay;

	struct {
		struct {
			bool_t indent; /* Format the JSON responses before returning them? */
			bool_t safeguard; /* Whether to require HTTPS for access to the portal. */
		} portal;
		struct {
			stringer_t *sender; /* Format the JSON responses before returning them? */
		} contact;
		bool_t statistics; /* Whether or not the statistics page is enabled. */
		bool_t registration; /* Whether or not the new user registration page is enabled. */
		stringer_t *tls_redirect; /* The TLS hostname and/or port for redirecting web requests which require transport security. */
	} web;

	struct {
		bool_t enabled; /* Whether or not dkim signing is enabled. */
		chr_t *domain;
		chr_t *selector;
		stringer_t *key; /* Location of the dkim private key at startup (replaced with contents later). */
	} dkim;

	struct {
		stringer_t *key;	/* The Dark Internet Mail Environment Primary Organizational Key. */
		stringer_t *signet;	/* The Dark Internet Mail Environment Organizational Signet. */
	} dime;

	struct {

		struct {
			uint32_t seed_length; /* How much data should be used to seed the random number generator. */
			bool_t dhparams_rotate; /* Should we generate new a DH prime parameter periodically. */
			bool_t dhparams_large_keys; /* Should we use large DH session keys. */
		} cryptography;

		struct {
			chr_t *host; /* The database host name. */
			chr_t *user; /* The database user name. */
			chr_t *password; /* The database password. */
			chr_t *schema; /* The database schema name. */
			uint32_t port; /* The database server port. */
			chr_t *socket_path; /* The database UNIX domain socket path. */

			struct {
				uint32_t timeout; /* The number of seconds to wait for a free database connection. */
				uint32_t connections; /* The number of database connections in the pool. */
			} pool;
		} database;

		struct {
			bool_t available; /* Is ClamAV loaded at runtime. */
			char *signatures; /* The signatures directory. */
		} virus;

		struct {
			chr_t *path; /* The IP location database files. */
			stringer_t *cache; /* The IP location caching strategy. [disable | index | mapped | memory]" */
		} location;

		struct {
			cache_t *host[MAGMA_CACHE_INSTANCES];
			struct {
				uint32_t timeout; /* The number of seconds to wait for a free cache context. */
				uint32_t connections; /* The number of cache client objects to hold in the pool. */
			} pool;
			uint32_t retry; /* How often should dead caching servers be retried. */
			uint32_t timeout; /* The TCP socket send/recv timeout. */
		} cache;

		struct {
			struct {
				uint32_t timeout; /* The number of seconds to wait for a free SPF instance. */
				uint32_t connections; /* The number of SPF instances in the pool. */
			} pool;
		} spf;

	} iface;

	// Global config section
	chr_t * spool; /* The spool directory. */
	int_t page_length; /* The memory page size. This value is used to align memory mapped files to page boundaries. */

	// Global variables section
	uint32_t init; /* How far into the initialization process we've gotten. */
	pthread_rwlock_t lock; /* Used to grab a global read or write lock on the configuration. */
	server_t *servers[MAGMA_SERVER_INSTANCES]; /* An array of server structures. */

} magma_t;

/// datatier.c
uint64_t   config_fetch_host_number(void);
table_t *  config_fetch_settings(void);

/// global.c
void            config_free(void);
magma_keys_t *  config_key_lookup(stringer_t *name);
bool_t          config_load_database_settings(void);
bool_t			config_load_cmdline_settings(void);
bool_t          config_load_defaults(void);
bool_t          config_load_file_settings(void);
void            config_output_help(void);
void            config_output_settings(void);
void			config_output_value_generic(chr_t *prefix, chr_t *name, M_TYPE type, void *val, bool_t required);
void            config_output_value(magma_keys_t *key);
bool_t          config_validate_settings(void);
bool_t          config_value_set(magma_keys_t *setting, stringer_t *value);

#endif
