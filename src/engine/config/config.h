
/**
 * @file /magma/engine/config/config.h
 *
 * @brief	The entry point for all configuration modules used by magma.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_ENGINE_OPTIONS_H
#define MAGMA_ENGINE_OPTIONS_H

// RFC 2181 specifies a maximum legal length of 253 characters for a dotted domain name. Individual levels of the overall name
// may contain a maximum of 63 characters.
#define MAGMA_HOSTNAME_MAX _POSIX_HOST_NAME_MAX

// The maximum number of characters in a file path... currently 4096.
#define MAGMA_FILEPATH_MAX PATH_MAX

// The maximum number of characters in a file name... currently 255.
#define MAGMA_FILENAME_MAX NAME_MAX

// The amount of memory allocated by default to hold the stack for spawned threads.
#define MAGMA_THREAD_STACK_SIZE 1048576

// The size of the thread local buffer.
#define MAGMA_THREAD_BUFFER_SIZE 1024

// The maximum number of worker threads allowed, even if the system limit is higher.
#define MAGMA_WORKER_THREAD_LIMIT 16384

// The amount of data used to seed the random number generator.
#define MAGMA_CRYPTOGRAPHY_SEED_SIZE 64

// The default resource paths.
#define MAGMA_LOGS "logs/"
#define MAGMA_RESOURCE_FONTS "resources/fonts/"
#define MAGMA_RESOURCE_PAGES "resources/pages/"
#define MAGMA_RESOURCE_VIRUS "resources/virus/"
#define MAGMA_RESOURCE_LOCATION "resources/location/"
#define MAGMA_RESOURCE_TEMPLATES "resources/templates/"

// The default location database caching policy.
#define MAGMA_LOCATION_CACHE CONSTANT("disable")

// The maximum number of server instances.
#define MAGMA_CACHE_INSTANCES 8

// The default caching server retry interval.
#define MAGMA_CACHE_SERVER_RETRY 600

// The default caching server connection timeout.
#define MAGMA_CACHE_SOCKET_TIMEOUT 10

// The maximum number of server instances.
#define MAGMA_BLACKLIST_INSTANCES 6

// The maximum number of relay instances.
#define MAGMA_RELAY_INSTANCES 8

// The maximum number of server instances.
#define MAGMA_SERVER_INSTANCES 32

// The default size of connection buffer. Can be changed via the config.
#define MAGMA_CONNECTION_BUFFER_SIZE 8192

// The maximum size of the HELO/EHLO string.
// RFC 2821, section 4.5.3.1 dictates a max length of 255 characters for a domain
#define MAGMA_SMTP_MAX_HELO_SIZE MAGMA_HOSTNAME_MAX

// RFC 2821, section 4.5.3.1 dictates a max length of 256 characters for an address.
#define MAGMA_SMTP_MAX_ADDRESS_SIZE 256

// The default limit on recipients.
#define MAGMA_SMTP_RECIPIENT_LIMIT 256

#define MAGMA_SMTP_RELAY_LIMIT 256

// The default maximum line length for SMTP messages.
// RFC 3676 / 4.2 ... Lines SHOULD be 78 characters or shorter, including any trailing
// white space and also including any space added as part of stuffing...
#define MAGMA_SMTP_LINE_WRAP_LENGTH 80

// The maximum size of a message accepted via SMTP.
#define MAGMA_SMTP_MAX_MESSAGE_SIZE 1073741824

// Macros because we have a lot of these checks
#define CONFIG_CHECK_EXISTS(option,ptype) \
	do { \
		if (option && !file_accessible(option)) { \
		log_critical(#ptype " specified in " #option " is not accessible: { path = %s, error = %s }", option, strerror_r(errno, bufptr, buflen)); \
		result = false; \
		} \
	} while (0)
#define CONFIG_CHECK_FILE_READABLE(x)	CONFIG_CHECK_EXISTS(x,"Filename")
#define CONFIG_CHECK_DIR_READABLE(x)	CONFIG_CHECK_EXISTS(x,"Directory")
#define CONFIG_CHECK_READWRITE(option,ptype) \
	do { \
		if (option && !file_readwritable(option)) { \
		log_critical(#ptype " specified in " #option " is not accessible for reading and writing: { path = %s, error = %s }", option, strerror_r(errno, bufptr, buflen)); \
		result = false; \
		} \
	} while (0)
#define CONFIG_CHECK_FILE_READWRITE(x)	CONFIG_CHECK_READWRITE(x,"Filename")
#define CONFIG_CHECK_DIR_READWRITE(x)	CONFIG_CHECK_READWRITE(x,"Directory")



#include "cache/cache.h"
#include "relay/relay.h"
#include "servers/servers.h"
#include "global/global.h"

#endif
