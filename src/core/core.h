
/**
 * @file /magma/core/core.h
 *
 * @brief	A collection of types, declarations and includes needed when accessing the core module and the type definitions needed to parse the header files that follow.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_H
#define MAGMA_CORE_H

/**
 * The type definitions used by Magma that are not defined by the system headers.
 * The bool type requires the inclusion of stdbool.h and the use of the C99.
 */
typedef char chr_t;
typedef bool bool_t;
typedef int32_t int_t;
typedef uint32_t uint_t;
typedef unsigned char uchr_t;
typedef unsigned char byte_t;

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

