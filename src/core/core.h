
/**
 * @file /magma/core/core.h
 *
 * @brief	A collection of types, declarations and includes needed when accessing the core module and the type definitions needed to parse the header files that follow.
 */

#ifndef MAGMA_CORE_H
#define MAGMA_CORE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <math.h>
#include <semaphore.h>
#include <dirent.h>
#include <limits.h>
#include <ftw.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <sys/resource.h>


//for core separation
#ifndef PACKAGE_MAGMA
#define CORE_THREAD_STACK_SIZE 1048576
#define CORE_SECURE_MEMORY_LENGTH 32768
#define CORE_PAGE_LENGTH getpagesize()
#define MAGMA_FILEPATH_MAX PATH_MAX
#endif
//end

/**
 * The type definitions used by Magma that are not defined by the system headers.
 * The bool type requires the inclusion of stdbool.h and the use of the C99.
 */

/// @typedef bool_t
#ifndef __bool_t_defined
#ifdef __bool_true_false_are_defined
typedef bool bool_t;
#else
typedef char bool_t;
#endif
#define __bool_t_defined
#endif

/// @typedef chr_t
#ifndef __chr_t_defined
typedef char chr_t;
#define __chr_t_defined
#endif

/// @typedef uchr_t
#ifndef __u_chr_t_defined
typedef unsigned char uchr_t;
#define __u_chr_t_defined
#endif

/// @typedef byte_t
#ifndef __byte_t_defined
typedef unsigned char byte_t;
#define __byte_t_defined
#endif

/// @typedef int_t
#ifndef __int_t_defined
typedef int32_t int_t;
#define __int_t_defined
#endif

/// @typedef uint_t
#ifndef __uint_t_defined
typedef uint32_t uint_t;
#define __uint_t_defined
#endif

/// @typedef int24_t
#ifndef __int24_t_defined
typedef struct __attribute__ ((packed)) {
	uint8_t byte0;
	uint8_t byte1;
	int8_t byte2;
} __int24_t;
typedef __int24_t int24_t;
#define __int24_t_defined
#endif

/// @typedef uint24_t
#ifndef __uint24_t_defined
typedef struct __attribute__ ((packed)) {
	uint8_t byte0;
	uint8_t byte1;
	uint8_t byte2;
} __uint24_t;
typedef __uint24_t uint24_t;
#define __uint24_t_defined
#endif

#ifndef INT24_MIN
#define INT24_MIN (-8388607)
#endif

#ifndef INT24_MAX
#define INT24_MAX (8388607)
#endif

#ifndef UINT24_MIN
#define UINT24_MIN (0)
#endif

#ifndef UINT24_MAX
#define UINT24_MAX (16777215)
#endif

/*

Should we ever need to create a 128 bit integer on a 64 bit system, GCC 3.1 and higher will allow it. Note that
this will not work on 32 bit systems, and don't forget to add the 128 bit type to the M_TYPE enumerator.

# if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && (WORDSIZE == 64)
typedef __uint128_t uint128_t;
typedef __int128_t int128_t;
# else
#  error "A 64 bit system, and GCC 4.6.0 or later is required to define the 128 bit integer types."
# endif

*/

/**
 * Different types used throughout.
 */
typedef enum {
	M_TYPE_EMPTY = 0,
	M_TYPE_MULTI = 1,   //!< M_TYPE_MULTI is multi_t
	M_TYPE_ENUM,		//!< M_TYPE_ENUM is enum
	M_TYPE_BOOLEAN, //!< M_TYPE_BOOLEAN is bool_t
	M_TYPE_BLOCK,   //!< M_TYPE_BLOCK is void pointer
	M_TYPE_NULLER,  //!< M_TYPE_NULLER is char pointer
	M_TYPE_PLACER,  //!< M_TYPE_PLACER is placer_t struct
	M_TYPE_STRINGER,//!< M_TYPE_STRINGER is stringer_t pointer
	M_TYPE_INT8,    //!< M_TYPE_INT8 is int8_t
	M_TYPE_INT16,   //!< M_TYPE_INT16 is int16_t
	M_TYPE_INT32,   //!< M_TYPE_INT32 is int32_t
	M_TYPE_INT64,   //!< M_TYPE_INT64 is int64_t
	M_TYPE_UINT8,   //!< M_TYPE_UINT8 is uint8_t
	M_TYPE_UINT16,  //!< M_TYPE_UINT16 is uint16_t
	M_TYPE_UINT32,  //!< M_TYPE_UINT32 is uint32_t
	M_TYPE_UINT64,  //!< M_TYPE_UINT64 is uint64_t
	M_TYPE_FLOAT,   //!< M_TYPE_FLOAT is float
	M_TYPE_DOUBLE   //!< M_TYPE_DOUBLE is double
} M_TYPE;

enum {
	EMPTY = 0
};

/************ TYPE ************/
char * type(M_TYPE type);
/************ TYPE ************/

#define log_pedantic(...) printf(__VA_ARGS__)
#define log_check(expr) do {} while (0)
#define log_info(...) printf(__VA_ARGS__)
#define log_error(...) printf(__VA_ARGS__)
#define log_critical(...) printf(__VA_ARGS__)
#define log_options(options, ...) printf(__VA_ARGS__)

#include "memory/memory.h"
#include "strings/strings.h"
#include "classify/classify.h"
#include "encodings/encodings.h"
#include "indexes/indexes.h"
#include "compare/compare.h"
#include "thread/thread.h"
#include "buckets/buckets.h"
#include "parsers/parsers.h"
#include "checksum/checksum.h"
#include "host/host.h"

#endif

