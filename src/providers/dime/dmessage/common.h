#ifndef DIME_DMSG_COMMON_H
#define DIME_DMSG_COMMON_H

#include "dime/sds/sds.h" 

#define DMIME_NUM_COMMON_HEADERS 7
#define CHUNK_LENGTH_SIZE 3
#define TRACING_HEADER_SIZE 4
#define MESSAGE_LENGTH_SIZE 4
#define MESSAGE_HEADER_SIZE 6
#define CHUNK_HEADER_SIZE 4

#define DMIME_CHUNK_TYPE_MAX 256

#define MINIMUM_PAYLOAD_SIZE 256

#define ALTERNATE_PADDING_ALGORITHM_ENABLED 1
#define ALTERNATE_USER_KEY_APPLIED_TO_DATE 2
#define GZIP_COMPRESSION_ENABLED 4
#define DATA_SEGMENT_CONTINUATION_ENABLED 128

#define DEFAULT_CHUNK_FLAGS 0

typedef struct {
    sds headers[DMIME_NUM_COMMON_HEADERS];
} dmime_common_headers_t;

typedef enum {
    META_BOUNCE = 1,
    DISPLAY_BOUNCE
} dmime_bounce_type_t;


// Chunk type, used as index to global table of chunk keys dmime_chunk_keys
typedef enum {
    //////////////
    // Envelope //
    //////////////

    CHUNK_TYPE_NONE = 0,
    // The only non-standard chunk for transmitting ephemeral ed25519 public
    // key.
    CHUNK_TYPE_EPHEMERAL = 2,
    // TODO nothing for alternate chunk types is implemented. Lots of functions
    // will need to be changed for this functionality to be implemented PLEASE
    // BE THOROUGH.
    CHUNK_TYPE_ALTERNATE,
    CHUNK_TYPE_ORIGIN,
    CHUNK_TYPE_DESTINATION,


    //////////////
    // Metadata //
    //////////////

    CHUNK_TYPE_META_COMMON = 33,
    CHUNK_TYPE_META_OTHER,


    /////////////
    // Display //
    /////////////

    //CHUNK_TYPE_DISPLAY_MULTI    = 65,
    //CHUNK_TYPE_DISPLAY_ALT      = 66,
    CHUNK_TYPE_DISPLAY_CONTENT = 67,


    /////////////////
    // Attachments //
    /////////////////

    //CHUNK_TYPE_ATTACH_MULTI = 129,
    //CHUNK_TYPE_ATTACH_ALT = 130,
    CHUNK_TYPE_ATTACH_CONTENT = 131,


    ////////////////
    // Signatures //
    ////////////////

    // All signatures are taken over encrypted data, excluding the first 5
    // bytes (type+length) of the DMIME header.

    // Required. For light clients wanting to verify chunk hashes provided by
    // the server.
    CHUNK_TYPE_SIG_AUTHOR_TREE = 225,
    // Required
    CHUNK_TYPE_SIG_AUTHOR_FULL = 226,
    // Only one of these bounce signatures is recommended, if both are present
    // then.
    CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE = 248,
    CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE = 249,
    // Required.
    CHUNK_TYPE_SIG_ORIGIN_FULL = 255
} dmime_chunk_type_t;

// Chunk section, is specified for every chunk_type_t in the global table of
// chunk keys dmime_chunk_keys
typedef enum {
    CHUNK_SECTION_NONE = 0,
    CHUNK_SECTION_ENVELOPE = 1,
    CHUNK_SECTION_METADATA = 2,
    CHUNK_SECTION_DISPLAY = 4,
    CHUNK_SECTION_ATTACH = 8,
    CHUNK_SECTION_SIG = 16,
} dmime_chunk_section_t;

// Chunk payload type is specified for every chunk_type_t in the global table
// of chunk keys dmime_chunk_keys
typedef enum {
    PAYLOAD_TYPE_NONE = 0,
    PAYLOAD_TYPE_EPHEMERAL,
    PAYLOAD_TYPE_STANDARD,
    PAYLOAD_TYPE_SIGNATURE,
} dmime_payload_type_t;

// Structure of a dmime_chunk_key_t, dmime_chunk_keys is a global table of
// these structures, 1 for every chunk_type_t.
typedef struct {
    unsigned int required;
    unsigned int unique;
    unsigned int encrypted;
    unsigned int sequential;

    dmime_chunk_section_t section;
    dmime_payload_type_t payload;

    unsigned int auth_keyslot;
    unsigned int orig_keyslot;
    unsigned int dest_keyslot;
    unsigned int recp_keyslot;

    const char *name;
    const char *description;
} dmime_chunk_key_t;

// global table of chunk types
extern dmime_chunk_key_t dmime_chunk_keys[DMIME_CHUNK_TYPE_MAX];

#endif
